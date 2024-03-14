
#include "FireflyAlgorithm.hpp"

FireflyAlgorithm::FireflyAlgorithm()
{
    this->dMutationProbability = 0.01;
    this->mutationProbaility = 1.0;
}
FireflyAlgorithm::~FireflyAlgorithm() {}

FireflyAlgorithm FireflyAlgorithm::Build()
{
    FireflyAlgorithm algorithm;
    return algorithm;
}
FireflyAlgorithm &FireflyAlgorithm::setMaxIteration(long maxIteration)
{
    this->maxIteration = maxIteration;
    return *this;
}

FireflyAlgorithm &FireflyAlgorithm::setMutationProbabiliy(double mutationProbaility)
{
    this->mutationProbaility = mutationProbaility;
    return *this;
}

FireflyAlgorithm &FireflyAlgorithm::setDMutationProbability(double mutationProbaility)
{
    this->dMutationProbability = dMutationProbability;
    return *this;
}

FireflyAlgorithm &FireflyAlgorithm::setMutationType(MutationType mutationType)
{

    this->mutationType = mutationType;
    return *this;
}
FireflyAlgorithm &FireflyAlgorithm::setRotationType(ROTATION_WAY rotationType)
{
    this->rotationType = rotationType;
    return *this;
}

FireflyAlgorithm &FireflyAlgorithm::setProblem(DatasetBinBacking dataSet)
{
    this->dataSet = dataSet;
    return *this;
}

FireflyAlgorithm &FireflyAlgorithm::setNumberOfIndividuals(int numberOfIndividuals)
{
    this->numberOfIndividuals = numberOfIndividuals;
    return *this;
}

FireflyAlgorithm &FireflyAlgorithm::setupIndex(int upIndex)
{
    this->upIndex = upIndex;
    return *this;
}

Poblacion FireflyAlgorithm::search()
{
    int n, numberFireflies, iteration, firefly, mostAttactiveFirefly, bestFirefly, distance;
    double gamma, bestIntensity, Ii, Ij;

    Poblacion fireflyPoblation = initialPoblation();

    n = fireflyPoblation[0].getGenome().getGenome().size();
    numberFireflies = fireflyPoblation.size();

    gamma = 1 / (double)n;

    for (iteration = 0; iteration < this->maxIteration; iteration++)
    {
        for (firefly = 0; firefly < numberFireflies; firefly++)
        {
            mostAttactiveFirefly = -1;
            bestIntensity = -1.0;

            // Find most atrractive firefly
            findMostAttractiveByIndexAndIntensity(fireflyPoblation, firefly, mostAttactiveFirefly, bestFirefly, gamma);

            for (int k = 0; k < this->upIndex; k++)
            {
                Individuo newPositionFirefly;
                if (bestIntensity == -1)
                {
                    DoubleGenome newFirefly = fireflyPoblation[firefly].getGenome();
                    mutationWithType(newFirefly, this->mutationType, this->mutationProbaility);

                    if (this->rotationType != ROTATION_WAY::ZERO_WAY)
                    {
                        flipMutation(newFirefly, this->dMutationProbability, this->rotationType);
                    }
                    newPositionFirefly.setGenome(newFirefly);
                }
                else
                {
                    // Se arcercan.
                    DoubleGenome newFirefly = crossOver(fireflyPoblation[firefly].getGenome(), fireflyPoblation[bestFirefly].getGenome());
                    mutationWithType(newFirefly, this->mutationType, this->mutationProbaility);
                    newPositionFirefly.setGenome(newFirefly);
                }
                evaluateFitness(newPositionFirefly, this->dataSet.totalItems, this->dataSet.bin);
                if (newPositionFirefly.getFitness() > fireflyPoblation[firefly].getFitness())
                {
                    fireflyPoblation[firefly] = newPositionFirefly;
                }
            }
        }
        rankPoblation(fireflyPoblation);

        if (fireflyPoblation[0].getFitness() == 1)
        {
            break;
        }
    }

    return fireflyPoblation;
}

void FireflyAlgorithm::findMostAttractiveByIndexAndIntensity(Poblacion fireflyPoblation, int currentFirefly, int &bestFireflyIndex, int &bestFireflyIntensity, double gamma)
{
    int bestFirefly, distance;
    double Ij, Ii;

    for (bestFirefly = 0; bestFirefly < this->numberOfIndividuals; bestFirefly++)
    {
        if (currentFirefly == bestFirefly)
        {
            continue;
        }

        distance = hamming(fireflyPoblation[currentFirefly].getGenome(), fireflyPoblation[bestFirefly].getGenome());
        Ii = lightIntensity(fireflyPoblation[currentFirefly].getFitness(), gamma, distance);
        Ij = lightIntensity(fireflyPoblation[bestFirefly].getFitness(), gamma, distance);

        if (Ij > Ii)
        {
            if (bestFireflyIndex == -1)
            {
                bestFireflyIndex = bestFirefly;
                bestFireflyIntensity = Ij;
            }
            else
            {
                if (bestFireflyIntensity < Ij)
                {
                    bestFireflyIndex = bestFirefly;
                    bestFireflyIntensity = Ij;
                }
            }
        }
    }
}

double FireflyAlgorithm::lightIntensity(double fitness, double gamma, int distance)
{
    return fitness / (1 + gamma * ((double)distance * distance));
}

Poblacion FireflyAlgorithm::initialPoblation()
{
    Poblacion heuristicPoblation = buildHeuristicPoblation(
        this->numberOfIndividuals, this->dataSet.bin, this->dataSet.totalItems);

    evaluatePoblation(heuristicPoblation, this->dataSet.totalItems, this->dataSet.bin);

    rankPoblation(heuristicPoblation);

    return heuristicPoblation;
}