#include "ExperimentRunner.hpp"
#include "ReadData.hpp"
#include "ExperimentResults.hpp"
#include "../genetic/FitnessCache.hpp"
#include "../util/RandomNumbers.hpp"
#include <chrono>

void runExperiments(const CommandLineOptions &o) {
    auto algorithms=o.algorithm=="all"?std::vector<std::string>{"GA","ABC","FFA"}:std::vector<std::string>{o.algorithm};
    auto names=o.dataset=="all"?std::vector<std::string>{"P1A2","P2A2","P3A2","P4A2","P5A2"}:std::vector<std::string>{o.dataset};
    auto rotations=o.rotation=="all"?std::vector<ROTATION_WAY>{ROTATION_WAY::ZERO_WAY,ROTATION_WAY::TWO_WAY,ROTATION_WAY::SIX_WAY}:
        std::vector<ROTATION_WAY>{o.rotation=="0"?ROTATION_WAY::ZERO_WAY:(o.rotation=="2"?ROTATION_WAY::TWO_WAY:ROTATION_WAY::SIX_WAY)};
    auto mutations=o.mutation=="all"?std::vector<MutationType>{MutationType::INVERSE_MUTATION,MutationType::C1,MutationType::C2}:
        std::vector<MutationType>{o.mutation=="inverse"?MutationType::INVERSE_MUTATION:(o.mutation=="c1"?MutationType::C1:MutationType::C2)};
    auto replacements=o.replacement=="both"?std::vector<bool>{true,false}:std::vector<bool>{o.replacement=="true"};
    // Validate every selected algorithm before any result file is written.
    for(const auto &algorithm:algorithms) o.configuration(algorithm,rotations.front(),mutations.front(),replacements.front());
    for(const auto &name:names) {
        const bool file=name.find_first_of("/\\")!=std::string::npos || (name.size()>=4 && (name.substr(name.size()-4)==".csv" || name.substr(name.size()-4)==".txt"));
        const bool br=name.size()>2 && name.substr(0,2)=="BR" && name.find_first_not_of("0123456789",2)==std::string::npos;
        const std::string path=file?name:(br?o.root+"/Instance/BR/"+name+".txt":o.root+"/Instance/"+name+".csv");
        const auto datasets=readData(path);const auto hash=datasetFingerprint(path);
        const auto limit=o.instances==0?datasets.size():std::min(datasets.size(),static_cast<size_t>(o.instances));
        std::string stem=path.substr(path.find_last_of("/\\")==std::string::npos?0:path.find_last_of("/\\")+1);
        if(stem.size()>4 && (stem.substr(stem.size()-4)==".csv" || stem.substr(stem.size()-4)==".txt")) stem.resize(stem.size()-4);
        for(const auto &algorithm:algorithms) for(auto rotation:rotations) for(auto mutation:mutations) for(bool replacement:replacements)
        for(int round=0;round<o.rounds;++round) {
            const auto config=o.configuration(algorithm,rotation,mutation,replacement);
            const auto seed=round==0?o.seed:instanceSeed(o.seed,round,0);
            const auto identity=config.identity(hash,seed)+";rootSeed="+std::to_string(o.seed)+";round="+std::to_string(round)+";instances="+std::to_string(limit);
            const auto directory=o.output.empty()?o.root+"/Results/cli":o.output;
            const auto prefix=directory+"/"+stem+"/"+algorithm+"/"+experimentFingerprint(identity);
            const auto fitness=prefix+"-fitness.csv",times=prefix+"-time.csv";
            auto results=loadExperimentResults(fitness,times,limit,identity);
            for(size_t i=0;i<limit;++i) {
                if(isExperimentComplete(results[i])) {std::cout<<algorithm<<" instance="<<datasets[i].id<<" round="<<round<<" resumed=complete\n";continue;}
                results[i]={-1,-1,ExperimentStatus::Running};saveExperimentResults(results,fitness,times,identity);
                seedRandomNumbers(instanceSeed(seed,datasets[i].id,datasets[i].PSeed));
                const auto start=std::chrono::steady_clock::now();auto best=config.run(datasets[i]);
                const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();
                if(best.empty()) throw std::runtime_error("Algorithm returned an empty population.");
                results[i]={best.front().getFitness(),elapsed,ExperimentStatus::Complete};saveExperimentResults(results,fitness,times,identity);
                const auto cache=fitnessCacheStats();
                std::cout<<algorithm<<" instance="<<datasets[i].id<<" round="<<round<<" fitness="<<best.front().getFitness()<<" milliseconds="<<elapsed<<" cache_hits="<<cache.hits<<" cache_misses="<<cache.misses<<'\n';
            }
        }
    }
}
