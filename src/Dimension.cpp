#include "Dimension.hpp"

Dim::Dim(double l, double w , double h){
    this->h=h;
    this->l=l;
    this->w=w;
    this->vol=l*w*h;
}
Dim::Dim(){
    h=0;l=0;w=0;vol=0;
}
Punto::Punto(){
    x=0; y=0; z=0;
}
Punto::Punto(double x, double y, double z):x(x),y(y),z(z){}

bool operator==(Dim &d1, Dim &d2)
{
    return (d1.h == d2.h && d1.l == d2.l && d1.w == d2.w);
}
bool operator!=(Dim &d1, Dim &d2)
{
    return !(d1.h == d2.h && d1.l == d2.l && d1.w == d2.w);
}
std::ostream &operator<<(std::ostream &COUT, Dim &DAT)
{
    COUT << "(" << DAT.l << "," << DAT.w << "," << DAT.h << ")";
    return COUT;
}
std::ostream &operator<<(std::ostream &COUT, Punto &DAT)
{
    COUT << "(" << DAT.x << "," << DAT.y << "," << DAT.z << ")";
    return COUT;
}