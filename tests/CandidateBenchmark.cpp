#include "../src/readData/ReadData.hpp"
#include "../src/heuristics/DBLF.hpp"
#include "../src/genetic/GenomeOperators.hpp"
#include "../src/util/RandomNumbers.hpp"
#include <chrono>
#include <iomanip>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#endif
int main(int argc,char **argv) {
 try {
    if(argc==2 && std::string(argv[1])=="--find") {
        seedRandomNumbers(881);
        for(int trial=0;trial<3000;++trial) {
            LoadedBins items;
            for(int id=1;id<=12;++id) items.push_back(ItemBin::Build().setId(id).setTipo(id).setLargo(randomInteger(1,7)).setAncho(randomInteger(1,7)).setAlto(randomInteger(1,7)).setRotations(Punto::Build(1,1,1)));
            auto original=Bin::Build().setLargo(10).setAncho(10).setAlto(10).setRotationWay(ROTATION_WAY::ZERO_WAY);
            auto legacy=original,projected=original; projected.setCandidatePolicy(DBLFCandidatePolicy::Projected);
            Chromosome ids; for(int i=1;i<=12;++i) ids.push_back(i);
            DBLF(legacy,ids,Chromosome(12,0),items); DBLF(projected,ids,Chromosome(12,0),items);
            if(projected.getLoadedVolume()>legacy.getLoadedVolume()) {
                std::cout<<"trial="<<trial<<" legacy="<<legacy.getLoadedVolume()<<" projected="<<projected.getLoadedVolume()<<'\n';
                for(const auto &item:items) { auto d=item.getDimension(); std::cout<<d.getLargo()<<','<<d.getAncho()<<','<<d.getAlto()<<'\n'; }
                return 0;
            }
        }
        throw std::runtime_error("No improved fixture found");
    }
    if(argc!=4) throw std::invalid_argument("Usage: candidate_benchmark policy output-prefix rounds");
    int policy=std::stoi(argv[1]),rounds=std::stoi(argv[3]);
    if(policy<0 || policy>2 || rounds<1) throw std::invalid_argument("Invalid benchmark options");
    std::ofstream rows(std::string(argv[2])+"-cases.csv"),summary(std::string(argv[2])+"-summary.csv");
    if(!rows || !summary) throw std::runtime_error("Cannot open benchmark output");
    rows<<"round,dataset,instance,rotation,seed,items,volume,occupancy,microseconds\n"<<std::setprecision(17);
    summary<<"round,policy,evaluations,milliseconds,mean_occupancy,peak_working_set_bytes\n"<<std::setprecision(17);
    std::vector<std::vector<DatasetBinBacking>> datasets;
    for(auto name:{"P1A2","P2A2","P3A2","P4A2","P5A2"}) datasets.push_back(readData(std::string("Instance/")+name+".csv"));
    for(int round=-1;round<rounds;++round) {
        double milliseconds=0,totalOccupancy=0; int count=0;
        for(size_t dataset=0;dataset<datasets.size();++dataset)
        for(size_t i=0;i<10;++i)
        for(auto mode:{ROTATION_WAY::ZERO_WAY,ROTATION_WAY::TWO_WAY,ROTATION_WAY::SIX_WAY})
        for(int seed:{123,456}) {
            auto &data=datasets[dataset][i]; seedRandomNumbers(seed);
            auto ids=generatePermutation(static_cast<int>(data.totalItems.size()));
            auto rotations=generateRandomRepeatedAlalleleChromosome(ids.size(),1,getIdFromRotationWay(mode));
            auto bin=data.bin; bin.setRotationWay(mode).setCandidatePolicy(static_cast<DBLFCandidatePolicy>(policy));
            auto start=std::chrono::steady_clock::now(); DBLF(bin,ids,rotations,data.totalItems);
            double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();
            double occupancy=static_cast<double>(bin.getLoadedVolume())/bin.getDimensions().getVolumen();
            milliseconds+=us/1000; totalOccupancy+=occupancy; ++count;
            if(round>=0) rows<<round<<','<<dataset+1<<','<<i+1<<','<<static_cast<int>(mode)<<','<<seed<<','<<bin.getNumberOfLoadedItems()<<','<<bin.getLoadedVolume()<<','<<occupancy<<','<<us<<'\n';
        }
        std::uint64_t peak=0;
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS counters{}; counters.cb=sizeof(counters);
        if(!K32GetProcessMemoryInfo(GetCurrentProcess(),&counters,sizeof(counters))) throw std::runtime_error("Cannot measure process memory");
        peak=counters.PeakWorkingSetSize;
#endif
        if(round>=0) summary<<round<<','<<policy<<','<<count<<','<<milliseconds<<','<<totalOccupancy/count<<','<<peak<<'\n';
    }
    return 0;
 } catch(const std::exception &e) { std::cerr<<e.what()<<'\n'; return 1; }
}
