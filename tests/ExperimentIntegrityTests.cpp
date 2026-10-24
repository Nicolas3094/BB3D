#include "../src/genetic/FitnessOperators.hpp"
#include "../src/metaheuristics/GeneticAlgorithm.hpp"
#include <stdexcept>
#include "../src/readData/Env.hpp"
#include "../src/readData/ExperimentConfiguration.hpp"
#include "../src/readData/ExperimentResults.hpp"
#include "../src/genetic/GenomeOperators.hpp"
#include "../src/util/RandomNumbers.hpp"
#include "../src/binPacking/ItemBinOperators.hpp"
void require(bool ok,const char *message) { if(!ok) throw std::runtime_error(message); }
void checkIdentity() {
    auto item=ItemBin::Build().setId(1).setTipo(1).setLargo(1).setAncho(1).setAlto(1).setRotations(Punto::Build(1,1,1));
    auto bin=Bin::Build().setLargo(2).setAncho(2).setAlto(2).setRotationWay(ROTATION_WAY::ZERO_WAY);
    auto individual=Individuo::Build().setGenome(DoubleGenome::Build().setGenome({1}).setDGenome({0}));
    individual.solution="stale";
    evaluateFitnessDecodificated(individual,{item},bin);
    require(individual.solution=="|1 0|","Decoded fitness left a stale solution identity");
}
template<class F> void rejects(F f) { bool caught=false; try { f(); } catch(const std::exception &) { caught=true; } require(caught,"Malformed input accepted"); }
std::string fixture(const std::string &name,const std::string &bytes) {
    std::string path="build/infrastructure-tests/"+name;
    std::ofstream file(path,std::ios::binary); file<<bytes; file.close(); require(!file.fail(),"Cannot write fixture"); return path;
}
DatasetBinBacking problem() {
    DatasetBinBacking data{}; data.id=1; data.PSeed=42;
    data.bin=Bin::Build().setLargo(10).setAncho(10).setAlto(10);
    for(int id=1;id<=6;++id) data.totalItems.push_back(ItemBin::Build().setId(id).setTipo(id).setLargo(id).setAncho(2).setAlto(1).setRotations(Punto::Build(1,1,1)));
    return data;
}
struct GeneticAlgorithmTestAccess {
    static void mutate(GeneticAlgorithm &a,Poblacion &p) { a.visitedIndividuals["stale"]=true; a.mutatePoblation(p,0,0); }
    static double probability(GeneticAlgorithm &a,Individuo i) { return a.adaptiveMutationProbabiliy(i,i); }
};
void checkGA() {
    auto data=problem(); GeneticAlgorithm ga; ga.setProblem(data).setMutationType(MutationType::INVERSE_MUTATION);
    auto genome=DoubleGenome::Build().setGenome({1,2,3,4,5,6}).setDGenome({0,0,0,0,0,0});
    auto ind=Individuo::Build().setGenome(genome).setFitness(0.25); ind.solution="stale";
    for(double p:{0.0,1.0}) {
        ga.setMutationProbabiliy(p); Poblacion population{ind}; seedRandomNumbers(123);
        GeneticAlgorithmTestAccess::mutate(ga,population);
        require((population[0].getGenome()==genome)==(p==0),"GA fixed mutation probability ignored");
        if(p==1) require(population[0].solution!="stale" && population[0].getFitness()==42.0/1000,"Mutation checked old solution identity");
    }
    ga.setAdaptiveMutation(true);
    require(GeneticAlgorithmTestAccess::probability(ga,ind)==0.75,"Cannot restore adaptive mutation");
}
void checkEnv() {
    auto path=fixture("settings.env","# comment\r\n LOCAL_PATH = \"C:/a=b folder\" \r\nOTHER=last\r\n");
    require(getEnv("LOCAL_PATH",path)=="C:/a=b folder","Environment parser lost a non-final key or equals sign");
    rejects([&]{getEnv("MISSING",path);});
    rejects([&]{getEnv("LOCAL_PATH",fixture("empty.env","LOCAL_PATH=\n"));});
    rejects([]{getEnv("KEY","build/infrastructure-tests/nonexistent.env");});
}
void checkReproducibility() {
    auto sample=[] {
        std::vector<double> values;
        values.push_back(uniformUnit()); values.push_back(randomInteger(-3,99)); values.push_back(defaultBinomialDistribution(10));
        for(int id:generatePermutation(10)) values.push_back(id);
        for(int id:generateRandomRepeatedAlalleleChromosome(10,1,6)) values.push_back(id);
        for(auto row:buildChromosomes(problem().totalItems,2)) for(int id:row) values.push_back(id);
        return values;
    };
    seedRandomNumbers(123); const auto first=sample(); seedRandomNumbers(123); require(sample()==first,"RNG streams not reseeded together");
    seedRandomNumbers(124); require(sample()!=first,"Different seeds produced the same sample");
    require(instanceSeed(12,1,42)!=instanceSeed(12,2,42),"Instance seeds not separated");
    rejects([]{parseExperimentSeed("-1");}); rejects([]{parseExperimentSeed("12x");});
    for(const std::string algorithm:{"GA","ABC","FFA"}) {
        auto config=makeExperimentConfiguration(algorithm,ROTATION_WAY::SIX_WAY,MutationType::C2,true);
        config.iterations=2; config.individuals=6; config.sites=3; config.eliteSites=1;
        auto data=problem(); auto seed=instanceSeed(99,data.id,data.PSeed);
        seedRandomNumbers(seed); auto first=config.run(data);
        sample(); seedRandomNumbers(seed); auto second=config.run(data);
        require(first.size()==second.size(),"Repeated search changed population size");
        for(size_t i=0;i<first.size();++i)
            require(first[i].solution==second[i].solution && first[i].getFitness()==second[i].getFitness() && first[i].getGenome()==second[i].getGenome(),"Search is not reproducible");
    }
}
void checkCheckpointIdentity() {
    auto path=fixture("identity-data.txt","dataset one"); auto hash=datasetFingerprint(path);
    auto config=makeExperimentConfiguration("GA",ROTATION_WAY::ZERO_WAY,MutationType::C2,true);
    auto identity=config.identity(hash,123);
    const std::string fitness="build/infrastructure-tests/identity-fitness.csv",times="build/infrastructure-tests/identity-time.csv";
    saveExperimentResults({{0.5,7,ExperimentStatus::Complete}},fitness,times,identity);
    require(isExperimentComplete(loadExperimentResults(fitness,times,1,identity)[0]),"Bound checkpoint roundtrip failed");
    rejects([&]{loadExperimentResults(fitness,times,1,config.identity(hash,124));});
    config.iterations++; rejects([&]{loadExperimentResults(fitness,times,1,config.identity(hash,123));}); config.iterations--;
    fixture("identity-data.txt","dataset two"); require(datasetFingerprint(path)!=hash,"Dataset fingerprint ignored content");
    rejects([&]{loadExperimentResults(fitness,times,1,config.identity(datasetFingerprint(path),123));});
    saveExperimentResults({{0.5,7,ExperimentStatus::Complete}},fitness,times);
    rejects([&]{loadExperimentResults(fitness,times,1,identity);});
}
void checkCandidates() {
    const int sides[12][3]={{2,4,5},{4,1,3},{3,1,2},{6,7,6},{7,4,4},{2,1,1},{1,1,7},{2,3,5},{3,3,5},{4,5,5},{2,4,1},{2,1,1}};
    LoadedBins items; Chromosome ids;
    for(int i=0;i<12;++i) { ids.push_back(i+1); items.push_back(ItemBin::Build().setId(i+1).setLargo(sides[i][0]).setAncho(sides[i][1]).setAlto(sides[i][2]).setRotations(Punto::Build(1,1,1))); }
    for(int backend=0;backend<3;++backend) {
        std::vector<Bin> results;
        for(auto policy:{DBLFCandidatePolicy::ThreePoints,DBLFCandidatePolicy::Projected,DBLFCandidatePolicy::BestOfBoth}) {
            auto bin=Bin::Build().setLargo(10).setAncho(10).setAlto(10).setRotationWay(ROTATION_WAY::ZERO_WAY).setCandidatePolicy(policy);
            if(backend==0) DBLF(bin,ids,Chromosome(12,0),items);
            else if(backend==1) DBLFQueue(bin,ids,Chromosome(12,0),items);
            else DBLF(bin,items);
            results.push_back(bin);
        }
        require(results[2].getLoadedVolume()==std::max(results[0].getLoadedVolume(),results[1].getLoadedVolume()),"BestOfBoth did not retain the best packing");
        if(backend==0) require(results[0].getLoadedVolume()==364 && results[1].getLoadedVolume()==516,"Projected-face counterexample changed");
    }
    seedRandomNumbers(999);
    for(int trial=0;trial<100;++trial) {
        auto source=Bin::Build().setLargo(12).setAncho(12).setAlto(12).setRotationWay(ROTATION_WAY::SIX_WAY);
        LoadedBins items; Chromosome ids,rotations;
        for(int id=1;id<=20;++id) {
            items.push_back(ItemBin::Build().setId(id).setLargo(randomInteger(1,8)).setAncho(randomInteger(1,8)).setAlto(randomInteger(1,8)).setRotations(Punto::Build(1,1,1)));
            ids.push_back(id); rotations.push_back(randomInteger(1,6));
        }
        auto legacy=source,best=source; best.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);
        DBLF(legacy,ids,rotations,items); DBLF(best,ids,rotations,items);
        require(best.getLoadedVolume()>=legacy.getLoadedVolume(),"Enhanced policy regressed volume");
        for(const auto &item:best.getLoadedItems()) {
            require(canBePlaced(source,item.getPunto()+item.getCurrentDimension()) && !isOverlapped(item.getPunto(),item,source),"Enhanced packing is not geometrically valid");
            source.addItem(item);
        }
    }
}
int main() {
    std::ostringstream output; auto *previous=std::cout.rdbuf(output.rdbuf());
    try { checkIdentity(); checkGA(); checkEnv(); checkReproducibility(); checkCheckpointIdentity(); checkCandidates();
        std::cout.rdbuf(previous); std::cout<<"PASS: GA mutation, identities, env, reproducibility, checkpoints, projected-face fixture and 100 valid enhanced packings\n"; return 0; }
    catch(const std::exception &e) { std::cout.rdbuf(previous); std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1; }
}
