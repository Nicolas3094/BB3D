#ifndef PERMUTATION_OPERATOS_H
#define PERMUTATION_OPERATOS_H

#include "../src/Individuo.hpp"
#include "RandomNumbers.hpp"

int hamming(DoubleGenome, DoubleGenome);
DoubleGenome crossOx(DoubleGenome, DoubleGenome, int, int);
void swapPointValue(DoubleGenome &, int, int);
void inverseMutation(DoubleGenome &, int, int);
void insertion(DoubleGenome &, int, int);
void insertionOfSubsequence(DoubleGenome &, int, int, int);
void randomReversingInsertionOfSubsequence(DoubleGenome &, int, int, int);
void swap(DoubleGenome &, int, int);
void swapSubsequences(DoubleGenome &, int, int, int, int);
void randomReversingSwapOfSubsequences(DoubleGenome &, int, int, int, int);
void combine1(DoubleGenome &, int, int, int, int);
void combine2(DoubleGenome &, int, int, int);
void mutateC1(DoubleGenome &, int randomStep = -1);
void mutateC2(DoubleGenome &, int randomStep = -1);
void mutateInversion(DoubleGenome &, int randomStep = -1);
void addRangeToChromosome(Chromosome &, Chromosome, int, int);
void addRangeToGenome(DoubleGenome &, DoubleGenome, int, int);
void addToGenome(DoubleGenome &, int, int x);
void copyEqualRangeToGenome(DoubleGenome &, DoubleGenome, int, int, int, int);
void copyReversedEqualRangeToGenome(DoubleGenome &, DoubleGenome, int, int, int, int);
DoubleGenome buildSubsequence(DoubleGenome, int, int);
void copySubsequence(DoubleGenome &, DoubleGenome, int, int);
void reverseGenome(DoubleGenome &);
void addSubsequence(DoubleGenome &, DoubleGenome, int, int, int);
DoubleGenome buildSubsequenceGenome(DoubleGenome, int, int);
#endif