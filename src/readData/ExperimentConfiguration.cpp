#include "ExperimentConfiguration.hpp"
#include "../metaheuristics/GeneticAlgorithm.hpp"
#include "../metaheuristics/ArtificialBeeColonyAlgorithm.hpp"
#include "../metaheuristics/FireflyAlgorithm.hpp"
#include "../genetic/FitnessCache.hpp"
#include <iomanip>
#include <limits>
#include <locale>

std::uint64_t parseExperimentSeed(const std::string &text) {
    if(text.empty() || text.find_first_not_of("0123456789")!=std::string::npos)
        throw std::invalid_argument("Seed must be an unsigned decimal integer.");
    size_t read=0; auto seed=std::stoull(text,&read);
    if(read!=text.size()) throw std::invalid_argument("Invalid seed.");
    return seed;
}
std::uint64_t instanceSeed(std::uint64_t root,int id,int datasetSeed) {
    // SplitMix64 finalizer: independent of which instances have already completed.
    auto value=root ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(id))<<32) ^ static_cast<std::uint32_t>(datasetSeed);
    value+=0x9e3779b97f4a7c15ULL;
    value=(value^(value>>30))*0xbf58476d1ce4e5b9ULL;
    value=(value^(value>>27))*0x94d049bb133111ebULL;
    return value^(value>>31);
}
ExperimentConfiguration makeExperimentConfiguration(const std::string &name,ROTATION_WAY rotation,MutationType mutation,bool replacement) {
    if(name!="GA" && name!="ABC" && name!="FFA") throw std::invalid_argument("Unknown experiment algorithm.");
    ExperimentConfiguration config; config.algorithm=name; config.rotation=rotation; config.mutation=mutation; config.replacement=replacement;
    if(name!="GA") { config.individuals=name=="ABC"?20:25; config.rotationMutation=0.01; }
    return config;
}
std::string ExperimentConfiguration::identity(const std::string &datasetHash,std::uint64_t seed) const {
    std::ostringstream out; out.imbue(std::locale::classic()); out<<std::setprecision(std::numeric_limits<double>::max_digits10);
    out<<"bb3d-v5;compiler="<<__VERSION__<<";rng=mt19937_64;dataset="<<datasetHash<<";seed="<<seed
       <<";algorithm="<<algorithm<<";rotation="<<static_cast<int>(rotation)<<";mutation="<<static_cast<int>(mutation)
       <<";candidates="<<static_cast<int>(candidates)<<";replacement="<<replacement<<";adaptive="<<adaptive
       <<";iterations="<<iterations<<";population="<<individuals<<";sites="<<sites<<";eliteSites="<<eliteSites
       <<";eliteBees="<<eliteBees<<";nonEliteBees="<<nonEliteBees<<";movements="<<movements
       <<";crossover="<<crossover<<";selection="<<selection<<";mutationProbability="<<mutationProbability
       <<";rotationMutation="<<rotationMutation<<";diversity="<<diversity<<";patience="<<patience
       <<";cacheEntries="<<cacheEntries<<";cacheBytes="<<cacheBytes;
    if(execution==ExecutionMode::ThirtySeconds)out<<";execution=30s;limit_ms=30000";
    if(decoder==PackingDecoder::Contact)out<<";decoder=contact-v1";
    return out.str();
}
Poblacion ExperimentConfiguration::run(DatasetBinBacking data) const {
    return execute(std::move(data)).population;
}
ExecutionResult ExperimentConfiguration::execute(DatasetBinBacking data) const {
    if(execution==ExecutionMode::ThirtySeconds)return ThirtySecondExecution().execute(*this,std::move(data));
    if(execution!=ExecutionMode::Iterations)throw std::invalid_argument("Unknown execution mode.");
    return IterationExecution().execute(*this,std::move(data));
}
Poblacion ExperimentConfiguration::runAlgorithm(DatasetBinBacking data,bool timeOnly) const {
    configureFitnessCache(cacheEntries,cacheBytes);
    data.bin.setCandidatePolicy(candidates);
    data.bin.setDecoder(decoder);
    if(decoder==PackingDecoder::Contact && candidates!=DBLFCandidatePolicy::ThreePoints)
        throw std::invalid_argument("Contact decoder requires legacy candidates.");
    const long maximumIterations=timeOnly?std::numeric_limits<long>::max():iterations;
    if(algorithm=="GA") {
        GeneticAlgorithm ga;
        ga.setMaxIteration(maximumIterations).setNumberOfIndividuals(individuals).setCrossProbability(crossover)
          .setSelectionProbability(selection).setMutationProbabiliy(mutationProbability).setAdaptiveMutation(adaptive)
          .setDMutationProbability(rotationMutation).setMaxDiversityIndex(diversity).setStagnationLimit(timeOnly?0:patience).setIsWithReplacement(replacement)
          .setRotationType(rotation).setMutationType(mutation).setProblem(data);
        return ga.evolve();
    }
    if(algorithm=="ABC") {
        ArtificialBeeColonyAlgorithm abc;
        abc.setMaxIteration(maximumIterations).setNumberOfIndividuals(individuals).setNumberOfSites(sites).setNumberOfEliteSites(eliteSites)
           .setNumberOfEliteBees(eliteBees).setNumberOfNonEliteBees(nonEliteBees).setMutationProbabiliy(mutationProbability)
           .setDMutationProbability(rotationMutation).setIsWithReplacement(replacement).setRotationType(rotation).setMutationType(mutation).setProblem(data);
        return abc.evolve();
    }
    if(algorithm!="FFA") throw std::invalid_argument("Unknown experiment algorithm.");
    FireflyAlgorithm ffa;
    ffa.setMaxIteration(maximumIterations).setNumberOfIndividuals(individuals).setupIndex(movements).setMutationProbabiliy(mutationProbability)
       .setDMutationProbability(rotationMutation).setIsWithReplacement(replacement).setRotationType(rotation).setMutationType(mutation).setProblem(data);
    return ffa.evolve();
}
