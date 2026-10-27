#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "../genetic/FitnessOperators.hpp"
#include "../genetic/MutationType.hpp"
#include "../genetic/GeneticsOperators.hpp"
#include "../genetic/PermutationOperators.hpp"
#include "../readData/DataSetBPP.hpp"
#include "EvolutiveAlgorithm.hpp"

class GeneticAlgorithm : public EvolutiveAlgorithm
{
private:
    friend struct GeneticAlgorithmTestAccess;
    std::map<std::string, bool> visitedIndividuals;
    long maxIteration;
    DatasetBinBacking dataSet;
    long numberOfIndividuals;
    int matingAttemptsPerIteration=-1; // -1 preserves the original binomial draw.

    double selectionProbability;
    double crossProability;
    double mutationProbaility;
    bool adaptiveMutation = true;
    double dMutationProbability;
    double maxDiversityIndex;
    long stagnationLimit=25,stagnantIterations=0;
    double bestSeenFitness=-1;

    bool isWithReplacement;

    MutationType mutationType;
    ROTATION_WAY rotationType;

    bool terminateCondition(Poblacion);
    void nextGeneration(Poblacion &);
    void nextGenerationByAdding(Poblacion &);
    Individuo instantiateChild(Individuo, Individuo);
    void mutate(Individuo &, int step = -1);
    Individuo crossover(Individuo, Individuo);

    std::vector<Individuo> getParents(const Poblacion &, int);
    std::vector<Individuo> getChildren(Individuo, Individuo);
    double adaptiveMutationProbabiliy(Individuo, Individuo);
    void mutatePoblation(Poblacion &, int, int);

    void addChildredToPoblation(Poblacion &, Individuo, Individuo);
    void replacePoblationWithChildren(Poblacion &, Individuo, Individuo, int);

    Poblacion evolveWithReplacement();
    Poblacion evolveWithAdded();

    double diversityIndex(Poblacion);

public:
    GeneticAlgorithm();
    ~GeneticAlgorithm();
    static GeneticAlgorithm Build();

    GeneticAlgorithm &setMaxDiversityIndex(double);
    // 0 disables early stopping by stagnation; iterations still bound the run.
    GeneticAlgorithm &setStagnationLimit(long);
    GeneticAlgorithm &setIsWithReplacement(bool);
    GeneticAlgorithm &setSelectionProbability(double);
    GeneticAlgorithm &setCrossProbability(double);
    GeneticAlgorithm &setMatingAttemptsPerIteration(int);
    GeneticAlgorithm &setMutationProbabiliy(double);
    GeneticAlgorithm &setAdaptiveMutation(bool enabled);
    GeneticAlgorithm &setDMutationProbability(double);
    GeneticAlgorithm &setMutationType(MutationType);
    GeneticAlgorithm &setRotationType(ROTATION_WAY);
    GeneticAlgorithm &setMaxIteration(long);
    // Parent selection requires at least two individuals.
    GeneticAlgorithm &setNumberOfIndividuals(long);
    GeneticAlgorithm &setProblem(DatasetBinBacking);

    Poblacion evolve();
};

#endif
