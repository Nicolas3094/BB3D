#ifndef DBLH_H
#define DBLH_H

#include "../genetic/Chromosome.hpp"
#include "../binPacking/ItemBinOperators.hpp"
#include "../binPacking/Bin.hpp"
#include "../structures/ColaPrioridad.hpp"
#include "../structures/PriorityPointQueue.hpp"
#include "../binPacking/RotationMode.hpp"

// Boxes marked with an invalid orientation are skipped without consuming points.
// Rotation genes remain aligned with their original item positions.
// Malformed IDs, dimensions, rotation genes or chromosome lengths throw
// invalid_argument before changing the bin. Unique subsets of input IDs are allowed.
void DBLF(Bin &, Chromosome, Chromosome, LoadedBins, bool print = false);
void DBLF(Bin &, LoadedBins);
void DBLFQueue(Bin &, Chromosome, Chromosome, LoadedBins);

#endif
