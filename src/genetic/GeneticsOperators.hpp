#ifndef GENETICS_OPERATORS_H
#define GENETICS_OPERATORS_H

#include "FitnessOperators.hpp"
#include "MutationType.hpp"
#include <map>

void mutationWithType(DoubleGenome &, MutationType, int step = -1);
void flipMutation(DoubleGenome &gen, double probability, ROTATION_WAY rotation);
DoubleGenome crossOver(DoubleGenome, DoubleGenome);
int tournament(Poblacion, int, double);
// Select proportionally to non-negative finite fitness; all-zero fitness uses
// uniform selection. Empty populations and invalid fitness throw invalid_argument.
int rouletteWheel(Poblacion);
void flipMutationO1(DoubleGenome &gen, double probability, ROTATION_WAY rotation);
int getNumberOfChanges(double probability, double unitProbability);

#endif
