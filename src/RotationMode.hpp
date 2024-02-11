#ifndef ROTATION_MODE_H
#define ROTATION_MODE_H
#include "../main.hpp"

enum ROTATION_MODE{
    ONE = 1,
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6
};

ROTATION_MODE getRotationFromId(uint);

#endif