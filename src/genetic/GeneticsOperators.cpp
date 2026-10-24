#include "GeneticsOperators.hpp"
#include "PermutationOperators.hpp"
#include <cmath>

void mutationWithType(DoubleGenome &gen, MutationType type, int step)
{
    int n;
    n = gen.getGenome().size();

    switch (type)
    {
    case C1:
        mutateC1(gen, step);
        break;
    case C2:
        mutateC2(gen, step);
        break;
    case INVERSE_MUTATION:
        mutateInversion(gen, step);
        break;
    }
}
void flipMutation(DoubleGenome &gen, double probability, ROTATION_WAY rotation)
{
    int n;
    n = gen.getGenome().size();
    for (int i = 0; i < n; i++)
    {
        if (uniformUnit() > probability)
        {
            continue;
        }
        int number = randomInteger(1, getIdFromRotationWay(rotation));
        gen.getDGenome()[i] = number;
    }
}

/** Instead of going one by one, we just figure out how many alleles have to be change.
 *  This way we reduce time complexity from O(n) to O(numberOfAllelesToChanges).
 *  Usually we're gonna get O(1).
 */
void flipMutationO1(DoubleGenome &gen, double probability, ROTATION_WAY rotation)
{
    int numberOfAllelesToChange, indexToChange, i, oldAllele, newAllele;
    std::map<int, bool> visitedAllele;

    numberOfAllelesToChange = getNumberOfChanges(uniformUnit(), probability);

    if (numberOfAllelesToChange == 0)
    {
        return;
    }
    for (i = 0; i < numberOfAllelesToChange; i++)
    {
        indexToChange = randomInteger(0, gen.getDGenome().size() - 1);
        while (visitedAllele[indexToChange])
        {
            indexToChange = randomInteger(0, gen.getDGenome().size() - 1);
        }
        visitedAllele[indexToChange] = true;

        oldAllele = gen.getDGenome()[indexToChange];

        newAllele = randomInteger(1, getIdFromRotationWay(rotation));
        while (newAllele == oldAllele)
        {
            newAllele = randomInteger(1, getIdFromRotationWay(rotation));
        }
        gen.getDGenome()[indexToChange] = newAllele;
    }
}

DoubleGenome crossOver(DoubleGenome gen1, DoubleGenome gen2)
{
    int n, start, end;
    n = gen1.getDGenome().size();
    start = randomInteger(3, static_cast<int>(std::floor(n / 2)));
    end = randomInteger(start + 1, n - 1);

    return crossOx(gen1, gen2, start, end);
}

int tournament(Poblacion poblation, int n, double probability)
{
    long i1, i2;
    double r;
    i1 = randomInteger(0, n - 1);
    i2 = randomInteger(0, n - 1);
    r = uniformUnit();
    while (i1 == i2)
    {
        i2 = randomInteger(0, n - 1);
    }

    if (r <= probability)
    {
        if (poblation[i1].getFitness() > poblation[i2].getFitness())
        {
            return i1;
        }
        else
        {
            return i2;
        }
    }
    else
    {
        if (poblation[i1].getFitness() > poblation[i2].getFitness())
        {
            return i2;
        }
        else
        {
            return i1;
        }
    }
}

int rouletteWheel(Poblacion poblation)
{
    if (poblation.empty())
    {
        throw std::invalid_argument("Roulette selection requires a non-empty population.");
    }
    double totalFitness = 0.0;
    for (auto &individual : poblation)
    {
        const double fitness = individual.getFitness();
        if (!std::isfinite(fitness) || fitness < 0.0)
        {
            throw std::invalid_argument("Roulette fitness must be finite and non-negative.");
        }
        totalFitness += fitness;
    }
    if (!std::isfinite(totalFitness))
    {
        throw std::invalid_argument("Roulette total fitness must be finite.");
    }
    if (totalFitness == 0.0)
    {
        return randomInteger(0, static_cast<int>(poblation.size()) - 1);
    }

    const double target = uniformUnit() * totalFitness;
    double cumulativeFitness = 0.0;
    int lastPositive = 0;
    for (int i = 0; i < static_cast<int>(poblation.size()); ++i)
    {
        const double fitness = poblation[i].getFitness();
        if (fitness == 0.0)
            continue;
        lastPositive = i;
        cumulativeFitness += fitness;
        if (target < cumulativeFitness)
        {
            return i;
        }
    }
    // Handle floating-point rounding without selecting a zero-weight entry.
    return lastPositive;
}

int getNumberOfChanges(double probability, double unitProbability)
{

    double currentPr = unitProbability;
    int index = 0;

    while (probability <= currentPr)
    {
        currentPr *= unitProbability;
        index++;
    }

    return index;
}
