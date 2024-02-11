
#ifndef GENOME_OPERATORS_H
#define GENOME_OPERATORS_H

#include "../main.hpp"
#include "Bin.hpp"
#include "Individuo.hpp"
#include "GenomeOperators.hpp"

// Function to return the next random number
uint getNum(std::vector<uint> &);

// Function to generate n non-repeating random numbers
Chromosome generatePermutation(uint);

LoadedBins decodeChromosome(Chromosome, LoadedBins);
Chromosome codeChromosome(LoadedBins);
std::vector<Chromosome> buildHeuristicChromosomes(LoadedBins, bool);
std::vector<Chromosome> buildCompleteHeuristicChromosomes(const LoadedBins &, const uint);
std::vector<Chromosome> buildChromosomes(const LoadedBins &, const uint);
Chromosome generateRandomRepeatedAlalleleChromosome(uint, uint, uint);

#endif