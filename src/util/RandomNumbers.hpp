#ifndef RANDOM_NUMBERS_H
#define RANDOM_NUMBERS_H

#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <cstdint>

// One shared stream, reproducible with the same build and call sequence.
void seedRandomNumbers(std::uint64_t seed);

double uniformUnit();
int randomInteger(int, int);
// Binomial(n, (n-1)/n): expected result n-1 for n>=1.
// Returns 0 for n=0; throws std::invalid_argument for negative n.
int defaultBinomialDistribution(int n);
#endif
