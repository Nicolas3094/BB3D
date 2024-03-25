#include "DBLF.hpp"
/** Deepest Bottom Left Fill with PriorityQueue
 *
 * Currently, this has a time complexity of O(n**4) mean
 */
void DBLFQueue(
    Bin &bin,
    Chromosome itemsIdToPack,
    Chromosome itemsRotation,
    LoadedBins itemDataOrderById)
{
    int countAllele = 0;
    PriorityPointQueue queue;
    Node<Punto> *rear;
    // First add the origin
    queue.enqueue(Punto::Build(0, 0, 0));

    for (int boxId : itemsIdToPack) // O(n)
    {
        // Rotate box accordingly with the rotation codification
        itemDataOrderById[boxId - 1].rotate(bin.getRotationWay(), getRotationFromId(itemsRotation[countAllele++]));

        rear = queue.getRear();
        while (rear != null) // O(n)
        {
            Punto currentPoint = rear->data;
            Punto dimensionPoint = itemDataOrderById[boxId - 1].getCurrentDimension();

            if (canBePlaced(bin, currentPoint + dimensionPoint)) // O(1)
            {
                if (!isOverlapped(currentPoint, itemDataOrderById[boxId - 1], bin)) // O(n)
                {
                    queue.removeNode(rear);

                    iterateByDeepestBottomLeft(currentPoint, itemDataOrderById[boxId - 1], bin); // Pron: O(N)

                    addItemToBinQueue(queue, bin, currentPoint, itemDataOrderById[boxId - 1]);

                    break;
                }
            }
            rear = rear->next;
        }
    }

    rear = null;
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
        itemDataOrderById[boxId - 1].rotate(bin.getRotationWay(), rotation);
        for (int j = queue.size() - 1; j >= 0; j--)
        {
            Punto currenPoint = queue[j];
            Punto dimensionPoint = itemDataOrderById[boxId - 1].getCurrentDimension();

            if (canBePlaced(bin, currenPoint + dimensionPoint))
            {
                if (!isOverlapped(currenPoint, itemDataOrderById[boxId - 1], bin))
                {
                    queue.remove(j);
                    iterateByDeepestBottomLeft(currenPoint, itemDataOrderById[boxId - 1], bin);
                    addItemToBin(queue, bin, currenPoint, itemDataOrderById[boxId - 1]);
                    break;
                }
            }
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