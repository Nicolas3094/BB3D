#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "../src/FitnessOperators.hpp"
#include "MutationType.hpp"
#include "GeneticsOperators.hpp"
#include "PermutationOperators.hpp"
#include "../readData/DataSetBPP.hpp"
class GeneticAlgorithm
{
private:
    long maxIteration;
    long numberOfIndividuals;
    double selectionProbability;
    double crossProability;
    double mutationProbaility;
    double dMutationProbability;
    DatasetBinBacking dataSet;

    MutationType mutationType;
    ROTATION_WAY rotationType;

    void nextGeneration(Poblacion &);
    Individuo instantiateChild(Individuo, Individuo);
    void mutate(DoubleGenome &, int step = -1);
    Individuo crossover(Individuo, Individuo);
    Poblacion initialPoblation();

public:
    GeneticAlgorithm();
    ~GeneticAlgorithm();
    static GeneticAlgorithm Build();

    GeneticAlgorithm &setSelectionProbability(double);
    GeneticAlgorithm &setCrossProbability(double);
    GeneticAlgorithm &setMutationProbabiliy(double);
    GeneticAlgorithm &setDMutationProbability(double);
    GeneticAlgorithm &setMutationType(MutationType);
    GeneticAlgorithm &setRotationType(ROTATION_WAY);
    GeneticAlgorithm &setMaxIteration(long);
    GeneticAlgorithm &setNumberOfIndividuals(long);
    GeneticAlgorithm &setProblem(DatasetBinBacking);

    Poblacion evolve();
};

#endif