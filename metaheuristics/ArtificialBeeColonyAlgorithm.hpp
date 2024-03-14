#ifndef ARTIFICIAL_BEE_COLONY_ALGORITHM_H
#define ARTIFICIAL_BEE_COLONY_ALGORITHM_H

#include "../src/FitnessOperators.hpp"
#include "MutationType.hpp"
#include "GeneticsOperators.hpp"
#include "PermutationOperators.hpp"
#include "../readData/DataSetBPP.hpp"
class ArtificialBeeColonyAlgorithm
{
private:
    long maxIteration;
    int numberOfIndividuals;
    int numberOfSites;
    int numberOfEliteSites;
    int numberOfEliteBees;
    int numberOfNonEliteBees;

    double mutationProbaility;
    double dMutationProbability;

    DatasetBinBacking dataSet;
    MutationType mutationType;
    ROTATION_WAY rotationType;

    Poblacion initialPoblation();
    void globalSearch(Poblacion &, int);
    void neighorhoodSearch(Poblacion &, int);

public:
    ArtificialBeeColonyAlgorithm();
    ~ArtificialBeeColonyAlgorithm();
    static ArtificialBeeColonyAlgorithm Build();

    ArtificialBeeColonyAlgorithm &setMaxIteration(long);

    ArtificialBeeColonyAlgorithm &setNumberOfIndividuals(int);

    ArtificialBeeColonyAlgorithm &setNumberOfSites(int);
    ArtificialBeeColonyAlgorithm &setNumberOfEliteSites(int);

    ArtificialBeeColonyAlgorithm &setNumberOfEliteBees(int);
    ArtificialBeeColonyAlgorithm &setNumberOfNonEliteBees(int);

    ArtificialBeeColonyAlgorithm &setMutationProbabiliy(double);
    ArtificialBeeColonyAlgorithm &setDMutationProbability(double);

    ArtificialBeeColonyAlgorithm &setMutationType(MutationType);
    ArtificialBeeColonyAlgorithm &setRotationType(ROTATION_WAY);

    ArtificialBeeColonyAlgorithm &setProblem(DatasetBinBacking);

    Poblacion search();
};

#endif