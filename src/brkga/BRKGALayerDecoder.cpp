#include "BRKGALayerDecoder.hpp"
#include "../heuristics/DBLFValidation.hpp"
#include "../util/ExecutionCheckpoint.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <tuple>

namespace brkga2012
{
namespace
{
/** @brief Comprueba que dos copias del mismo tipo tengan dimensiones y permisos coherentes. */
bool sameType(const ItemBin &first, const ItemBin &second)
{
    const auto a = first.getDimension(), b = second.getDimension();

    return first.getTipo() == second.getTipo() && first.getRotations() == second.getRotations() &&
           a.getLargo() == b.getLargo() && a.getAncho() == b.getAncho() && a.getAlto() == b.getAlto();
}

/** @brief Cuenta las copias representadas por los tipos, para dimensionar el cromosoma. */
std::size_t itemCount(const std::vector<BoxType> &types)
{
    std::size_t count = 0;

    for (const auto &type : types)
        count += type.itemIds.size();

    return count;
}
}

std::vector<BoxType> buildBoxTypes(const DatasetBinBacking &data)
{
    auto bin = data.bin;
    bin.setRotationWay(ROTATION_WAY::SIX_WAY);
    validateDBLFInput(bin, data.totalItems);
    std::vector<BoxType> types;

    for (size_t index = 0; index < data.totalItems.size(); ++index)
    {
        checkExecutionDeadline();
        const auto &item = data.totalItems[index];

        if (item.getId() != static_cast<int>(index + 1))
            throw std::invalid_argument("BRKGA requires consecutive item IDs indexed from 1");

        auto found = std::find_if(types.begin(), types.end(),
                                  [&](const BoxType &type)
                                  {
                                      return type.prototype.getTipo() == item.getTipo();
                                  });
        if (found == types.end())
        {
            types.push_back({item, {item.getId()}});
            continue;
        }
        if (!sameType(found->prototype, item))
            throw std::invalid_argument("BRKGA: inconsistent dimensions or permissions within a type");

        found->itemIds.push_back(item.getId());
    }
    if (types.empty())
        throw std::invalid_argument("BRKGA requires at least one box");

    return types;
}

std::vector<std::size_t> decodeTypeSequence(const RandomKeys &keys, const std::vector<BoxType> &types)
{
    const auto count = itemCount(types);

    if (keys.size() != 2 * count)
        throw std::invalid_argument("BRKGA chromosome must contain exactly 2M keys");

    for (double key : keys)
        if (!std::isfinite(key) || key < 0 || key > 1)
            throw std::invalid_argument("BRKGA keys must be finite and in [0,1]");

    std::vector<size_t> sourceTypes;

    for (size_t type = 0; type < types.size(); ++type)
        sourceTypes.insert(sourceTypes.end(), types[type].itemIds.size(), type);
    std::vector<size_t> order(count);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](size_t first, size_t second)
                     {
                         return keys[first] < keys[second];
                     });
    std::vector<size_t> sequence;

    for (size_t index : order)
        sequence.push_back(sourceTypes[index]);

    return sequence;
}

std::vector<Layer> enumerateLayers(const BoxType &type, const MaximalSpace &space, size_t remaining)
{
    const int axes[6][2] = {{1, 2}, {2, 1}, {1, 0}, {0, 1}, {2, 0}, {0, 2}};
    const std::int64_t available[3] = {space.maximum.x - space.minimum.x, space.maximum.y - space.minimum.y,
                                       space.maximum.z - space.minimum.z};
    std::vector<Layer> layers;

    if (!remaining)
        return layers;

    for (int rotation = 1; rotation <= 6; ++rotation)
    {
        checkExecutionDeadline();
        auto item = type.prototype;
        item.rotate(ROTATION_WAY::SIX_WAY, getRotationFromId(rotation));

        if (!item.isValidOrientation())
            continue;

        const auto dimensions = item.getCurrentDimension();
        const std::int64_t sides[3] = {dimensions.x, dimensions.y, dimensions.z};

        if (sides[0] > available[0] || sides[1] > available[1] || sides[2] > available[2])
            continue;

        for (const auto &plane : axes)
        {
            Layer layer;
            layer.rotation = rotation;
            layer.itemDimensions = dimensions;
            layer.firstAxis = plane[0];
            layer.secondAxis = plane[1];
            // Full rows/columns keep the layer rectangular. Any remaining copies
            // are considered again at later occurrences of this type in BTPS.
            layer.counts[plane[0]] = std::min<std::int64_t>(available[plane[0]] / sides[plane[0]], remaining);
            layer.counts[plane[1]] = std::min<std::int64_t>(available[plane[1]] / sides[plane[1]],
                                                            remaining / layer.counts[plane[0]]);
            layer.boxCount = layer.counts[plane[0]] * layer.counts[plane[1]];
            layers.push_back(layer);
        }
    }
    return layers;
}

