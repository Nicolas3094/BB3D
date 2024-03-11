#include "ItemBin.hpp"

ItemBin::ItemBin()
{
    this->dimension = Dim::Build().setAlto(0).setAncho(0).setLargo(0);
    this->rotation = ROTATION_WAY::ZERO_WAY;
    this->currentDimension = Punto::Build(0, 0, 0);
    this->hasValidRotation = true;
}
ItemBin::~ItemBin() {}

ItemBin ItemBin::Build()
{
    ItemBin item;
    return item;
}

ItemBin &ItemBin::setId(int id)
{
    this->id = id;
    return *this;
}

ItemBin &ItemBin::setTipo(int id)
{
    this->tipo = id;
    return *this;
}

ItemBin &ItemBin::setPosicion(Punto point)
{
    this->posicion = point;
    return *this;
}

ItemBin &ItemBin::setAlto(int alto)
{
    this->dimension.setAlto(alto);
    this->currentDimension.z = alto;
    return *this;
}

ItemBin &ItemBin::setLargo(int longitud)
{
    this->dimension.setLargo(longitud);
    this->currentDimension.x = longitud;
    return *this;
}

ItemBin &ItemBin::setAncho(int ancho)
{
    this->dimension.setAncho(ancho);
    this->currentDimension.y = ancho;
    return *this;
}

ItemBin &ItemBin::setCurrentDimension(Punto dim)
{
    this->currentDimension = dim;
    return *this;
}

ItemBin &ItemBin::setRotations(Punto rots)
{
    this->rotations = rots;
    return *this;
}

ItemBin &ItemBin::setRotation(ROTATION_WAY rot)
{
    this->rotation = rot;
    return *this;
}

int ItemBin::getId()
{
    return this->id;
}

int ItemBin::getTipo()
{
    return this->tipo;
}

int ItemBin::getLargo()
{
    return this->currentDimension.x;
}

int ItemBin::getAncho()
{
    return this->currentDimension.y;
}

int ItemBin::getAlto()
{
    return this->currentDimension.z;
}

Punto ItemBin::getRotations()
{
    return this->rotations;
}

ROTATION_WAY ItemBin::getRotation()
{
    return this->rotation;
}

Punto ItemBin::getPunto()
{
    return this->posicion;
}

Dim ItemBin::getDimension()
{
    return this->dimension;
}

ulong ItemBin::getVolumen()
{
    return this->dimension.getVolumen();
}

Punto ItemBin::getOriginalDimension()
{
    return this->dimension.getVector();
}

Punto ItemBin::getCurrentDimension()
{
    return this->currentDimension;
}

void ItemBin::rotate(ROTATION_WAY rotationWay, ROTATION_MODE rotationMode)
{

    switch (rotationWay)
    {
    case TWO:
        switch (rotationMode)
        {
        case ONE:
            this->currentDimension = this->dimension.getVector();
            this->hasValidRotation = this->rotations.z == 1;
            break;
        case TWO:
            this->currentDimension.x = this->dimension.getAncho();
            this->currentDimension.y = this->dimension.getLargo();
            this->currentDimension.z = this->dimension.getAlto();
            this->hasValidRotation = this->rotations.z == 1;
            break;
        default:
            break;
        }
        break;
    case SIX:
        switch (rotationMode)
        {
        case ONE:
            this->currentDimension = this->dimension.getVector();
            this->hasValidRotation = this->rotations.z == 1;
            break;
        case TWO:
            this->currentDimension.x = this->dimension.getLargo();
            this->currentDimension.y = this->dimension.getAlto();
            this->currentDimension.z = this->dimension.getAncho();
            this->hasValidRotation = this->rotations.y == 1;
            break;
        case THREE:
            this->currentDimension.x = this->dimension.getAncho();
            this->currentDimension.y = this->dimension.getLargo();
            this->currentDimension.z = this->dimension.getAlto();
            this->hasValidRotation = this->rotations.z == 1;
            break;
        case FOUR:
            this->currentDimension.x = this->dimension.getAncho();
            this->currentDimension.y = this->dimension.getAlto();
            this->currentDimension.z = this->dimension.getLargo();
            this->hasValidRotation = this->rotations.x == 1;
            break;
        case FIVE:
            this->currentDimension.x = this->dimension.getAlto();
            this->currentDimension.y = this->dimension.getLargo();
            this->currentDimension.z = this->dimension.getAncho();
            this->hasValidRotation = this->rotations.y == 1;
            break;
        case SIX:
            this->currentDimension.x = this->dimension.getAlto();
            this->currentDimension.y = this->dimension.getAncho();
            this->currentDimension.z = this->dimension.getLargo();
            this->hasValidRotation = this->rotations.x == 1;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

bool ItemBin::isValidOrientation()
{
    return this->hasValidRotation;
}

bool operator<(ItemBin bin1, ItemBin bin2)
{
    return bin1.getVolumen() < bin2.getVolumen();
}

bool operator>(ItemBin bin1, ItemBin bin2)
{
    return bin1.getVolumen() > bin2.getVolumen();
}

bool operator==(ItemBin bin1, ItemBin bin2)
{
    return bin1.getTipo() == bin2.getTipo();
}

std::ostream &operator<<(std::ostream &COUT, ItemBin &item)
{
    COUT << "ITEM BIN\n"
         << "Id: " << item.getId() << "\n"
         << "Type: " << item.getTipo() << "\n"
         << "Length: " << item.getDimension().getLargo() << "\n"
         << "Width: " << item.getDimension().getAncho() << "\n"
         << "Height: " << item.getDimension().getAlto() << "\n"
         << "Rotation vector: " << item.getRotations() << "\n"
         << "Current dimensions: " << item.getCurrentDimension() << "\n"
         << "Current rotation: " << getIdFromRotationWay(item.getRotation()) << "\n"
         << "Has valid rotation: " << item.isValidOrientation() << "\n";

    return COUT;
}