#ifndef INDIVIDUO_H
#define INDIVIDUO_H

#include "../main.hpp"
#include "Chromosome.hpp"
#include "Bin.hpp"

class Individuo
{
private:
    DoubleGenome genome;
    double fitness = 0;

public:
    Individuo();

    Individuo &setFitness(double);
    Individuo &setGenome(DoubleGenome);

    static Individuo Build();

    double getFitness();
    DoubleGenome &getGenome();
};

std::ostream &operator<<(std::ostream &, Individuo &);

typedef std::vector<Individuo> Poblacion;

struct greater_fitness
{
    inline bool operator()(Individuo struct1, Individuo struct2)
    {
        return (struct1.getFitness() > struct2.getFitness());
    }
};

LoadedBins decodificateIndividual(Individuo, LoadedBins, Bin);
void rankPoblation(Poblacion &);

#endif