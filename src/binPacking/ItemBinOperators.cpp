#include "ItemBinOperators.hpp"

bool canBePlaced(const Bin &bin, const Punto &possiblePoint)
{
    return possiblePoint.z <= bin.getDimensions().getAlto() &&
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
    int position[3] = {punto.x, punto.y, punto.z};
    const int extent[3] = {dimension.x, dimension.y, dimension.z};

    // Move toward each wall in Deepest (X), Bottom (Z), Left (Y) order.
    for (int axis : {0, 2, 1})
    {
        if (position[axis] == 0) continue;
        int contact = 0; // The wall is the default stopping surface.
        for (const ItemBin &obstacle : bin.getLoadedItems())
        {
            const Punto origin = obstacle.getPunto();
            const Punto end = origin + obstacle.getCurrentDimension();
            const int minimum[3] = {origin.x, origin.y, origin.z};
            const int maximum[3] = {end.x, end.y, end.z};
            // Only obstacles behind the current position can block this movement.
            if (maximum[axis] > position[axis]) continue;
            bool projectionsOverlap = true;
            for (int other = 0; other < 3; ++other)
            {
                if (other == axis) continue;
                if (position[other] >= maximum[other] ||
                    position[other] + extent[other] <= minimum[other])
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
}
