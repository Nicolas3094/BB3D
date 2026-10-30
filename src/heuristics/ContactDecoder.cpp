#include "ContactDecoder.hpp"
#include "DBLFValidation.hpp"
#include "../util/ExecutionCheckpoint.hpp"
#include <tuple>

namespace
{
bool dblBefore(Punto a, Punto b)
{
    return std::tie(a.x, a.z, a.y) < std::tie(b.x, b.z, b.y);
}
}

long double placementContactArea(const Bin &bin, const ItemBin &item, Punto p)
{
    const auto d = item.getCurrentDimension();
    const auto bounds = bin.getDimensions();
    const std::int64_t low[3] = {p.x, p.y, p.z}, size[3] = {d.x, d.y, d.z},
                       limit[3] = {bounds.getLargo(), bounds.getAncho(), bounds.getAlto()};
    long double area = 0;

    for (int axis = 0; axis < 3; ++axis)
    {
        const int u = (axis + 1) % 3, v = (axis + 2) % 3;

        if (low[axis] == 0)
            area += static_cast<long double>(size[u]) * size[v];
        if (low[axis] + size[axis] == limit[axis])
            area += static_cast<long double>(size[u]) * size[v];
    }
    for (const auto &box : bin.getLoadedItems())
    {
        checkExecutionDeadline();
        const auto q = box.getPunto(), e = box.getCurrentDimension();
        const std::int64_t other[3] = {q.x, q.y, q.z}, extent[3] = {e.x, e.y, e.z};

        for (int axis = 0; axis < 3; ++axis)
        {
            if (low[axis] + size[axis] != other[axis] && other[axis] + extent[axis] != low[axis])
                continue;

            const int u = (axis + 1) % 3, v = (axis + 2) % 3;
            const auto a = std::max<std::int64_t>(0, std::min(low[u] + size[u], other[u] + extent[u]) -
                                                         std::max(low[u], other[u]));
            const auto b = std::max<std::int64_t>(0, std::min(low[v] + size[v], other[v] + extent[v]) -
                                                         std::max(low[v], other[v]));
            area += static_cast<long double>(a) * b;
        }
    }
    // On a given candidate face, two touching obstacles cannot cover overlapping
    // positive-area patches without overlapping each other. Wall and item patches
    // cannot overlap either in a feasible in-bounds packing. Edges have zero area.
    return area;
}

ContactPlacement selectContactPlacement(const Bin &bin, const ItemBin &item, ColaPuntosDBL &queue)
{
    ContactPlacement best;

    for (int j = queue.size() - 1; j >= 0; --j)
    {
        checkExecutionDeadline();
        auto p = queue[j];

        if (!canBePlaced(bin, p + item.getCurrentDimension()) || isOverlapped(p, item, bin))
            continue;

        iterateByDeepestBottomLeft(p, item, bin);
        const auto score = placementContactArea(bin, item, p);

        if (!best.found || score > best.area || (score == best.area && dblBefore(p, best.point)))
            best = {true, p, j, score};
    }
    return best;
}

void contactDecode(Bin &bin, LoadedBins items)
{
    validateDBLFInput(bin, items);

    if (bin.getCandidatePolicy() != DBLFCandidatePolicy::ThreePoints)
        throw std::invalid_argument(
            "Contact decoder requires legacy three-point generation; projections are a separate experiment.");
    ColaPuntosDBL queue;
    queue.push(Punto::Build(0, 0, 0));

    for (auto &item : items)
    {
        checkExecutionDeadline();

        if (bin.getRotationWay() == ROTATION_WAY::ZERO_WAY)
            item.rotate(ROTATION_WAY::ZERO_WAY, ROTATION_MODE::NONE);
        if (!item.isValidOrientation())
            continue;

        const auto choice = selectContactPlacement(bin, item, queue);

        if (choice.found)
        {
            queue.remove(choice.sourceIndex);
            addItemToBin(queue, bin, choice.point, item);
        }
    }
}

void contactDecode(Bin &bin, const Chromosome &ids, const Chromosome &rotations, LoadedBins items)
{
    validateDBLFInput(bin, ids, rotations, items);
    LoadedBins ordered;
    ordered.reserve(ids.size());

    for (size_t i = 0; i < ids.size(); ++i)
    {
        checkExecutionDeadline();
        auto item = items[ids[i] - 1];
        item.rotate(bin.getRotationWay(), getRotationFromId(rotations[i]));
        ordered.push_back(item);
    }
    contactDecode(bin, std::move(ordered));
}
