#include "ExperimentExecution.hpp"
#include "../readData/ExperimentConfiguration.hpp"
#include "../heuristics/DBLFValidation.hpp"
#include "../util/ExecutionBudget.hpp"
#include <utility>
constexpr std::int64_t ThirtySecondExecution::limitMilliseconds;
namespace {
std::int64_t elapsed(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();
}
}
ExecutionResult IterationExecution::execute(const ExperimentConfiguration &config,DatasetBinBacking data) const {
    const auto start=std::chrono::steady_clock::now();ExecutionResult result;
    result.population=config.runAlgorithm(std::move(data),false);result.milliseconds=elapsed(start);
    if(!result.population.empty() && result.population.front().getFitness()==1)result.termination="optimal";
    return result;
}
ExecutionResult ThirtySecondExecution::execute(const ExperimentConfiguration &config,DatasetBinBacking data) const {
    const auto start=std::chrono::steady_clock::now();
    if(data.totalItems.empty())throw std::invalid_argument("Timed execution requires non-empty items.");
    data.bin.setRotationWay(config.rotation).setCandidatePolicy(config.candidates);
    Chromosome ids,rotations;ids.reserve(data.totalItems.size());rotations.reserve(data.totalItems.size());
    for(const auto &item:data.totalItems){ids.push_back(item.getId());rotations.push_back(config.rotation==ROTATION_WAY::ZERO_WAY?0:1);}
    validateDBLFInput(data.bin,ids,rotations,data.totalItems);
    auto fallback=Individuo::Build().setGenome(DoubleGenome::Build().setGenome(ids).setDGenome(rotations)).setFitness(0);
    fallback.solution="|";
    ExecutionBudget budget(std::chrono::milliseconds(limitMilliseconds),std::move(fallback),now);
    ExecutionResult result;
    try {
        const auto population=config.runAlgorithm(std::move(data),true);
        for(const auto &candidate:population)budget.observe(candidate,false);
    } catch(const ExecutionTimeLimit &) {result.timeLimitReached=true;result.termination="time_limit";}
    result.population={budget.best()};result.partialEvaluation=budget.partial();result.milliseconds=elapsed(start);
    if(!result.timeLimitReached && result.population.front().getFitness()==1)result.termination="optimal";
    return result;
}
