#ifndef FITNESS_CACHE_H
#define FITNESS_CACHE_H
#include "Individuo.hpp"
#include <cstddef>
struct FitnessCacheStats {
    std::size_t hits=0,misses=0,evictions=0,contextChanges=0,entries=0,payloadBytes=0,peakPayloadBytes=0;
};
// Serial evaluator cache. Bounds cover entry count and stored string payload;
// container/allocator overhead is additional and bounded by the entry count.
void configureFitnessCache(std::size_t entries,std::size_t payloadBytes);
FitnessCacheStats fitnessCacheStats();
// Internal helpers used by the encoded fitness evaluator.
bool lookupFitness(Individuo &,const LoadedBins &,const Bin &,std::string &key);
void rememberFitness(const Individuo &,const std::string &key);
#endif
