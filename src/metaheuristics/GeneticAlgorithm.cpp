#include "GeneticAlgorithm.hpp"

GeneticAlgorithm::GeneticAlgorithm()
{
    this->maxIteration = 0;
    this->selectionProbability = 0.0;
    this->crossProability = 0.0;
    this->mutationProbaility = 0.0;
    this->dMutationProbability = 0.01;
    this->mutationType = MutationType::INVERSE_MUTATION;
    this->rotationType = ROTATION_WAY::ZERO_WAY;
}
GeneticAlgorithm::~GeneticAlgorithm()
{
}

GeneticAlgorithm GeneticAlgorithm::Build()
{
    GeneticAlgorithm g;
    return g;
}

GeneticAlgorithm &GeneticAlgorithm::setSelectionProbability(double selectionProbability)
{
    this->selectionProbability = selectionProbability;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setCrossProbability(double crossProability)
{
    this->crossProability = crossProability;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setMutationProbabiliy(double mutationProbaility)
{
    this->mutationProbaility = mutationProbaility;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setDMutationProbability(double dMutationProbability)
{
    this->dMutationProbability = dMutationProbability;
    return *this;
}

GeneticAlgorithm &GeneticAlgorithm::setMutationType(MutationType mutationType)
{
    this->mutationType = mutationType;
    return *this;
}

GeneticAlgorithm &GeneticAlgorithm::setRotationType(ROTATION_WAY rotationType)
{
    this->rotationType = rotationType;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setMaxIteration(long maxIteration)
{
    this->maxIteration = maxIteration;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setProblem(DatasetBinBacking dataset)
{
    this->dataSet = dataset;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setNumberOfIndividuals(long numberOfIndividuals)
{
    this->numberOfIndividuals = numberOfIndividuals;
    return *this;
}

// O(n)
void GeneticAlgorithm::mutate(Individuo &individual, int step)
{

    if (rotationType != ROTATION_WAY::ZERO_WAY)
    {
        flipMutationO1(individual.getGenome(), dMutationProbability, rotationType);
    }
    mutationWithType(individual.getGenome(), mutationType, step);
}

// O(n)
Individuo GeneticAlgorithm::crossover(Individuo parent1, Individuo parent2)
{
    int n, init, end;

    n = parent1.getGenome().getGenome().size();
    init = randomInteger(3, (n / 2) - 1);
    end = randomInteger(init + 1, n - 1);

    return Individuo::Build()
        .setGenome(crossOx(parent1.getGenome(), parent2.getGenome(), init, end))
        .setFitness(0.0);
}

// O(n**4)
Individuo GeneticAlgorithm::instantiateChild(Individuo parent1, Individuo parent2)
{

    Individuo child = crossover(parent1, parent2);

    if (uniformUnit() < adaptiveMutationProbabiliy(parent1, parent2))
    {
        mutate(child);
    }

    evaluateFitness(child, dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

    return child;
}

void GeneticAlgorithm::nextGeneration(Poblacion &poblation)
{
    int numberOfDeletedIndividual, initialIndex, i;

    numberOfDeletedIndividual = randomInteger(numberOfIndividuals / 4, numberOfIndividuals / 2);

    initialIndex = numberOfIndividuals - numberOfDeletedIndividual - 1;

    // Delete from i = initialIndex -> ns
    for (i = initialIndex; i < numberOfIndividuals; i++)
    {
        // Tournament from i = 0 -> initialIndex - 1
        std::vector<Individuo> parents = getParents(poblation, initialIndex - 1);

        if (uniformUnit() <= crossProability)
        {
            std::vector<Individuo> children = getChildren(parents[0], parents[1]);

            if (!visitedIndividuals[children[0].solution] &&
                visitedIndividuals[children[1].solution])
            {
                visitedIndividuals[children[0].solution] = true;

                poblation[i] = children[0];
            }
            else if (visitedIndividuals[children[0].solution] &&
                     !visitedIndividuals[children[1].solution])
            {
                visitedIndividuals[children[1].solution] = true;

                poblation[i] = children[1];
            }
            else if (!visitedIndividuals[children[0].solution] &&
                     !visitedIndividuals[children[1].solution])
            {
                visitedIndividuals[children[0].solution] = true;
                visitedIndividuals[children[1].solution] = true;

                if (children[0].getFitness() > children[1].getFitness())
                {
                    poblation[i] = children[0];
                }
                else
                {
                    poblation[i] = children[1];
                }
            }
        }
    }

    rankPoblation(poblation);
}

void GeneticAlgorithm::nextGenerationByAdding(Poblacion &poblation)
{
    int numberOfAddedIndividuals, i;

    numberOfAddedIndividuals = randomInteger(numberOfIndividuals / 4, numberOfIndividuals / 2);

    for (i = 0; i < numberOfAddedIndividuals; i++)
    {
        std::vector<Individuo> parents = getParents(poblation, numberOfAddedIndividuals);

        if (uniformUnit() <= crossProability)
        {
            std::vector<Individuo> children = getChildren(parents[0], parents[1]);

            if (!visitedIndividuals[children[0].solution] &&
                visitedIndividuals[children[1].solution])
            {
                visitedIndividuals[children[0].solution] = true;
                poblation.push_back(children[0]);
            }
            else if (visitedIndividuals[children[0].solution] &&
                     !visitedIndividuals[children[1].solution])
            {
                visitedIndividuals[children[1].solution] = true;
                poblation.push_back(children[1]);
            }
            else if (!visitedIndividuals[children[0].solution] &&
                     !visitedIndividuals[children[1].solution])
            {
                visitedIndividuals[children[0].solution] = true;
                poblation.push_back(children[0]);

                visitedIndividuals[children[1].solution] = true;
                poblation.push_back(children[1]);
            }
        }
    }

    rankPoblation(poblation);

    poblation.erase(poblation.begin() + numberOfIndividuals, poblation.end());
}

Poblacion GeneticAlgorithm::evolve()
{
    int i;
    Poblacion poblation = initialHeuristicPoblation(numberOfIndividuals,
                                                    dataSet.bin,
                                                    dataSet.totalItems,
                                                    rotationType);

    for (auto &ind : poblation)
    {
        visitedIndividuals[ind.solution] = true;
    }

    for (i = 0; i < maxIteration; i++)
    {
        nextGeneration(poblation);

        if (terminateCondition(poblation))
        {
            break;
        }
    }
    return poblation;
}

Poblacion GeneticAlgorithm::evolveWithAdded()
{
    int i;
    Poblacion poblation = initialHeuristicPoblation(numberOfIndividuals,
                                                    dataSet.bin,
                                                    dataSet.totalItems,
                                                    rotationType);

    for (auto &ind : poblation)
    {
        visitedIndividuals[ind.solution] = true;
    }

    for (i = 0; i < maxIteration; i++)
    {
        nextGenerationByAdding(poblation);

        if (terminateCondition(poblation))
        {
            break;
        }
    }
    return poblation;
}

bool GeneticAlgorithm::terminateCondition(Poblacion currentPoblation)
{
    return currentPoblation[0].getFitness() == 1 ||
           (currentPoblation[0].getFitness() -
            currentPoblation[numberOfIndividuals - 1].getFitness()) /
                   (currentPoblation[0].getFitness() * currentPoblation[0].getFitness()) <=
               0.001;
}

double GeneticAlgorithm::adaptiveMutationProbabiliy(Individuo individual, Individuo otherIndividual)
{
    return 1.0 - (individual.getFitness() + otherIndividual.getFitness()) / 2.0;
}

void GeneticAlgorithm::mutatePoblation(Poblacion &poblation, int initIndex, int endIndex)
{
    for (int i = endIndex; i == initIndex; i--)
    {
        if (uniformUnit() <= adaptiveMutationProbabiliy(poblation[i], poblation[0]))
        {
            mutate(poblation[i]);

            if (visitedIndividuals[poblation[i].solution])
            {
                poblation[i].setFitness(0.0);
            }
            else
            {
                evaluateFitness(poblation[i],
                                dataSet.totalItems,
                                dataSet.bin.setRotationWay(rotationType));
                visitedIndividuals[poblation[i].solution] = true;
            }
        }
    }
}

std::vector<Individuo> GeneticAlgorithm::getParents(Poblacion poblation, int maxIndexOfPoblation)
{
    int indexParent, indexOtherParent;

    std::vector<Individuo> parents(2);

    indexParent = tournament(poblation, maxIndexOfPoblation, selectionProbability);
    indexOtherParent = tournament(poblation, maxIndexOfPoblation, selectionProbability);

    while (indexParent == indexOtherParent)
    {
        indexOtherParent = tournament(poblation, maxIndexOfPoblation, selectionProbability);
    }

    parents[0] = poblation[indexParent];
    parents[1] = poblation[indexOtherParent];

    return parents;
}

std::vector<Individuo> GeneticAlgorithm::getChildren(Individuo parent, Individuo otherParent)
{
    std::vector<Individuo> children(2);

    children[0] = crossover(parent, otherParent);
    children[1] = crossover(otherParent, parent);

    if (uniformUnit() <= adaptiveMutationProbabiliy(parent, otherParent))
    {
        mutate(children[0]);
    }

    if (uniformUnit() <= adaptiveMutationProbabiliy(parent, otherParent))
    {
        mutate(children[1]);
    }

    evaluateFitness(children[0], dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));
    evaluateFitness(children[1], dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

    return children;
}
