#include "ArtificialBeeColonyAlgorithm.hpp"

Poblacion ArtificialBeeColonyAlgorithm::evolve()
{
    int iteration, eliteSite, noneEliteSite, eliteBee, nonEliteBee, restOfSites;

    Poblacion sites =
        initialHeuristicPoblation(numberOfIndividuals, dataSet.bin, dataSet.totalItems, rotationType);

    for (auto &site : sites)
    {
        visitedIndividuals[site.solution] = true;
    }

    for (iteration = 0; iteration < maxIteration; iteration++)
    {
        // Select elite sites
        for (eliteSite = 0; eliteSite < numberOfEliteSites; eliteSite++)
        {
            for (eliteBee = 0; eliteBee < numberOfEliteBees; eliteBee++)
            {
                neighorhoodSearch(sites, eliteSite);
            }
        }

        // Select non-elite sites
        for (noneEliteSite = numberOfEliteSites; noneEliteSite < numberOfSites; noneEliteSite++)
        {
            for (nonEliteBee = 0; nonEliteBee < numberOfNonEliteBees; nonEliteBee++)
            {
                neighorhoodSearch(sites, noneEliteSite);
            }
        }

        // Random all rest of sites
        for (restOfSites = numberOfSites; restOfSites < numberOfIndividuals; restOfSites++)
        {
            globalSearch(sites, restOfSites);
        }

        orderSites(sites);

        if (sites[0].getFitness() == 1)
        {
            break;
        }
    }

    return sites;
}

void ArtificialBeeColonyAlgorithm::orderSites(Poblacion &sites)
{
    rankPoblation(sites);

    if (!isWithReplacement)
    {
        sites.erase(sites.begin() + numberOfIndividuals, sites.end());
    }
}

void ArtificialBeeColonyAlgorithm::globalSearch(Poblacion &sites, int site)
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
        sites[site] = newSite;
    }
    else
    {
        if (!visitedIndividuals[newSite.solution])
        {
            visitedIndividuals[newSite.solution] = true;
            sites.push_back(newSite);
        }
    }
}

void ArtificialBeeColonyAlgorithm::neighorhoodSearch(Poblacion &sites, int site)
{
    DoubleGenome siteGenome = sites[site].getGenome();

    mutationWithType(siteGenome, mutationType);

    if (rotationType != ROTATION_WAY::ZERO_WAY)
    {
        flipMutation(siteGenome, dMutationProbability, rotationType);
    }

    Individuo newSite = Individuo::Build().setGenome(siteGenome);

    evaluateFitness(newSite, dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

    if (newSite.getFitness() > sites[site].getFitness())
    {
        if (isWithReplacement)
        {
            sites[site] = newSite;
        }
        else
        {
            if (!visitedIndividuals[newSite.solution])
            {
                visitedIndividuals[newSite.solution] = true;
                sites.push_back(newSite);
            }
        }
    }
}

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

ArtificialBeeColonyAlgorithm &ArtificialBeeColonyAlgorithm::setIsWithReplacement(bool isWithReplacement)
{
    this->isWithReplacement = isWithReplacement;
    return *this;
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