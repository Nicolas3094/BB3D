#ifndef ARTIFICIAL_BEE_COLONY_ALGORITHM_H
#define ARTIFICIAL_BEE_COLONY_ALGORITHM_H

#include "../genetic/FitnessOperators.hpp"
#include "../genetic/MutationType.hpp"
#include "../genetic/GeneticsOperators.hpp"
#include "../genetic/PermutationOperators.hpp"
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

    void globalSearch(Poblacion &, int, bool);
    void neighorhoodSearch(Poblacion &, int, bool);
    Poblacion search(bool);

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

    Poblacion searchWithReplacement();
    Poblacion searchWithAdded();
};

#endif