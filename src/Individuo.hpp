#ifndef INDIVIDUO_H
#define INDIVIDUO_H

#include "../main.hpp"
#include "Chromosome.hpp"

class Individuo
{
private:
    DoubleGenome genome;
    double fitness = 0;

public:
    Individuo();

    Individuo &setFitness(double);
    Individuo &setGenome(DoubleGenome);

    static Individuo &Build();

    double getFitness();
    DoubleGenome &getGenome();
};

DoubleGenome *createGenome(Chromosome, Chromosome);

std::ostream &operator<<(std::ostream &, Individuo &);
typedef std::vector<Individuo> Poblacion;

#endif