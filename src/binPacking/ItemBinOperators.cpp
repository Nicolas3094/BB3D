#include "ItemBinOperators.hpp"

bool canBePlaced(Bin bin, const Punto &possiblePoint)
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
bool isOverlapped(Punto position, ItemBin currentItem, Bin bin)
{
    for (ItemBin loadedItem : bin.getLoadedItems())
    {
        if (isABIntersect(
                /* Amax= */ position + currentItem.getCurrentDimension(),
                /* Amin= */ position,
                /* Bmax= */ loadedItem.getPunto() + loadedItem.getCurrentDimension(),
                /* Bmin= */ loadedItem.getPunto()))
        {
            return true;
        }
    }

    return false;
}

// Min O(1), Max O(N**2), Prom O(N)
void iterateByDeepestBottomLeft(Punto &punto, ItemBin item, Bin bin)
{
    if (bin.getNumberOfLoadedItems() <= 1)
    {
        return;
    }
    int count = 0;
    // Go first by Deepest - X
    if (punto.x != 0)
    {
        while (!isOverlapped(punto, item, bin))
        {
            count += 1;
            punto.x -= 1;
            if (punto.x == -1)
            {
                break;
            }
        }
        punto.x += 1;
    }
    // Go Second by Bottom - Z
    if (punto.z != 0)
    {
        while (!isOverlapped(punto, item, bin))
        {
            count += 1;
            punto.z -= 1;
            if (punto.z == -1)
            {
                break;
            }
        }
        punto.z += 1;
    }

    // Go Thirsd by With - Y
    if (punto.y != 0)
    {
        while (!isOverlapped(punto, item, bin))
        {
            count += 1;
            punto.y -= 1;
            if (punto.y == -1)
            {
                break;
            }
        }
        punto.y += 1;
    }
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