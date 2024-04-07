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

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setDMutationProbability(double dMutationProbability)
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

Poblacion ArtificialBeeColonyAlgorithm::search(bool isWithReplacement)
{
    int numberOfBees, iteration, eliteSite, noneEliteSite, eliteBee, nonEliteBee, restOfSites;

    Poblacion colonyWorker =
        initialHeuristicPoblation(numberOfIndividuals, dataSet.bin, dataSet.totalItems, rotationType);

    for (iteration = 0; iteration < maxIteration; iteration++)
    {
        // Select elite sites
        for (eliteSite = 0; eliteSite < numberOfEliteSites; eliteSite++)
        {
            for (eliteBee = 0; eliteBee < numberOfEliteBees; eliteBee++)
            {
                neighorhoodSearch(colonyWorker, eliteSite, isWithReplacement);
            }
        }

        // Select non-elite sites
        for (noneEliteSite = numberOfEliteSites; noneEliteSite < numberOfSites; noneEliteSite++)
        {
            for (nonEliteBee = 0; nonEliteBee < numberOfNonEliteBees; nonEliteBee++)
            {
                neighorhoodSearch(colonyWorker, noneEliteSite, isWithReplacement);
            }
        }

        // Random all rest of sites
        for (restOfSites = numberOfSites; restOfSites < colonyWorker.size(); restOfSites++)
        {
            globalSearch(colonyWorker, restOfSites, isWithReplacement);
        }

        if (!isWithReplacement)
        {
            colonyWorker.erase(colonyWorker.begin() + numberOfIndividuals, colonyWorker.end());
        }

        rankPoblation(colonyWorker);

        if (colonyWorker[0].getFitness() == 1)
        {
            break;
        }
    }

    return colonyWorker;
}

Poblacion ArtificialBeeColonyAlgorithm::evolveWithReplacement()
{
    return search(/* isWithReplacement= */ true);
}

Poblacion ArtificialBeeColonyAlgorithm::evolveWithAdded()
{
    return search(/* isWithReplacement= */ false);
}

void ArtificialBeeColonyAlgorithm::globalSearch(Poblacion &colony, int site, bool isWithReplacement)
{
    Individuo newSite = Individuo::Build()
                            .setGenome(DoubleGenome::Build()
                                           .setGenome(generatePermutation(dataSet.totalItems.size()))
                                           .setDGenome(
                                               generateRandomRepeatedAlalleleChromosome(
                                                   /* NumberCount= */ dataSet.totalItems.size(),
                                                   /* minimum= */ 1,
                                                   /* maximum= */ getIdFromRotationWay(rotationType))));

    evaluateFitness(newSite, dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

    if (isWithReplacement)
    {
        colony[site] = newSite;
    }
    else
    {
        colony.push_back(newSite);
    }
}

void ArtificialBeeColonyAlgorithm::neighorhoodSearch(Poblacion &colony, int bee, bool isWithReplacement)
{
    DoubleGenome beeGenome = colony[bee].getGenome();

    mutationWithType(beeGenome, mutationType);

    if (rotationType != ROTATION_WAY::ZERO_WAY)
    {
        flipMutation(beeGenome, dMutationProbability, rotationType);
    }

    Individuo newBee = Individuo::Build().setGenome(beeGenome);

    evaluateFitness(newBee, dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

    if (newBee.getFitness() > colony[bee].getFitness())
    {
        if (isWithReplacement)
        {
            colony[bee] = newBee;
        }
        else
        {
            colony.push_back(newBee);
        }
    }
}