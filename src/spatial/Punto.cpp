#include "Punto.hpp"
#include <limits>
#include <stdexcept>

namespace {
using Coordinate = std::int64_t;
const Coordinate maximum = std::numeric_limits<Coordinate>::max();
const Coordinate minimum = std::numeric_limits<Coordinate>::min();
Coordinate add(Coordinate a, Coordinate b) {
    if ((b > 0 && a > maximum-b) || (b < 0 && a < minimum-b))
        throw std::overflow_error("Coordinate addition overflow.");
    return a+b;
}
Coordinate subtract(Coordinate a, Coordinate b) {
    if ((b > 0 && a < minimum+b) || (b < 0 && a > maximum+b))
        throw std::overflow_error("Coordinate subtraction overflow.");
    return a-b;
}
Coordinate multiply(Coordinate a, Coordinate b) {
    if (a == 0 || b == 0) return 0;
    if ((a > 0 && b > 0 && a > maximum/b) ||
        (a > 0 && b < 0 && b < minimum/a) ||
        (a < 0 && b > 0 && a < minimum/b) ||
        (a < 0 && b < 0 && a < maximum/b))
        throw std::overflow_error("Coordinate multiplication overflow.");
    return a*b;
}
Coordinate divide(Coordinate a, int b) {
    if (b == 0) throw std::invalid_argument("Coordinate division by zero.");
    if (a == minimum && b == -1) throw std::overflow_error("Coordinate division overflow.");
    return a/b;
}
}

Punto::Punto()
{
    x = 0;
    y = 0;
    z = 0;
}
Punto::Punto(std::int64_t x, std::int64_t y, std::int64_t z) : x(x), y(y), z(z) {}

Punto Punto::Build(std::int64_t x, std::int64_t y, std::int64_t z)
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
    return Punto::Build(add(pt1.x,pt2.x), add(pt1.y,pt2.y), add(pt1.z,pt2.z));
}
Punto operator-(Punto pt1, Punto pt2)
{
    return Punto::Build(subtract(pt1.x,pt2.x), subtract(pt1.y,pt2.y), subtract(pt1.z,pt2.z));
}
Punto operator*(int k, Punto pt)
{
    return Punto::Build(multiply(pt.x,k), multiply(pt.y,k), multiply(pt.z,k));
}
Punto operator*(Punto pt, int k)
{
    return k * pt;
}
Punto operator/(Punto pt, int k)
{
    return Punto::Build(divide(pt.x,k), divide(pt.y,k), divide(pt.z,k));
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
