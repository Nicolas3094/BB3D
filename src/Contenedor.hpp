#ifndef CONTENEDOR_P
#define CONTENEDOR_P

#include <vector>

#include "Dimension.hpp"

typedef std::string cadena;
typedef std::vector<int> Code;

class Caja{
public:
    int id;
    int tipo;
    Punto pos;
    Caja();
    Caja(int,Punto);
    Caja(int,int,Punto);
};
class Bin{
public:
    static std::vector<Caja> cajas;
    static std::vector<Dim> tipos;
    static int ANCHO; static int ALTO; static int PROFUNDIDAD;
};

struct Contenedor{
    std::vector<Caja> cajas;
    Code ci;
    long carga;
    double volCarga;
};
Code cod(std::vector<Caja> );
std::ostream&  operator <<(std::ostream& ,Code& );

#endif