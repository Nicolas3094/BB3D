#ifndef DBLF_VALIDATION_H
#define DBLF_VALIDATION_H

#include "../binPacking/Bin.hpp"

// Validate all inputs before packing/decoding. A unique subset of IDs is allowed.
// Encoded data must be indexed by consecutive IDs 1..N; decoded items may be reordered.
// Throws invalid_argument on malformed data; forbidden orientations are skipped by DBLF.
// The container must be empty; current dimensions must permute original dimensions.
void validateDBLFInput(const Bin &, const LoadedBins &);
void validateDBLFInput(const Bin &, const Chromosome &, const Chromosome &, const LoadedBins &);

#endif
