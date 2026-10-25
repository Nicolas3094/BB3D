#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H
#include "ExperimentConfiguration.hpp"
#include <vector>
struct CommandLineOptions {
    bool help=false,interactive=false,sessionReport=false;
    std::string algorithm="GA",dataset="P1A2",root,output,rotation="0",mutation="inverse",replacement="true";
    DBLFCandidatePolicy candidates=DBLFCandidatePolicy::ThreePoints;
    ExecutionMode execution=ExecutionMode::Iterations;
    std::uint64_t seed=20261024;
    int population=-1,iterations=1000,rounds=1,instances=0,firstInstance=1,sites=-1,eliteSites=-1,eliteBees=4,nonEliteBees=3,movements=2,patience=25;
    double mutationProbability=-1,selection=0.85,crossover=0.75,rotationMutation=-1,diversity=0.001;
    std::size_t cacheEntries=0,cacheBytes=16*1024*1024;
    ExperimentConfiguration configuration(const std::string &,ROTATION_WAY,MutationType,bool) const;
};
CommandLineOptions parseCommandLine(const std::vector<std::string> &arguments);
std::string commandLineHelp();
#endif
