#include "Individuo.hpp"

Individuo::Individuo()
{
    this->fitness = 0;
}

Individuo Individuo::Build()
{
    Individuo newInd;
    return newInd;
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
    COUT << ind.getGenome() << " || " << ind.getFitness();

    return COUT;
}

LoadedBins decodificateIndividual(Individuo indivudual, LoadedBins dataSet, Bin bin)
{
    int genomeSize = indivudual.getGenome().getGenome().size();
    LoadedBins itemsToPack(genomeSize);
    for (int i = 0; i < genomeSize; i++)
    {
        ItemBin item = dataSet[indivudual.getGenome().getGenome()[i] - 1];
        if (bin.getRotationWay() != ROTATION_WAY::ZERO_WAY)
        {
            item.rotate(
                bin.getRotationWay(),
                getRotationFromId(indivudual.getGenome().getDGenome()[i]));
        }

        itemsToPack[i] = item;
    }

    return itemsToPack;
}

void rankPoblation(Poblacion &poblation)
{
    std::sort(poblation.begin(), poblation.end(), greater_fitness());
}

std::ostream &operator<<(std::ostream &COUT, Poblacion &pob)
{
    for (auto individual : pob)
    {
        COUT << individual << "\n";
    }
    return COUT;
}

std::string codificateIndividual(Individuo ind)
{
    std::string resp = "|";

    for (int i = 0; i < ind.getGenome().getGenome().size(); i++)
    {
        resp += std::to_string(ind.getGenome().getGenome()[i]) + " " + std::to_string(ind.getGenome().getDGenome()[i]) + "|";
    }

    return resp;
}
