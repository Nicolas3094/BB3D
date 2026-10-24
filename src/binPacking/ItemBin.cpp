#include "ItemBin.hpp"

ItemBin::ItemBin()
{
    this->id = 0;
    this->tipo = 0;
    this->dimension = Dim::Build().setAlto(0).setAncho(0).setLargo(0);
    this->rotation = ROTATION_WAY::ZERO_WAY;
    this->currentDimension = Punto::Build(0, 0, 0);
    this->hasValidRotation = true;
    rotationId = 0;
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

int ItemBin::getId() const
{
    return this->id;
}

int ItemBin::getTipo() const
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

Punto ItemBin::getRotations() const
{
    return this->rotations;
}

ROTATION_WAY ItemBin::getRotation()
{
    return this->rotation;
}

Punto ItemBin::getPunto() const
{
    return this->posicion;
}

Dim ItemBin::getDimension() const
{
    return dimension;
}

std::int64_t ItemBin::getVolumen() const
{
    return dimension.getVolumen();
}

Punto ItemBin::getOriginalDimension()
{
    return dimension.getVector();
}

Punto ItemBin::getCurrentDimension() const
{
    return currentDimension;
}
int ItemBin::getRotationId() const
{
    return rotationId;
}
void ItemBin::rotate(ROTATION_WAY rotationWay, ROTATION_MODE rotationMode)
{
    this->rotationId = getIdFromRotation(rotationMode);
    this->rotation = rotationWay;
    currentDimension = dimension.getVector();
    hasValidRotation = false;
    if (rotationWay == ROTATION_WAY::ZERO_WAY)
    {
        hasValidRotation = rotationMode == ROTATION_MODE::NONE && rotations.z == 1;
        return;
    }
    if (rotationWay == ROTATION_WAY::TWO_WAY)
    {
        switch (rotationMode)
        {
        case ONE:
            currentDimension = dimension.getVector();
            hasValidRotation = rotations.z == 1;
            break;
        case TWO:
            currentDimension.x = dimension.getAncho();
            currentDimension.y = dimension.getLargo();
            currentDimension.z = dimension.getAlto();
            hasValidRotation = rotations.z == 1;
            break;
        default:
            break;
        }
    }
    else if (rotationWay == ROTATION_WAY::SIX_WAY)
    {
        switch (rotationMode)
        {
        case ONE:
            currentDimension = dimension.getVector();
            hasValidRotation = rotations.z == 1;
            break;
        case TWO:
            currentDimension.x = dimension.getLargo();
            currentDimension.y = dimension.getAlto();
            currentDimension.z = dimension.getAncho();
            hasValidRotation = rotations.y == 1;
            break;
        case THREE:
            currentDimension.x = dimension.getAncho();
            currentDimension.y = dimension.getLargo();
            currentDimension.z = dimension.getAlto();
            hasValidRotation = rotations.z == 1;
            break;
        case FOUR:
            currentDimension.x = dimension.getAncho();
            currentDimension.y = dimension.getAlto();
            currentDimension.z = dimension.getLargo();
            hasValidRotation = rotations.x == 1;
            break;
        case FIVE:
            currentDimension.x = dimension.getAlto();
            currentDimension.y = dimension.getLargo();
            currentDimension.z = dimension.getAncho();
            hasValidRotation = rotations.y == 1;
            break;
        case SIX:
            currentDimension.x = dimension.getAlto();
            currentDimension.y = dimension.getAncho();
            currentDimension.z = dimension.getLargo();
            hasValidRotation = rotations.x == 1;
            break;
        default:
            break;
        }
    }
}

bool ItemBin::isValidOrientation() const
{
    return hasValidRotation;
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
