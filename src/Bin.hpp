#ifndef BIN_H
#define BIN_H

#include "../main.hpp"
#include "ItemBin.hpp"
#include "Chromosome.hpp"

typedef std::vector<ItemBin> LoadedBins;

class Bin{
private:
   Dim dimensions;
   ulong loadedVolume;
   ulong numberOfLoadedItems;
   LoadedBins loadedItems;
   ROTATION_WAY rotation;
public:
    Bin();
    ~Bin();

    static Bin& Build();

    Bin& addItem(ItemBin);
    Bin& setLargo(uint);
    Bin& setAncho(uint);
    Bin& setAlto(uint);
    Bin& setRotationWay(ROTATION_WAY);

    const Dim getDimensions() const;
    ulong getLoadedVolume ();
    uint getNumberOfLoadedItems() ;
    LoadedBins getLoadedItems();
    std::vector<Punto> getPostitionOfItems();
    std::vector<uint> getIdItems();
    ROTATION_WAY getRotationWay();

};



#endif