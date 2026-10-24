#include "../src/heuristics/DBLF.hpp"
#include <random>

Bin pack(LoadedBins items, bool legacy)
{
    Bin bin = Bin::Build().setLargo(8).setAncho(8).setAlto(8).setRotationWay(ROTATION_WAY::ZERO_WAY);
    ColaPuntosDBL points;
    points.push(Punto::Build(0, 0, 0));
    for (auto item : items)
    for (int j = points.size() - 1; j >= 0; --j)
    {
        Punto p = points[j];
        if (!canBePlaced(bin, p + item.getCurrentDimension()) || isOverlapped(p, item, bin)) continue;
        points.remove(j);
        iterateByDeepestBottomLeft(p, item, bin);
        bin.addItem(item.setPosicion(p));
        const Punto end = p + item.getCurrentDimension();
        if (end.x < 8) points.push(Punto::Build(end.x, p.y, p.z));
        if (end.y < 8) points.push(Punto::Build(p.x, end.y, p.z));
        if (end.z < 8) points.push(Punto::Build(p.x, p.y, end.z));
        if (legacy)
            for (int i = points.size() - 1; i > 0; --i)
            {
                const Punto a = points[i], b = points[i - 1];
                if ((a.x == b.x && a.z == b.z) || (a.x < b.x && a.y == b.y && a.z == b.z))
                    points.remove(i - 1);
            }
        break;
    }
    return bin;
}

int main()
{
    std::mt19937 random(20261023);
    std::uniform_int_distribution<int> side(1, 6);
    for (int trial = 0; trial < 100000; ++trial)
    {
        LoadedBins items;
        for (int id = 1; id <= 10; ++id)
        {
            const int x = side(random), y = side(random), z = side(random);
            items.push_back(ItemBin::Build().setId(id).setTipo(id).setLargo(x).setAncho(y).setAlto(z)
                .setRotations(Punto::Build(1, 1, 1)));
        }
        Bin before = pack(items, true), after = pack(items, false);
        if (after.getLoadedVolume() <= before.getLoadedVolume()) continue;
        std::cout << "trial=" << trial << " before=" << before.getLoadedVolume() << " after=" << after.getLoadedVolume() << '\n';
        for (auto item : items) std::cout << item.getId() << ": " << item.getCurrentDimension() << '\n';
        for (auto bin : {before, after})
        {
            std::cout << "packed: ";
            for (const auto &item : bin.getLoadedItems()) std::cout << item.getId() << '@' << item.getPunto() << ' ';
            std::cout << '\n';
        }
        return 0;
    }
    return 1;
}
