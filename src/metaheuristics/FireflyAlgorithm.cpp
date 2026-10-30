#include "FireflyAlgorithm.hpp"
#include "SearchValidation.hpp"
#include "../util/RandomNumbers.hpp"

Poblacion FireflyAlgorithm::evolve()
{
    return search();
}

Poblacion FireflyAlgorithm::search()
{
    validateConfiguration();
    visitedIndividuals.clear();
    int n, firefly, bestFirefly;
    long iteration;
    double gamma, bestIntensity;

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
        checkExecutionDeadline();

        for (firefly = 0; firefly < numberOfIndividuals; firefly++)
        {
            checkExecutionDeadline();
            bestFirefly = -1;
            bestIntensity = -1.0;

            // Find most atrractive firefly
            findMostAttractiveByIndexAndIntensity(fireflyPoblation, firefly, bestFirefly, bestIntensity,
                                                  gamma);

            for (int k = 0; k < upIndex; k++)
            {
                Individuo newPositionFirefly;

                if (bestFirefly == -1)
                {
                    newPositionFirefly.setGenome(randomMovement(fireflyPoblation[firefly].getGenome()));
                }
                else
                {
                    // Se arcercan.
                    newPositionFirefly.setGenome(randomMovement(crossOver(
                        fireflyPoblation[firefly].getGenome(), fireflyPoblation[bestFirefly].getGenome())));
                }

                evaluateFitness(newPositionFirefly, dataSet.totalItems,
                                dataSet.bin.setRotationWay(rotationType));

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

void FireflyAlgorithm::findMostAttractiveByIndexAndIntensity(const Poblacion &fireflyPoblation,
                                                             int currentFirefly, int &bestFireflyIndex,
                                                             double &bestFireflyIntensity, double gamma)
{
    int bestFirefly, distance;
    double Ij, Ii;

    for (bestFirefly = 0; bestFirefly < this->numberOfIndividuals; bestFirefly++)
    {
        checkExecutionDeadline();

        if (currentFirefly == bestFirefly)
        {
            continue;
        }

        distance =
            hamming(fireflyPoblation[currentFirefly].getGenome(), fireflyPoblation[bestFirefly].getGenome());

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

    if (mutationProbaility == 1 || (mutationProbaility > 0 && uniformUnit() < mutationProbaility))
        mutationWithType(newFireflyGen, mutationType);

    if (rotationType != ROTATION_WAY::ZERO_WAY)
    {
        flipMutation(newFireflyGen, dMutationProbability, rotationType);
    }

    return newFireflyGen;
}

Poblacion FireflyAlgorithm::searchD()
{
    validateConfiguration();
    visitedIndividuals.clear();
    int n, fireflyIndex;
    long iteration;
    double gamma;

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
        checkExecutionDeadline();

        for (fireflyIndex = 0; fireflyIndex < numberOfIndividuals; fireflyIndex++)
        {
            checkExecutionDeadline();

            Individuo *mostAttactiveFirefly =
                findMostAttractiveFirefly(fireflyPoblation, fireflyPoblation[fireflyIndex], gamma);
            const bool hasTarget = mostAttactiveFirefly != nullptr;
            // Adding children may reallocate the population. Keep an owned genome
            // before entering the loop rather than retaining a pointer into it.
            DoubleGenome targetGenome;

            if (hasTarget)
            {
                targetGenome = mostAttactiveFirefly->getGenome();
            }

            for (int k = 0; k < upIndex; k++)
            {
                Individuo newPositionFirefly;

                // If there is no most attractive firefly, just move randomly.
                if (!hasTarget)
                {
                    newPositionFirefly.setGenome(randomMovement(fireflyPoblation[fireflyIndex].getGenome()));
                }
                // Otherwise, moves closer to the most attractive firefly and then, perform a random movement.
                else
                {
                    newPositionFirefly.setGenome(
                        randomMovement(crossOver(fireflyPoblation[fireflyIndex].getGenome(), targetGenome)));
                }

                evaluateFitness(newPositionFirefly, dataSet.totalItems,
                                dataSet.bin.setRotationWay(rotationType));

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

void FireflyAlgorithm::handleFireflyIntoPoblation(Individuo newPositionFirefly, Poblacion &fireflyPoblation,
                                                  int currentFireflyIndex)
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

Individuo *FireflyAlgorithm::findMostAttractiveFirefly(Poblacion &fireflyPoblation, Individuo currentFirefly,
                                                       double gamma)
{
    double lightIntensityCurrentFireFly, lightIntensityFirefly;
    int distance;

    Individuo *mostAttractive = nullptr;
    double bestIntensity = -1.0;

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

        // Retain the previous candidate's actual intensity at its own distance.
        // Zero current intensity is valid and must still allow improvement.
        if (lightIntensityFirefly > bestIntensity)
        {
            mostAttractive = &firefly;
            bestIntensity = lightIntensityFirefly;
        }
    }

    return mostAttractive;
}

double FireflyAlgorithm::lightIntensity(double fitness, double gamma, int distance)
{
    return fitness / (1 + gamma * ((double)distance * distance));
}

FireflyAlgorithm::FireflyAlgorithm()
{
    maxIteration = 1000;
    numberOfIndividuals = 25;
    upIndex = 2;
    isWithReplacement = true;
    mutationType = MutationType::INVERSE_MUTATION;
    rotationType = ROTATION_WAY::ZERO_WAY;
    this->dMutationProbability = 0.01;
    this->mutationProbaility = 1.0;
}

FireflyAlgorithm::~FireflyAlgorithm() {}

void FireflyAlgorithm::validateConfiguration() const
{
    validateSearch(dataSet, numberOfIndividuals, maxIteration, mutationProbaility, dMutationProbability,
                   mutationType, rotationType);
    if (upIndex < 0)
        throw std::invalid_argument("Firefly movement count must be non-negative.");
}

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
