#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "../genetic/FitnessOperators.hpp"
#include "../genetic/MutationType.hpp"
#include "../genetic/GeneticsOperators.hpp"
#include "../genetic/PermutationOperators.hpp"
#include "../readData/DataSetBPP.hpp"
#include "EvolutiveAlgorithm.hpp"
#include <map>

class GeneticAlgorithm : public EvolutiveAlgorithm
{
private:
    std::map<std::string, bool> visitedIndividuals;
    long maxIteration;
    DatasetBinBacking dataSet;
    long numberOfIndividuals;

    double selectionProbability;
    double crossProability;
    double mutationProbaility;
    double dMutationProbability;

    MutationType mutationType;
    ROTATION_WAY rotationType;

    bool terminateCondition(Poblacion);
    void nextGeneration(Poblacion &);
    void nextGenerationByAdding(Poblacion &);
    Individuo instantiateChild(Individuo, Individuo);
    void mutate(Individuo &, int step = -1);
    Individuo crossover(Individuo, Individuo);

    std::vector<Individuo> getParents(Poblacion, int);
    std::vector<Individuo> getChildren(Individuo, Individuo);
    double adaptiveMutationProbabiliy(Individuo, Individuo);
    void mutatePoblation(Poblacion &, int, int);

    void addChildredToPoblation(Poblacion &, Individuo, Individuo);
    void replacePoblationWithChildren(Poblacion &, Individuo, Individuo, int);

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

    Poblacion evolveWithReplacement();
    Poblacion evolveWithAdded();
};

#endif