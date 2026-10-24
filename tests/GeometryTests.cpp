#include "../src/heuristics/DBLF.hpp"
#include "../src/binPacking/ItemBinOperators.hpp"
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef GEOMETRY_BEHAVIOR_ONLY
static_assert(std::is_same<decltype(&canBePlaced), bool (*)(const Bin &, const Punto &)>::value,
              "canBePlaced must borrow the bin");
static_assert(std::is_same<decltype(&isOverlapped), bool (*)(Punto, const ItemBin &, const Bin &)>::value,
              "isOverlapped must borrow the item and bin");
static_assert(std::is_same<decltype(&iterateByDeepestBottomLeft), void (*)(Punto &, const ItemBin &, const Bin &)>::value,
              "Compaction must borrow the item and bin");
static_assert(std::is_same<decltype(std::declval<const Bin &>().getLoadedItems()), const LoadedBins &>::value,
              "Loaded items must be a const reference from a const bin");
#endif

void require(bool condition, const char *message)
{
    if (!condition) throw std::runtime_error(message);
}

ItemBin cube(int id, int size, Punto position)
{
    return ItemBin::Build().setId(id).setTipo(id).setLargo(size).setAncho(size)
        .setAlto(size).setPosicion(position).setRotations(Punto::Build(1, 1, 1));
}

int main()
{
    try
    {
        Bin bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10)
            .setRotationWay(ROTATION_WAY::ZERO_WAY);
        ItemBin item = cube(3, 1, Punto::Build(7, 7, 7));
        require(canBePlaced(bin, Punto::Build(10, 10, 10)), "Boundary must fit");
        for (Punto outside : {Punto::Build(11, 1, 1), Punto::Build(1, 11, 1), Punto::Build(1, 1, 11)})
            require(!canBePlaced(bin, outside), "Outside envelope must not fit");
        require(!isOverlapped(Punto::Build(0, 0, 0), item, bin), "Empty bin cannot overlap");
        bin.addItem(cube(1, 2, Punto::Build(0, 0, 0)));
        bin.addItem(cube(2, 2, Punto::Build(8, 8, 8)));
        require(isOverlapped(Punto::Build(1, 1, 1), item, bin), "Intersection not detected");
        for (Punto touching : {Punto::Build(2, 0, 0), Punto::Build(0, 2, 0), Punto::Build(0, 0, 2)})
            require(!isOverlapped(touching, item, bin), "Face contact must not overlap");
        Punto x = Punto::Build(5, 0, 0), y = Punto::Build(0, 5, 0), z = Punto::Build(0, 0, 5);
        iterateByDeepestBottomLeft(x, item, bin);
        iterateByDeepestBottomLeft(y, item, bin);
        iterateByDeepestBottomLeft(z, item, bin);
        require(x == Punto::Build(2, 0, 0) && y == Punto::Build(0, 2, 0) && z == Punto::Build(0, 0, 2),
                "Compaction positions changed");
        require(item.getPunto() == Punto::Build(7, 7, 7) && item.getCurrentDimension() == Punto::Build(1, 1, 1),
                "Geometry query modified item");
        require(bin.getNumberOfLoadedItems() == 2 && bin.getLoadedVolume() == 16,
                "Geometry query modified bin");

#ifndef GEOMETRY_BEHAVIOR_ONLY
        const Bin &readonly = bin;
        const LoadedBins &items = readonly.getLoadedItems();
        require(&items == &readonly.getLoadedItems() && items.data() == bin.getLoadedItems().data(),
                "getLoadedItems returned copied storage");
        const ItemBin &readonlyItem = item;
        require(!isOverlapped(x, readonlyItem, readonly), "Const geometry queries failed");
#endif
        Bin packed = Bin::Build().setLargo(4).setAncho(4).setAlto(4)
            .setRotationWay(ROTATION_WAY::ZERO_WAY);
        LoadedBins data;
        for (int id = 1; id <= 8; ++id) data.push_back(cube(id, 2, Punto::Build(0, 0, 0)));
        DBLF(packed, Chromosome({1, 2, 3, 4, 5, 6, 7, 8}), Chromosome(8, 0), data);
        require(packed.getNumberOfLoadedItems() == 8 && packed.getLoadedVolume() == 64,
                "DBLF cube fixture changed");
        std::cout << "PASS: geometry boundaries, collisions, compaction, unchanged inputs and DBLF fixture\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
