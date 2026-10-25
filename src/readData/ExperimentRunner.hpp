#ifndef EXPERIMENT_RUNNER_H
#define EXPERIMENT_RUNNER_H
#include "CommandLine.hpp"
#include <iosfwd>
struct ExperimentOutcome {
    std::string dataset,algorithm,checkpoint;
    int position=0,instanceId=0,datasetSeed=0,round=0;
    ROTATION_WAY rotation=ROTATION_WAY::ZERO_WAY;
    MutationType mutation=MutationType::INVERSE_MUTATION;
    bool replacement=true,resumed=false;
    ExecutionMode execution=ExecutionMode::Iterations;
    PackingDecoder decoder=PackingDecoder::DBLF;
    std::string termination="completed";
    bool partialEvaluation=false;
    std::uint64_t seed=0;
    double fitness=0;
    std::int64_t milliseconds=0;
};
std::vector<ExperimentOutcome> runExperiments(const CommandLineOptions &);
void printExperimentSummary(std::ostream &,const std::vector<ExperimentOutcome> &);
#endif