size_t layerIndex(double key, size_t layerCount)
{
    if (!layerCount || !std::isfinite(key) || key < 0 || key > 1)
        throw std::invalid_argument("Invalid BRKGA layer selection");

    if (key == 0)
        return 0;

    return std::min(layerCount - 1, static_cast<size_t>(std::ceil(key * layerCount) - 1));
}

void decodeLayers(Bin &bin, const std::vector<BoxType> &types, const RandomKeys &keys)
{
    if (!bin.getLoadedItems().empty() || bin.getLoadedVolume() != 0 || types.empty())
        throw std::invalid_argument("BRKGA decoder needs an empty bin and validated box types");

    const auto sequence = decodeTypeSequence(keys, types);
    const auto dimensions = bin.getDimensions();

    if (dimensions.getLargo() <= 0 || dimensions.getAncho() <= 0 || dimensions.getAlto() <= 0)
        throw std::invalid_argument("BRKGA container dimensions must be positive");

    dimensions.getVolumen();
    bin.setRotationWay(ROTATION_WAY::SIX_WAY);
    std::vector<MaximalSpace> spaces = {
        {{0, 0, 0}, {dimensions.getLargo(), dimensions.getAncho(), dimensions.getAlto()}}};
    std::vector<size_t> used(types.size(), 0);
    std::vector<bool> skipped(types.size(), false);

    for (size_t position = 0; position < sequence.size(); ++position)
    {
        checkExecutionDeadline();
        const auto typeIndex = sequence[position];
        const auto &type = types[typeIndex];

        if (skipped[typeIndex] || used[typeIndex] == type.itemIds.size())
            continue;

        const size_t remaining = type.itemIds.size() - used[typeIndex];
        std::vector<Layer> layers;
        MaximalSpace chosenSpace;
        // Geometry keeps spaces ordered by X, Z, Y and deterministic maximum corners.
        for (const auto &space : spaces)
        {
            checkExecutionDeadline();
            layers = enumerateLayers(type, space, remaining);

            if (!layers.empty())
            {
                chosenSpace = space;
                break;
            }
        }
        if (layers.empty())
        {
            // Fig. 11 prints Skip=False on failure, which cannot terminate.
            // For variant U, free volume only shrinks, so mark this type skipped.
            skipped[typeIndex] = true;
            continue;
        }
        const auto &layer = layers[layerIndex(keys[sequence.size() + position], layers.size())];
        const std::int64_t sides[3] = {layer.itemDimensions.x, layer.itemDimensions.y,
                                       layer.itemDimensions.z};
        for (std::int64_t second = 0; second < layer.counts[layer.secondAxis]; ++second)
            for (std::int64_t first = 0; first < layer.counts[layer.firstAxis]; ++first)
            {
                checkExecutionDeadline();
                std::int64_t offset[3] = {0, 0, 0};
                offset[layer.firstAxis] = first * sides[layer.firstAxis];
                offset[layer.secondAxis] = second * sides[layer.secondAxis];
                auto item = type.prototype;
                item.setId(type.itemIds[used[typeIndex]++]);
                item.rotate(ROTATION_WAY::SIX_WAY, getRotationFromId(layer.rotation));
                item.setPosicion(chosenSpace.minimum + Punto(offset[0], offset[1], offset[2]));
                bin.addItem(item);
            }
        const Punto layerDimensions(layer.counts[0] * sides[0], layer.counts[1] * sides[1],
                                    layer.counts[2] * sides[2]);
        subtractOccupiedSpace(spaces, {chosenSpace.minimum, chosenSpace.minimum + layerDimensions});
    }
}
}
