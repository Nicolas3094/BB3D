
#ifndef EVOLUTIVE_ALGORITHM_H
#define EVOLUTIVE_ALGORITHM_H

#include "../genetic/Individuo.hpp"
#include "../readData/DataSetBPP.hpp"
#include <map>

class EvolutiveAlgorithm
{
public:
    virtual ~EvolutiveAlgorithm() = 0;
    virtual Poblacion evolve() = 0;
};

#endif