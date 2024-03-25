#ifndef DBLH_H
#define DBLH_H

#include "../genetic/Chromosome.hpp"
#include "../binPacking/ItemBinOperators.hpp"
#include "../binPacking/Bin.hpp"
#include "../structures/ColaPrioridad.hpp"
#include "../structures/PriorityPointQueue.hpp"
#include "../binPacking/RotationMode.hpp"

void DBLF(Bin &, Chromosome, Chromosome, LoadedBins, bool print = false);
void DBLF(Bin &, LoadedBins);
void DBLFQueue(Bin &, Chromosome, Chromosome, LoadedBins);

#endif