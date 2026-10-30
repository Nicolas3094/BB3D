#ifndef SEARCH_VALIDATION_H
#define SEARCH_VALIDATION_H
#include "../readData/DataSetBPP.hpp"
#include "../heuristics/DBLFValidation.hpp"
#include "../genetic/MutationType.hpp"
#include <cmath>
#include <limits>

inline void validateSearch(const DatasetBinBacking &data, int count, long iterations, double mutation,
                           double rotationMutation, MutationType type, ROTATION_WAY rotation)
{
    if (count <= 0 || iterations < 0 || data.totalItems.empty() ||
        data.totalItems.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
        throw std::invalid_argument(
            "Search requires a positive population, non-empty items and non-negative iterations.");
    for (double p : {mutation, rotationMutation})
        if (!std::isfinite(p) || p < 0 || p > 1)
            throw std::invalid_argument("Search probabilities must be finite and in [0,1].");

    if (type != MutationType::C1 && type != MutationType::C2 && type != MutationType::INVERSE_MUTATION)
        throw std::invalid_argument("Unsupported mutation type.");

    Bin bin = data.bin;
    bin.setRotationWay(rotation);
    validateDBLFInput(bin, data.totalItems);
}
#endif
