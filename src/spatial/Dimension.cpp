#include "Dimension.hpp"
#include <limits>
#include <stdexcept>

Dim::~Dim() {}

Dim::Dim(int l, int w, int h)
{
    this->h = h;
    this->l = l;
    this->w = w;
}

Dim::Dim()
{
    h = 0;
    l = 0;
    w = 0;
}

Dim Dim::Build()
{
    Dim dim;

    return dim;
}

Dim &Dim::setAncho(int ancho)
{
    this->w = ancho;

    return *this;
}

Dim &Dim::setLargo(int largo)
{
    this->l = largo;

    return *this;
}

Dim &Dim::setAlto(int alto)
{
    this->h = alto;

    return *this;
}

const int Dim::getAlto() const
{
    return h;
}

const int Dim::getLargo() const
{
    return l;
}

const int Dim::getAncho() const
{
    return w;
}

std::int64_t Dim::getVolumen() const
{
    std::int64_t volume = 1;

    for (int side : {l, w, h})
    {
        if (side < 0)
            throw std::invalid_argument("Volume dimensions must be non-negative.");

        if (side == 0)
            return 0;

        if (volume > std::numeric_limits<std::int64_t>::max() / side)
            throw std::overflow_error("Volume exceeds int64 range.");

        volume *= side;
    }
    return volume;
}

Punto Dim::getVector()
{
    Punto punto = Punto(this->l, this->w, this->h);

    return punto;
}

bool operator==(Dim &d1, Dim &d2)
{
    return (d1.getAlto() == d2.getAlto() && d1.getLargo() == d2.getLargo() && d1.getAncho() == d2.getAncho());
}

bool operator!=(Dim &d1, Dim &d2)
{
    return !(d1.getAlto() == d2.getAlto() && d1.getLargo() == d2.getLargo() &&
             d1.getAncho() == d2.getAncho());
}

std::ostream &operator<<(std::ostream &COUT, Dim &DAT)
{
    COUT << "(" << DAT.getLargo() << "," << DAT.getAncho() << "," << DAT.getAlto() << ")";

    return COUT;
}
