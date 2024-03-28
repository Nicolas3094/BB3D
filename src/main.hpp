#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <vector>
#include "metaheuristics/EvolutiveAlgorithm.hpp"
#include "readData/ReadData.hpp"
#include "genetic/MutationType.hpp"
#include "readData/DataSetBPP.hpp"

using namespace std;

#define print(x) std::cout << x << "\n"

void testDblf();
void genetic();
void evaluatePoblationTime();
void iterGeneticAll();
void printResults(std::vector<double>, std::vector<long int>, const string, const string);
void testDblfOneInstance();
void testRotationsGA(std::string);
void testInverseMutation();
std::string getAlgorithmNameFromNumber(std::string);
EvolutiveAlgorithm *getEvolutiveAlgorithm(std::string algorithmName, ROTATION_WAY rotation, MutationType mutation, DatasetBinBacking dataSet);
MutationType getMutationTypeFromUser(std::string mutationType);
#endif