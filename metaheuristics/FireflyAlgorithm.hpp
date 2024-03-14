
#ifndef FIREFLY_ALGORITHM_H
#define FIREFLY_ALGORITHM_H

#include "../src/FitnessOperators.hpp"
#include "MutationType.hpp"
#include "GeneticsOperators.hpp"
#include "PermutationOperators.hpp"
#include "../readData/DataSetBPP.hpp"

class FireflyAlgorithm
{
private:
    long maxIteration;
    int numberOfIndividuals;
    int upIndex;

    double mutationProbaility;
    double dMutationProbability;

    DatasetBinBacking dataSet;
    MutationType mutationType;
    ROTATION_WAY rotationType;

    Poblacion initialPoblation();
    double lightIntensity(double, double, int);
    void findMostAttractiveByIndexAndIntensity(Poblacion fireflyPoblation, int currentFirefly, int &bestFireflyIndex, int &bestFireflyIntensity, double gamma);

public:
    FireflyAlgorithm();
    ~FireflyAlgorithm();
    static FireflyAlgorithm Build();

    FireflyAlgorithm &setMaxIteration(long);

    FireflyAlgorithm &setNumberOfIndividuals(int);

    FireflyAlgorithm &setupIndex(int);

    FireflyAlgorithm &setMutationProbabiliy(double);
    FireflyAlgorithm &setDMutationProbability(double);

    FireflyAlgorithm &setMutationType(MutationType);
    FireflyAlgorithm &setRotationType(ROTATION_WAY);

    FireflyAlgorithm &setProblem(DatasetBinBacking);

    Poblacion search();
};

#endif