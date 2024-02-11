#ifndef DIMENSION_H
#define DIMENSION_H

#include "../main.hpp"
#include "Punto.hpp"
#include <iostream>

class Dim{
private:
    uint l; uint w; uint h;
public:
    Dim();
    ~Dim();
    Dim(uint,uint,uint);

    static Dim& Build();

    Punto getVector();
    const uint getLargo() const;
    const uint getAncho() const;
    const uint getAlto() const;
    const ulong getVolumen() const;


    Dim& setLargo(uint);
    Dim& setAncho(uint);
    Dim& setAlto(uint);
};


std::ostream&  operator <<(std::ostream& , Dim& );

bool operator==(Dim &, Dim &);
bool operator!=(Dim &, Dim &);

#endif