#include "../src/heuristics/DBLF.hpp"
#include "../src/binPacking/ItemBinOperators.hpp"
#include "../src/genetic/FitnessOperators.hpp"
#include "../src/metaheuristics/GeneticAlgorithm.hpp"
#include "../src/metaheuristics/ArtificialBeeColonyAlgorithm.hpp"
#include "../src/metaheuristics/FireflyAlgorithm.hpp"
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <random>
#include <sstream>
#include <cmath>

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

// Reference implementation: retain the original unit-step behavior for parity tests.
void legacyCompaction(Punto &point, const ItemBin &item, const Bin &bin)
{
    if (bin.getNumberOfLoadedItems() <= 1) return;
    for (int *coordinate : {&point.x, &point.z, &point.y})
    {
        if (*coordinate == 0) continue;
        while (!isOverlapped(point, item, bin))
        {
            --*coordinate;
            if (*coordinate == -1) break;
        }
        ++*coordinate;
    }
}

void checkCompactionParity()
{
    std::mt19937 random(20261023);
    std::uniform_int_distribution<int> coordinate(0, 30), size(1, 4);
    int checked = 0;
    for (int trial = 0; trial < 2000; ++trial)
    {
        Bin bin = Bin::Build().setLargo(40).setAncho(40).setAlto(40);
        const int count = trial % 9;
        for (int i = 0; i < count; ++i)
        {
            const int length = size(random), width = size(random), height = size(random);
            const int x = coordinate(random), y = coordinate(random), z = coordinate(random);
            bin.addItem(cube(i, 1, Punto::Build(x, y, z))
                .setLargo(length).setAncho(width).setAlto(height));
        }
        const int length = size(random), width = size(random), height = size(random);
        ItemBin item = cube(100, 1, Punto::Build(0, 0, 0))
            .setLargo(length).setAncho(width).setAlto(height);
        const int x = coordinate(random), y = coordinate(random), z = coordinate(random);
        Punto before = Punto::Build(x, y, z), after = before;
        if (isOverlapped(before, item, bin)) continue; // DBLF's precondition.
        legacyCompaction(before, item, bin);
        iterateByDeepestBottomLeft(after, item, bin);
        require(before == after, "Contact compaction differs from unit-step reference");
        require(!isOverlapped(after, item, bin), "Compaction introduced overlap");
        require(after.x >= 0 && after.y >= 0 && after.z >= 0, "Compaction crossed wall");
        ++checked;
    }
    require(checked > 1900, "Too few valid parity cases");
    std::cout << "PASS: " << checked << " seeded unit-step/contact parity cases\n";
}

void checkRestrictedOrientations()
{
    // The decoded overload prints diagnostics; restore cout even on failure.
    std::ostringstream diagnostics;
    struct Silence
    {
        std::streambuf *previous;
        explicit Silence(std::streambuf *sink) : previous(std::cout.rdbuf(sink)) {}
        ~Silence() { std::cout.rdbuf(previous); }
    } silence(diagnostics.rdbuf());

    for (ROTATION_WAY way : {ROTATION_WAY::TWO_WAY, ROTATION_WAY::SIX_WAY})
    for (int mask = 0; mask < 8; ++mask)
    for (int mode = 1; mode <= getIdFromRotationWay(way); ++mode)
    {
        const Punto permitted = Punto::Build(mask & 1, (mask >> 1) & 1, (mask >> 2) & 1);
        // Vertical original axis per mode: TWO_WAY always Z; SIX_WAY Z,Y,Z,X,Y,X.
        const int verticalAxis[] = {2, 1, 2, 0, 1, 0};
        const int axis = way == ROTATION_WAY::TWO_WAY ? 2 : verticalAxis[mode - 1];
        const bool allowed = ((mask >> axis) & 1) != 0;
        LoadedBins data{
            cube(1, 1, Punto::Build(0, 0, 0)).setLargo(2).setAncho(3).setAlto(4).setRotations(permitted),
            cube(2, 1, Punto::Build(0, 0, 0)).setLargo(2).setAncho(3).setAlto(4)};
        const Chromosome order{1, 2}, rotations{mode, mode};
        Individuo individual = Individuo::Build().setGenome(
            DoubleGenome::Build().setGenome(order).setDGenome(rotations));
        for (int variant = 0; variant < 3; ++variant)
        {
            Bin bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10).setRotationWay(way);
            if (variant == 0) DBLF(bin, order, rotations, data);
            else if (variant == 1) DBLFQueue(bin, order, rotations, data);
            else evaluateFitnessDecodificated(individual, data, bin);
            require(bin.getNumberOfLoadedItems() == (allowed ? 2 : 1),
                    "DBLF packed a forbidden orientation or skipped a valid one");
            require(bin.getLoadedVolume() == (allowed ? 48 : 24), "Forbidden orientation increased loaded volume");
            const auto &loaded = bin.getLoadedItems();
            require(loaded.front().getId() == (allowed ? 1 : 2), "Skipping a box misaligned the chromosome");
            if (!allowed)
                require(loaded.front().getPunto() == Punto::Build(0, 0, 0), "Skipped box consumed origin");
            if (variant == 2)
                require(std::abs(individual.getFitness() - (allowed ? 0.048 : 0.024)) < 1e-8,
                        "Decoded fitness counts forbidden volume");
        }
        Bin bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10).setRotationWay(way);
        evaluateFitness(individual, data, bin);
        require(std::abs(individual.getFitness() - (allowed ? 0.048 : 0.024)) < 1e-8,
                "Fitness counts forbidden volume");
        if (!allowed)
            require(individual.solution == "|2 " + std::to_string(mode) + "|",
                    "Solution code includes forbidden box");
    }
    // Different genes expose accidental reuse of the skipped item's rotation.
    for (bool allForbidden : {false, true})
    for (int variant = 0; variant < 3; ++variant)
    {
        LoadedBins data{
            cube(1, 1, Punto::Build(0, 0, 0)).setLargo(2).setAncho(3).setAlto(4)
                .setRotations(Punto::Build(1, 0, 0)),
            cube(2, 1, Punto::Build(0, 0, 0)).setLargo(2).setAncho(3).setAlto(4)
                .setRotations(allForbidden ? Punto::Build(0, 0, 0) : Punto::Build(1, 1, 1))};
        Individuo individual = Individuo::Build().setGenome(DoubleGenome::Build()
            .setGenome({1, 2}).setDGenome({1, 4}));
        Bin bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10)
            .setRotationWay(ROTATION_WAY::SIX_WAY);
        if (variant == 0) DBLF(bin, {1, 2}, {1, 4}, data);
        else if (variant == 1) DBLFQueue(bin, {1, 2}, {1, 4}, data);
        else evaluateFitnessDecodificated(individual, data, bin);
        require(bin.getNumberOfLoadedItems() == (allForbidden ? 0 : 1),
                "All-forbidden/alignment case packed wrong number of boxes");
        require(bin.getLoadedVolume() == (allForbidden ? 0 : 24), "Forbidden boxes contributed volume");
        if (!allForbidden)
        {
            const auto &loaded = bin.getLoadedItems().front();
            require(loaded.getId() == 2 && loaded.getRotationId() == 4 &&
                    loaded.getCurrentDimension() == Punto::Build(3, 4, 2),
                    "Skipped item shifted the following rotation gene");
        }
        evaluateFitness(individual, data, bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10)
            .setRotationWay(ROTATION_WAY::SIX_WAY));
        require(std::abs(individual.getFitness() - (allForbidden ? 0.0 : 0.024)) < 1e-8,
                "All-forbidden/alignment case produced invalid fitness");
        require(individual.solution == (allForbidden ? "|" : "|2 4|"),
                "All-forbidden/alignment case produced invalid solution code");
    }
}

