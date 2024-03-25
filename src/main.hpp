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
void printResults(std::vector<double>, std::vector<long int>, std::string, std::string, std::string, int);
void testDblfOneInstance();
void testRotationsGA(std::string);
void testInverseMutation();
#endif