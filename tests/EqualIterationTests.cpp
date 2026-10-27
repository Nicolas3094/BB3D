#include "../src/readData/CommandLine.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/util/RandomNumbers.hpp"
#include <stdexcept>
void require(bool value){if(!value)throw std::runtime_error("Equal iteration test failed");}
int main(){try {
    DatasetBinBacking data{};data.bin=Bin::Build().setLargo(50).setAncho(50).setAlto(50);
    for(int id=1;id<=8;++id)data.totalItems.push_back(ItemBin::Build().setId(id).setTipo(id).setLargo(id).setAncho(2).setAlto(1).setRotations(Punto::Build(1,1,1)));
    for(int iterations:{0,1,3})for(std::uint64_t seed:{123,456,789})for(const std::string algorithm:{"GA","ABC","FFA"}) {
        auto options=parseCommandLine({"--ga-matings","20","--crossover","1","--replacement","false","--population","20","--decoder","contact","--iterations",std::to_string(iterations),"--mutation-probability","1","--rotation-mutation","0.05","--patience","0","--sites","10","--elite-sites","5","--elite-bees","4","--non-elite-bees","2","--movements","2"});
        auto config=options.configuration(algorithm,ROTATION_WAY::SIX_WAY,MutationType::C2,false);
        seedRandomNumbers(seed);auto result=config.execute(data);
        require(fitnessCacheStats().misses==static_cast<std::size_t>(20+40*iterations));
        require(result.population.size()==20 && !result.timeLimitReached);
    }
    auto standard=CommandLineOptions().configuration("GA",ROTATION_WAY::SIX_WAY,MutationType::C2,false);
    require(standard.gaMatings==-1);
    auto fixed=standard;fixed.gaMatings=20;require(fixed.identity("hash",123)!=standard.identity("hash",123));
    bool rejected=false;try{auto invalid=parseCommandLine({"--ga-matings","20"});invalid.configuration("GA",ROTATION_WAY::SIX_WAY,MutationType::C2,true);}catch(const std::exception &){rejected=true;}require(rejected);
    std::cout<<"PASS: GA/ABC/FFA: exactly 20 initial + 40 per complete iteration, N=0/1/3, three seeds, original default preserved\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}}
