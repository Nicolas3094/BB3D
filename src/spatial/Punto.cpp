#include "Punto.hpp"

Punto::Punto()
{
    x = 0;
    y = 0;
    z = 0;
}
Punto::Punto(const int x, const int y, const int z) : x(x), y(y), z(z) {}

Punto Punto::Build(const int x, const int y, const int z)
{
    Punto pt;
    pt.x = x;
    pt.y = y;
    pt.z = z;
    return pt;
}

bool operator==(Punto pt1, Punto pt2)
{
    return (pt1.x == pt2.x && pt1.y == pt2.y && pt1.z == pt2.z);
}
bool operator!=(Punto pt1, Punto pt2)
{
    return (pt1.x != pt2.x || pt1.y != pt2.y || pt1.z != pt2.z);
}
Punto operator+(Punto pt1, Punto pt2)
{
    return Punto::Build(pt1.x + pt2.x, pt1.y + pt2.y, pt1.z + pt2.z);
}
Punto operator-(Punto pt1, Punto pt2)
{
    return Punto::Build(pt1.x - pt2.x, pt1.y - pt2.y, pt1.z - pt2.z);
}
Punto operator*(int k, Punto pt)
{
    return Punto::Build(pt.x * k, pt.y * k, pt.z * k);
}
Punto operator*(Punto pt, int k)
{
    return Punto::Build(pt.x * k, pt.y * k, pt.z * k);
}
Punto operator/(Punto pt, int k)
{
    return Punto::Build(pt.x / k, pt.y / k, pt.z / k);
}
std::ostream &operator<<(std::ostream &COUT, Punto DAT)
{
    COUT << "(" << DAT.x << "," << DAT.y << "," << DAT.z << ")";
    return COUT;
}
std::ostream &operator<<(Punto DAT, std::ostream &COUT)
{
    COUT << "(" << DAT.x << "," << DAT.y << "," << DAT.z << ")";
    return COUT;
}