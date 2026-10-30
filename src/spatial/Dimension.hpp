#ifndef DIMENSION_H
#define DIMENSION_H

#include "Punto.hpp"
#include <cstdint>

class Dim
{
  private:
    int l;
    int w;
    int h;

  public:
    Dim();
    ~Dim();
    Dim(int, int, int);

    static Dim Build();

    Punto getVector();
    const int getLargo() const;
    const int getAncho() const;
    const int getAlto() const;
    std::int64_t getVolumen() const;

    Dim &setLargo(int);
    Dim &setAncho(int);
    Dim &setAlto(int);
};

std::ostream &operator<<(std::ostream &, Dim &);

bool operator==(Dim &, Dim &);
bool operator!=(Dim &, Dim &);
#endif
