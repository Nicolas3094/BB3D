#ifndef ITEM_BIN_H
#define ITEM_BIN_H
#include "Dimension.hpp"
#include "RotationWay.hpp"
#include "RotationMode.hpp"
#include "../main.hpp"

class ItemBin
{
private:
    uint id;
    uint tipo;
    Dim dimension;
    Punto posicion;
    Punto currentDimension;
    ROTATION_WAY rotation;
    Punto rotations;
    bool hasValidRotation;

public:
    ItemBin();
    ~ItemBin();

    static ItemBin Build();

    ItemBin &setId(int);
    ItemBin &setTipo(int);
    ItemBin &setPosicion(Punto);
    ItemBin &setLargo(int);
    ItemBin &setAncho(int);
    ItemBin &setAlto(int);
    ItemBin &setCurrentDimension(Punto);
    ItemBin &setRotations(Punto);
    ItemBin &setRotation(ROTATION_WAY);

    Punto getOriginalDimension();
    Punto getCurrentDimension();
    int getId();
    int getTipo();
    int getLargo();
    int getAncho();
    int getAlto();
    Dim getDimension();
    ulong getVolumen();
    Punto getPunto();
    Punto getRotations();
    ROTATION_WAY getRotation();

    bool isValidOrientation();

    void rotate(ROTATION_WAY, ROTATION_MODE);
};

struct VolumeGreater
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getLargo() * lx.getAncho() * lx.getAlto() < rx.getLargo() * rx.getAncho() * rx.getAlto();
    }
};
struct VolumeLess
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getLargo() * lx.getAncho() * lx.getAlto() > rx.getLargo() * rx.getAncho() * rx.getAlto();
    }
};
struct LengthGreater
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getLargo() < rx.getLargo();
    }
};
struct LengthLess
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getLargo() > rx.getLargo();
    }
};
struct WidthGreater
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getAncho() < rx.getAncho();
    }
};
struct WidthLess
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getAncho() > rx.getAncho();
    }
};

struct HeightLess
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getAlto() > rx.getAlto();
    }
};

struct HeighGreater
{
    bool operator()(ItemBin lx, ItemBin rx)
    {
        return lx.getAlto() < rx.getAlto();
    }
};

bool operator<(ItemBin, ItemBin);
bool operator>(ItemBin, ItemBin);
bool operator==(ItemBin, ItemBin);
std::ostream &operator<<(std::ostream &, ItemBin &);

#endif