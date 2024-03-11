#ifndef GENETICS_OPERATORS_H
#define GENETICS_OPERATORS_H

#include "../src/FitnessOperators.hpp"
#include "MutationType.hpp"

Individuo crossIndivituals(LoadedBins, Bin, MutationType, Individuo, Individuo, double, double);
void mutationWithType(DoubleGenome &, MutationType, double, int);
void flipMutation(DoubleGenome &, double, ROTATION_WAY);
DoubleGenome crossOver(DoubleGenome, DoubleGenome);
int tournament(Poblacion, double);
int rouletteWheel(Poblacion);

#endif