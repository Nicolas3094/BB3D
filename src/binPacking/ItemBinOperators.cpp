#include "ItemBinOperators.hpp"
#include <array>
#include <algorithm>

namespace {
std::vector<Punto> projectedCandidates(const Bin &bin,const Punto &point,const ItemBin &item) {
    std::vector<Punto> candidates;
    if(bin.getCandidatePolicy()!=DBLFCandidatePolicy::Projected) return candidates;
    const Punto end=point+item.getCurrentDimension();
    auto add=[&](Punto p) {
        if(!canBePlaced(bin,p) || p.x==bin.getDimensions().getLargo() || p.y==bin.getDimensions().getAncho() || p.z==bin.getDimensions().getAlto()) return;
        for(const auto &obstacle:bin.getLoadedItems()) {
            const auto start=obstacle.getPunto(), finish=start+obstacle.getCurrentDimension();
            if(p.x>=start.x && p.x<finish.x && p.y>=start.y && p.y<finish.y && p.z>=start.z && p.z<finish.z) return;
        }
        candidates.push_back(p);
    };
    auto project=[&](Punto p,int axis) {
        std::int64_t coordinates[3]={p.x,p.y,p.z};
        std::int64_t contact=0;
        for(const auto &obstacle:bin.getLoadedItems()) {
            const auto origin=obstacle.getPunto(),finish=origin+obstacle.getCurrentDimension();
            const std::int64_t low[3]={origin.x,origin.y,origin.z},high[3]={finish.x,finish.y,finish.z};
            if(high[axis]>coordinates[axis]) continue;
            bool intersects=true;
            for(int other=0;other<3;++other)
                if(other!=axis && (coordinates[other]<low[other] || coordinates[other]>=high[other])) intersects=false;
            if(intersects) contact=std::max(contact,high[axis]);
        }
        coordinates[axis]=contact;
        return Punto::Build(coordinates[0],coordinates[1],coordinates[2]);
    };
    // Project each new face corner in all six axis orders. At most 54
    // intermediate candidates per item, independent of the coordinate scale.
    for(auto start:{Punto::Build(end.x,point.y,point.z),Punto::Build(point.x,end.y,point.z),Punto::Build(point.x,point.y,end.z)}) {
        std::array<int,3> order{{0,1,2}};
        do { auto p=start; for(int axis:order) { p=project(p,axis); add(p); } }
        while(std::next_permutation(order.begin(),order.end()));
    }
    std::sort(candidates.begin(),candidates.end(),[](Punto a,Punto b){return a.x!=b.x?a.x<b.x:(a.y!=b.y?a.y<b.y:a.z<b.z);});
    candidates.erase(std::unique(candidates.begin(),candidates.end()),candidates.end());
    return candidates;
}
}

bool canBePlaced(const Bin &bin, const Punto &possiblePoint)
{
    return possiblePoint.x >= 0 && possiblePoint.y >= 0 && possiblePoint.z >= 0 &&
           possiblePoint.z <= bin.getDimensions().getAlto() &&
           possiblePoint.y <= bin.getDimensions().getAncho() &&
           possiblePoint.x <= bin.getDimensions().getLargo();
}

bool isABIntersect(Punto Amax, Punto Amin, Punto Bmax, Punto Bmin)
{
    if ((Amin.x >= Bmax.x || Amax.x <= Bmin.x) || (Amin.y >= Bmax.y || Amax.y <= Bmin.y) || (Amin.z >= Bmax.z || Amax.z <= Bmin.z))
    {
        return false;
    }
    return true;
}
// O(n)
bool isOverlapped(Punto position, const ItemBin &currentItem, const Bin &bin)
{
    const Punto envelope = position + currentItem.getCurrentDimension();
    for (const ItemBin &loadedItem : bin.getLoadedItems())
    {
        if (isABIntersect(
                /* Amax= */ envelope,
                /* Amin= */ position,
                /* Bmax= */ loadedItem.getPunto() + loadedItem.getCurrentDimension(),
                /* Bmin= */ loadedItem.getPunto()))
        {
            return true;
        }
    }

    return false;
}

