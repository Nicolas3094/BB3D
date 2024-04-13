#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <vector>
#include "metaheuristics/EvolutiveAlgorithm.hpp"
#include "readData/ReadData.hpp"
#include "genetic/MutationType.hpp"
#include "readData/DataSetBPP.hpp"

using namespace std;

#define print(x) cout << x << "\n"

void testDblf();
void genetic();
void evaluatePoblationTime();
void iterGeneticAll();
void printResults(vector<double>, std::vector<long int>, const string, const string);
void testDblfOneInstance();
void testRotationsGA(std::string);
void testInverseMutation();
void handlePaths(bool, string, string, MutationType, ROTATION_WAY, string &, string &);
void handleUserInputs(string &, string &, string, string, bool &);
string getAlgorithmNameFromNumber(string);
Poblacion getEvolutiveAlgorithm(string, ROTATION_WAY, MutationType, DatasetBinBacking, bool);
MutationType getMutationTypeFromUser(string mutationType);
#endif