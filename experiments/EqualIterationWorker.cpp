#include "../src/readData/BRInstanceReader.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/util/RandomNumbers.hpp"
#include "../src/heuristics/DBLF.hpp"
#include "../tests/PackingAudit.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <map>
#include <thread>
#include <atomic>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#error This experiment worker uses Windows process affinity and atomic file claims.
#endif
namespace {
void writeAtomic(const std::string &path,const std::string &contents,int attempts=200) {
    static unsigned long serial=0;
    const auto temporary=path+".tmp-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(++serial);
    std::ofstream file(temporary);file<<contents;file.close();
    if(!file)throw std::runtime_error("Cannot stage "+path);
    DWORD error=0;
    for(int attempt=0;attempt<attempts;++attempt) {
        if(MoveFileExA(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))return;
        error=GetLastError();
        if(error!=ERROR_ACCESS_DENIED && error!=ERROR_SHARING_VIOLATION && error!=ERROR_LOCK_VIOLATION)break;
        Sleep(30);
    }
    DeleteFileA(temporary.c_str()); // Only our uncommitted temporary file.
    throw std::runtime_error("Cannot commit "+path+"; Windows error="+std::to_string(error));
}
bool writeProgress(const std::string &path,const std::string &contents) {
    try{writeAtomic(path,contents,8);return true;}
    catch(const std::exception &e){std::cerr<<"WARNING: progress update skipped: "<<e.what()<<std::endl;return false;}
}
void directory(const std::string &path) {
    if(!CreateDirectoryA(path.c_str(),nullptr) && GetLastError()!=ERROR_ALREADY_EXISTS)throw std::runtime_error("Cannot create "+path);
}
struct Claim {
    HANDLE handle=INVALID_HANDLE_VALUE;
    explicit Claim(const std::string &path) {
        handle=CreateFileA(path.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,nullptr);
        if(handle==INVALID_HANDLE_VALUE) {
            const auto error=GetLastError();
            if(error!=ERROR_FILE_EXISTS && error!=ERROR_ALREADY_EXISTS && error!=ERROR_SHARING_VIOLATION && !(error==ERROR_ACCESS_DENIED && GetFileAttributesA(path.c_str())!=INVALID_FILE_ATTRIBUTES))throw std::runtime_error("Cannot claim task, Windows error "+std::to_string(error));
        }
    }
    ~Claim(){if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);}
    explicit operator bool()const{return handle!=INVALID_HANDLE_VALUE;}
};
void topology() {
    DWORD bytes=0;GetLogicalProcessorInformationEx(RelationProcessorCore,nullptr,&bytes);
    std::vector<char> buffer(bytes);
    if(!GetLogicalProcessorInformationEx(RelationProcessorCore,reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),&bytes))throw std::runtime_error("Cannot query physical CPU topology");
    unsigned index=0;
    for(DWORD offset=0;offset<bytes;) {
        auto entry=reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()+offset);
        if(entry->Relationship==RelationProcessorCore && entry->Processor.GroupCount==1 && entry->Processor.GroupMask[0].Group==0) {
            const auto mask=static_cast<unsigned long long>(entry->Processor.GroupMask[0].Mask);
            std::cout<<index++<<' '<<(mask & (~mask+1))<<' '<<mask<<'\n';
        }
        offset+=entry->Size;
    }
}
}
int main(int argc,char **argv){try {
    if(argc==2 && std::string(argv[1])=="--topology"){topology();return 0;}
    if(argc==3 && std::string(argv[1])=="--io-test") {
        const std::string path=std::string(argv[2])+"/locked-state.csv";writeAtomic(path,"old\n");
        HANDLE reader=CreateFileA(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        if(reader==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot create test reader");
        std::thread release([reader]{Sleep(180);CloseHandle(reader);});
        try{writeAtomic(path,"new\n");}catch(...){release.join();throw;}
        release.join();std::ifstream input(path);std::string contents;std::getline(input,contents);
        if(contents!="new")throw std::runtime_error("Retry failed");
        input.close();
        reader=CreateFileA(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        if(reader==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot create second reader");
        std::thread lateRelease([reader]{Sleep(400);CloseHandle(reader);});
        const auto written=writeProgress(path,"progress\n");lateRelease.join();
        if(written)throw std::runtime_error("Test did not exercise skipped progress update");
        writeAtomic(path,"final\n");
        std::cout<<"PASS: real Windows reader lock retried; progress failure is nonfatal; later updates recover\n";return 0;
    }
    if(argc!=7)throw std::invalid_argument("Usage: worker OUTPUT ROOT WORKER ITERATIONS TASK_LIMIT LOGICAL_CPU_MASK");
    const std::string output=argv[1],root=argv[2];
    const int worker=std::stoi(argv[3]),iterations=std::stoi(argv[4]),limit=std::stoi(argv[5]);const auto mask=std::stoull(argv[6]);
    if(worker<1 || iterations<0 || limit<1 || limit>4500)throw std::invalid_argument("Invalid worker inputs");
    if(!SetProcessAffinityMask(GetCurrentProcess(),static_cast<DWORD_PTR>(mask)))throw std::runtime_error("Cannot pin worker to a logical CPU");
    const auto state=output+"/worker-"+std::to_string(worker)+".csv";
    auto announce=[&](const std::string &status,const std::string &task){
        writeProgress(state,"worker,pid,status,task,cpu_mask\n"+std::to_string(worker)+","+std::to_string(GetCurrentProcessId())+","+status+","+task+","+std::to_string(mask)+"\n");
    };
    announce("starting","");
    std::map<int,std::vector<DatasetBinBacking>> datasets;
    for(int task=0;task<limit;++task) {
        if(std::ifstream(output+"/STOP")){announce("stopped","");return 0;}
        const int dataset=task/300+1,position=(task/3)%100;const std::string algorithm=std::vector<std::string>{"GA","ABC","FFA"}[task%3];
        const auto name="BR"+std::to_string(dataset)+"-i"+std::to_string(position+1)+"-"+algorithm;
        const auto folder=output+"/tasks/"+name;directory(folder);
        const auto resultFile=folder+"/result.csv";
        if(std::ifstream(resultFile))continue;
        Claim claim(folder+"/claim.lock");if(!claim)continue;
        if(std::ifstream(resultFile))continue;
        announce("working",name);std::cout<<"[worker "<<worker<<" pid "<<GetCurrentProcessId()<<"] CLAIM "<<name<<std::endl;
        if(!datasets.count(dataset))datasets[dataset]=BRInstanceReader().read(root+"/Instance/BR/BR"+std::to_string(dataset)+".txt");
        const auto &data=datasets.at(dataset).at(position);
        CommandLineOptions options;options.population=20;options.decoder=PackingDecoder::Contact;options.iterations=iterations;options.patience=0;
        options.gaMatings=20;options.crossover=1;options.selection=0.85;options.mutationProbability=1;options.rotationMutation=0.05;
        options.sites=10;options.eliteSites=5;options.eliteBees=4;options.nonEliteBees=2;options.movements=2;
        const auto config=options.configuration(algorithm,ROTATION_WAY::SIX_WAY,MutationType::C2,false);
        const auto seed=instanceSeed(123,data.id,data.PSeed);seedRandomNumbers(seed);
        const auto result=config.execute(data);const auto evaluations=fitnessCacheStats().misses;
        const auto &best=result.population.front();const auto expected=20ULL+40ULL*iterations;
        if((best.getFitness()!=1 && evaluations!=expected) || evaluations>expected || evaluations<20 || (evaluations-20)%40)throw std::runtime_error("Per-iteration evaluation count mismatch");
        auto code=best.solution;std::replace(code.begin(),code.end(),'|',' ');std::istringstream parse(code);Chromosome ids,genes;int id,gene;
        while(parse>>id>>gene){ids.push_back(id);genes.push_back(gene);}
        auto bin=data.bin;bin.setRotationWay(config.rotation).setDecoder(config.decoder).setCandidatePolicy(config.candidates);
        DBLF(bin,ids,genes,data.totalItems);auditPacking(bin,data.totalItems,best.getGenome().getGenome(),best.getGenome().getDGenome());
        if(static_cast<double>(bin.getLoadedVolume())/bin.getDimensions().getVolumen()!=best.getFitness())throw std::runtime_error("Replayed packing differs from incumbent");
        std::ostringstream genome,insertion,packing;
        genome<<"position,id,rotation\n";const auto &order=best.getGenome().getGenome(),&rotations=best.getGenome().getDGenome();
        for(size_t i=0;i<order.size();++i)genome<<i<<','<<order[i]<<','<<rotations[i]<<'\n';
        insertion<<"position,id,rotation\n";for(size_t i=0;i<ids.size();++i)insertion<<i<<','<<ids[i]<<','<<genes[i]<<'\n';
        packing<<"id,x,y,z,dx,dy,dz,rotation\n";
        for(const auto &item:bin.getLoadedItems()){auto p=item.getPunto(),d=item.getCurrentDimension();packing<<item.getId()<<','<<p.x<<','<<p.y<<','<<p.z<<','<<d.x<<','<<d.y<<','<<d.z<<','<<item.getRotationId()<<'\n';}
        writeAtomic(folder+"/genome.csv",genome.str());writeAtomic(folder+"/insertion.csv",insertion.str());writeAtomic(folder+"/packing.csv",packing.str());
        writeAtomic(folder+"/configuration.txt",config.identity("BR-source",seed)+"\n");
        std::ostringstream row;row<<std::setprecision(17)<<"dataset,instance,algorithm,iterations_requested,iterations_completed,evaluations,expected_evaluations,occupation_percent,milliseconds,termination,valid,worker,actual_seed\n"
            <<"BR"<<dataset<<','<<position+1<<','<<algorithm<<','<<iterations<<','<<(evaluations-20)/40<<','<<evaluations<<','<<expected<<','<<100*best.getFitness()<<','<<result.milliseconds<<','<<result.termination<<",1,"<<worker<<','<<seed<<'\n';
        writeAtomic(resultFile,row.str()); // Only this final marker makes a task complete.
        std::cout<<"[worker "<<worker<<"] DONE "<<name<<" occupation="<<100*best.getFitness()<<"% evaluations="<<evaluations<<std::endl;
        announce("idle",name);
    }
    announce("finished","");return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<std::endl;return 1;}}
