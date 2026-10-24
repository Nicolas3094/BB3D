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
    std::int64_t loadedVolume;
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
    std::int64_t getLoadedVolume() const;
    int getNumberOfLoadedItems() const;
    // Borrowed view: valid while this Bin lives; element references/iterators
    // may be invalidated by addItem(). Copy explicitly when a snapshot is needed.
    const LoadedBins &getLoadedItems() const;
    std::vector<Punto> getPostitionOfItems();
    std::vector<int> getIdItems();
    ROTATION_WAY getRotationWay() const;
};

std::ostream &operator<<(std::ostream &, const LoadedBins &);
std::ostream &operator<<(std::ostream &, Bin);

#endif
