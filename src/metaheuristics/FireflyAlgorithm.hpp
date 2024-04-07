
#ifndef FIREFLY_ALGORITHM_H
#define FIREFLY_ALGORITHM_H

#include "../genetic/FitnessOperators.hpp"
#include "../genetic/MutationType.hpp"
#include "../genetic/GeneticsOperators.hpp"
#include "../genetic/PermutationOperators.hpp"
#include "../readData/DataSetBPP.hpp"
#include "EvolutiveAlgorithm.hpp"

class FireflyAlgorithm : public EvolutiveAlgorithm
{
private:
    std::map<std::string, bool> visitedIndividuals;

    long maxIteration;
    int numberOfIndividuals;
    int upIndex;

    double mutationProbaility;
    double dMutationProbability;

    bool isWithReplacement;

    DatasetBinBacking dataSet;
    MutationType mutationType;
    ROTATION_WAY rotationType;

    double lightIntensity(double, double, int);
    void findMostAttractiveByIndexAndIntensity(Poblacion, int, int &, int &, double);
    Poblacion search();
    Poblacion searchD();

    DoubleGenome randomMovement(DoubleGenome);
    void handleFireflyIntoPoblation(Individuo, Poblacion &, int);
    void orderFireFlyPoblation(Poblacion &);

public:
    FireflyAlgorithm();
    ~FireflyAlgorithm();
    static FireflyAlgorithm Build();

    FireflyAlgorithm &setIsWithReplacement(bool);

    FireflyAlgorithm &setMaxIteration(long);

    FireflyAlgorithm &setNumberOfIndividuals(int);

    FireflyAlgorithm &setupIndex(int);

    FireflyAlgorithm &setMutationProbabiliy(double);
    FireflyAlgorithm &setDMutationProbability(double);

    FireflyAlgorithm &setMutationType(MutationType);
    FireflyAlgorithm &setRotationType(ROTATION_WAY);

    FireflyAlgorithm &setProblem(DatasetBinBacking);

    Individuo &findMostAttractiveFirefly(Poblacion &, Individuo, double);

    Poblacion evolve();
};

#endif