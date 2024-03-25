#ifndef ITEM_BIN_OPERATORS_H
#define CHROMOSOME_H

#include "Bin.hpp"
#include "../structures/ColaPrioridad.hpp"
#include "../structures/PriorityPointQueue.hpp"

bool canBePlaced(Bin, const Punto &);
bool isABIntersect(Punto, Punto, Punto, Punto);
bool isOverlapped(Punto, ItemBin, Bin);
void iterateByDeepestBottomLeft(Punto &, ItemBin, Bin);
void addItemToBin(ColaPuntosDBL &, Bin &, const Punto &, ItemBin);
void addItemToBinQueue(PriorityPointQueue &, Bin &, const Punto &, ItemBin);

#endif