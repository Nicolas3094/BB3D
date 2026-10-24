#ifndef PUNTO_H
#define PUNTO_H

#include <iostream>
#include <cstdint>

class Punto
{
public:
    std::int64_t x;
    std::int64_t y;
    std::int64_t z;
    Punto();
    Punto(std::int64_t, std::int64_t, std::int64_t);

    static Punto Build(std::int64_t, std::int64_t, std::int64_t);
};

std::ostream &operator<<(std::ostream &, Punto);
std::ostream &operator<<(Punto, std::ostream &);

bool operator==(Punto, Punto);
bool operator!=(Punto, Punto);
Punto operator+(Punto, Punto);
Punto operator-(Punto, Punto);
Punto operator*(int, Punto);
Punto operator*(Punto, int);
Punto operator/(Punto, int);

#endif
