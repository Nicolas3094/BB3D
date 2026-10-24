#include "../src/readData/ReadData.hpp"
#include "../src/readData/ExperimentConfiguration.hpp"
#include "../src/genetic/FitnessOperators.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/util/RandomNumbers.hpp"
#include <chrono>
#include <iomanip>

std::string witness(const Poblacion &population) {
    std::ostringstream out;out<<std::setprecision(17);
    for(const auto &ind:population) {
        out<<ind.getFitness()<<'|'<<ind.solution<<'|';
        for(int id:ind.getGenome().getGenome())out<<id<<',';
        out<<'|';for(int rotation:ind.getGenome().getDGenome())out<<rotation<<',';
        out<<'\n';
    }
    return out.str();
}
int main(int argc,char **argv) {
 try {
    if(argc!=3) throw std::invalid_argument("Usage: cache_benchmark output.csv rounds");
    const int rounds=std::stoi(argv[2]);if(rounds<1)throw std::invalid_argument("Rounds must be positive");
    auto data=readData("Instance/P5A2.csv").front();data.bin.setRotationWay(ROTATION_WAY::SIX_WAY);
    const int n=static_cast<int>(data.totalItems.size());
    std::ofstream output(argv[1]);if(!output)throw std::runtime_error("Cannot open benchmark output");
    output<<"workload,round,cache,requests,milliseconds,hits,dbfl_evaluations,evictions,entries,peak_payload_bytes\n"<<std::setprecision(17);
    for(const std::string workload:{"repeat98","unique","ga"}) {
        seedRandomNumbers(123);std::vector<DoubleGenome> cases;
        if(workload!="ga")for(int i=0;i<(workload=="repeat98"?20:1000);++i)
            cases.push_back(DoubleGenome::Build().setGenome(generatePermutation(n)).setDGenome(generateRandomRepeatedAlalleleChromosome(n,1,6)));
        std::string reference;
        for(int round=-1;round<rounds;++round)for(int index=0;index<2;++index) {
            const bool enabled=(round%2==0)?index==1:index==0;
            configureFitnessCache(enabled?256:0,8*1024*1024);seedRandomNumbers(456);
            const auto start=std::chrono::steady_clock::now();Poblacion results;
            if(workload=="ga") {
                auto config=makeExperimentConfiguration("GA",ROTATION_WAY::SIX_WAY,MutationType::C2,true);
                config.individuals=20;config.iterations=30;config.patience=0;config.cacheEntries=enabled?256:0;config.cacheBytes=8*1024*1024;
                results=config.run(data);
            } else {
                results.reserve(1000);
                for(int i=0;i<1000;++i) {auto ind=Individuo::Build().setGenome(cases[i%cases.size()]);evaluateFitness(ind,data.totalItems,data.bin);results.push_back(std::move(ind));}
            }
            const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
            const auto stats=fitnessCacheStats();const auto actual=witness(results);
            if(reference.empty())reference=actual;
            if(reference!=actual)throw std::runtime_error("Cache changed exact fitness, solution or genome results");
            if(stats.entries>256 || stats.payloadBytes>8*1024*1024)throw std::runtime_error("Cache exceeded bounds");
            if(round>=0)output<<workload<<','<<round<<','<<enabled<<','<<stats.hits+stats.misses<<','<<ms<<','<<stats.hits<<','<<stats.misses<<','<<stats.evictions<<','<<stats.entries<<','<<stats.peakPayloadBytes<<'\n';
        }
    }
    std::cout<<"PASS: cached/uncached exact results match; bounded cache across repeated, unique and GA workloads\n";
    return 0;
 } catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
}
