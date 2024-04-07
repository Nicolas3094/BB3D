
#ifndef EVOLUTIVE_ALGORITHM_H
#define EVOLUTIVE_ALGORITHM_H

#include "../genetic/Individuo.hpp"
#include "../readData/DataSetBPP.hpp"

class EvolutiveAlgorithm
{
public:
    virtual ~EvolutiveAlgorithm() = 0;
    virtual Poblacion evolveWithReplacement() = 0;
    virtual Poblacion evolveWithAdded() = 0;
};

#endif