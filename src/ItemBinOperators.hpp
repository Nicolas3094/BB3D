#ifndef ITEM_BIN_OPERATORS_H
#define CHROMOSOME_H

#include "Bin.hpp"
#include "ColaPrioridad.hpp"

bool canBePlaced(Bin,const Punto&);
bool isABIntersect(Punto,Punto,Punto,Punto);
bool isOverlapped(Punto,ItemBin,LoadedBins,Bin, ROTATION_MODE);
void iterateByDeepestBottomLeft(Punto&, ItemBin,LoadedBins, Bin,ROTATION_MODE);
void addItemToBin(ColaPuntosDBL&, Bin&, Punto&, ItemBin, LoadedBins,ROTATION_MODE);

#endif