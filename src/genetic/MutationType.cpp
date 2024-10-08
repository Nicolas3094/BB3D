#include "MutationType.hpp"

std::ostream &operator<<(std::ostream &COUT, MutationType &type)
{
    if (type == MutationType::INVERSE_MUTATION)
    {
        COUT << "INVERSE_MUTATION";
    }
    else if (type == MutationType::C1)
    {
        COUT << "C1_MUTATION";
    }
    else
    {
        COUT << "C2_MUTATION";
    }
    return COUT;
}

std::string getMutationName(MutationType type)
{
    if (type == MutationType::INVERSE_MUTATION)
    {
        return "InverseMutation";
    }
    else if (type == MutationType::C1)
    {
        return "C1Mutation";
    }
    else
    {
        return "C2Mutation";
    }
}