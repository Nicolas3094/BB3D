#include "../src/heuristics/DBLF.hpp"
#include "../src/readData/BRInstanceReader.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/util/RandomNumbers.hpp"
#include "../tests/PackingAudit.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <random>
#include <sstream>
using Clock=std::chrono::steady_clock;
void saveGenome(const std::string &path,const Chromosome &ids,const Chromosome &genes) {
    std::ofstream out(path);out<<"position,id,rotation\n";
    for(size_t i=0;i<ids.size();++i)out<<i<<','<<ids[i]<<','<<genes[i]<<'\n';
    if(!out)throw std::runtime_error("Cannot save genome");
}
void savePacking(const std::string &path,const Bin &bin,const LoadedBins &input) {
    std::ofstream out(path);out<<"id,x,y,z,dx,dy,dz,rotation,original_x,original_y,original_z,allow_x,allow_y,allow_z\n";
    for(const auto &box:bin.getLoadedItems()) {
        const auto p=box.getPunto(),d=box.getCurrentDimension();const auto &original=input[box.getId()-1];const auto o=original.getDimension();const auto allow=original.getRotations();
        out<<box.getId()<<','<<p.x<<','<<p.y<<','<<p.z<<','<<d.x<<','<<d.y<<','<<d.z<<','<<box.getRotationId()<<','<<o.getLargo()<<','<<o.getAncho()<<','<<o.getAlto()<<','<<allow.x<<','<<allow.y<<','<<allow.z<<'\n';
    }
    if(!out)throw std::runtime_error("Cannot save packing");
}
int main(int argc,char **argv){try {
    if(argc<3)throw std::invalid_argument("Usage: decoder_comparison OUTPUT_DIR PROJECT_ROOT [fixed|search|all]");
    const std::string output=argv[1],root=argv[2],mode=argc>3?argv[3]:"all";
    std::ofstream fixed,search,parameters;
    if(mode!="search"){fixed.open(output+"/evaluations.csv");fixed<<std::setprecision(17)<<"dataset,instance,seed,genome,decoder,occupation_percent,microseconds,boxes,valid,packing\n";}
    if(mode!="fixed"){search.open(output+"/search.csv");search<<std::setprecision(17)<<"dataset,instance,seed,algorithm,decoder,occupation_percent,milliseconds,evaluations,termination,partial,valid,packing\n";}
    parameters.open(output+"/parameters-"+mode+".txt");parameters<<"Compiler="<<__VERSION__<<"; -O3 -DNDEBUG; C++14; 1 search thread; wall-clock 30 seconds; cache disabled\n";
    parameters<<"First instance BR0 and BR15; seeds 123 and 456; rotation SIX_WAY obeying item permissions; legacy three-point generation for both decoders.\n";
    for(const std::string dataset:{"BR0","BR15"}) {
        const auto data=BRInstanceReader().read(root+"/Instance/BR/"+dataset+".txt").front();
        for(std::uint64_t seed:{123,456}) {
            if(mode!="search") {
                std::mt19937_64 rng(seed);Chromosome ids(data.totalItems.size());std::iota(ids.begin(),ids.end(),1);
                for(int genome=0;genome<16;++genome) {
                    if(genome==0)std::iota(ids.begin(),ids.end(),1);
                    else if(genome<=2)std::stable_sort(ids.begin(),ids.end(),[&](int a,int b){return genome==1?data.totalItems[a-1].getVolumen()>data.totalItems[b-1].getVolumen():data.totalItems[a-1].getVolumen()<data.totalItems[b-1].getVolumen();});
                    else std::shuffle(ids.begin(),ids.end(),rng);
                    Chromosome genes;
                    for(int id:ids) {
                        const auto allow=data.totalItems[id-1].getRotations();std::vector<int> permitted;
                        if(allow.z){permitted.push_back(1);permitted.push_back(3);}if(allow.y){permitted.push_back(2);permitted.push_back(5);}if(allow.x){permitted.push_back(4);permitted.push_back(6);}
                        if(permitted.empty())throw std::runtime_error("No permitted orientation");
                        genes.push_back(permitted[rng()%permitted.size()]);
                    }
                    const auto label=dataset+"-s"+std::to_string(seed)+"-g"+std::to_string(genome);
                    saveGenome(output+"/"+label+"-genome.csv",ids,genes);
                    // Alternate measured order; both receive exactly the same input.
                    for(int order=0;order<2;++order) {
                        const bool contact=(order+genome)%2;const std::string decoder=contact?"contact":"dblf";
                        auto bin=data.bin;bin.setRotationWay(ROTATION_WAY::SIX_WAY).setCandidatePolicy(DBLFCandidatePolicy::ThreePoints).setDecoder(contact?PackingDecoder::Contact:PackingDecoder::DBLF);
                        const auto start=Clock::now();DBLF(bin,ids,genes,data.totalItems);
                        const auto us=std::chrono::duration<double,std::micro>(Clock::now()-start).count();
                        auditPacking(bin,data.totalItems,ids,genes);
                        const auto file=label+"-"+decoder+"-packing.csv";savePacking(output+"/"+file,bin,data.totalItems);
                        fixed<<dataset<<",1,"<<seed<<','<<genome<<','<<decoder<<','<<100.0*bin.getLoadedVolume()/bin.getDimensions().getVolumen()<<','<<us<<','<<bin.getNumberOfLoadedItems()<<",1,"<<file<<std::endl;
                    }
                }
                std::cout<<"Fixed genomes validated: "<<dataset<<" seed="<<seed<<std::endl;
            }
            if(mode!="fixed")for(const std::string algorithm:{"GA","ABC","FFA"})for(int order=0;order<2;++order) {
                const bool contact=(order+(seed==456))%2;const std::string decoder=contact?"contact":"dblf";
                CommandLineOptions options;options.population=20;options.execution=ExecutionMode::ThirtySeconds;
                options.decoder=contact?PackingDecoder::Contact:PackingDecoder::DBLF;
                auto config=options.configuration(algorithm,ROTATION_WAY::SIX_WAY,MutationType::C2,true);
                const auto actualSeed=instanceSeed(seed,data.id,data.PSeed);seedRandomNumbers(actualSeed);
                parameters<<dataset<<' '<<seed<<' '<<algorithm<<' '<<decoder<<' '<<config.identity("BR-source",actualSeed)<<std::endl;
                const auto result=config.execute(data);const auto &best=result.population.front();const auto count=fitnessCacheStats().misses;
                // Replay the recorded insertion sequence (including interrupted valid packings).
                auto code=best.solution;std::replace(code.begin(),code.end(),'|',' ');std::istringstream parse(code);Chromosome packedIds,packedGenes;int id,gene;
                while(parse>>id>>gene){packedIds.push_back(id);packedGenes.push_back(gene);}
                auto bin=data.bin;bin.setRotationWay(config.rotation).setDecoder(config.decoder).setCandidatePolicy(config.candidates);
                DBLF(bin,packedIds,packedGenes,data.totalItems);
                auditPacking(bin,data.totalItems,best.getGenome().getGenome(),best.getGenome().getDGenome());
                if(static_cast<double>(bin.getLoadedVolume())/bin.getDimensions().getVolumen()!=best.getFitness())throw std::runtime_error("Incumbent replay differs from timed result");
                const auto label=dataset+"-s"+std::to_string(seed)+"-"+algorithm+"-"+decoder;
                saveGenome(output+"/"+label+"-genome.csv",best.getGenome().getGenome(),best.getGenome().getDGenome());
                saveGenome(output+"/"+label+"-insertion.csv",packedIds,packedGenes);
                const auto file=label+"-packing.csv";savePacking(output+"/"+file,bin,data.totalItems);
                search<<dataset<<",1,"<<seed<<','<<algorithm<<','<<decoder<<','<<100*best.getFitness()<<','<<result.milliseconds<<','<<count<<','<<result.termination<<','<<result.partialEvaluation<<",1,"<<file<<std::endl;
                std::cout<<dataset<<" seed="<<seed<<' '<<algorithm<<' '<<decoder<<" occupation="<<100*best.getFitness()<<"% evaluations="<<count<<std::endl;
            }
        }
    }
    return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<std::endl;return 1;}}
