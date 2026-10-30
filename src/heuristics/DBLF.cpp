#include "DBLF.hpp"
#include "DBLFValidation.hpp"
#include "ContactDecoder.hpp"
#include "MaximalSpacesDecoder.hpp"
#include "../util/ExecutionCheckpoint.hpp"

/** Deepest Bottom Left Fill with PriorityQueue
 *
 * ThreePoints: worst-case O(n^3) for n input items and an initially empty bin:
 * O(n) candidate points per item, O(n) overlap checks per candidate,
 * and O(n) contact compaction after a feasible candidate is found.
 * The vector-backed DBLF below has the same bound (including queue updates).
 * The previous unit-step compaction added O(n^2 * D), where D bounds travel.
 * Projected adds at most 54 contact projections per placed item: O(n)
 * candidates, O(n^3) worst-case time and O(n) candidate space, with larger
 * constants. BestOfBoth runs both policies and retains
 * the greater loaded volume, preferring ThreePoints on ties.
 */
void DBLFQueue(Bin &bin, Chromosome itemsIdToPack, Chromosome itemsRotation, LoadedBins itemDataOrderById)
{
    validateDBLFInput(bin, itemsIdToPack, itemsRotation, itemDataOrderById);

    if (bin.getDecoder() == PackingDecoder::MaximalSpaces)
    {
        maximalSpacesDecode(bin, itemsIdToPack, itemsRotation, itemDataOrderById);

        return;
    }
    if (bin.getDecoder() == PackingDecoder::Contact)
    {
        contactDecode(bin, itemsIdToPack, itemsRotation, std::move(itemDataOrderById));

        return;
    }
    if (bin.getCandidatePolicy() == DBLFCandidatePolicy::BestOfBoth)
    {
        Bin legacy = bin, projected = bin;
        legacy.setCandidatePolicy(DBLFCandidatePolicy::ThreePoints);
        projected.setCandidatePolicy(DBLFCandidatePolicy::Projected);
        try
        {
            DBLFQueue(legacy, itemsIdToPack, itemsRotation, itemDataOrderById);
            DBLFQueue(projected, itemsIdToPack, itemsRotation, itemDataOrderById);
        }
        catch (const ExecutionTimeLimit &)
        {
            bin = projected.getLoadedVolume() > legacy.getLoadedVolume() ? std::move(projected)
                                                                         : std::move(legacy);
            bin.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);
            throw;
        }
        bin =
            projected.getLoadedVolume() > legacy.getLoadedVolume() ? std::move(projected) : std::move(legacy);
        bin.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);

        return;
    }
    int countAllele = 0;
    PriorityPointQueue queue;
    Node<Punto> *rear;
    // First add the origin
    queue.enqueue(Punto::Build(0, 0, 0));

    for (int boxId : itemsIdToPack) // O(n)
    {
        checkExecutionDeadline();
        // Rotate box accordingly with the rotation codification
        itemDataOrderById[boxId - 1].rotate(bin.getRotationWay(),
                                            getRotationFromId(itemsRotation[countAllele++]));
        if (!itemDataOrderById[boxId - 1].isValidOrientation())
        {
            continue;
        }

        rear = queue.getRear();

        while (rear != null) // O(n)
        {
            checkExecutionDeadline();
            Punto currentPoint = rear->data;
            Punto dimensionPoint = itemDataOrderById[boxId - 1].getCurrentDimension();

            if (canBePlaced(bin, currentPoint + dimensionPoint)) // O(1)
            {
                if (!isOverlapped(currentPoint, itemDataOrderById[boxId - 1], bin)) // O(n)
                {
                    queue.removeNode(rear);

                    iterateByDeepestBottomLeft(currentPoint, itemDataOrderById[boxId - 1], bin); // O(n)

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

void DBLF(Bin &bin, Chromosome itemsIdToPack, Chromosome itemsRotation, LoadedBins itemDataOrderById,
          bool print)
{
    validateDBLFInput(bin, itemsIdToPack, itemsRotation, itemDataOrderById);

    if (bin.getDecoder() == PackingDecoder::MaximalSpaces)
    {
        maximalSpacesDecode(bin, itemsIdToPack, itemsRotation, itemDataOrderById);

        return;
    }
    if (bin.getDecoder() == PackingDecoder::Contact)
    {
        contactDecode(bin, itemsIdToPack, itemsRotation, std::move(itemDataOrderById));

        return;
    }
    if (bin.getCandidatePolicy() == DBLFCandidatePolicy::BestOfBoth)
    {
        Bin legacy = bin, projected = bin;
        legacy.setCandidatePolicy(DBLFCandidatePolicy::ThreePoints);
        projected.setCandidatePolicy(DBLFCandidatePolicy::Projected);
        try
        {
            DBLF(legacy, itemsIdToPack, itemsRotation, itemDataOrderById, print);
            DBLF(projected, itemsIdToPack, itemsRotation, itemDataOrderById, print);
        }
        catch (const ExecutionTimeLimit &)
        {
            bin = projected.getLoadedVolume() > legacy.getLoadedVolume() ? std::move(projected)
                                                                         : std::move(legacy);
            bin.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);
            throw;
        }
        bin =
            projected.getLoadedVolume() > legacy.getLoadedVolume() ? std::move(projected) : std::move(legacy);
        bin.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);

        return;
    }
    ColaPuntosDBL queue = ColaPuntosDBL::Build();
    queue.push(Punto::Build(0, 0, 0));
    int countAllele = 0;

    for (int boxId : itemsIdToPack)
    {
        checkExecutionDeadline();
        ROTATION_MODE rotation = getRotationFromId(itemsRotation[countAllele++]);
        itemDataOrderById[boxId - 1].rotate(bin.getRotationWay(), rotation);

        if (!itemDataOrderById[boxId - 1].isValidOrientation())
        {
            continue;
        }
        for (int j = queue.size() - 1; j >= 0; j--)
        {
            checkExecutionDeadline();
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

void DBLF(Bin &bin, LoadedBins itemsToPack)
{
    validateDBLFInput(bin, itemsToPack);

    if (bin.getDecoder() == PackingDecoder::MaximalSpaces)
    {
        maximalSpacesDecode(bin, std::move(itemsToPack));

        return;
    }
    if (bin.getDecoder() == PackingDecoder::Contact)
    {
        contactDecode(bin, std::move(itemsToPack));

        return;
    }
    if (bin.getCandidatePolicy() == DBLFCandidatePolicy::BestOfBoth)
    {
        Bin legacy = bin, projected = bin;
        legacy.setCandidatePolicy(DBLFCandidatePolicy::ThreePoints);
        projected.setCandidatePolicy(DBLFCandidatePolicy::Projected);
        try
        {
            DBLF(legacy, itemsToPack);
            DBLF(projected, itemsToPack);
        }
        catch (const ExecutionTimeLimit &)
        {
            bin = projected.getLoadedVolume() > legacy.getLoadedVolume() ? std::move(projected)
                                                                         : std::move(legacy);
            bin.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);
            throw;
        }
        bin =
            projected.getLoadedVolume() > legacy.getLoadedVolume() ? std::move(projected) : std::move(legacy);
        bin.setCandidatePolicy(DBLFCandidatePolicy::BestOfBoth);

        return;
    }
    ROTATION_WAY rotationWay = bin.getRotationWay();
    ColaPuntosDBL queue = ColaPuntosDBL::Build();
    queue.push(Punto::Build(0, 0, 0));

    for (ItemBin &item : itemsToPack)
    {
        checkExecutionDeadline();

        if (rotationWay == ROTATION_WAY::ZERO_WAY)
            item.rotate(ROTATION_WAY::ZERO_WAY, ROTATION_MODE::NONE);
        if (!item.isValidOrientation())
        {
            continue;
        }
        std::cout << "Box: " << item.getId() << "\n";
        std::cout << "Available points: " << queue;

        for (int j = queue.size() - 1; j >= 0; j--)
        {
            checkExecutionDeadline();
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
