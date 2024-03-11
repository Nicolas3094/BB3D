#include "DBLF.hpp"

void DBLF(
    Bin &bin,
    Chromosome itemsIdToPack,
    Chromosome itemsRotation,
    LoadedBins itemDataOrderById,
    bool print)
{
    // std::cout << "genome: " << itemsIdToPack << "\n";
    ColaPuntosDBL queue = ColaPuntosDBL::Build();
    queue.push(Punto::Build(0, 0, 0));
    int countAllele = 0;
    for (int boxId : itemsIdToPack)
    {
        // std::cout << "Box: " << boxId << "\n";
        ROTATION_MODE rotation = getRotationFromId(itemsRotation[countAllele++]);
        // Item to pack
        itemDataOrderById[boxId - 1].rotate(bin.getRotationWay(), rotation);
        // std::cout << "Available points: " << queue;
        for (int j = queue.size() - 1; j >= 0; j--)
        {
            Punto currenPoint = queue[j];
            Punto dimensionPoint = itemDataOrderById[boxId - 1].getCurrentDimension();
            // std::cout << "Prueba con punto: " << currenPoint << "\n";
            // std::cout << "Dimension point: " << dimensionPoint << "\n";
            // std::cout << "Envelope: " << currenPoint + dimensionPoint << "\n";
            if (canBePlaced(bin, currenPoint + dimensionPoint))
            {
                // std::cout << "Se puede poner." << "\n";
                bool isOverlap = isOverlapped(currenPoint, itemDataOrderById[boxId - 1], bin);
                if (!isOverlap)
                {
                    queue.remove(j);
                    iterateByDeepestBottomLeft(currenPoint, itemDataOrderById[boxId - 1], bin);
                    addItemToBin(queue, bin, currenPoint, itemDataOrderById[boxId - 1]);
                    // std::cout << "Se inserto " << boxId << "\n\n";
                    break;
                }
                /*else
                {
                    std::cout << "Traslapa con caja."
                              << "\n\n";
                }*/
            }
            /*else
            {
                std::cout << "No entra en contenedor."
                          << "\n";
            }*/
        }
    }
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