#ifndef COLA_P
#define COLA_P
#include <vector>
#include "Dimension.hpp"

enum DIRECCION{
    X=1,Y=2,Z=3
};
class ColaPuntosDBL {
private:
    std::vector<Punto> cola;
    int x,y,z;

    bool condition(Punto, Punto);
    bool condition1(Punto, Punto);
    bool condition2(Punto, Punto);
    bool condition3(Punto, Punto);
public:
    ColaPuntosDBL();

    static ColaPuntosDBL& Build();

    void push(Punto&);
    void remove(int);
    void pop();
    void clear();
    void update();
    Punto& top();
    Punto back();
    int size();
    Punto& operator [](int);
    bool empty();
    void printQ();
};

#endif