void checkMutationIntegration()
{
    DatasetBinBacking problem{};
    problem.bin = Bin::Build().setLargo(4).setAncho(4).setAlto(4).setRotationWay(ROTATION_WAY::SIX_WAY);
    for (int id = 1; id <= 8; ++id) problem.totalItems.push_back(cube(id, 1, Punto::Build(0,0,0)));
    auto verify = [](Poblacion population) {
        require(population.size() == 6, "Metaheuristic changed population size");
        for (auto &individual : population)
        {
            auto ids = individual.getGenome().getGenome(); std::sort(ids.begin(), ids.end());
            require(ids == Chromosome({1,2,3,4,5,6,7,8}), "Metaheuristic mutation corrupted item permutation");
            require(std::isfinite(individual.getFitness()) && std::abs(individual.getFitness() - 0.125) < 1e-12,
                    "Metaheuristic mutation produced incorrect fitness");
        }
    };
    for (MutationType type : {MutationType::C1, MutationType::C2})
    {
        for (bool replacement : {false,true})
        {
            GeneticAlgorithm ga;
            ga.setProblem(problem).setNumberOfIndividuals(6).setMaxIteration(2).setRotationType(ROTATION_WAY::SIX_WAY)
                .setMutationType(type).setIsWithReplacement(replacement).setCrossProbability(1.0).setSelectionProbability(0.8);
            verify(ga.evolve());
        }
        ArtificialBeeColonyAlgorithm abc;
        abc.setProblem(problem).setNumberOfIndividuals(6).setMaxIteration(2).setRotationType(ROTATION_WAY::SIX_WAY)
            .setMutationType(type).setIsWithReplacement(true).setNumberOfSites(3).setNumberOfEliteSites(1)
            .setNumberOfEliteBees(2).setNumberOfNonEliteBees(1);
        verify(abc.evolve());
        FireflyAlgorithm firefly;
        firefly.setProblem(problem).setNumberOfIndividuals(6).setMaxIteration(2).setRotationType(ROTATION_WAY::SIX_WAY)
            .setMutationType(type).setIsWithReplacement(true).setupIndex(2);
        verify(firefly.evolve());
    }
    std::cout << "PASS: real C1/C2 mutations through GA, ABC and Firefly\n";
}

int main()
{
    try
    {
        checkMutationIntegration();
        checkRestrictedOrientations();
        std::cout << "PASS: all 8 permission masks, 2/6 rotations, three DBLF variants and both fitness paths\n";
        checkCompactionParity();
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
        // The direct calculation must also work for long movements without a unit loop.
        Punto distant = Punto::Build(100000000, 0, 0);
        Bin wideBin = bin;
        wideBin.setLargo(100000001);
        iterateByDeepestBottomLeft(distant, item, wideBin);
        require(distant == Punto::Build(2, 0, 0), "Long-distance contact is incorrect");

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
