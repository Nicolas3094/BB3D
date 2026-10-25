#include "FitnessCache.hpp"
#include "../heuristics/DBLFValidation.hpp"
#include <list>
#include <unordered_map>
#include <cstdint>
#include <limits>

namespace {
struct Entry { double fitness; std::string solution; std::list<std::string>::iterator position; std::size_t charge; };
struct Cache {
    std::size_t maxEntries=0,maxBytes=0,bytes=0;
    std::string context;
    std::list<std::string> lru;
    std::unordered_map<std::string,Entry> entries;
    FitnessCacheStats stats;
    void clear() { entries.clear(); lru.clear(); bytes=0; }
};
Cache &cache() { static Cache value; return value; }
void append(std::string &key,std::int64_t value) {
    auto bits=static_cast<std::uint64_t>(value);
    for(int byte=0;byte<8;++byte) { key.push_back(static_cast<char>(bits&255)); bits>>=8; }
}
void point(std::string &key,Punto p) { append(key,p.x);append(key,p.y);append(key,p.z); }
std::string context(const LoadedBins &items,const Bin &bin) {
    std::string result;
    const auto d=bin.getDimensions(); append(result,d.getLargo());append(result,d.getAncho());append(result,d.getAlto());
    append(result,static_cast<int>(bin.getRotationWay()));append(result,static_cast<int>(bin.getCandidatePolicy()));append(result,items.size());
    for(const auto &item:items) {
        checkExecutionDeadline();
        const auto d=item.getDimension();
        append(result,item.getId());append(result,item.getTipo());append(result,d.getLargo());append(result,d.getAncho());append(result,d.getAlto());
        point(result,item.getCurrentDimension());point(result,item.getRotations());point(result,item.getPunto());
        append(result,item.getRotationId());append(result,item.isValidOrientation());
    }
    return result;
}
}
void configureFitnessCache(std::size_t entries,std::size_t bytes) {
    auto &c=cache();c.clear();std::string{}.swap(c.context);c.entries.rehash(0);c.stats={};c.maxEntries=entries;c.maxBytes=bytes;
}
FitnessCacheStats fitnessCacheStats() {
    auto &c=cache();auto result=c.stats;result.entries=c.entries.size();result.payloadBytes=c.bytes;return result;
}
bool lookupFitness(Individuo &ind,const LoadedBins &items,const Bin &bin,std::string &key) {
    auto &c=cache(); key.clear();
    if(c.maxEntries==0 || c.maxBytes==0) {++c.stats.misses;return false;}
    validateDBLFInput(bin,ind.getGenome().getGenome(),ind.getGenome().getDGenome(),items);
    auto current=context(items,bin);
    if(current.size()>c.maxBytes) { c.clear();c.context.clear();++c.stats.misses;return false; }
    if(current!=c.context) {
        c.clear();c.context=std::move(current);c.bytes=c.context.size();++c.stats.contextChanges;
        c.stats.peakPayloadBytes=std::max(c.stats.peakPayloadBytes,c.bytes);
    }
    const auto &genome=ind.getGenome();append(key,genome.getGenome().size());
    for(int id:genome.getGenome()) append(key,id);
    for(int rotation:genome.getDGenome()) append(key,rotation);
    auto found=c.entries.find(key);
    if(found==c.entries.end()) { ++c.stats.misses;return false; }
    ++c.stats.hits;c.lru.splice(c.lru.begin(),c.lru,found->second.position);
    ind.setFitness(found->second.fitness);ind.solution=found->second.solution;return true;
}
void rememberFitness(const Individuo &ind,const std::string &key) {
    auto &c=cache();
    if(c.maxEntries==0 || c.maxBytes==0 || key.empty()) return;
    if(key.size()>(std::numeric_limits<std::size_t>::max()-ind.solution.size())/2) return;
    const auto charge=2*key.size()+ind.solution.size();
    if(charge>c.maxBytes-c.context.size() || c.entries.count(key)) return;
    while(!c.entries.empty() && (c.entries.size()>=c.maxEntries || charge>c.maxBytes-c.bytes)) {
        auto last=c.entries.find(c.lru.back());c.bytes-=last->second.charge;c.entries.erase(last);c.lru.pop_back();++c.stats.evictions;
    }
    c.lru.push_front(key);
    try { c.entries.emplace(key,Entry{ind.getFitness(),ind.solution,c.lru.begin(),charge}); }
    catch(...) { c.lru.pop_front();throw; }
    c.bytes+=charge;c.stats.peakPayloadBytes=std::max(c.stats.peakPayloadBytes,c.bytes);
}
