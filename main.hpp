#ifndef MAIN_H
#define MAIN_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
#include <cmath>
#include <cstdlib>
#include <random>
#include <time.h>
#include <algorithm> 
#include <functional>
#include "src/Dimension.hpp"
#include "src/Contenedor.hpp"
#include "src/ColaPrioridad.hpp"

#define print(x) std::cout<< x <<"\n"
#define MAX_RAN 32767

struct Individuo{
    Code code;
    Contenedor cont;
    double fi;
};
struct Poblacion{
    std::vector<Individuo> pobl;
    double Ftot;
};

cadena strPunto(const Punto );
bool Intersec(const Dim ,const Punto ,const std::vector<Caja> );
bool EsEmpacable(const Dim ,const Punto ,const std::vector<Caja>  );
void PobInicialH(std::vector<Individuo>&, long );
void TDLF(Individuo & );

#endif 