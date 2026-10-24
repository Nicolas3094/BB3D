#include "Bin.hpp"
#include <limits>
#include <stdexcept>

Bin::~Bin() {}

Bin &Bin::setCandidatePolicy(DBLFCandidatePolicy policy) {
    if(policy!=DBLFCandidatePolicy::ThreePoints && policy!=DBLFCandidatePolicy::Projected && policy!=DBLFCandidatePolicy::BestOfBoth)
        throw std::invalid_argument("Unsupported DBLF candidate policy.");
    candidatePolicy=policy; return *this;
}

Bin::Bin()
{
    this->rotation = ROTATION_WAY::ZERO_WAY;
    this->dimensions = Dim::Build().setAlto(0).setAncho(0).setLargo(0);
    this->numberOfLoadedItems = 0;
    this->loadedVolume = 0;
}
Bin Bin::Build()
{
    Bin bin;
    return bin;
}

Bin &Bin::addItem(ItemBin itemBin)
{
    const std::int64_t volume = itemBin.getVolumen();
    if (loadedVolume > std::numeric_limits<std::int64_t>::max() - volume)
        throw std::overflow_error("Loaded volume exceeds int64 range.");
    this->loadedItems.push_back(itemBin);
    this->numberOfLoadedItems += 1;
    this->loadedVolume += volume;
    return *this;
}
Bin &Bin::setLargo(int largo)
{
    this->dimensions.setLargo(largo);
    return *this;
}
Bin &Bin::setAncho(int ancho)
{
    this->dimensions.setAncho(ancho);
    return *this;
}
Bin &Bin::setAlto(int alto)
{
    this->dimensions.setAlto(alto);
    return *this;
}

Bin &Bin::setRotationWay(ROTATION_WAY rotation)
{
    this->rotation = rotation;
    return *this;
}

const Dim Bin::getDimensions() const
{
    return dimensions;
}
std::int64_t Bin::getLoadedVolume() const
{
    return this->loadedVolume;
}

int Bin::getNumberOfLoadedItems() const
{
    return this->numberOfLoadedItems;
}

const LoadedBins &Bin::getLoadedItems() const
{
    return this->loadedItems;
}
std::vector<Punto> Bin::getPostitionOfItems()
{
    std::vector<Punto> points;
    for (int i = 0; i < loadedItems.size(); i++)
    {
        points.push_back(loadedItems[i].getPunto());
    }
    return points;
}

std::vector<int> Bin::getIdItems()
{
    std::vector<int> ids;
    for (int i = 0; i < loadedItems.size(); i++)
    {
        ids.push_back(loadedItems[i].getId());
    }
    return ids;
}

ROTATION_WAY Bin::getRotationWay() const
{
    return this->rotation;
}

std::ostream &operator<<(std::ostream &COUT, const LoadedBins &loadedBins)
{
    COUT << "LOADED BINS\n"
         << "size: " << loadedBins.size()
         << "\nLoaded Items: |";

    for (const auto &items : loadedBins)
    {
        COUT << items.getId() << "|";
    }
    COUT << "\n";

    return COUT;
}
std::ostream &operator<<(std::ostream &COUT, Bin bin)
{
    COUT << "BIN\n"
         << "Length: " << bin.getDimensions().getLargo() << "\n"
         << "Width: " << bin.getDimensions().getAncho() << "\n"
         << "Height: " << bin.getDimensions().getAlto() << "\n"
         << "Number Of LoadedItems: " << bin.getNumberOfLoadedItems() << "\n"
         << "Loaded volume: " << bin.getLoadedVolume() << "\n"
         << "Rotation way: " << getIdFromRotationWay(bin.getRotationWay()) << "\n";

    return COUT;
}
