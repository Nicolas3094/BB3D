#ifndef MAIN_H
#define MAIN_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <random>
#include <time.h>
#include <algorithm> 

#define print(x) std::cout<< x <<"\n"
#define MAX_RAN 32767
#define LONGITUD 1600
#define ANCHO 604
#define ALTO 640
typedef std::string cadena;
typedef std::vector<int> Code;

struct Dim{
    double l;
    double w;
    double h;
    double vol;
};
struct Coord{
    double x;
    double y;
    double z;
};

struct Caja{
    int id;
    int tipo;
    Coord pos;
};
struct Universo{
    std::vector<Caja> cajas;
    std::vector<Dim> tipos;
};
struct Contenedor{
    std::vector<Caja> cajas;
    Code ci;
    long carga;
    double volCarga;
};
struct Individuo{
    Code code;
    Contenedor cont;
    double fi;
};
struct Poblacion{
    std::vector<Individuo> pobl;
    double Ftot;
};
Universo LEER_DATA();
std::ostream&  operator <<(std::ostream& , Dim& );
std::ostream&  operator <<(std::ostream& ,Code& );

bool  operator ==(Dim& , Dim& );
bool  operator !=(Dim& , Dim& );
void RandSolucion(Contenedor&);
void FFit(Code ,Contenedor );

#endif MAIN_H