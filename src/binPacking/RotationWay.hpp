#ifndef ROTATION_WAY_H
#define ROTATION_WAY_H
#include <string>

enum ROTATION_WAY
{
    ZERO_WAY = 0,
    TWO_WAY = 2,
    SIX_WAY = 6
};

int getIdFromRotationWay(ROTATION_WAY);
ROTATION_WAY getRotationWayFromId(int);
std::string getNameFromRotationWay(ROTATION_WAY);
bool operator==(ROTATION_WAY, ROTATION_WAY);
bool operator!=(ROTATION_WAY, ROTATION_WAY);

#endif