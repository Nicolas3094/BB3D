#include "../src/metaheuristics/ArtificialBeeColonyAlgorithm.hpp"
#include "../src/metaheuristics/FireflyAlgorithm.hpp"
#include "../src/binPacking/ItemBinOperators.hpp"
#include <limits>
#include <cmath>
#include <type_traits>

int mutationDraws = 0;
double unit = 0.5;
double uniformUnit() { return unit; }
int randomInteger(int low, int high) { ++mutationDraws; if (low > high) throw std::runtime_error("Bad random range"); return low; }
int defaultBinomialDistribution(int) { return 1; }
void require(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
template<class F> void rejects(F f) { bool caught=false; try { f(); } catch (const std::exception &) { caught=true; } require(caught,"Invalid configuration accepted"); }
struct ArtificialBeeColonyAlgorithmTestAccess {
    static void poison(ArtificialBeeColonyAlgorithm &a) { a.visitedIndividuals["previous-run"]=true; }
    static bool stale(ArtificialBeeColonyAlgorithm &a) { return a.visitedIndividuals.count("previous-run") != 0; }
};
struct FireflyAlgorithmTestAccess {
    using Attraction = void (FireflyAlgorithm::*)(const Poblacion &,int,int &,double &,double);
    static_assert(std::is_same<decltype(&FireflyAlgorithm::findMostAttractiveByIndexAndIntensity),Attraction>::value,
        "Attraction must borrow the population");
    static void poison(FireflyAlgorithm &a) { a.visitedIndividuals["previous-run"]=true; }
    static bool stale(FireflyAlgorithm &a) { return a.visitedIndividuals.count("previous-run") != 0; }
    static Poblacion directed(FireflyAlgorithm &a) { return a.searchD(); }
};
DatasetBinBacking problem()
{
    DatasetBinBacking data{};
    data.bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10);
    for (int i=1;i<=4;++i) data.totalItems.push_back(ItemBin::Build().setId(i).setTipo(i).setLargo(i).setAncho(1).setAlto(1).setRotations(Punto::Build(1,1,1)));
    return data;
}
template<class A> void probabilities(A &algorithm)
{
    algorithm.setProblem(problem()).setNumberOfIndividuals(4).setMaxIteration(1)
        .setIsWithReplacement(true).setRotationType(ROTATION_WAY::ZERO_WAY).setMutationType(MutationType::C2);
    for (double p : {0.0,1.0}) {
        mutationDraws=0; algorithm.setMutationProbabiliy(p); algorithm.evolve();
        require((mutationDraws>0) == (p==1),"Mutation probability endpoint ignored");
    }
    algorithm.setMutationProbabiliy(0.5);
    for (double sample : {0.25,0.75}) {
        unit=sample; mutationDraws=0; algorithm.evolve();
        require((mutationDraws>0)==(sample<0.5),"Mutation threshold ignored");
    }
    unit=0.5;
    for(double p : {-0.1,1.1,std::numeric_limits<double>::quiet_NaN()}) {
        algorithm.setMutationProbabiliy(p); rejects([&]{algorithm.evolve();});
    }
    algorithm.setMutationProbabiliy(1);
    algorithm.setDMutationProbability(-1); rejects([&]{algorithm.evolve();}); algorithm.setDMutationProbability(0);
    algorithm.setMaxIteration(-1); rejects([&]{algorithm.evolve();}); algorithm.setMaxIteration(0);
    algorithm.setNumberOfIndividuals(0); rejects([&]{algorithm.evolve();}); algorithm.setNumberOfIndividuals(4);
    auto empty=problem(); empty.totalItems.clear(); algorithm.setProblem(empty); rejects([&]{algorithm.evolve();});
}
void geometry()
{
    const int maximum=std::numeric_limits<int>::max();
    Punto end=Punto::Build(maximum,1,1)+Punto::Build(maximum,1,1);
    require(end.x==2LL*maximum,"Coordinate addition overflowed");
    require((Punto::Build(maximum,1,1)*2).x==2LL*maximum,"Coordinate multiplication overflowed");
    require((Punto::Build(-maximum,0,0)-Punto::Build(maximum,0,0)).x==-2LL*maximum,"Coordinate subtraction overflowed");
    auto data=problem(); data.bin.setLargo(maximum).setAncho(1).setAlto(1);
    require(!canBePlaced(data.bin,end),"Overflowing envelope accepted");
    require(!canBePlaced(data.bin,Punto::Build(-1,0,0)),"Negative envelope accepted");
    auto high=std::numeric_limits<std::int64_t>::max();
    auto low=std::numeric_limits<std::int64_t>::min();
    rejects([&]{ auto p=Punto::Build(high,0,0)+Punto::Build(1,0,0); (void)p; });
    rejects([&]{ auto p=Punto::Build(low,0,0)-Punto::Build(1,0,0); (void)p; });
    rejects([&]{ auto p=Punto::Build(low,0,0)*-1; (void)p; });
    rejects([&]{ auto p=Punto::Build(high,0,0)*2; (void)p; });
    rejects([&]{ auto p=Punto::Build(low,0,0)/-1; (void)p; });
    rejects([&]{ auto p=Punto::Build(1,0,0)/0; (void)p; });
    require((Punto::Build(-2,3,4)*-2).x==4,"Valid negative multiplication rejected");
    LoadedBins items=data.totalItems;
    items.resize(2);
    for(auto &item:items) item.setLargo(1500000000);
    data.bin.setRotationWay(ROTATION_WAY::ZERO_WAY);
    DBLF(data.bin,{1,2},{0,0},items);
    require(data.bin.getNumberOfLoadedItems()==1 && data.bin.getLoadedVolume()==1500000000LL,
        "Large-coordinate packing accepted an out-of-bounds second box");
}
int main()
{
    try {
        geometry();
        ArtificialBeeColonyAlgorithm abc;
        // All sites searched locally: scouting now correctly uses the shared RNG
        // and would otherwise contaminate the mutation draw counter.
        abc.setNumberOfSites(4).setNumberOfEliteSites(1).setNumberOfEliteBees(1).setNumberOfNonEliteBees(1);
        probabilities(abc);
        abc.setProblem(problem()).setNumberOfSites(5); rejects([&]{abc.evolve();});
        abc.setNumberOfSites(2).setNumberOfEliteSites(3); rejects([&]{abc.evolve();});
        FireflyAlgorithm firefly; firefly.setupIndex(1); probabilities(firefly);
        firefly.setProblem(problem()).setupIndex(-1); rejects([&]{firefly.evolve();});
        ArtificialBeeColonyAlgorithm defaultsABC; defaultsABC.setProblem(problem()).setMaxIteration(0);
        FireflyAlgorithm defaultsFFA; defaultsFFA.setProblem(problem()).setMaxIteration(0);
        require(!defaultsABC.evolve().empty() && !defaultsFFA.evolve().empty(),"Default search parameters unusable");
        ArtificialBeeColonyAlgorithmTestAccess::poison(defaultsABC);
        FireflyAlgorithmTestAccess::poison(defaultsFFA);
        defaultsABC.evolve(); defaultsFFA.evolve();
        require(!ArtificialBeeColonyAlgorithmTestAccess::stale(defaultsABC) && !FireflyAlgorithmTestAccess::stale(defaultsFFA),
            "Repeated evolve retained a previous run's history");
        FireflyAlgorithmTestAccess::poison(defaultsFFA); FireflyAlgorithmTestAccess::directed(defaultsFFA);
        require(!FireflyAlgorithmTestAccess::stale(defaultsFFA),"Directed search retained previous history");
        std::cout<<"PASS: wide coordinates, mutation endpoints, invalid configurations and initialized defaults\n";
        return 0;
    } catch(const std::exception &e) { std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1; }
}
