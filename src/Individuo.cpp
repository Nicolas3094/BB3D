#include "Individuo.hpp"

Individuo::Individuo()
{
    this->fitness = 0;
}

Individuo &Individuo::Build()
{
    Individuo *newInd = new Individuo();
    return *newInd;
}

Individuo &Individuo::setFitness(double fitness)
{
    this->fitness = fitness;
    return *this;
}

Individuo &Individuo::setGenome(DoubleGenome genome)
{
    this->genome = genome;
    return *this;
}

double Individuo::getFitness()
{
    return this->fitness;
}

DoubleGenome &Individuo::getGenome()
{
    return this->genome;
}

std::ostream &operator<<(std::ostream &COUT, Individuo &ind)
{
    COUT << "fi:" << ind.getFitness() << "\n"
         << "genome:\n"
         << ind.getGenome() << "\n";
    return COUT;
}

LoadedBins decodificateIndividual(Individuo indivudual, LoadedBins dataSet, Bin bin)
{
    int genomeSize = indivudual.getGenome().getGenome().size();
    LoadedBins itemsToPack(genomeSize);
    for (int i = 0; i < genomeSize; i++)
    {
        ItemBin item = dataSet[indivudual.getGenome().getGenome()[i] - 1];
        itemsToPack[i] = item.rotate(
            bin.getRotationWay(),
            getRotationFromId(indivudual.getGenome().getDGenome()[i]));
    }

    return itemsToPack;
}