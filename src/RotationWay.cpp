#include "RotationWay.hpp"

int getIdFromRotationWay(ROTATION_WAY rotation)
{
    switch (rotation)
    {
    case SIX_WAY:
        return 6;
    case TWO_WAY:
        return 2;
    default:
        return 0;
    }
}