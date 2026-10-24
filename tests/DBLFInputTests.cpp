#include "../src/heuristics/DBLF.hpp"
#include "../src/genetic/FitnessOperators.hpp"
#include <functional>
#include <sstream>

void require(bool value, const char *message)
{
    if (!value) throw std::runtime_error(message);
}
ItemBin box(int id, int x = 1, int y = 1, int z = 1)
{
    return ItemBin::Build().setId(id).setTipo(id).setLargo(x).setAncho(y).setAlto(z)
        .setRotations(Punto::Build(1, 1, 1));
}
Bin container()
{
    return Bin::Build().setLargo(8).setAncho(8).setAlto(8).setRotationWay(ROTATION_WAY::ZERO_WAY);
}
void checkPruning()
{
    const Punto low = Punto::Build(0, 0, 0), high = Punto::Build(0, 2, 0);
    Bin occupied = container();
    occupied.addItem(box(99).setPosicion(low));
    ItemBin candidate = box(100);
    require(isOverlapped(low, candidate, occupied) && !isOverlapped(high, candidate, occupied),
            "Counterexample must have a blocked low point and a feasible high point");
    ColaPuntosDBL points;
    points.push(low); points.push(high); points.push(high); points.update();
    require(points.size() == 2, "Pruning removed a distinct useful point");
    PriorityPointQueue linked;
    linked.enqueue(low); linked.enqueue(high); linked.enqueue(high); linked.update();
    require(linked.getSize() == 2, "Linked pruning removed a distinct useful point");

    const Punto dimensions[] = {{2,6,6}, {2,5,3}, {6,4,6}, {1,1,1}, {3,1,4},
                                {6,2,3}, {1,4,3}, {1,5,4}, {1,3,5}, {5,3,3}};
    LoadedBins data;
    for (int i = 0; i < 10; ++i) data.push_back(box(i + 1, dimensions[i].x, dimensions[i].y, dimensions[i].z));
    for (bool useLinked : {false, true})
    {
        Bin bin = container();
        Chromosome ids{1,2,3,4,5,6,7,8,9,10}, rotations(10, 0);
        if (useLinked) DBLFQueue(bin, ids, rotations, data);
        else DBLF(bin, ids, rotations, data);
        require(bin.getNumberOfLoadedItems() == 9 && bin.getLoadedVolume() == 243,
                "Full pruning counterexample lost the tenth box (legacy volume 198)");
        require(bin.getLoadedItems().back().getId() == 10 &&
                bin.getLoadedItems().back().getPunto() == Punto::Build(3,5,3),
                "Tenth box did not occupy the recovered candidate");
    }
    std::cout << "PASS: pruning counterexample: 8 to 9 boxes, volume 198 to 243\n";
}

void expectRejected(const std::function<void(Bin &)> &operation)
{
    Bin bin = container();
    bool rejected = false;
    try { operation(bin); }
    catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "Invalid DBLF input was not rejected");
    require(bin.getNumberOfLoadedItems() == 0 && bin.getLoadedVolume() == 0 && bin.getLoadedItems().empty(),
            "DBLF modified the bin before validating all input");
}
void checkInputs()
{
    for (bool linked : {false, true})
    {
        auto reject = [linked](Chromosome ids, Chromosome rotations, LoadedBins data,
                               ROTATION_WAY mode = ROTATION_WAY::ZERO_WAY) {
            expectRejected([&](Bin &bin) {
                bin.setRotationWay(mode);
                if (linked) DBLFQueue(bin, ids, rotations, data);
                else DBLF(bin, ids, rotations, data);
            });
        };
        const LoadedBins valid{box(1), box(2)};
        reject({1,1}, {0,0}, valid); // Safe first failure against old code.
        reject({1,2}, {0}, valid);
        reject({1}, {0,0}, valid);
        reject({0}, {0}, valid);
        reject({-1}, {0}, valid);
        reject({3}, {0}, valid);
        reject({1,2}, {0,0}, {box(1), box(2, 0, 1, 1)});
        reject({1,2}, {0,0}, {box(1), box(2, 1, -1, 1)});
        reject({1,2}, {0,0}, {box(2), box(1)});
        reject({1,2}, {0,0}, {box(1), box(1)});
        reject({1}, {1}, valid); // ZERO_WAY only accepts gene 0.
        reject({1}, {0}, valid, ROTATION_WAY::TWO_WAY);
        reject({1}, {3}, valid, ROTATION_WAY::TWO_WAY);
        reject({1}, {7}, valid, ROTATION_WAY::SIX_WAY);
        reject({1}, {-1}, valid, ROTATION_WAY::SIX_WAY);
        reject({1}, {0}, valid, static_cast<ROTATION_WAY>(42));
        expectRejected([&](Bin &bin) {
            bin.setAlto(0);
            if (linked) DBLFQueue(bin, {1}, {0}, valid); else DBLF(bin, {1}, {0}, valid);
        });
        Bin empty = container();
        if (linked) DBLFQueue(empty, {}, {}, {}); else DBLF(empty, {}, {}, {});
        require(empty.getLoadedItems().empty(), "Empty packing must be a no-op");
        Bin subset = container();
        if (linked) DBLFQueue(subset, {2}, {0}, valid); else DBLF(subset, {2}, {0}, valid);
        require(subset.getLoadedItems().size() == 1 && subset.getLoadedItems()[0].getId() == 2,
                "A unique subset of input IDs must remain supported");
    }
    expectRejected([](Bin &bin) { DBLF(bin, LoadedBins{box(1), box(1)}); });
    expectRejected([](Bin &bin) { DBLF(bin, LoadedBins{box(1), box(2, 1, 1, 0)}); });
    expectRejected([](Bin &bin) { DBLF(bin, LoadedBins{box(0)}); });
    expectRejected([](Bin &bin) {
        Individuo individual = Individuo::Build().setGenome(DoubleGenome::Build().setGenome({1,2}).setDGenome({0}));
        evaluateFitnessDecodificated(individual, {box(1), box(2)}, bin);
    });
    std::cout << "PASS: DBLF input validation and rejection before mutation\n";
}

int main(int argc, char **argv)
{
    std::ostringstream diagnostics;
    auto *previous = std::cout.rdbuf(diagnostics.rdbuf());
    try
    {
        if (argc == 1 || std::string(argv[1]) != "--validation-only") checkPruning();
        checkInputs();
        std::cout.rdbuf(previous);
        std::cout << diagnostics.str();
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cout.rdbuf(previous);
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
