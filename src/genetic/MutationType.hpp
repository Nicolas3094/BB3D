#ifndef MUTATION_TYPE_H
#define MUTATION_TYPE_H
#include <iostream>

enum MutationType
{
    C1,
    C2,
    INVERSE_MUTATION
};
std::string getMutationName(MutationType);
std::ostream &operator<<(std::ostream &, MutationType &);
#endif