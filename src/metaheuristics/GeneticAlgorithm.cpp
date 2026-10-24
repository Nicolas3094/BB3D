#include "GeneticAlgorithm.hpp"
#include <cmath>
#include <limits>

Poblacion GeneticAlgorithm::evolve()
{
    if (numberOfIndividuals < 4 || numberOfIndividuals > std::numeric_limits<int>::max() || maxIteration < 0)
        throw std::invalid_argument("GA requires at least four individuals and non-negative iterations.");
    for (double probability : {selectionProbability, crossProability, mutationProbaility, dMutationProbability})
        if (!std::isfinite(probability) || probability < 0.0 || probability > 1.0)
            throw std::invalid_argument("GA probabilities must be finite and in [0,1].");
    if (!std::isfinite(maxDiversityIndex) || maxDiversityIndex < 0.0)
        throw std::invalid_argument("GA diversity threshold must be finite and non-negative.");
    visitedIndividuals.clear();
    if (isWithReplacement)
    {
        return evolveWithReplacement();
    }

    return evolveWithAdded();
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
    return Individuo::Build()
        .setGenome(crossOver(parent1.getGenome(), parent2.getGenome()))
        .setFitness(0.0);
}

// O(n^3): dominated by DBLF fitness evaluation for n items.
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

Poblacion GeneticAlgorithm::evolveWithReplacement()
{
    int i;
    Poblacion poblation = initialHeuristicPoblation(numberOfIndividuals, dataSet.bin, dataSet.totalItems, rotationType);

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
    Poblacion poblation = initialHeuristicPoblation(numberOfIndividuals, dataSet.bin, dataSet.totalItems, rotationType);

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
    if (currentPoblation.empty()) throw std::invalid_argument("GA population cannot be empty.");
    return currentPoblation[0].getFitness() == 1 || diversityIndex(currentPoblation) < maxDiversityIndex;
}

double GeneticAlgorithm::diversityIndex(Poblacion currentPoblation)
{
    if (currentPoblation.empty()) throw std::invalid_argument("GA population cannot be empty.");
    const double bestFitness = currentPoblation.front().getFitness();
    const double worstFitness = currentPoblation.back().getFitness();
    if (!std::isfinite(bestFitness) || !std::isfinite(worstFitness) || worstFitness < 0.0 || bestFitness < worstFitness)
        throw std::invalid_argument("GA diversity requires sorted non-negative finite fitness.");
    if (bestFitness == 0.0) return 0.0;
    return ((bestFitness - worstFitness) / bestFitness) / bestFitness;
}

double GeneticAlgorithm::adaptiveMutationProbabiliy(Individuo individual, Individuo otherIndividual)
{
    return 1.0 - (individual.getFitness() + otherIndividual.getFitness()) / 2.0;
}

void GeneticAlgorithm::mutatePoblation(Poblacion &poblation, int initIndex, int endIndex)
{
    for (int i = endIndex; i >= initIndex; i--)
    {
        if (uniformUnit() < adaptiveMutationProbabiliy(poblation[i], poblation[0]))
        {
            mutate(poblation[i]);

            if (visitedIndividuals[poblation[i].solution])
            {
                poblation[i].setFitness(0.0);
            }
            else
            {
                evaluateFitness(poblation[i], dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));
                visitedIndividuals[poblation[i].solution] = true;
            }
        }
    }
}

std::vector<Individuo> GeneticAlgorithm::getParents(const Poblacion &poblation, int eligibleCount)
{
    if (eligibleCount < 2 || static_cast<size_t>(eligibleCount) > poblation.size())
        throw std::invalid_argument("GA requires at least two eligible parents.");
    const int first = tournament(poblation, eligibleCount, selectionProbability);
    const int second = tournament(poblation, eligibleCount, selectionProbability, first);
    return {poblation[first], poblation[second]};
}

std::vector<Individuo> GeneticAlgorithm::getChildren(Individuo parent, Individuo otherParent)
{
    std::vector<Individuo> children(2);

    children[0] = crossover(parent, otherParent);
    children[1] = crossover(otherParent, parent);

    if (uniformUnit() < adaptiveMutationProbabiliy(parent, otherParent))
    {
        mutate(children[0]);
    }

    if (uniformUnit() < adaptiveMutationProbabiliy(parent, otherParent))
    {
        mutate(children[1]);
    }

    evaluateFitness(children[0], dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));
    evaluateFitness(children[1], dataSet.totalItems, dataSet.bin.setRotationWay(rotationType));

    return children;
}

