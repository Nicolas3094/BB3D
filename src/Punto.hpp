#ifndef PUNTO_H
#define PUNTO_H

#include "../main.hpp"
#include <iostream>

class Punto{
public:
    uint x; uint y; uint z;
    Punto();
    Punto(const uint,const uint,const uint);

     static Punto& Build(const uint,const uint,const uint);
};

std::ostream&  operator <<(std::ostream& , Punto& );

bool operator==(Punto, Punto);
bool operator!=(Punto, Punto);
Punto operator+(Punto, Punto);
Punto operator-(Punto, Punto);
Punto operator*(uint, Punto);
Punto operator*(Punto, uint);
Punto operator/(Punto, uint);

#endif