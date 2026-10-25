#include "../src/readData/BRInstanceReader.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/util/RandomNumbers.hpp"
#include "../src/heuristics/DBLF.hpp"
#include "../tests/PackingAudit.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <set>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
int main(int argc,char **argv){try {
    if(argc!=4 && argc!=5)throw std::invalid_argument("Usage: br_contact_adding OUTPUT ROOT GA|ABC|FFA [--all]");
    const bool all=argc==5 && std::string(argv[4])=="--all";
    if(argc==5 && !all)throw std::invalid_argument("Expected --all");
    const std::string output=argv[1],root=argv[2],algorithm=argv[3];
    if(algorithm!="GA" && algorithm!="ABC" && algorithm!="FFA")throw std::invalid_argument("Unknown algorithm");
    const unsigned mask=algorithm=="GA"?1:(algorithm=="ABC"?4:16);
#ifdef _WIN32
    if(!SetProcessAffinityMask(GetCurrentProcess(),mask))throw std::runtime_error("Could not set independent logical CPU affinity");
#endif
    std::set<std::string> completed;
    if(all) {
        std::ifstream previous(output+"/"+algorithm+".csv");std::string line;std::getline(previous,line);
        while(std::getline(previous,line)) {
            if(line.empty())continue;
            std::istringstream row(line);std::vector<std::string> fields;std::string value;while(std::getline(row,value,','))fields.push_back(value);
            if(fields.size()!=16 || fields[2]!=algorithm || fields[3]!="contact" || fields[4]!="C2" || fields[5]!="false" || fields[6]!="20" || fields[7]!="123" || fields[14]!="1" || !std::ifstream(output+"/"+fields[15]))throw std::runtime_error("Invalid resume row");
            if(!completed.insert(fields[0]+":"+fields[1]).second)throw std::runtime_error("Duplicate resume row");
        }
    }
    std::ofstream rows(output+"/"+algorithm+".csv",all?std::ios::app:std::ios::out),parameters(output+"/"+algorithm+"-parameters.txt",all?std::ios::app:std::ios::out);
    rows<<std::setprecision(17);
    if(completed.empty())rows<<"dataset,instance,algorithm,decoder,mutation,replacement,population,seed,actual_seed,occupation_percent,milliseconds,evaluations,termination,partial,valid,packing\n";
    parameters<<"Compiler "<<__VERSION__<<"; Release -O3 -DNDEBUG C++14; CPU affinity mask="<<mask<<"; one search thread\n";
    for(int index=1;index<=15;++index) {
        const std::string dataset="BR"+std::to_string(index);
        const auto instances=BRInstanceReader().read(root+"/Instance/BR/"+dataset+".txt");
        for(size_t position=0;position<(all?instances.size():1);++position) {
        if(completed.count(dataset+":"+std::to_string(position+1)))continue;
        const auto &data=instances[position];
        CommandLineOptions options;options.population=20;options.decoder=PackingDecoder::Contact;options.execution=ExecutionMode::ThirtySeconds;
        auto config=options.configuration(algorithm,ROTATION_WAY::SIX_WAY,MutationType::C2,false);
        const auto seed=instanceSeed(123,data.id,data.PSeed);seedRandomNumbers(seed);
        parameters<<dataset<<' '<<config.identity("BR-source",seed)<<std::endl;
        const auto result=config.execute(data);const auto &best=result.population.front();const auto evaluations=fitnessCacheStats().misses;
        auto code=best.solution;std::replace(code.begin(),code.end(),'|',' ');std::istringstream parsed(code);
        Chromosome ids,genes;int id,gene;while(parsed>>id>>gene){ids.push_back(id);genes.push_back(gene);}
        auto packed=data.bin;packed.setRotationWay(config.rotation).setDecoder(config.decoder).setCandidatePolicy(config.candidates);
        DBLF(packed,ids,genes,data.totalItems);
        auditPacking(packed,data.totalItems,best.getGenome().getGenome(),best.getGenome().getDGenome());
        if(static_cast<double>(packed.getLoadedVolume())/packed.getDimensions().getVolumen()!=best.getFitness())throw std::runtime_error("Replayed incumbent fitness mismatch");
        const auto stem=dataset+(position?"-i"+std::to_string(position+1):"")+"-"+algorithm;
        std::ofstream genome(output+"/"+stem+"-genome.csv");genome<<"position,id,rotation\n";
        const auto &order=best.getGenome().getGenome(),&rotations=best.getGenome().getDGenome();
        for(size_t i=0;i<order.size();++i)genome<<i<<','<<order[i]<<','<<rotations[i]<<'\n';
        std::ofstream insertion(output+"/"+stem+"-insertion.csv");insertion<<"position,id,rotation\n";
        for(size_t i=0;i<ids.size();++i)insertion<<i<<','<<ids[i]<<','<<genes[i]<<'\n';
        const auto file=stem+"-packing.csv";std::ofstream packing(output+"/"+file);
        packing<<"id,x,y,z,dx,dy,dz,rotation\n";
        for(const auto &item:packed.getLoadedItems()) {
            const auto p=item.getPunto(),d=item.getCurrentDimension();
            packing<<item.getId()<<','<<p.x<<','<<p.y<<','<<p.z<<','<<d.x<<','<<d.y<<','<<d.z<<','<<item.getRotationId()<<'\n';
        }
        if(!packing || !genome || !insertion)throw std::runtime_error("Could not write packing/genome");
        rows<<dataset<<','<<position+1<<','<<algorithm<<",contact,C2,false,20,123,"<<seed<<','<<100*best.getFitness()<<','<<result.milliseconds<<','<<evaluations<<','<<result.termination<<','<<result.partialEvaluation<<",1,"<<file<<std::endl;
        if(!rows)throw std::runtime_error("Could not save result");
        std::cout<<dataset<<" instance="<<position+1<<' '<<algorithm<<" occupation="<<100*best.getFitness()<<"% ms="<<result.milliseconds<<std::endl;
        }
    }
    return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<std::endl;return 1;}}
