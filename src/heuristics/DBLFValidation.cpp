#include "DBLFValidation.hpp"
#include <stdexcept>
#include <unordered_set>

namespace
{
bool binary(int value) { return value == 0 || value == 1; }
}

void validateDBLFInput(const Bin &bin, const LoadedBins &items)
{
    const Dim dimensions = bin.getDimensions();
    if (dimensions.getLargo() <= 0 || dimensions.getAncho() <= 0 || dimensions.getAlto() <= 0)
        throw std::invalid_argument("DBLF: container dimensions must be positive.");
    switch (bin.getRotationWay())
    {
    case ROTATION_WAY::ZERO_WAY:
    case ROTATION_WAY::TWO_WAY:
    case ROTATION_WAY::SIX_WAY: break;
    default: throw std::invalid_argument("DBLF: unsupported rotation mode.");
    }
    std::unordered_set<int> ids;
    for (const auto &item : items)
    {
        if (item.getId() <= 0 || !ids.insert(item.getId()).second)
            throw std::invalid_argument("DBLF: item IDs must be positive and unique.");
        const Dim original = item.getDimension();
        const Punto current = item.getCurrentDimension();
        if (original.getLargo() <= 0 || original.getAncho() <= 0 || original.getAlto() <= 0 ||
            current.x <= 0 || current.y <= 0 || current.z <= 0)
            throw std::invalid_argument("DBLF: item dimensions must be positive.");
        const Punto rotations = item.getRotations();
        if (!binary(rotations.x) || !binary(rotations.y) || !binary(rotations.z))
            throw std::invalid_argument("DBLF: rotation permissions must be 0 or 1.");
    }
}

void validateDBLFInput(const Bin &bin, const Chromosome &order,
                       const Chromosome &rotations, const LoadedBins &items)
{
    validateDBLFInput(bin, items);
    if (order.size() != rotations.size())
        throw std::invalid_argument("DBLF: item and rotation chromosomes must have equal lengths.");
    for (size_t i = 0; i < items.size(); ++i)
        if (items[i].getId() != static_cast<int>(i + 1))
            throw std::invalid_argument("DBLF: item data must be indexed by consecutive IDs 1..N.");
    const int maximumRotation = getIdFromRotationWay(bin.getRotationWay());
    std::vector<bool> visited(items.size(), false);
    for (size_t i = 0; i < order.size(); ++i)
    {
        const int id = order[i];
        if (id <= 0 || static_cast<size_t>(id) > items.size())
            throw std::invalid_argument("DBLF: chromosome contains an out-of-range item ID.");
        if (visited[id - 1])
            throw std::invalid_argument("DBLF: chromosome contains a repeated item ID.");
        visited[id - 1] = true;
        if ((maximumRotation == 0 && rotations[i] != 0) ||
            (maximumRotation != 0 && (rotations[i] < 1 || rotations[i] > maximumRotation)))
            throw std::invalid_argument("DBLF: rotation gene is outside the configured mode.");
    }
}
