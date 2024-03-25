#ifndef ROTATION_MODE_H
#define ROTATION_MODE_H

enum ROTATION_MODE
{
    NONE = 0,
    ONE = 1,
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6
};

ROTATION_MODE getRotationFromId(int);
int getIdFromRotation(ROTATION_MODE);

#endif