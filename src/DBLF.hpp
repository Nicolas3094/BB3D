#ifndef DBLH_H
#define DBLH_H

#include "Chromosome.hpp"
#include "ItemBinOperators.hpp"
#include "Bin.hpp"
#include "../structures/ColaPrioridad.hpp"
#include "../structures/PriorityPointQueue.hpp"

void DBLF(Bin &, Chromosome, Chromosome, LoadedBins, bool print = false);
void DBLF(Bin &, LoadedBins);
void DBLFQueue(Bin &, Chromosome, Chromosome, LoadedBins);

#endif