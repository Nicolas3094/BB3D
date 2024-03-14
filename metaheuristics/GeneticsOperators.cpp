#include "GeneticsOperators.hpp"
#include "PermutationOperators.hpp"

Individuo crossIndivituals(
    LoadedBins dataSet,
    Bin bin,
    MutationType mutationType,
    Individuo parent1,
    Individuo parent2,
    double probMutation,
    double probMutationRot)
{
    DoubleGenome childGenome = crossOver(parent1.getGenome(), parent2.getGenome());
    if (bin.getRotationWay() != ROTATION_WAY::ZERO_WAY)
    {
        flipMutation(childGenome, probMutationRot, bin.getRotationWay());
    }
    mutationWithType(childGenome, mutationType, probMutation, -1);
    Individuo newChild = Individuo::Build().setGenome(childGenome);
    evaluateFitness(newChild, dataSet, bin);

    return newChild;
}
void mutationWithType(DoubleGenome &gen, MutationType type, double probability, int step)
{
    int n;
    n = gen.getGenome().size();
    double r = uniformUnit();
    if (probability < r)
    {
        return;
    }
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
DoubleGenome crossOver(DoubleGenome gen1, DoubleGenome gen2)
{
    int n, start, end;
    n = gen1.getDGenome().size();
    start = randomInteger(3, static_cast<int>(std::floor(n / 2)));
    end = randomInteger(start + 1, n - 1);

    return crossOx(gen1, gen2, start, end);
}

int tournament(Poblacion poblation, double probability)
{
    long n, i1, i2;
    double r;
    n = poblation.size();
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
    int totalFitness, init, countWeight, n, i;
    double r;
    n = poblation.size();
    std::vector<double>
        weightedFitnessPoblation(n);

    totalFitness = 0;
    for (auto individual : poblation)
    {
        totalFitness += individual.getFitness();
    }
    // Calculate weights
    for (i = 0; i < n; i++)
    {
        weightedFitnessPoblation[i] = poblation[i].getFitness() / totalFitness;
    }
    init = randomInteger(0, n - 1);
    r = uniformUnit();
    countWeight = 0;
    i = init;

    while (countWeight < r)
    {
        countWeight += weightedFitnessPoblation[i++];
        if (countWeight >= r)
        {
            break;
        }
        if (i == n - 1)
        {
            i = 0;
        }
    }

    return i;
}