void GeneticAlgorithm::addChildredToPoblation(Poblacion &poblation, Individuo child, Individuo secondChild)
{
    if (!visitedIndividuals[child.solution] && visitedIndividuals[secondChild.solution])
    {
        visitedIndividuals[child.solution] = true;
        poblation.push_back(child);
    }
    else if (visitedIndividuals[child.solution] && !visitedIndividuals[secondChild.solution])
    {
        visitedIndividuals[secondChild.solution] = true;
        poblation.push_back(secondChild);
    }
    else if (!visitedIndividuals[child.solution] && !visitedIndividuals[secondChild.solution])
    {
        visitedIndividuals[child.solution] = true;
        poblation.push_back(child);

        visitedIndividuals[secondChild.solution] = true;
        poblation.push_back(secondChild);
    }
}

void GeneticAlgorithm::replacePoblationWithChildren(
    Poblacion &poblation, Individuo child, Individuo secondChild, int atIndex)
{
    if (!visitedIndividuals[child.solution] && visitedIndividuals[secondChild.solution])
    {
        visitedIndividuals[child.solution] = true;

        if (poblation[atIndex].getFitness() < child.getFitness())
        {
            poblation[atIndex] = child;
        }
    }
    else if (visitedIndividuals[child.solution] && !visitedIndividuals[secondChild.solution])
    {
        visitedIndividuals[secondChild.solution] = true;

        if (poblation[atIndex].getFitness() < secondChild.getFitness())
        {
            poblation[atIndex] = secondChild;
        }
    }
    else if (!visitedIndividuals[child.solution] && !visitedIndividuals[secondChild.solution])
    {
        visitedIndividuals[child.solution] = true;
        visitedIndividuals[secondChild.solution] = true;

        if (child.getFitness() > secondChild.getFitness())
        {
            if (poblation[atIndex].getFitness() < child.getFitness())
            {
                poblation[atIndex] = child;
            }
        }
        else
        {
            if (poblation[atIndex].getFitness() < secondChild.getFitness())
            {
                poblation[atIndex] = secondChild;
            }
        }
    }
}

void GeneticAlgorithm::nextGeneration(Poblacion &poblation)
{
    if (poblation.size() != static_cast<size_t>(numberOfIndividuals) || poblation.size() < 2)
        throw std::invalid_argument("GA generation has an invalid population size.");
    int numberOfDeletedIndividual, initialIndex, i;

    numberOfDeletedIndividual = defaultBinomialDistribution(numberOfIndividuals);

    initialIndex = numberOfIndividuals - numberOfDeletedIndividual;

    Poblacion oldPoblation = poblation;

    for (i = initialIndex; i < numberOfIndividuals; i++)
    {

        std::vector<Individuo> parents = getParents(oldPoblation, static_cast<int>(oldPoblation.size()));

        if (uniformUnit() < crossProability)
        {
            Poblacion childrenPoblation = getChildren(parents[0], parents[1]);
            replacePoblationWithChildren(poblation, childrenPoblation[0], childrenPoblation[1], i);
        }
    }

    rankPoblation(poblation);
}

void GeneticAlgorithm::nextGenerationByAdding(Poblacion &poblation)
{
    if (poblation.size() != static_cast<size_t>(numberOfIndividuals) || poblation.size() < 2)
        throw std::invalid_argument("GA generation has an invalid population size.");
    int numberOfAddedIndividuals, i;
    const int parentCount = static_cast<int>(poblation.size());

    numberOfAddedIndividuals = defaultBinomialDistribution(numberOfIndividuals);

    for (i = 0; i < numberOfAddedIndividuals; i++)
    {
        std::vector<Individuo> parents = getParents(poblation, parentCount);

        if (uniformUnit() < crossProability)
        {
            Poblacion childrenPoblation = getChildren(parents[0], parents[1]);
            addChildredToPoblation(poblation, childrenPoblation[0], childrenPoblation[1]);
        }
    }

    rankPoblation(poblation);

    poblation.erase(poblation.begin() + numberOfIndividuals, poblation.end());
}

GeneticAlgorithm::GeneticAlgorithm()
{
    this->numberOfIndividuals = 0;
    this->isWithReplacement = true;
    this->maxIteration = 0;
    this->selectionProbability = 0.0;
    this->crossProability = 0.0;
    this->mutationProbaility = 0.0;
    this->dMutationProbability = 0.01;
    this->mutationType = MutationType::INVERSE_MUTATION;
    this->rotationType = ROTATION_WAY::ZERO_WAY;
    this->maxDiversityIndex = 0.001;
}
GeneticAlgorithm::~GeneticAlgorithm()
{
}

GeneticAlgorithm GeneticAlgorithm::Build()
{
    GeneticAlgorithm g;
    return g;
}

GeneticAlgorithm &GeneticAlgorithm::setMaxDiversityIndex(double maxDiversityIndex)
{
    this->maxDiversityIndex = maxDiversityIndex;
    return *this;
}

GeneticAlgorithm &GeneticAlgorithm::setIsWithReplacement(bool isWithReplacement)
{
    this->isWithReplacement = isWithReplacement;
    return *this;
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
