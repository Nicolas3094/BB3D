#ifndef ITEM_BIN_OPERATORS_H
#define CHROMOSOME_H

#include "Bin.hpp"
#include "ColaPrioridad.hpp"

bool canBePlaced(Bin, const Punto &);
bool isABIntersect(Punto, Punto, Punto, Punto);
bool isOverlapped(Punto, ItemBin, Bin);
void iterateByDeepestBottomLeft(Punto &, ItemBin, Bin);
void addItemToBin(ColaPuntosDBL &, Bin &, const Punto &, ItemBin);

#endif