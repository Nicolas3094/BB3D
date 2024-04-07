#include "FireflyAlgorithm.hpp"

Poblacion FireflyAlgorithm::evolve()
{
    return search();
}

Poblacion FireflyAlgorithm::search()
{
    int n, numberFireflies, iteration, firefly, mostAttactiveFirefly, bestFirefly, distance;
    double gamma, bestIntensity, Ii, Ij;

    Poblacion fireflyPoblation =
        initialHeuristicPoblation(numberOfIndividuals, dataSet.bin, dataSet.totalItems, rotationType);

    for (auto &firefly : fireflyPoblation)
    {
        visitedIndividuals[firefly.solution] = true;
    }

    n = fireflyPoblation[0].getGenome().getGenome().size();

    gamma = 1 / (double)n;

    for (iteration = 0; iteration < this->maxIteration; iteration++)
    {
        for (firefly = 0; firefly < numberOfIndividuals; firefly++)
        {
            mostAttactiveFirefly = -1;
            bestIntensity = -1.0;

            // Find most atrractive firefly
            findMostAttractiveByIndexAndIntensity(fireflyPoblation, firefly, mostAttactiveFirefly, bestFirefly, gamma);

            for (int k = 0; k < upIndex; k++)
            {
                Individuo newPositionFirefly;

                if (bestIntensity == -1)
                {
                    newPositionFirefly.setGenome(randomMovement(fireflyPoblation[firefly].getGenome()));
                }
                else
                {
                    // Se arcercan.
                    newPositionFirefly.setGenome(
                        randomMovement(
                            crossOver(
                                fireflyPoblation[firefly].getGenome(), fireflyPoblation[bestFirefly].getGenome())));
                }

                evaluateFitness(newPositionFirefly, dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

                handleFireflyIntoPoblation(newPositionFirefly, fireflyPoblation, firefly);
            }
        }

        orderFireFlyPoblation(fireflyPoblation);

        if (fireflyPoblation[0].getFitness() == 1)
        {
            break;
        }
    }

    return fireflyPoblation;
}

void FireflyAlgorithm::findMostAttractiveByIndexAndIntensity(
    Poblacion fireflyPoblation, int currentFirefly, int &bestFireflyIndex, int &bestFireflyIntensity, double gamma)
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

DoubleGenome FireflyAlgorithm::randomMovement(DoubleGenome currentFireflyGenome)
{
    DoubleGenome newFireflyGen = currentFireflyGenome;

    mutationWithType(newFireflyGen, mutationType);

    if (rotationType != ROTATION_WAY::ZERO_WAY)
    {
        flipMutation(newFireflyGen, dMutationProbability, rotationType);
    }

    return newFireflyGen;
}

Poblacion FireflyAlgorithm::searchD()
{
    int n, numberFireflies, iteration, fireflyIndex;
    double gamma, bestIntensity, Ii, Ij;
    Individuo *mostAttactiveFirefly;

    Poblacion fireflyPoblation =
        initialHeuristicPoblation(numberOfIndividuals, dataSet.bin, dataSet.totalItems, rotationType);

    for (auto &firefly : fireflyPoblation)
    {
        visitedIndividuals[firefly.solution] = true;
    }

    n = fireflyPoblation[0].getGenome().getGenome().size();

    gamma = 1 / (double)n;

    for (iteration = 0; iteration < this->maxIteration; iteration++)
    {
        for (fireflyIndex = 0; fireflyIndex < numberOfIndividuals; fireflyIndex++)
        {

            *mostAttactiveFirefly = findMostAttractiveFirefly(fireflyPoblation, fireflyPoblation[fireflyIndex], gamma);

            for (int k = 0; k < upIndex; k++)
            {
                Individuo newPositionFirefly;

                // If there is no most attractive firefly, just move randomly.
                if (mostAttactiveFirefly == nullptr)
                {
                    newPositionFirefly.setGenome(randomMovement(fireflyPoblation[fireflyIndex].getGenome()));
                }
                // Otherwise, moves closer to the most attractive firefly and then, perform a random movement.
                else
                {
                    newPositionFirefly.setGenome(
                        randomMovement(
                            crossOver(fireflyPoblation[fireflyIndex].getGenome(), mostAttactiveFirefly->getGenome())));
                }

                evaluateFitness(newPositionFirefly, dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

                handleFireflyIntoPoblation(newPositionFirefly, fireflyPoblation, fireflyIndex);
            }
        }

        orderFireFlyPoblation(fireflyPoblation);

        if (fireflyPoblation[0].getFitness() == 1)
        {
            break;
        }
    }

    return fireflyPoblation;
}

void FireflyAlgorithm::handleFireflyIntoPoblation(
    Individuo newPositionFirefly, Poblacion &fireflyPoblation, int currentFireflyIndex)
{
    if (isWithReplacement)
    {
        if (newPositionFirefly.getFitness() > fireflyPoblation[currentFireflyIndex].getFitness())
        {
            fireflyPoblation[currentFireflyIndex] = newPositionFirefly;
        }
    }
    else
    {
        if (!visitedIndividuals[newPositionFirefly.solution])
        {
            visitedIndividuals[newPositionFirefly.solution] = true;
            fireflyPoblation.push_back(newPositionFirefly);
        }
    }
}

void FireflyAlgorithm::orderFireFlyPoblation(Poblacion &fireflyPoblation)
{
    rankPoblation(fireflyPoblation);

    if (!isWithReplacement)
    {
        fireflyPoblation.erase(fireflyPoblation.begin() + numberOfIndividuals, fireflyPoblation.end());
    }
}

Individuo &FireflyAlgorithm::findMostAttractiveFirefly(
    Poblacion &fireflyPoblation, Individuo currentFirefly, double gamma)
{
    double lightIntensityCurrentFireFly, lightIntensityFirefly;
    int distance;

    Individuo *mostAttractive = nullptr;

    for (Individuo &firefly : fireflyPoblation)
    {
        distance = hamming(currentFirefly.getGenome(), firefly.getGenome());

        lightIntensityCurrentFireFly = lightIntensity(currentFirefly.getFitness(), gamma, distance);
        lightIntensityFirefly = lightIntensity(firefly.getFitness(), gamma, distance);

        // Si su distancia es 0, implica que es la misma solucion o si la luz de la libelula es menor o igual
        //  que la actual, no hacer nada.
        if (distance == 0 || lightIntensityFirefly <= lightIntensityCurrentFireFly)
        {
            continue;
        }

        if (lightIntensityCurrentFireFly)

            if (mostAttractive == nullptr)
            {
                mostAttractive = &firefly;
            }
            else
            {
                if (lightIntensity(mostAttractive->getFitness(), gamma, distance) < lightIntensityFirefly)
                {
                    mostAttractive = &firefly;
                }
            }
    }

    return *mostAttractive;
}

double FireflyAlgorithm::lightIntensity(double fitness, double gamma, int distance)
{
    return fitness / (1 + gamma * ((double)distance * distance));
}

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

FireflyAlgorithm &FireflyAlgorithm::setIsWithReplacement(bool isWithReplacement)
{
    this->isWithReplacement = isWithReplacement;
    return *this;
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

FireflyAlgorithm &FireflyAlgorithm::setDMutationProbability(double dMutationProbability)
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