#ifndef DIMENSION_H
#define DIMENSION_H

#include <iostream>

class Dim{
public:
    double l; double w; double h; double vol;
    Dim();
    Dim(double,double,double);
};
class Punto{
public:
    double x; double y; double z;
    Punto();
    Punto(double,double,double);
};
std::ostream&  operator <<(std::ostream& , Dim& );
std::ostream&  operator <<(std::ostream& , Punto& );

bool operator==(Dim &, Dim &);
bool operator!=(Dim &, Dim &);

#endif