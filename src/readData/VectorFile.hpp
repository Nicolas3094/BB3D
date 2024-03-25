#ifndef VECTOR_FILE_H
#define VECTOR_FILE_H
#include "../binPacking/Bin.hpp"

using namespace std;

enum VECTOR_FILE
{
    ID_SEED = 2,
    BIN_DIMENSIONS = 3,
    NUMBER_OF_TYPES = 1,
    ITEM_DATA = 8
};

VECTOR_FILE getVectorFileFromValue(int);
int geValueFromVectorFile(VECTOR_FILE);

#endif