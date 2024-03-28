
#ifndef FIREFLY_ALGORITHM_H
#define FIREFLY_ALGORITHM_H

#include "../genetic/FitnessOperators.hpp"
#include "../genetic/MutationType.hpp"
#include "../genetic/GeneticsOperators.hpp"
#include "../genetic/PermutationOperators.hpp"
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

    double lightIntensity(double, double, int);
    void findMostAttractiveByIndexAndIntensity(Poblacion, int, int &, int &, double);
    Poblacion search(bool);

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

    Poblacion searchWithReplacement();
    Poblacion searchWithAdded();
};

#endif