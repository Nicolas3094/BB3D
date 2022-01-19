#ifndef COLA_P
#define COLA_P
#include <vector>
#include "Dimension.hpp"

enum DIRECCION{
    X=1,Y=2,Z=3
};
class ColaPuntos {
    std::vector<Punto> cola;
    int x,y,z;
public:
    ColaPuntos();
    void push(Punto,DIRECCION);
    void pop();
    Punto& top();
    Punto back();
    int size();
    Punto& operator [](int);
    bool empty();
    void printQ();
};

#endif