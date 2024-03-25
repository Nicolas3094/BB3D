#ifndef PUNTO_H
#define PUNTO_H

#include <iostream>

class Punto
{
public:
    int x;
    int y;
    int z;
    Punto();
    Punto(const int, const int, const int);

    static Punto Build(const int, const int, const int);
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