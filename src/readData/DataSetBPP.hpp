#ifndef DATA_SET_BPP_H
#define DATA_SET_BPP_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <fstream>
#include <vector>
#include "../binPacking/Bin.hpp"

using namespace std;

struct DatasetBinBacking
{
    int id;
    int PSeed;
    Bin bin;
    LoadedBins itemsByTpe;
    LoadedBins totalItems;
    ROTATION_WAY rotationWay;
};

std::ostream &operator<<(std::ostream &, DatasetBinBacking &);

#endif