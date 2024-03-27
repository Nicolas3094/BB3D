#ifndef COLA_P
#define COLA_P

#include "../spatial/Dimension.hpp"
#include <vector>

class ColaPuntosDBL
{
private:
    std::vector<Punto> cola;

    bool condition(Punto, Punto);
    bool condition1(Punto, Punto);
    bool condition2(Punto, Punto);
    bool condition3(Punto, Punto);

public:
    ColaPuntosDBL();

    static ColaPuntosDBL Build();

    void push(Punto);
    void remove(int);
    void pop();
    void clear();
    void update();
    Punto &top();
    Punto back();
    int size();
    Punto &operator[](int);
    bool empty();
    void printQ();
};

std::ostream &operator<<(std::ostream &, ColaPuntosDBL &);

#endif