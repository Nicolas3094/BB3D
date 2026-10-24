#ifndef EXPERIMENT_CONFIGURATION_H
#define EXPERIMENT_CONFIGURATION_H
#include "DataSetBPP.hpp"
#include "../genetic/Individuo.hpp"
#include "../genetic/MutationType.hpp"
#include <cstdint>

struct ExperimentConfiguration {
    std::string algorithm;
    ROTATION_WAY rotation=ROTATION_WAY::ZERO_WAY;
    MutationType mutation=MutationType::INVERSE_MUTATION;
    DBLFCandidatePolicy candidates=DBLFCandidatePolicy::ThreePoints;
    bool replacement=true,adaptive=true;
    long iterations=1000;
    long patience=25;
    std::size_t cacheEntries=0,cacheBytes=16*1024*1024;
    int individuals=50,sites=11,eliteSites=8,eliteBees=4,nonEliteBees=3,movements=2;
    double crossover=0.75,selection=0.85,mutationProbability=1,rotationMutation=0.05,diversity=0.001;
    std::string identity(const std::string &datasetHash,std::uint64_t seed) const;
    Poblacion run(DatasetBinBacking data) const;
};
ExperimentConfiguration makeExperimentConfiguration(const std::string &,ROTATION_WAY,MutationType,bool);
std::uint64_t parseExperimentSeed(const std::string &);
std::uint64_t instanceSeed(std::uint64_t root,int instanceId,int datasetSeed);
#endif
