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
        flipMutation(childGenome);
    }
    mutation(childGenome, mutationType, probMutation);
    Individuo newChild = Individuo::Build().setGenome(childGenome);
    evaluateFitness(newChild, dataSet, bin);

    return newChild;
}
void mutation(DoubleGenome &gen, MutationType type, double probability)
{
    int n;
    n = gen.getGenome().size();

    if ((rand() % (n - 2) + 1) > probability)
    {
        return;
    }
    switch (type)
    {
    case C1:
        mutateC1(gen, -1);
        break;
    case C2:
        mutateC2(gen, -1);
        break;
    case INVERSE_MUTATION:
        mutateInversion(gen, -1);
        break;
    }
}
void flipMutation(DoubleGenome &gen, double probability, ROTATION_WAY rotation)
{
    int n;
    n = gen.getGenome().size();
    for (int i = 0; i < n; i++)
    {
        if ((rand() % (n - 2) + 1) > probability)
        {
            continue;
        }
        gen.getDGenome()[i] = std::experimental::randint(1, getIdFromRotationWay(rotation))
    }
}
DoubleGenome crossOver(DoubleGenome gen1, DoubleGenome gen2)
{
    int n, start, end;
    n = gen1.getDGenome().size();
    start = std::experimental::randint(3, static_cast<int>(std::floor(n / 2)));
    end = std::experimental::randint(start + 1, n - 1);
    return crossOx(gen1, gen2, start, end);
}

int tournament(Poblacion poblation, double probability)
{
    int n, i1, i2;
    double r;
    i1 = std::experimental::randint(0, n);
    i2 = std::experimental::randint(0, n);
    r = rand() % (n - 2) + 1;
    while (i1 == i2)
    {
        i2 = std::experimental::randint(0, n);
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
    int totalFitness, init, countWeight, i;
    double r;
    std::vector<double> weightedFitnessPoblation(poblation.size());

    totalFitness = 0;
    for (auto individual : poblation)
    {
        totalFitness += individual.getFitness();
    }
    // Calculate weights
    for (int i = 0; i < poblation.size(); i++)
    {
        fitnessPoblation[i] = poblation[i].getFitness() / totalFitness;
    }
    init = std::experimental::randint(0, poblation.size() - 1);
    r = rand() % (n - 2) + 1;
    countWeight = 0;
    i = init;
    while (countWeight < r)
    {
        countWeight += weightedFitnessPoblation[i++];
        if (countWeight >= r)
        {
            break;
        }
        if (i == poblation.size() - 1)
        {
            i = 0;
        }
    }
    return i;
}