#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <vector>

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

#endif