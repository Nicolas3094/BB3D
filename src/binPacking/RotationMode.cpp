#include "RotationMode.hpp"

ROTATION_MODE getRotationFromId(int id)
{
    switch (id)
    {
    case 1:
        return ROTATION_MODE::ONE;
    case 2:
        return ROTATION_MODE::TWO;
    case 3:
        return ROTATION_MODE::THREE;
    case 4:
        return ROTATION_MODE::FOUR;
    case 5:
        return ROTATION_MODE::FIVE;
    case 6:
        return ROTATION_MODE::SIX;
    default:
        return ROTATION_MODE::NONE;
    }
}

int getIdFromRotation(ROTATION_MODE rotation)
{
    switch (rotation)
    {
    case ROTATION_MODE::ONE:
        return 1;
    case ROTATION_MODE::TWO:
        return 2;
    case ROTATION_MODE::THREE:
        return 3;
    case ROTATION_MODE::FOUR:
        return 4;
    case ROTATION_MODE::FIVE:
        return 5;
    case ROTATION_MODE::SIX:
        return 6;
    default:
        return 0;
    }
}
