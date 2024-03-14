#include "ArtificialBeeColonyAlgorithm.hpp"

ArtificialBeeColonyAlgorithm::ArtificialBeeColonyAlgorithm()
{
    this->dMutationProbability = 0.01;
    this->mutationProbaility = 1.0;
}
ArtificialBeeColonyAlgorithm::~ArtificialBeeColonyAlgorithm() {}

ArtificialBeeColonyAlgorithm ArtificialBeeColonyAlgorithm::Build()
{
    ArtificialBeeColonyAlgorithm algorithm;
    return algorithm;
}
ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setMaxIteration(long maxIteration)
{
    this->maxIteration = maxIteration;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setNumberOfSites(int numberOfSites)
{
    this->numberOfSites = numberOfSites;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setNumberOfEliteSites(int numberOfEliteSites)
{
    this->numberOfEliteSites = numberOfEliteSites;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setNumberOfEliteBees(int numberOfEliteBees)
{
    this->numberOfEliteBees = numberOfEliteBees;
    return *this;
}
ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setNumberOfNonEliteBees(int numberOfNonEliteBees)
{
    this->numberOfNonEliteBees = numberOfNonEliteBees;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setMutationProbabiliy(double mutationProbaility)
{
    this->mutationProbaility = mutationProbaility;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setDMutationProbability(double mutationProbaility)
{
    this->dMutationProbability = dMutationProbability;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setMutationType(MutationType mutationType)
{

    this->mutationType = mutationType;
    return *this;
}
ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setRotationType(ROTATION_WAY rotationType)
{
    this->rotationType = rotationType;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setProblem(DatasetBinBacking dataSet)
{
    this->dataSet = dataSet;
    return *this;
}

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setNumberOfIndividuals(int numberOfIndividuals)
{
    this->numberOfIndividuals = numberOfIndividuals;
    return *this;
}

Poblacion ArtificialBeeColonyAlgorithm::search()
{
    int numberOfBees, iteration, eliteSite, noneEliteSite, eliteBee, nonEliteBee, restOfSites;

    Poblacion colonyWorker = initialPoblation();

    for (iteration = 0; iteration < this->maxIteration; iteration++)
    {
        // Select elite sites
        for (eliteSite = 0; eliteSite < this->numberOfEliteSites; eliteSite++)
        {
            for (eliteBee = 0; eliteBee < this->numberOfEliteBees; eliteBee++)
            {
                neighorhoodSearch(colonyWorker, eliteSite);
            }
        }

        // Select non-elite sites
        for (noneEliteSite = this->numberOfEliteSites; noneEliteSite < this->numberOfSites; noneEliteSite++)
        {
            for (nonEliteBee = 0; nonEliteBee < this->numberOfNonEliteBees; nonEliteBee++)
            {
                neighorhoodSearch(colonyWorker, noneEliteSite);
            }
        }

        // Random all rest of sites
        for (restOfSites = this->numberOfSites; restOfSites < colonyWorker.size(); restOfSites++)
        {
            globalSearch(colonyWorker, restOfSites);
        }

        rankPoblation(colonyWorker);

        if (colonyWorker[0].getFitness() == 1)
        {
            break;
        }
    }

    return colonyWorker;
}

void ArtificialBeeColonyAlgorithm::globalSearch(Poblacion &colony, int site)
{
    Individuo newSite = Individuo::Build()
                            .setGenome(DoubleGenome::Build()
                                           .setGenome(generatePermutation(this->dataSet.totalItems.size()))
                                           .setDGenome(generateRandomRepeatedAlalleleChromosome(
                                               /* NumberCount= */ this->dataSet.totalItems.size(),
                                               /* minimum= */ 1,
                                               /* maximum= */ getIdFromRotationWay(this->rotationType))));
    evaluateFitness(newSite, this->dataSet.totalItems, this->dataSet.bin);
    colony[site] = newSite;
}

void ArtificialBeeColonyAlgorithm::neighorhoodSearch(Poblacion &colony, int bee)
{
    DoubleGenome beeGenome = colony[bee].getGenome();

    mutationWithType(beeGenome, this->mutationType, this->mutationProbaility);

    if (this->rotationType != ROTATION_WAY::ZERO_WAY)
    {
        flipMutation(beeGenome, this->dMutationProbability, this->rotationType);
    }

    Individuo newBee = Individuo::Build().setGenome(beeGenome);

    evaluateFitness(newBee, this->dataSet.totalItems, this->dataSet.bin);

    if (newBee.getFitness() > colony[bee].getFitness())
    {
        colony[bee] = newBee;
    }
}

Poblacion ArtificialBeeColonyAlgorithm::initialPoblation()
{
    Poblacion heuristicPoblation = buildHeuristicPoblation(
        this->numberOfIndividuals, this->dataSet.bin, this->dataSet.totalItems);

    evaluatePoblation(heuristicPoblation, this->dataSet.totalItems, this->dataSet.bin);

    rankPoblation(heuristicPoblation);

    return heuristicPoblation;
}