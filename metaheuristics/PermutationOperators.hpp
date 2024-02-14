#ifndef PERMUTATION_OPERATOS_H
#define PERMUTATION_OPERATOS_H

#include "../src/Individuo.hpp"

int hamming(DoubleGenome, DoubleGenome);

DoubleGenome crossOx(DoubleGenome, DoubleGenome, int, int);
void swapPointValue(DoubleGenome &, int, int);
void inverseMutation(DoubleGenome &, int, int);
void randomInsertion(DoubleGenome &, int, int);
DoubleGenome randomInsertionOfSubsequence(DoubleGenome, int, int, int);
DoubleGenome randomReversingInsertionOfSubsequence(DoubleGenome, int, int, int);
void randomSwap(DoubleGenome &, int, int);
void randomSwapSubsequences(DoubleGenome &, int, int, int, int);
void randomReversingSwapOfSubsequences(DoubleGenome &, int, int, int, int);
void combine1(DoubleGenome &, int, int, int, int);
void combine2(DoubleGenome &, int, int, int);
void mutateC1(DoubleGenome &, int);
void mutateC2(DoubleGenome &, int);
void mutateInversion(DoubleGenome &, int);
void addRangeToChromosome(Chromosome &, Chromosome, int, int);
void addRangeToGenome(DoubleGenome &, DoubleGenome, int, int);
void addToGenome(DoubleGenome &, int, int x);
void copyEqualRangeToGenome(DoubleGenome &, DoubleGenome, int, int, int, int);
void copyReversedEqualRangeToGenome(DoubleGenome &, DoubleGenome, int, int, int, int);
#endif