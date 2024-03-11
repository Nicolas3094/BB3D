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
ROTATION_WAY getRotationWayFromId(int id)
{
    if (id == 6)
    {
        return ROTATION_WAY::SIX_WAY;
    }
    else if (id == 2)
    {
        return ROTATION_WAY::TWO_WAY;
    }
    else
    {
        return ROTATION_WAY::ZERO_WAY;
    }
}
bool operator==(ROTATION_WAY way1, ROTATION_WAY way2)
{
    return getIdFromRotationWay(way1) == getIdFromRotationWay(way2);
}
bool operator!=(ROTATION_WAY way1, ROTATION_WAY way2)
{
    return getIdFromRotationWay(way1) != getIdFromRotationWay(way2);
}