#ifndef ARTIFICIAL_BEE_COLONY_ALGORITHM_H
#define ARTIFICIAL_BEE_COLONY_ALGORITHM_H

#include "../genetic/FitnessOperators.hpp"
#include "../genetic/MutationType.hpp"
#include "../genetic/GeneticsOperators.hpp"
#include "../genetic/PermutationOperators.hpp"
#include "../readData/DataSetBPP.hpp"
#include "EvolutiveAlgorithm.hpp"

class ArtificialBeeColonyAlgorithm : public EvolutiveAlgorithm
{
private:
    std::map<std::string, bool> visitedIndividuals;

    long maxIteration;
    int numberOfIndividuals;
    int numberOfSites;
    int numberOfEliteSites;
    int numberOfEliteBees;
    int numberOfNonEliteBees;

    bool isWithReplacement;

    double mutationProbaility;
    double dMutationProbability;

    DatasetBinBacking dataSet;
    MutationType mutationType;
    ROTATION_WAY rotationType;

    void globalSearch(Poblacion &, int);
    void neighorhoodSearch(Poblacion &, int);
    void orderSites(Poblacion &);

public:
    ArtificialBeeColonyAlgorithm();
    ~ArtificialBeeColonyAlgorithm();
    static ArtificialBeeColonyAlgorithm Build();

    ArtificialBeeColonyAlgorithm &setIsWithReplacement(bool);

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

    Poblacion evolve();
};

#endif