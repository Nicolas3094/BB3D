#ifndef INDIVIDUO_H
#define INDIVIDUO_H

#include "Chromosome.hpp"
#include "../binPacking/Bin.hpp"
#include <algorithm>
#include <functional>

class Individuo
{
private:
    DoubleGenome genome;
    double fitness = 0;

public:
    std::string solution;

    Individuo();

    Individuo &setFitness(double);
    Individuo &setGenome(DoubleGenome);

    static Individuo Build();

    double getFitness();
    DoubleGenome &getGenome();
};

typedef std::vector<Individuo> Poblacion;

struct greater_fitness
{
    inline bool operator()(Individuo struct1, Individuo struct2)
    {
        return (struct1.getFitness() > struct2.getFitness());
    }
};

LoadedBins decodificateIndividual(Individuo, LoadedBins, Bin);

std::string codificateIndividual(Individuo);

void rankPoblation(Poblacion &);
std::ostream &operator<<(std::ostream &, Individuo &);
std::ostream &operator<<(std::ostream &, Poblacion &);

#endif