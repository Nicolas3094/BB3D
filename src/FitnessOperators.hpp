#ifndef FITNESS_OPERATOR_H
#define FITNESS_OPERATOR_H

#include "../main.hpp"
#include "Individuo.hpp"
#include "DBLF.hpp"
#include "GenomeOperators.hpp"

void evaluateFitness(Individuo &, LoadedBins, Bin &);
void evaluateFitnessDecodificated(Individuo &, LoadedBins, Bin &);
DoubleGenome codificateItemsToPacked(LoadedBins &allBinsToLoad);
Poblacion buildHeuristicPoblation(const uint, Bin, LoadedBins);
void evaluatePoblation(Poblacion &, LoadedBins, Bin);

#endif