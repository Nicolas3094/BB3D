#ifndef INDEPENDENT_PACKING_AUDIT_HPP
#define INDEPENDENT_PACKING_AUDIT_HPP
#include "../src/binPacking/Bin.hpp"
#include <array>
#include <unordered_set>
#include <stdexcept>
// Independent from the decoder's rotation, collision and feasibility helpers.
inline void auditPacking(const Bin &packed,const LoadedBins &source,const Chromosome &ids,const Chromosome &genes) {
    auto demand=[&](bool ok){if(!ok)throw std::runtime_error("Independent packing audit failed");};
    std::unordered_set<int> seen;std::int64_t volume=0;
    const auto container=packed.getDimensions();
    const std::int64_t limits[3]={container.getLargo(),container.getAncho(),container.getAlto()};
    const int permutations[7][3]={{0,1,2},{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    for(const auto &box:packed.getLoadedItems()) {
        demand(box.getId()>0 && static_cast<size_t>(box.getId())<=source.size() && seen.insert(box.getId()).second);
        const auto &original=source[box.getId()-1];const auto dim=original.getDimension();
        const std::int64_t sides[3]={dim.getLargo(),dim.getAncho(),dim.getAlto()};
        auto it=std::find(ids.begin(),ids.end(),box.getId());demand(it!=ids.end());
        const auto rotation=genes[it-ids.begin()];demand(box.getRotationId()==rotation && rotation>=0 && rotation<=6);
        const int twoWay[3]={1,0,2};const int *order=packed.getRotationWay()==ROTATION_WAY::TWO_WAY && rotation==2?twoWay:permutations[rotation];
        const auto allow=original.getRotations();const std::int64_t flags[3]={allow.x,allow.y,allow.z};demand(flags[order[2]]==1);
        const auto p=box.getPunto(),d=box.getCurrentDimension();
        const std::int64_t low[3]={p.x,p.y,p.z},length[3]={d.x,d.y,d.z};
        for(int a=0;a<3;++a)demand(length[a]==sides[order[a]] && low[a]>=0 && length[a]>0 && low[a]+length[a]<=limits[a]);
        volume+=length[0]*length[1]*length[2];
    }
    const auto &boxes=packed.getLoadedItems();
    for(size_t i=0;i<boxes.size();++i)for(size_t j=0;j<i;++j) {
        const auto p=boxes[i].getPunto(),d=boxes[i].getCurrentDimension(),q=boxes[j].getPunto(),e=boxes[j].getCurrentDimension();
        const bool separate=p.x+d.x<=q.x || q.x+e.x<=p.x || p.y+d.y<=q.y || q.y+e.y<=p.y || p.z+d.z<=q.z || q.z+e.z<=p.z;
        demand(separate);
    }
    demand(volume==packed.getLoadedVolume());
}
#endif
