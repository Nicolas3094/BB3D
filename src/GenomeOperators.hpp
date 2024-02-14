
#ifndef GENOME_OPERATORS_H
#define GENOME_OPERATORS_H

#include "../main.hpp"
#include "Bin.hpp"
#include "Individuo.hpp"
#include "GenomeOperators.hpp"

// Function to return the next random number
int getNum(std::vector<int> &);

// Function to generate n non-repeating random numbers
Chromosome generatePermutation(int);

LoadedBins decodeChromosome(Chromosome, LoadedBins);
Chromosome codeChromosome(LoadedBins);
std::vector<Chromosome> buildHeuristicChromosomes(LoadedBins, bool);
std::vector<Chromosome> buildCompleteHeuristicChromosomes(const LoadedBins &, const int);
std::vector<Chromosome> buildChromosomes(const LoadedBins &, const int);
Chromosome generateRandomRepeatedAlalleleChromosome(int, int, int);

#endif