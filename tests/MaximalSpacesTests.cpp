#include "../src/spatial/MaximalSpace.hpp"
#include "../src/heuristics/MaximalSpacesDecoder.hpp"
#include "../src/heuristics/DBLF.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/genetic/FitnessOperators.hpp"
#include "../src/util/RandomNumbers.hpp"
#include "../src/util/ExecutionBudget.hpp"
#include "PackingAudit.hpp"
#include <random>

namespace
{
void require(bool condition, const char *message)
{
    if (!condition)
        throw std::runtime_error(message);
}

ItemBin makeItem(int id, int length, int width, int height)
{
    return ItemBin::Build().setId(id).setTipo(id).setLargo(length)
        .setAncho(width).setAlto(height).setRotations({1, 1, 1});
}

Bin makeContainer()
{
    return Bin::Build().setLargo(10).setAncho(10).setAlto(10);
}

void testSpaceGeometry()
{
    const MaximalSpace containerSpace = {{0, 0, 0}, {10, 10, 10}};
    const MaximalSpace occupied = {{2, 3, 4}, {7, 8, 9}};
    const auto pieces = splitFreeSpace(containerSpace, occupied);
    require(pieces.size() == 6, "An interior box must produce six slabs");
    for (const auto &space : pieces)
        require(hasPositiveDimensions(space) && containsSpace(containerSpace, space) &&
                !spacesIntersect(space, occupied), "A split slab is not empty and in bounds");

    // Independent voxel oracle: overlapping slabs must cover exactly the empty portion.
    for (int x = 0; x < 10; ++x)
        for (int y = 0; y < 10; ++y)
            for (int z = 0; z < 10; ++z)
            {
                auto containsCell = [&](const MaximalSpace &space) {
                    return x >= space.minimum.x && x < space.maximum.x &&
                           y >= space.minimum.y && y < space.maximum.y &&
                           z >= space.minimum.z && z < space.maximum.z;
                };
                bool covered = false;
                for (const auto &space : pieces)
                    covered |= containsCell(space);
                require(covered == !containsCell(occupied), "Split changed the empty-space union");
            }

    std::vector<MaximalSpace> redundant = {
        containerSpace, containerSpace, {{1, 1, 1}, {2, 2, 2}}, {{0, 0, 0}, {0, 1, 1}}
    };
    removeRedundantSpaces(redundant);
    require(redundant.size() == 1, "Pruning must remove duplicates, containment and zero extent");

    std::vector<MaximalSpace> overlapping = {
        {{0, 0, 0}, {10, 5, 10}}, {{0, 0, 0}, {5, 10, 10}}
    };
    const MaximalSpace cornerBox = {{0, 0, 0}, {3, 3, 3}};
    subtractOccupiedSpace(overlapping, cornerBox);
    require(overlapping.size() == 6, "Both overlapping spaces must be updated");
    for (const auto &space : overlapping)
        require(!spacesIntersect(space, cornerBox), "An updated space still intersects the box");
}

void testPlacementAndReference(const LoadedBins &cubes)
{
    auto exactFit = makeContainer();
    exactFit.setDecoder(PackingDecoder::MaximalSpaces);
    const LoadedBins input = {makeItem(1, 11, 1, 1), makeItem(2, 10, 10, 10), makeItem(3, 1, 1, 1)};
    DBLF(exactFit, {1, 2, 3}, {0, 0, 0}, input);
    auditPacking(exactFit, input, {1, 2, 3}, {0, 0, 0});
    require(exactFit.getLoadedVolume() == 1000 && exactFit.getLoadedItems()[0].getId() == 2,
            "A rejected box must not prevent a later exact fit");

    auto floating = makeContainer();
    floating.setDecoder(PackingDecoder::MaximalSpaces);
    const LoadedBins bridge = {makeItem(1, 2, 2, 2), makeItem(2, 10, 10, 2)};
    DBLF(floating, {1, 2}, {0, 0}, bridge);
    auditPacking(floating, bridge, {1, 2}, {0, 0});
    require(floating.getNumberOfLoadedItems() == 2 &&
            floating.getLoadedItems()[1].getPunto() == Punto(0, 0, 2),
            "The decoder must not introduce a full-support restriction");

    // Frozen original DBLF fixture: X, then Z, then Y, with unchanged compaction.
    auto original = makeContainer();
    DBLF(original, {1, 2, 3}, {0, 0, 0}, cubes);
    require(original.getLoadedVolume() == 375, "DBLF reference volume changed");
    require(original.getLoadedItems()[0].getPunto() == Punto(0, 0, 0), "DBLF origin changed");
    require(original.getLoadedItems()[1].getPunto() == Punto(0, 5, 0), "DBLF Y priority changed");
    require(original.getLoadedItems()[2].getPunto() == Punto(0, 0, 5), "DBLF Z priority changed");

    auto packed = makeContainer();
    packed.setDecoder(PackingDecoder::MaximalSpaces);
    DBLF(packed, {1, 2, 3}, {0, 0, 0}, cubes);
    for (size_t index = 0; index < 3; ++index)
        require(packed.getLoadedItems()[index].getPunto() == original.getLoadedItems()[index].getPunto(),
                "Maximal spaces must follow the DBL fixture order");

    auto preoriented = makeContainer();
    preoriented.setDecoder(PackingDecoder::MaximalSpaces);
    maximalSpacesDecode(preoriented, cubes);
    require(preoriented.getLoadedVolume() == 375, "The preoriented entry point changed");
    auto queueEntry = makeContainer();
    queueEntry.setDecoder(PackingDecoder::MaximalSpaces);
    DBLFQueue(queueEntry, {1, 2, 3}, {0, 0, 0}, cubes);
    require(queueEntry.getLoadedVolume() == 375, "The queue entry point did not dispatch correctly");
}

void testRandomPackings()
{
    std::mt19937 random(123);
    for (int trial = 0; trial < 100; ++trial)
    {
        LoadedBins items;
        Chromosome itemIds, rotationGenes;
        for (int id = 1; id <= 15; ++id)
        {
            items.push_back(makeItem(id, 1 + random() % 7, 1 + random() % 7, 1 + random() % 7));
            itemIds.push_back(id);
            rotationGenes.push_back(1 + random() % 6);
        }
        std::shuffle(itemIds.begin(), itemIds.end(), random);
        auto packed = makeContainer();
        packed.setRotationWay(ROTATION_WAY::SIX_WAY).setDecoder(PackingDecoder::MaximalSpaces);
        auto repeated = packed;
        DBLF(packed, itemIds, rotationGenes, items);
        DBLF(repeated, itemIds, rotationGenes, items);
        auditPacking(packed, items, itemIds, rotationGenes);
        require(packed.getLoadedVolume() == repeated.getLoadedVolume() &&
                packed.getLoadedItems().size() == repeated.getLoadedItems().size(),
                "Repeated evaluation changed the loaded boxes");
        for (size_t index = 0; index < packed.getLoadedItems().size(); ++index)
            require(packed.getLoadedItems()[index].getPunto() == repeated.getLoadedItems()[index].getPunto(),
                    "Repeated evaluation changed a placement");
    }
}

void testOrientationPermissions()
{
    for (int permissionMask = 0; permissionMask < 8; ++permissionMask)
    {
        for (auto rotationWay : {ROTATION_WAY::SIX_WAY, ROTATION_WAY::TWO_WAY})
        {
            const int maximumGene = getIdFromRotationWay(rotationWay);
            for (int gene = 1; gene <= maximumGene; ++gene)
            {
                auto item = makeItem(1, 2, 3, 4);
                item.setRotations({permissionMask & 1, (permissionMask >> 1) & 1, (permissionMask >> 2) & 1});
                auto packed = makeContainer();
                packed.setDecoder(PackingDecoder::MaximalSpaces).setRotationWay(rotationWay);
                DBLF(packed, {1}, {gene}, {item});
                auditPacking(packed, {item}, {1}, {gene});

                // Independent mapping of the original dimension that becomes vertical.
                int verticalAxis = 2;
                if (rotationWay == ROTATION_WAY::SIX_WAY)
                {
                    if (gene == 2 || gene == 5)
                        verticalAxis = 1;
                    if (gene == 4 || gene == 6)
                        verticalAxis = 0;
                }
                const int permitted = (permissionMask >> verticalAxis) & 1;
                require(packed.getNumberOfLoadedItems() == permitted, "Orientation permission was ignored");
            }
        }
    }
}

void testConfigurationAndCache(const LoadedBins &cubes)
{
    const auto options = parseCommandLine({"--decoder", "maximal-spaces"});
    require(options.decoder == PackingDecoder::MaximalSpaces, "CLI decoder selection failed");
    const auto config = options.configuration("GA", ROTATION_WAY::ZERO_WAY, MutationType::C2, false);
    auto legacy = config;
    legacy.decoder = PackingDecoder::DBLF;
    require(config.identity("a", 123) != legacy.identity("a", 123), "Decoder checkpoint identities collided");

    configureFitnessCache(10, 100000);
    auto individual = Individuo::Build().setGenome(
        DoubleGenome::Build().setGenome({1, 2, 3}).setDGenome({0, 0, 0}));
    auto bin = makeContainer();
    evaluateFitness(individual, cubes, bin);
    evaluateFitness(individual, cubes, bin);
    require(fitnessCacheStats().hits == 1, "Identical evaluations should use the cache");
    bin.setDecoder(PackingDecoder::MaximalSpaces);
    evaluateFitness(individual, cubes, bin);
    require(fitnessCacheStats().misses == 2, "Changing decoder must invalidate the cache");
    auto changedItems = cubes;
    changedItems[0].setLargo(4);
    evaluateFitness(individual, changedItems, bin);
    require(fitnessCacheStats().misses == 3, "Changing items must invalidate the cache");
    bin.setAlto(11);
    evaluateFitness(individual, changedItems, bin);
    require(fitnessCacheStats().misses == 4, "Changing container must invalidate the cache");
}

void testInterruptedPackings(const LoadedBins &cubes)
{
    for (int cutoff : {10, 30, 80, 200})
    {
        auto partial = makeContainer();
        partial.setDecoder(PackingDecoder::MaximalSpaces);
        int deadlineChecks = 0;
        {
            ExecutionBudget budget(std::chrono::seconds(30), Individuo::Build(), [&] {
                const bool expired = ++deadlineChecks > cutoff;
                return ExecutionBudget::Clock::time_point{} + std::chrono::seconds(expired ? 31 : 0);
            });
            try
            {
                DBLF(partial, {1, 2, 3}, {0, 0, 0}, cubes);
            }
            catch (const ExecutionTimeLimit &)
            {
                // The current partial packing is the result under test.
            }
        }
        auditPacking(partial, cubes, {1, 2, 3}, {0, 0, 0});
    }
}

void testMetaheuristicIntegration(const LoadedBins &cubes)
{
    DatasetBinBacking data{};
    data.bin = makeContainer();
    data.totalItems = cubes;
    const auto options = parseCommandLine({"--decoder", "maximal-spaces"});
    auto config = options.configuration("GA", ROTATION_WAY::ZERO_WAY, MutationType::C2, false);
    config.individuals = 12;
    config.sites = 4;
    config.eliteSites = 2;
    config.iterations = 1;
    for (const std::string algorithm : {"GA", "ABC", "FFA"})
    {
        config.algorithm = algorithm;
        seedRandomNumbers(123);
        const auto result = config.execute(data);
        require(!result.population.empty(), "A metaheuristic returned an empty population");
        auto replay = makeContainer();
        replay.setDecoder(PackingDecoder::MaximalSpaces);
        const auto best = result.population.front();
        const auto itemIds = best.getGenome().getGenome();
        const auto rotationGenes = best.getGenome().getDGenome();
        DBLF(replay, itemIds, rotationGenes, cubes);
        auditPacking(replay, cubes, itemIds, rotationGenes);
        require(best.getFitness() == static_cast<double>(replay.getLoadedVolume()) / 1000,
                "Reconstruction changed the best individual's fitness");
    }
}
}

int main()
{
    try
    {
        const LoadedBins cubes = {makeItem(1, 5, 5, 5), makeItem(2, 5, 5, 5), makeItem(3, 5, 5, 5)};
        testSpaceGeometry();
        testPlacementAndReference(cubes);
        testRandomPackings();
        testOrientationPermissions();
        testConfigurationAndCache(cubes);
        testInterruptedPackings(cubes);
        testMetaheuristicIntegration(cubes);
        std::cout << "PASS: split/voxel union, pruning, overlapping updates, exact fit, skip, DBLF fixture, "
                     "100 random audited packings, orientations, cache and GA/ABC/FFA\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
