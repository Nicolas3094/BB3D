#ifndef ITEM_BIN_OPERATORS_H
#define ITEM_BIN_OPERATORS_H

#include "Bin.hpp"
#include "../structures/ColaPrioridad.hpp"
#include "../structures/PriorityPointQueue.hpp"

bool canBePlaced(const Bin &, const Punto &);
bool isABIntersect(Punto, Punto, Punto, Punto);
bool isOverlapped(Punto, const ItemBin &, const Bin &);
// Compact a feasible non-overlapping placement in X,Z,Y order: O(loaded items).
void iterateByDeepestBottomLeft(Punto &, const ItemBin &, const Bin &);
void addItemToBin(ColaPuntosDBL &, Bin &, const Punto &, ItemBin);
void addItemToBinQueue(PriorityPointQueue &, Bin &, const Punto &, ItemBin);

#endif
