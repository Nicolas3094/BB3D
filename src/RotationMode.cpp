#include "RotationMode.hpp"

ROTATION_MODE getRotationFromId(uint id){
    switch (id)
    {
        case 3:
            return ROTATION_MODE::THREE;
        case 4:
            return ROTATION_MODE::FOUR;
        case 5:
            return ROTATION_MODE::FIVE;
        case 6:
            return ROTATION_MODE::SIX;
        default:
            return ROTATION_MODE::ONE;
    }
}
