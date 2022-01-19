#include "Contenedor.hpp"

Caja::Caja() {}
Caja::Caja(int tipo, Punto pt) : tipo(tipo), pos(pt) {}
Caja::Caja(int id, int tipo, Punto pt) : id(id), tipo(tipo), pos(pt) {}

Code cod(std::vector<Caja> cajas)
{
    Code ct;
    for (int i = 0; i < cajas.size(); i++)
    {
        ct.push_back(cajas[i].tipo);
    }
    return ct;
}

std::vector<Dim> Bin::tipos;
std::vector<Caja> Bin::cajas;
int Bin::ALTO; int Bin::ANCHO ; int Bin::PROFUNDIDAD;