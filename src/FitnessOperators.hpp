#ifndef FITNESS_OPERATOR_H
#define FITNESS_OPERATOR_H

#include "../main.hpp"
#include "Individuo.hpp"
#include "DBLF.hpp"
#include "GenomeOperators.hpp"

void evaluateFitness(Individuo &, LoadedBins, Bin &);
DoubleGenome codificateItemsToPacked(LoadedBins &allBinsToLoad);
Poblacion buildHeuristicPoblation(const uint, LoadedBins, Bin &, ROTATION_WAY);
void evaluatePoblation(Poblacion &, LoadedBins, Bin, ROTATION_WAY);

#endif