#ifndef GENETICS_OPERATORS_H
#define GENETICS_OPERATORS_H

#include "FitnessOperators.hpp"
#include "MutationType.hpp"
#include <map>

void mutationWithType(DoubleGenome &, MutationType, int step = -1);
void flipMutation(DoubleGenome &gen, double probability, ROTATION_WAY rotation);
DoubleGenome crossOver(DoubleGenome, DoubleGenome);
// n is the eligible prefix length, not a last index. An optional excluded
// index supports selecting distinct parents without random retry loops.
int tournament(const Poblacion &, int n, double probability, int excludedIndex = -1);
// Select proportionally to non-negative finite fitness; all-zero fitness uses
// uniform selection. Empty populations and invalid fitness throw invalid_argument.
int rouletteWheel(Poblacion);
void flipMutationO1(DoubleGenome &gen, double probability, ROTATION_WAY rotation);

#endif
