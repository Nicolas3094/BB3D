#include "ExperimentRunner.hpp"
#include "ReadData.hpp"
#include "ExperimentResults.hpp"
#include "../genetic/FitnessCache.hpp"
#include "../util/RandomNumbers.hpp"
#include <chrono>
#include <iomanip>
#include <locale>

void printExperimentSummary(std::ostream &output,const std::vector<ExperimentOutcome> &rows) {
    std::ostringstream summary;summary.imbue(std::locale::classic());
    summary<<std::fixed<<std::setprecision(3)<<"\n=== RESULTADOS FINALES ===\n";
    for(const auto &row:rows) {
        const char *reason=row.termination=="time_limit"?"limite de tiempo":(row.termination=="optimal"?"ocupacion completa":"completada");
        summary<<row.dataset<<" | problema "<<row.position<<" (ID "<<row.instanceId<<") | "<<row.algorithm
               <<" | ronda "<<row.round<<" | rotaciones "<<static_cast<int>(row.rotation)
               <<" | mutacion "<<(row.mutation==MutationType::INVERSE_MUTATION?"inversion":(row.mutation==MutationType::C1?"C1":"C2"))
               <<" | "<<(row.replacement?"con reemplazo":"sin reemplazo")
               <<" | ocupacion: "<<100*row.fitness<<" % | tiempo: "<<row.milliseconds<<" ms | "<<reason;
        if(row.partialEvaluation)summary<<" | evaluacion parcial (empaquetado valido)";
        if(row.resumed)summary<<" | recuperado del archivo";
        summary<<'\n';
    }
    summary<<"Total: "<<rows.size()<<" resultados.\n";
    summary<<"\n=== PORCENTAJE DE OCUPACION FINAL ===\n";
    for(const auto &row:rows) {
        summary<<row.dataset<<" | problema "<<row.position<<" | "<<row.algorithm<<" | ronda "<<row.round
               <<" | rotaciones "<<static_cast<int>(row.rotation)
               <<" | "<<(row.mutation==MutationType::INVERSE_MUTATION?"inversion":(row.mutation==MutationType::C1?"C1":"C2"))
               <<" | "<<(row.replacement?"con reemplazo":"sin reemplazo")
               <<" -> OCUPACION FINAL: "<<100*row.fitness<<" %\n";
    }
    output<<summary.str();
}

std::vector<ExperimentOutcome> runExperiments(const CommandLineOptions &o) {
    std::vector<ExperimentOutcome> outcomes;
    if(o.firstInstance<1 || o.instances<0) throw std::invalid_argument("Instance selection must be non-negative and start at a positive position.");
    auto algorithms=o.algorithm=="all"?std::vector<std::string>{"GA","ABC","FFA"}:std::vector<std::string>{o.algorithm};
    auto names=o.dataset=="all"?std::vector<std::string>{"P1A2","P2A2","P3A2","P4A2","P5A2"}:std::vector<std::string>{o.dataset};
    if(o.dataset=="BR") {names.clear();for(int file=0;file<=18;++file)names.push_back("BR"+std::to_string(file));}
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
        const auto startIndex=static_cast<size_t>(o.firstInstance-1);
        if(startIndex>=datasets.size()) throw std::invalid_argument("First instance exceeds dataset size.");
        const auto available=datasets.size()-startIndex;
        const auto limit=o.instances==0?available:std::min(available,static_cast<size_t>(o.instances));
        std::string stem=path.substr(path.find_last_of("/\\")==std::string::npos?0:path.find_last_of("/\\")+1);
        if(stem.size()>4 && (stem.substr(stem.size()-4)==".csv" || stem.substr(stem.size()-4)==".txt")) stem.resize(stem.size()-4);
        for(const auto &algorithm:algorithms) for(auto rotation:rotations) for(auto mutation:mutations) for(bool replacement:replacements)
        for(int round=0;round<o.rounds;++round) {
            const auto config=o.configuration(algorithm,rotation,mutation,replacement);
            const auto seed=round==0?o.seed:instanceSeed(o.seed,round,0);
            auto identity=config.identity(hash,seed)+";rootSeed="+std::to_string(o.seed)+";round="+std::to_string(round)+";instances="+std::to_string(limit);
            if(o.firstInstance!=1) identity+=";firstInstance="+std::to_string(o.firstInstance);
            const auto directory=o.output.empty()?o.root+"/Results/cli":o.output;
            const auto prefix=directory+"/"+stem+"/"+algorithm+"/"+experimentFingerprint(identity);
            const auto fitness=prefix+"-fitness.csv",times=prefix+"-time.csv";
            auto results=loadExperimentResults(fitness,times,limit,identity);
            for(size_t i=0;i<limit;++i) {
                const auto &data=datasets[startIndex+i];
                const bool resumed=isExperimentComplete(results[i]);
                const auto actualSeed=instanceSeed(seed,data.id,data.PSeed);
                if(resumed) std::cout<<algorithm<<" instance="<<data.id<<" round="<<round<<" resumed=complete\n";
                else {
                    results[i]={-1,-1,ExperimentStatus::Running};saveExperimentResults(results,fitness,times,identity);
                    seedRandomNumbers(actualSeed);
                    const auto result=config.execute(data);
                    if(result.population.empty()) throw std::runtime_error("Algorithm returned an empty population.");
                    results[i]={result.population.front().getFitness(),result.milliseconds,ExperimentStatus::Complete,result.termination,result.partialEvaluation};saveExperimentResults(results,fitness,times,identity);
                    const auto cache=fitnessCacheStats();
                    std::cout<<algorithm<<" instance="<<data.id<<" round="<<round<<" fitness="<<result.population.front().getFitness()<<" milliseconds="<<result.milliseconds<<" termination="<<result.termination<<" cache_hits="<<cache.hits<<" cache_misses="<<cache.misses<<'\n';
                }
                ExperimentOutcome row;row.dataset=stem;row.algorithm=algorithm;row.checkpoint=fitness+".checkpoint";
                row.position=static_cast<int>(startIndex+i+1);row.instanceId=data.id;row.datasetSeed=data.PSeed;row.round=round+1;
                row.rotation=rotation;row.mutation=mutation;row.replacement=replacement;row.resumed=resumed;row.seed=actualSeed;
                row.fitness=results[i].fitness;row.milliseconds=results[i].milliseconds;outcomes.push_back(std::move(row));
                outcomes.back().execution=config.execution;outcomes.back().termination=results[i].termination;outcomes.back().partialEvaluation=results[i].partialEvaluation;
            }
        }
    }
    printExperimentSummary(std::cout,outcomes);
    return outcomes;
}
