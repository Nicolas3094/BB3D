#ifndef MAIN_H
#define MAIN_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
#include <cmath>
#include <cstdlib>
#include <random>
#include <time.h>
#include <algorithm>
#include <functional>
#include <limits.h>
#include <experimental/random>

#define print(x) std::cout << x << "\n"
#define MAX_RAN INT_MAX

typedef unsigned int uint;
typedef unsigned long ulong;
void genetic();
void evaluatePoblationTime();
void iterGeneticAll(std::string, std::string, std::string, int);
void printResults(std::vector<double>, std::vector<long int>, std::string, std::string, std::string, int);
#endif