// O(k) time and O(1) auxiliary space for k loaded items; independent of travel distance.
// Precondition: point is non-negative and the item does not overlap loaded items.
void iterateByDeepestBottomLeft(Punto &punto, const ItemBin &item, const Bin &bin)
{
    // Preserve the existing DBLF behavior for the first two items.
    if (bin.getNumberOfLoadedItems() <= 1)
    {
        return;
    }
    const Punto dimension = item.getCurrentDimension();
    std::int64_t position[3] = {punto.x, punto.y, punto.z};
    const std::int64_t extent[3] = {dimension.x, dimension.y, dimension.z};

    // Move toward each wall in Deepest (X), Bottom (Z), Left (Y) order.
    for (int axis : {0, 2, 1})
    {
        if (position[axis] == 0) continue;
        std::int64_t contact = 0; // The wall is the default stopping surface.
        for (const ItemBin &obstacle : bin.getLoadedItems())
        {
            const Punto origin = obstacle.getPunto();
            const Punto end = origin + obstacle.getCurrentDimension();
            const std::int64_t minimum[3] = {origin.x, origin.y, origin.z};
            const std::int64_t maximum[3] = {end.x, end.y, end.z};
            // Only obstacles behind the current position can block this movement.
            if (maximum[axis] > position[axis]) continue;
            bool projectionsOverlap = true;
            for (int other = 0; other < 3; ++other)
            {
                if (other == axis) continue;
                if (position[other] >= maximum[other] ||
                    (position[other] <= minimum[other] && extent[other] <= minimum[other] - position[other]))
                {
                    projectionsOverlap = false;
                    break;
                }
            }
            if (projectionsOverlap && maximum[axis] > contact)
            {
                contact = maximum[axis];
            }
        }
        position[axis] = contact;
    }
    punto = Punto::Build(position[0], position[1], position[2]);
}

void addItemToBin(ColaPuntosDBL &queue, Bin &bin, const Punto &punto, ItemBin item)
{
    bin.addItem(item.setPosicion(punto));
    Punto resultPoint = punto + item.getCurrentDimension();
    if (resultPoint.x < bin.getDimensions().getLargo())
    {
        Punto newPoint;
        newPoint = Punto::Build(resultPoint.x, punto.y, punto.z);
        queue.push(newPoint);
    }
    if (resultPoint.y < bin.getDimensions().getAncho())
    {
        Punto newPoint;
        newPoint = Punto::Build(punto.x, resultPoint.y, punto.z);
        queue.push(newPoint);
    }
    if (resultPoint.z < bin.getDimensions().getAlto())
    {
        Punto newPoint;
        newPoint = Punto::Build(punto.x, punto.y, resultPoint.z);
        queue.push(newPoint);
    }
    queue.update();
    for(const auto &candidate:projectedCandidates(bin,punto,item)) queue.push(candidate);
}

void addItemToBinQueue(PriorityPointQueue &queue, Bin &bin, const Punto &punto, ItemBin item)
{
    bin.addItem(item.setPosicion(punto));

    Punto resultPoint = punto + item.getCurrentDimension();

    if (resultPoint.x < bin.getDimensions().getLargo())
    {
        Punto newPoint;
        newPoint = Punto::Build(resultPoint.x, punto.y, punto.z);
        queue.enqueue(newPoint);
    }
    if (resultPoint.y < bin.getDimensions().getAncho())
    {
        Punto newPoint;
        newPoint = Punto::Build(punto.x, resultPoint.y, punto.z);
        queue.enqueue(newPoint);
    }
    if (resultPoint.z < bin.getDimensions().getAlto())
    {
        Punto newPoint;
        newPoint = Punto::Build(punto.x, punto.y, resultPoint.z);
        queue.enqueue(newPoint);
    }
    queue.update();
    for(const auto &candidate:projectedCandidates(bin,punto,item)) queue.enqueue(candidate);
}
