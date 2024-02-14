#include "VectorFile.hpp"

VECTOR_FILE getVectorFileFromValue(int id)
{
    if (id == 2)
    {
        return VECTOR_FILE::ID_SEED;
    }
    else if (id == 3)
    {
        return VECTOR_FILE::BIN_DIMENSIONS;
    }
    else if (id == 1)
    {
        return VECTOR_FILE::NUMBER_OF_TYPES;
    }
    else if (id == 8)
    {
        return VECTOR_FILE::ITEM_DATA;
    }
    else
    {
        throw invalid_argument("Undefined id vector file.");
    }
}

int geValueFromVectorFile(VECTOR_FILE vectorFile)
{
    switch (vectorFile)
    {
    case ID_SEED:
        return 2;
    case BIN_DIMENSIONS:
        return 3;
    case NUMBER_OF_TYPES:
        return 1;
    case ITEM_DATA:
        return 8;
    default:
        throw invalid_argument("No vector file object.");
    }
}
