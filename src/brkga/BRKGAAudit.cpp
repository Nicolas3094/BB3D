#include "BRKGAAudit.hpp"
#include <array>
#include <limits>
#include <unordered_set>
#include <stdexcept>

namespace brkga2012
{
void verifyPacking(const Bin &packing, const DatasetBinBacking &data)
{
    auto require = [](bool condition)
    {
        if (!condition)
            throw std::runtime_error("BRKGA independent geometric audit failed");
    };
    const auto expected = data.bin.getDimensions(), actual = packing.getDimensions();
    require(expected.getLargo() == actual.getLargo() && expected.getAncho() == actual.getAncho() &&
            expected.getAlto() == actual.getAlto());
    const std::int64_t limits[3] = {actual.getLargo(), actual.getAncho(), actual.getAlto()};

    for (auto limit : limits)
        require(limit > 0);
    const int permutations[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
    std::unordered_set<int> used;
    std::int64_t volume = 0;

    for (const auto &item : packing.getLoadedItems())
    {
        const int id = item.getId(), rotation = item.getRotationId();
        require(id > 0 && static_cast<size_t>(id) <= data.totalItems.size() && used.insert(id).second);
        require(rotation >= 1 && rotation <= 6);
        const auto &source = data.totalItems[id - 1];
        require(source.getId() == id && source.getTipo() == item.getTipo());
        const auto original = source.getDimension();
        const auto permissions = source.getRotations();
        const std::int64_t sides[3] = {original.getLargo(), original.getAncho(), original.getAlto()};
        const std::int64_t flags[3] = {permissions.x, permissions.y, permissions.z};
        const auto position = item.getPunto(), dimensions = item.getCurrentDimension();
        const std::int64_t low[3] = {position.x, position.y, position.z};
        const std::int64_t lengths[3] = {dimensions.x, dimensions.y, dimensions.z};
        const auto &order = permutations[rotation - 1];
        require(flags[order[2]] == 1);
        std::int64_t itemVolume = 1;

        for (int axis = 0; axis < 3; ++axis)
        {
            require(lengths[axis] == sides[order[axis]] && lengths[axis] > 0 && low[axis] >= 0 &&
                    lengths[axis] <= limits[axis] && low[axis] <= limits[axis] - lengths[axis]);
            require(itemVolume <= std::numeric_limits<std::int64_t>::max() / lengths[axis]);
            itemVolume *= lengths[axis];
        }
        require(volume <= std::numeric_limits<std::int64_t>::max() - itemVolume);
        volume += itemVolume;
    }
    const auto &items = packing.getLoadedItems();

    for (size_t first = 0; first < items.size(); ++first)
        for (size_t second = 0; second < first; ++second)
        {
            const auto p = items[first].getPunto(), d = items[first].getCurrentDimension();
            const auto q = items[second].getPunto(), e = items[second].getCurrentDimension();
            require(p.x + d.x <= q.x || q.x + e.x <= p.x || p.y + d.y <= q.y || q.y + e.y <= p.y ||
                    p.z + d.z <= q.z || q.z + e.z <= p.z);
        }
    require(volume == packing.getLoadedVolume() &&
            items.size() == static_cast<size_t>(packing.getNumberOfLoadedItems()));
}
}
