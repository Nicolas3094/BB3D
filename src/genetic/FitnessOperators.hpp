#ifndef FITNESS_OPERATOR_H
#define FITNESS_OPERATOR_H

#include "Individuo.hpp"
#include "GenomeOperators.hpp"
#include "../heuristics/DBLF.hpp"

void evaluateFitness(Individuo &, LoadedBins, Bin);
void evaluateFitnessDecodificated(Individuo &, LoadedBins, Bin &);
DoubleGenome codificateItemsToPacked(LoadedBins &allBinsToLoad);
Poblacion buildHeuristicPoblation(const int, Bin, LoadedBins);
void evaluatePoblation(Poblacion &, LoadedBins, Bin);
Poblacion initialHeuristicPoblation(int, Bin, LoadedBins, ROTATION_WAY);

#endif