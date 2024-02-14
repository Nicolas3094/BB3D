#ifndef DATA_SET_BPP_H
#define DATA_SET_BPP_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <fstream>
#include <vector>
#include "../src/Bin.hpp"

using namespace std;

struct DatasetBinBacking
{
    int id;
    int PSeed;
    Bin bin;
    LoadedBins itemsByTpe;
    LoadedBins totalItems;
};

std::ostream &operator<<(std::ostream &, DatasetBinBacking &);

#endif