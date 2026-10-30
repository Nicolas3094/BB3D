#include "MaximalSpacesDecoder.hpp"
#include "DBLFValidation.hpp"
#include "../spatial/MaximalSpace.hpp"
#include "../util/ExecutionCheckpoint.hpp"
#include <algorithm>
#include <tuple>
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
#include "../spatial/MaximalSpacesProfile.hpp"
#endif

namespace
{
/**
 * @brief Busca la esquina minima factible con prioridad X, Z y despues Y.
 * @param spaces Espacios libres, de dimensiones positivas y dentro del contenedor.
 * @param itemDimensions Dimensiones positivas de la caja ya orientada.
 * @param[out] bestPosition Posicion elegida; no se modifica si ninguna caja cabe.
 * @return true si se encontro una posicion; false si la caja no cabe.
 * @note Ante posiciones iguales conserva la primera encontrada. Las dimensiones
 * se comparan por diferencias para evitar desbordamiento al sumar coordenadas.
 * @throws ExecutionTimeLimit Si vence el presupuesto activo.
 */
bool findDeepestBottomLeftPlacement(const std::vector<MaximalSpace> &spaces, Punto itemDimensions,
                                    Punto &bestPosition)
{
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
    auto &profile = maximalSpacesProfile();
    MaximalSpacesPhaseTimer placementTimer(profile.placementMicroseconds);
#endif
    bool found = false;

    for (const auto &space : spaces)
    {
        checkExecutionDeadline();
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
        ++profile.candidateSpaces;
#endif
        // Compare extents before addition to avoid coordinate overflow.
        const bool fits = itemDimensions.x <= space.maximum.x - space.minimum.x &&
                          itemDimensions.y <= space.maximum.y - space.minimum.y &&
                          itemDimensions.z <= space.maximum.z - space.minimum.z;
        if (!fits)
            continue;

        const auto position = space.minimum;

        if (!found || std::tie(position.x, position.z, position.y) <
                          std::tie(bestPosition.x, bestPosition.z, bestPosition.y))
        {
            found = true;
            bestPosition = position;
        }
    }
    return found;
}
}

void maximalSpacesDecode(Bin &bin, LoadedBins orientedItems, MaximalSpacesStats *stats)
{
    validateDBLFInput(bin, orientedItems);
    const auto dimensions = bin.getDimensions();
    std::vector<MaximalSpace> freeSpaces = {
        {{0, 0, 0}, {dimensions.getLargo(), dimensions.getAncho(), dimensions.getAlto()}}};
    if (stats)
        stats->peakRetainedSpaces = 1;

    for (auto &item : orientedItems)
    {
        checkExecutionDeadline();

        if (bin.getRotationWay() == ROTATION_WAY::ZERO_WAY)
            item.rotate(ROTATION_WAY::ZERO_WAY, ROTATION_MODE::NONE);
        if (!item.isValidOrientation())
            continue;

        const auto itemDimensions = item.getCurrentDimension();
        Punto position;

        if (!findDeepestBottomLeftPlacement(freeSpaces, itemDimensions, position))
            continue;

        item.setPosicion(position);
        bin.addItem(item);
        subtractOccupiedSpace(freeSpaces, {position, position + itemDimensions});

        if (stats)
            stats->peakRetainedSpaces = std::max(stats->peakRetainedSpaces, freeSpaces.size());
    }
}

void maximalSpacesDecode(Bin &bin, const Chromosome &itemIds, const Chromosome &rotationGenes,
                         const LoadedBins &itemsById, MaximalSpacesStats *stats)
{
    validateDBLFInput(bin, itemIds, rotationGenes, itemsById);
    LoadedBins orientedItems;
    orientedItems.reserve(itemIds.size());

    for (size_t geneIndex = 0; geneIndex < itemIds.size(); ++geneIndex)
    {
        checkExecutionDeadline();
        auto item = itemsById[itemIds[geneIndex] - 1];
        item.rotate(bin.getRotationWay(), getRotationFromId(rotationGenes[geneIndex]));
        orientedItems.push_back(item);
    }
    maximalSpacesDecode(bin, std::move(orientedItems), stats);
}
