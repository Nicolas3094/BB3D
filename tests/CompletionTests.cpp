#include "../src/metaheuristics/GeneticAlgorithm.hpp"
#include <stdexcept>
#include "../src/readData/CommandLine.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/util/RandomNumbers.hpp"
#include <limits>
void require(bool ok,const char *message) { if(!ok) throw std::runtime_error(message); }
DoubleGenome genome(Chromosome ids) { return DoubleGenome::Build().setGenome(ids).setDGenome(Chromosome(ids.size(),0)); }
Individuo individual(Chromosome ids,const std::string &key,double fitness=0.25) {
    auto ind=Individuo::Build().setGenome(genome(ids)).setFitness(fitness); ind.solution=key; return ind;
}
struct GeneticAlgorithmTestAccess {
    static void add(GeneticAlgorithm &ga,Poblacion &p,Individuo a,Individuo b) { ga.addChildredToPoblation(p,a,b); }
    static bool stop(GeneticAlgorithm &ga,Poblacion p) { return ga.terminateCondition(p); }
};
void duplicates() {
    GeneticAlgorithm ga; Poblacion p{individual({1,2,3},"old")};
    auto child=individual({2,1,3},"new");
    GeneticAlgorithmTestAccess::add(ga,p,child,child);
    require(p.size()==2,"GA added both identical children");
    GeneticAlgorithmTestAccess::add(ga,p,child,child);require(p.size()==2,"GA reinserted a visited child");
    GeneticAlgorithmTestAccess::add(ga,p,individual({3,2,1},"x"),individual({1,3,2},"y"));
    require(p.size()==4,"GA discarded distinct children");
}
template<class F> void rejects(F f) { bool failed=false;try{f();}catch(const std::exception &){failed=true;}require(failed,"Invalid input was accepted"); }
void ranges() {
    auto destination=genome({1,2,3,4,5}),source=genome({10,20,30,40,50});
    copyEqualRangeToGenome(destination,source,1,2,2,3);
    require(destination.getGenome()==Chromosome({1,30,40,4,5}),"Range copy changed a value outside its interval");
    source.setDGenome({1,2,3,4,5});
    copyReversedEqualRangeToGenome(destination,source,0,4,0,4);
    require(destination.getGenome()==Chromosome({50,40,30,20,10}) && destination.getDGenome()==Chromosome({5,4,3,2,1}),"Reverse copy lost paired genes");
    copyEqualRangeToGenome(destination,destination,1,4,0,3);
    require(destination.getGenome()==Chromosome({50,50,40,30,20}),"Overlapping self-copy did not use a snapshot");
    copyEqualRangeToGenome(destination,source,4,4,0,0);
    require(destination.getGenome().back()==10 && destination.getDGenome().back()==1,"Singleton copy failed");
    for(bool reverse:{false,true}) for(auto indexes:std::vector<std::vector<int>>{{-1,1,0,2},{0,5,0,5},{0,2,0,1},{2,1,0,0},{0,1,3,2},{0,1,-1,0},{0,1,4,5}}) {
        auto before=destination;
        rejects([&]{if(reverse) copyReversedEqualRangeToGenome(destination,source,indexes[0],indexes[1],indexes[2],indexes[3]);
                    else copyEqualRangeToGenome(destination,source,indexes[0],indexes[1],indexes[2],indexes[3]);});
        require(destination==before,"Rejected range partially modified destination");
    }
    auto bad=source;bad.setDGenome({1});rejects([&]{copyEqualRangeToGenome(destination,bad,0,0,0,0);});
}
void stopping() {
    GeneticAlgorithm ga;
    Poblacion p{individual({1,2,3},"a"),individual({3,2,1},"b")};
    require(!GeneticAlgorithmTestAccess::stop(ga,p),"Equal fitness incorrectly stopped genetically diverse population");
    for(int i=0;i<40;++i) require(!GeneticAlgorithmTestAccess::stop(ga,p),"Stagnation stopped a genetically diverse population");
    GeneticAlgorithm clones;clones.setStagnationLimit(2);
    Poblacion same{individual({1,2,3},"a"),individual({1,2,3},"b")};
    require(!GeneticAlgorithmTestAccess::stop(clones,same),"Premature convergence on first observation");
    require(!GeneticAlgorithmTestAccess::stop(clones,same),"Patience was not respected");
    require(GeneticAlgorithmTestAccess::stop(clones,same),"Converged stagnant population never stopped");
    for(auto &ind:same) ind.setFitness(0.5);
    require(!GeneticAlgorithmTestAccess::stop(clones,same),"Improvement did not reset patience");
    clones.setStagnationLimit(0);for(int i=0;i<10;++i) require(!GeneticAlgorithmTestAccess::stop(clones,same),"Disabled stagnation stopped search");
    same.front().setFitness(1);require(GeneticAlgorithmTestAccess::stop(clones,same),"Perfect fitness did not stop search");
}
DatasetBinBacking problem() {
    DatasetBinBacking d{};d.bin=Bin::Build().setLargo(10).setAncho(10).setAlto(10);
    for(int id=1;id<=6;++id)d.totalItems.push_back(ItemBin::Build().setId(id).setTipo(id).setLargo(id).setAncho(2).setAlto(1).setRotations(Punto::Build(1,1,1)));
    return d;
}
void cacheTests() {
    auto data=problem();auto a=individual({1,2,3,4,5,6},"stale"),b=individual({2,1,3,4,5,6},"b"),c=individual({3,2,1,4,5,6},"c");
    configureFitnessCache(2,8192);evaluateFitness(a,data.totalItems,data.bin);auto expected=a;
    a.setFitness(-1);a.solution="stale";evaluateFitness(a,data.totalItems,data.bin);
    require(a.getFitness()==expected.getFitness() && a.solution==expected.solution && fitnessCacheStats().hits==1,"Cache did not restore full fitness result");
    evaluateFitness(b,data.totalItems,data.bin);evaluateFitness(a,data.totalItems,data.bin);evaluateFitness(c,data.totalItems,data.bin);
    auto before=fitnessCacheStats();evaluateFitness(b,data.totalItems,data.bin);
    require(fitnessCacheStats().misses==before.misses+1 && fitnessCacheStats().evictions>0,"LRU evicted the recently used key");
    auto changed=data.bin;changed.setLargo(20);evaluateFitness(a,data.totalItems,changed);
    require(a.getFitness()==42.0/2000 && fitnessCacheStats().contextChanges==2,"Cache reused a different container");
    data.totalItems[0].setLargo(2);evaluateFitness(a,data.totalItems,changed);
    require(a.getFitness()==44.0/2000 && fitnessCacheStats().contextChanges==3,"Cache reused a changed dataset");
    changed.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);evaluateFitness(a,data.totalItems,changed);
    require(fitnessCacheStats().contextChanges==4,"Cache ignored candidate policy");
    auto invalid=a;invalid.getGenome().getGenome()[0]=0;rejects([&]{evaluateFitness(invalid,data.totalItems,changed);});
    configureFitnessCache(2,8192);seedRandomNumbers(1);
    for(int i=0;i<100;++i) {a.setGenome(genome(generatePermutation(6)));evaluateFitness(a,data.totalItems,data.bin);auto stats=fitnessCacheStats();require(stats.entries<=2 && stats.payloadBytes<=8192,"Cache exceeded its bounds");}
    configureFitnessCache(2,1);evaluateFitness(a,data.totalItems,data.bin);require(fitnessCacheStats().entries==0 && fitnessCacheStats().payloadBytes==0,"Oversized context entered cache");
    configureFitnessCache(4,8192);auto empty=individual({},"old");evaluateFitness(empty,{},data.bin);evaluateFitness(empty,{},data.bin);
    require(empty.getFitness()==0 && empty.solution=="|" && fitnessCacheStats().hits==1,"Zero fitness was not cached correctly");
    for(const auto &algorithm:{"GA","ABC","FFA"}) {
        auto config=makeExperimentConfiguration(algorithm,ROTATION_WAY::ZERO_WAY,MutationType::C2,true);
        config.iterations=3;config.individuals=6;config.sites=3;config.eliteSites=1;config.cacheEntries=0;
        seedRandomNumbers(22);auto uncached=config.run(problem());config.cacheEntries=64;
        seedRandomNumbers(22);auto cached=config.run(problem());
        require(uncached.size()==cached.size(),"Cache changed population size");
        for(size_t i=0;i<cached.size();++i)require(uncached[i].getGenome()==cached[i].getGenome() && uncached[i].solution==cached[i].solution && uncached[i].getFitness()==cached[i].getFitness(),"Cache changed seeded search results");
    }
    configureFitnessCache(0,0);
}
void cli() {
    require(parseCommandLine({}).help && parseCommandLine({"--help"}).help,"Help requires configuration");
    require(parseCommandLine({}).cacheEntries==0,"Cache must remain opt-in after profiling");
    auto o=parseCommandLine({"--algorithm","all","--dataset","P5A2","--population","4","--iterations","2","--rounds","3","--cache-entries","0","--patience","7"});
    auto c=o.configuration("ABC",ROTATION_WAY::SIX_WAY,MutationType::C1,false);
    require(c.individuals==4 && c.sites<=4 && c.eliteSites<=c.sites && c.patience==7 && c.cacheEntries==0,"CLI overrides were not applied");
    require(parseCommandLine({"--batch"}).algorithm=="all","Batch omitted algorithms");
    for(auto args:std::vector<std::vector<std::string>>{{"--bad","1"},{"--seed"},{"--seed","-1"},{"--rounds","0"},{"--population","0"},{"--iterations","-1"},{"--rotation","3"},{"--algorithm","bad"},{"--selection","nan"},{"--seed","1","--seed","2"}})
        rejects([&]{parseCommandLine(args);});
    rejects([]{parseCommandLine({"--population","1"}).configuration("GA",ROTATION_WAY::ZERO_WAY,MutationType::C1,true);});
    rejects([]{parseCommandLine({"--population","4","--sites","5"}).configuration("ABC",ROTATION_WAY::ZERO_WAY,MutationType::C1,true);});
}
int main(int argc,char **argv) {
    try { int group=argc>1?std::stoi(argv[1]):0;
        if(group==0||group==1) duplicates(); if(group==0||group==2) ranges(); if(group==0||group==3) stopping();
        if(group==0||group==4) cacheTests();if(group==0||group==5) cli();
        std::cout<<"PASS: unique children, safe ranges, genetic stopping, bounded LRU cache and CLI configuration\n"; return 0;
    } catch(const std::exception &e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
