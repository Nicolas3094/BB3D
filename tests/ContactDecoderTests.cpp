#include "../src/heuristics/ContactDecoder.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/genetic/FitnessOperators.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "PackingAudit.hpp"
#include "../src/util/ExecutionBudget.hpp"
#include <random>
void require(bool value){if(!value)throw std::runtime_error("Contact decoder test failed");}
ItemBin box(int id,int x,int y,int z,Punto p=Punto::Build(0,0,0)) {
    return ItemBin::Build().setId(id).setTipo(id).setLargo(x).setAncho(y).setAlto(z).setRotations(Punto::Build(1,1,1)).setPosicion(p);
}
Bin container(int x=10,int y=10,int z=10){return Bin::Build().setLargo(x).setAncho(y).setAlto(z).setRotationWay(ROTATION_WAY::ZERO_WAY);}
long double unitFaceOracle(const Bin &bin,const ItemBin &item) {
    const auto p=item.getPunto(),d=item.getCurrentDimension();const auto bounds=bin.getDimensions();
    const long low[3]={static_cast<long>(p.x),static_cast<long>(p.y),static_cast<long>(p.z)},size[3]={static_cast<long>(d.x),static_cast<long>(d.y),static_cast<long>(d.z)},limit[3]={bounds.getLargo(),bounds.getAncho(),bounds.getAlto()};
    long double count=0;
    for(int axis=0;axis<3;++axis)for(int side=0;side<2;++side) {
        const int u=(axis+1)%3,v=(axis+2)%3;const auto plane=low[axis]+side*size[axis];
        for(long a=0;a<size[u];++a)for(long b=0;b<size[v];++b) {
            bool touching=plane==0 || plane==limit[axis];
            for(const auto &other:bin.getLoadedItems()) {
                auto q=other.getPunto(),e=other.getCurrentDimension();
                const std::int64_t origin[3]={q.x,q.y,q.z},extent[3]={e.x,e.y,e.z};
                if((side?origin[axis]:origin[axis]+extent[axis])==plane && low[u]+a>=origin[u] && low[u]+a<origin[u]+extent[u] && low[v]+b>=origin[v] && low[v]+b<origin[v]+extent[v])touching=true;
            }
            if(touching)++count;
        }
    }
    return count;
}
int main(){try {
    auto bin=container();auto item=box(1,2,3,4);
    require(placementContactArea(bin,item,Punto::Build(0,0,0))==26);
    require(placementContactArea(bin,item,Punto::Build(1,1,1))==0);
    bin.addItem(box(2,1,1,4,Punto::Build(0,0,0)));
    bin.addItem(box(3,1,2,4,Punto::Build(0,1,0)));
    // Adjacent disjoint faces cover 4+8, not twice a whole 3x4 face.
    require(placementContactArea(bin,item,Punto::Build(1,0,0))==26);
    auto empty=container();ColaPuntosDBL points;points.push(Punto::Build(5,5,5));points.push(Punto::Build(0,0,0));
    auto best=selectContactPlacement(empty,item,points);require(best.found && best.point==Punto::Build(0,0,0));
    ColaPuntosDBL later;later.push(Punto::Build(0,5,5));later.push(Punto::Build(5,0,0));
    best=selectContactPlacement(empty,box(1,1,1,1),later);require(best.point==Punto::Build(5,0,0)); // Better contact beats first feasible.
    ColaPuntosDBL ties;ties.push(Punto::Build(0,7,0));ties.push(Punto::Build(0,0,6));
    best=selectContactPlacement(empty,item,ties);require(best.point==Punto::Build(0,7,0)); // X then Z then Y.
    std::mt19937 rng(123);bool differs=false;
    for(int trial=0;trial<250;++trial) {
        LoadedBins input;Chromosome ids,genes;
        for(int i=1;i<=12;++i){input.push_back(box(i,1+rng()%5,1+rng()%5,1+rng()%5));ids.push_back(i);genes.push_back(0);}
        std::shuffle(ids.begin(),ids.end(),rng);
        auto legacy=container(),contact=container();contact.setDecoder(PackingDecoder::Contact);
        DBLF(legacy,ids,genes,input);DBLF(contact,ids,genes,input);
        auditPacking(legacy,input,ids,genes);auditPacking(contact,input,ids,genes);
        if(trial<25)for(const auto &placed:contact.getLoadedItems()) {
            auto others=container();for(const auto &other:contact.getLoadedItems())if(other.getId()!=placed.getId())others.addItem(other);
            require(placementContactArea(others,placed,placed.getPunto())==unitFaceOracle(others,placed));
        }
        auto repeat=container();repeat.setDecoder(PackingDecoder::Contact);DBLF(repeat,ids,genes,input);
        require(contact.getLoadedVolume()==repeat.getLoadedVolume());
        const auto &a=contact.getLoadedItems(),&b=repeat.getLoadedItems();require(a.size()==b.size());
        for(size_t i=0;i<a.size();++i)require(a[i].getPunto()==b[i].getPunto());
        differs|=contact.getLoadedVolume()!=legacy.getLoadedVolume();
    }
    require(differs);
    for(int cutoff:{10,30,80,200}) {
        LoadedBins input;Chromosome ids,genes;
        for(int id=1;id<=12;++id){input.push_back(box(id,2,2,2));ids.push_back(id);genes.push_back(0);}
        auto partial=container();partial.setDecoder(PackingDecoder::Contact);int ticks=0;
        {
            ExecutionBudget budget(std::chrono::seconds(30),Individuo::Build(),[&]{return ExecutionBudget::Clock::time_point{}+std::chrono::seconds(++ticks>cutoff?31:0);});
            try{DBLF(partial,ids,genes,input);}catch(const ExecutionTimeLimit &){}
        }
        auditPacking(partial,input,ids,genes);
    }
    auto bad=container();bad.addItem(box(1,2,2,2));bad.addItem(box(2,2,2,2));
    bool auditRejected=false;try{auditPacking(bad,{box(1,2,2,2),box(2,2,2,2)},{1,2},{0,0});}catch(const std::exception &){auditRejected=true;}require(auditRejected);
    auto denied=box(1,2,3,4);denied.setRotations(Punto::Build(0,0,0));auto invalid=container();invalid.setDecoder(PackingDecoder::Contact);
    DBLF(invalid,{1},{0},{denied});require(invalid.getLoadedVolume()==0);
    for(int mask=0;mask<8;++mask)for(auto way:{ROTATION_WAY::TWO_WAY,ROTATION_WAY::SIX_WAY})for(int gene=1;gene<=getIdFromRotationWay(way);++gene) {
        auto allowed=item;allowed.setRotations(Punto::Build(mask&1,(mask>>1)&1,(mask>>2)&1));
        const int vertical=way==ROTATION_WAY::TWO_WAY?2:((gene==1 || gene==3)?2:((gene==2 || gene==5)?1:0));
        auto rotated=container();rotated.setRotationWay(way).setDecoder(PackingDecoder::Contact);DBLF(rotated,{1},{gene},{allowed});auditPacking(rotated,{allowed},{1},{gene});
        require(rotated.getNumberOfLoadedItems()==((mask>>vertical)&1));
    }
    ColaPuntosDBL floating;floating.push(Punto::Build(0,5,5));require(selectContactPlacement(empty,box(1,1,1,1),floating).found);
    auto opt=parseCommandLine({"--decoder","contact"});require(opt.decoder==PackingDecoder::Contact);
    auto config=opt.configuration("GA",ROTATION_WAY::ZERO_WAY,MutationType::C2,true);
    auto baseline=config;baseline.decoder=PackingDecoder::DBLF;require(config.identity("hash",123)!=baseline.identity("hash",123));
    bool rejected=false;try{parseCommandLine({"--decoder","bad"});}catch(const std::exception &){rejected=true;}require(rejected);
    configureFitnessCache(8,1024*1024);auto individual=Individuo::Build().setGenome(DoubleGenome::Build().setGenome({1}).setDGenome({0}));
    auto c=container();evaluateFitness(individual,{item},c);c.setDecoder(PackingDecoder::Contact);evaluateFitness(individual,{item},c);require(fitnessCacheStats().contextChanges==2);
    std::cout<<"PASS: exact contact areas, split surfaces, DBL ties, 250 paired packings, deterministic placement, permissions, cache and identity isolation\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}}
