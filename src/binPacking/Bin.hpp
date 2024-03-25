#ifndef BIN_H
#define BIN_H

#include "ItemBin.hpp"
#include "../genetic/Chromosome.hpp"
#include "../spatial/Dimension.hpp"
#include "RotationWay.hpp"
#include <vector>

typedef std::vector<ItemBin> LoadedBins;

class Bin
{
private:
    Dim dimensions;
    long loadedVolume;
    long numberOfLoadedItems;
    LoadedBins loadedItems;
    ROTATION_WAY rotation;

public:
    Bin();
    ~Bin();

    static Bin Build();

    Bin &addItem(ItemBin);
    Bin &setLargo(int);
    Bin &setAncho(int);
    Bin &setAlto(int);
    Bin &setRotationWay(ROTATION_WAY);

    const Dim getDimensions() const;
    int getLoadedVolume();
    int getNumberOfLoadedItems();
    LoadedBins getLoadedItems();
    std::vector<Punto> getPostitionOfItems();
    std::vector<int> getIdItems();
    ROTATION_WAY getRotationWay();
};

std::ostream &operator<<(std::ostream &, const LoadedBins &);
std::ostream &operator<<(std::ostream &, Bin);

#endif