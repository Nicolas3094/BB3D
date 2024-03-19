#include "DBLF.hpp"

int BOX_ID = 6;
void DBLFQueue(
    Bin &bin,
    Chromosome itemsIdToPack,
    Chromosome itemsRotation,
    LoadedBins itemDataOrderById)
{
    PriorityPointQueue queue;
    Node<Punto> *rear, *tmp;
    queue.enqueue(Punto::Build(0, 0, 0));
    int countAllele = 0;
    for (int boxId : itemsIdToPack)
    {
        ROTATION_MODE rotation = getRotationFromId(itemsRotation[countAllele++]);
        itemDataOrderById[boxId - 1].rotate(bin.getRotationWay(), rotation);
        rear = queue.getRear();
        // std::cout << "\nboxId: " << boxId << "\n";
        while (rear != null)
        {
            Punto currentPoint = rear->data;
            Punto dimensionPoint = itemDataOrderById[boxId - 1].getCurrentDimension();
            /* std::cout << "dim: " << dimensionPoint << "\n";
            std::cout << "point: " << currentPoint << "\n";*/
            if (canBePlaced(bin, currentPoint + dimensionPoint))
            {
                if (!isOverlapped(currentPoint, itemDataOrderById[boxId - 1], bin))
                {
                    queue.removeNode(rear);
                    iterateByDeepestBottomLeft(currentPoint, itemDataOrderById[boxId - 1], bin);
                    addItemToBinQueue(queue, bin, currentPoint, itemDataOrderById[boxId - 1]);
                    /*std::cout << "new points added\n";
                    std::cout << queue << "\n";
                    if (boxId == BOX_ID)
                    {
                        std::cout << queue << "\n";
                        return;
                    }*/
                    break;
                }
            }
            rear = rear->next;
        }
    }
    // std::cout << queue << "\n";
    rear = null;
    if (tmp != null)
    {
        tmp = null;
        delete tmp;
    }
    delete rear;
}

void DBLF(
    Bin &bin,
    Chromosome itemsIdToPack,
    Chromosome itemsRotation,
    LoadedBins itemDataOrderById,
    bool print)
{
    ColaPuntosDBL queue = ColaPuntosDBL::Build();
    queue.push(Punto::Build(0, 0, 0));
    int countAllele = 0;
    for (int boxId : itemsIdToPack)
    {
        ROTATION_MODE rotation = getRotationFromId(itemsRotation[countAllele++]);
        // Item to pack
        itemDataOrderById[boxId - 1].rotate(bin.getRotationWay(), rotation);
        // std::cout << "\nboxId: " << boxId << "\n";
        for (int j = queue.size() - 1; j >= 0; j--)
        {
            Punto currenPoint = queue[j];
            Punto dimensionPoint = itemDataOrderById[boxId - 1].getCurrentDimension();
            // std::cout << "dim: " << dimensionPoint << "\n";
            // std::cout << "point: " << currenPoint << "\n";
            if (canBePlaced(bin, currenPoint + dimensionPoint))
            {
                if (!isOverlapped(currenPoint, itemDataOrderById[boxId - 1], bin))
                {
                    queue.remove(j);
                    iterateByDeepestBottomLeft(currenPoint, itemDataOrderById[boxId - 1], bin);
                    addItemToBin(queue, bin, currenPoint, itemDataOrderById[boxId - 1]);
                    // std::cout << "new points added\n";
                    /*if (boxId == BOX_ID)
                    {
                        std::cout << queue << "\n";
                        return;
                    }*/
                    break;
                }
            }
        }
    }
    // std::cout << queue << "\n";
}

void DBLF(
    Bin &bin,
    LoadedBins itemsToPack)
{
    ROTATION_WAY rotationWay = bin.getRotationWay();
    ColaPuntosDBL queue = ColaPuntosDBL::Build();
    queue.push(Punto::Build(0, 0, 0));

    for (ItemBin &item : itemsToPack)
    {
        std::cout << "Box: " << item.getId() << "\n";
        std::cout << "Available points: " << queue;
        for (int j = queue.size() - 1; j >= 0; j--)
        {
            Punto currenPoint = queue[j];
            Punto dimensionPoint = item.getCurrentDimension();
            std::cout << "Prueba con punto: " << currenPoint << "\n";
            std::cout << "Dimension point: " << dimensionPoint << "\n";
            std::cout << "Envelope: " << currenPoint + dimensionPoint << "\n";

            if (canBePlaced(bin, currenPoint + dimensionPoint))
            {
                std::cout << "Se puede poner."
                          << "\n";
                bool isOverlap = isOverlapped(currenPoint, item, bin);
                if (!isOverlap)
                {
                    queue.remove(j);
                    iterateByDeepestBottomLeft(currenPoint, item, bin);
                    addItemToBin(queue, bin, currenPoint, item);
                    std::cout << "Se inserto " << item.getId() << "\n";
                    break;
                }
                else
                {
                    std::cout << "Traslapa con caja."
                              << "\n\n";
                }
            }
            else
            {
                std::cout << "No entra en contenedor."
                          << "\n";
            }
        }
    }
}