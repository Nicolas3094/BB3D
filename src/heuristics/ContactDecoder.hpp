#ifndef CONTACT_DECODER_HPP
#define CONTACT_DECODER_HPP
#include "DBLF.hpp"
// Area of all touching wall/item faces, counting each candidate face patch once.
// Preconditions: the placement and the existing packing are feasible.
long double placementContactArea(const Bin &, const ItemBin &, Punto);

struct ContactPlacement
{
    bool found = false;
    Punto point;
    int sourceIndex = -1;
    long double area = 0;
};

ContactPlacement selectContactPlacement(const Bin &, const ItemBin &, ColaPuntosDBL &);
void contactDecode(Bin &, LoadedBins);
void contactDecode(Bin &, const Chromosome &, const Chromosome &, LoadedBins);
#endif
