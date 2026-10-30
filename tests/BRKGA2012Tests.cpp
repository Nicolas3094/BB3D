#include "../src/brkga/BRKGA2012.hpp"
#include "../src/brkga/BRKGAAudit.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/BRExperimentMenu.hpp"
#include <limits>
#include <sstream>
#include <cmath>

using namespace brkga2012;
namespace
{
void require(bool condition, const char *message)
{
    if (!condition) throw std::runtime_error(message);
}
template<class Function> void rejects(Function operation)
{
    bool rejected = false;
    try { operation(); } catch (const std::exception &) { rejected = true; }
    require(rejected, "Invalid BRKGA input was accepted");
}
ItemBin item(int id, int type, int x, int y, int z)
{
    return ItemBin::Build().setId(id).setTipo(type).setLargo(x).setAncho(y).setAlto(z).setRotations({1,1,1});
}
DatasetBinBacking instance(int length = 10, int width = 10, int height = 10)
{
    DatasetBinBacking data{};
    data.id = 1;
    data.bin = Bin::Build().setLargo(length).setAncho(width).setAlto(height).setRotationWay(ROTATION_WAY::SIX_WAY);
    return data;
}
void verifyRepeat(const SearchResult &first, const SearchResult &second)
{
    require(first.keys == second.keys && first.packing.getLoadedVolume() == second.packing.getLoadedVolume(),
            "BRKGA repeat changed genotype or fitness");
    const auto &a = first.packing.getLoadedItems(), &b = second.packing.getLoadedItems();
    require(a.size() == b.size(), "BRKGA repeat changed quantities");
    for (size_t index = 0; index < a.size(); ++index)
        require(a[index].getId() == b[index].getId() && a[index].getRotationId() == b[index].getRotationId() &&
                a[index].getPunto() == b[index].getPunto(), "BRKGA repeat changed placement");
}

void testEncoding()
{
    auto data = instance();
    const int labels[8] = {1,1,2,2,2,3,4,4};
    for (int id = 1; id <= 8; ++id) data.totalItems.push_back(item(id, labels[id - 1], 1,1,1));
    const auto types = buildBoxTypes(data);
    RandomKeys keys = {.45,.67,.35,.49,.07,.78,.87,.17, 0,0,0,0,0,0,0,0};
    require(decodeTypeSequence(keys, types) == std::vector<size_t>({1,3,1,0,1,0,2,3}),
            "Published Fig. 3 type sequence differs");
    require(decodeTypeSequence(RandomKeys(16, 0), types) == std::vector<size_t>({0,0,1,1,1,2,3,3}),
            "Equal random keys must preserve canonical type order");
    rejects([&] { decodeTypeSequence(RandomKeys(15,0), types); });
    for (double bad : {-1.0, 1.01, std::numeric_limits<double>::quiet_NaN()})
    {
        keys[0] = bad;
        rejects([&] { decodeTypeSequence(keys, types); });
    }
    auto mixed = data;
    mixed.totalItems[1].setLargo(2);
    rejects([&] { buildBoxTypes(mixed); });
    auto duplicate = data;
    duplicate.totalItems[1].setId(1);
    rejects([&] { buildBoxTypes(duplicate); });
    rejects([&] { buildBoxTypes(instance()); });

    auto differentTypes = instance();
    differentTypes.totalItems = {item(1,1,2,3,4), item(2,2,3,4,2)};
    auto packed = differentTypes.bin;
    decodeLayers(packed, buildBoxTypes(differentTypes), {.9,.1,0,1});
    verifyPacking(packed, differentTypes);
    require(packed.getLoadedItems().front().getId() == 2 && packed.getLoadedItems().front().getRotationId() == 1,
            "VLT must use sorted-position index, not the source key index");
}

void testLayers()
{
    auto data = instance();
    data.totalItems = {item(1,1,2,3,4)};
    const auto type = buildBoxTypes(data).front();
    const MaximalSpace space = {{0,0,0},{10,12,16}};
    const auto layers = enumerateLayers(type, space, 10);
    require(layers.size() == 36, "Six legal rotations must enumerate six layer types each");
    for (const auto &layer : layers)
    {
        const std::int64_t sizes[3] = {layer.itemDimensions.x, layer.itemDimensions.y, layer.itemDimensions.z};
        std::uint64_t count = 1;
        for (int axis = 0; axis < 3; ++axis)
        {
            require(layer.counts[axis] > 0, "Layer has nonpositive extent");
            count *= layer.counts[axis];
        }
        require(count == layer.boxCount && count <= 10, "Layer exceeded quantity");
        require(layer.counts[0]*sizes[0] <= 10 && layer.counts[1]*sizes[1] <= 12 &&
                layer.counts[2]*sizes[2] <= 16, "Layer exceeded space bounds");
    }
    require(layerIndex(0,36) == 0 && layerIndex(1,36) == 35 && layerIndex(.5,36) == 17,
            "Layer mapping must use ceil with defined endpoints");
    rejects([] { layerIndex(.5,0); });
    rejects([] { layerIndex(-.1,36); });
    for (int mask = 0; mask < 8; ++mask)
    {
        auto allowed = type;
        allowed.prototype.setRotations({mask&1,(mask>>1)&1,(mask>>2)&1});
        const auto candidates = enumerateLayers(allowed, space, 1);
        const int permissions = (mask&1) + ((mask>>1)&1) + ((mask>>2)&1);
        require(candidates.size() == static_cast<size_t>(12*permissions), "Layer permission filtering differs");
        for (const auto &layer : candidates)
        {
            auto original = data;
            original.totalItems[0] = allowed.prototype;
            auto packed = original.bin;
            auto placed = allowed.prototype;
            placed.rotate(ROTATION_WAY::SIX_WAY, getRotationFromId(layer.rotation));
            placed.setPosicion({0,0,0});
            packed.addItem(placed);
            verifyPacking(packed, original);
        }
    }
}

void testPackingAndAudit()
{
    auto exact = instance();
    for (int id = 1; id <= 8; ++id) exact.totalItems.push_back(item(id,1,5,5,5));
    auto packed = exact.bin;
    decodeLayers(packed, buildBoxTypes(exact), RandomKeys(16,0));
    verifyPacking(packed, exact);
    require(packed.getLoadedVolume() == 1000 && packed.getNumberOfLoadedItems() == 8, "Exact layered fit failed");
    rejects([&] { decodeLayers(packed, buildBoxTypes(exact), RandomKeys(16,0)); });

    auto skipped = instance();
    skipped.totalItems = {item(1,1,11,11,11), item(2,2,2,2,2), item(3,2,2,2,2)};
    auto later = skipped.bin;
    decodeLayers(later, buildBoxTypes(skipped), RandomKeys(6,0));
    verifyPacking(later, skipped);
    require(later.getLoadedVolume() == 16 && later.getNumberOfLoadedItems() == 2,
            "Failed type must terminate and continue to later boxes");
    auto denied = skipped;
    denied.totalItems[1].setRotations({0,0,0});
    denied.totalItems[2].setRotations({0,0,0});
    auto empty = denied.bin;
    decodeLayers(empty, buildBoxTypes(denied), RandomKeys(6,0));
    require(empty.getLoadedVolume() == 0, "Forbidden rotations were loaded");

    auto overlaps = exact.bin;
    auto first = exact.totalItems[0];
    first.rotate(ROTATION_WAY::SIX_WAY, ROTATION_MODE::ONE);
    overlaps.addItem(first);
    auto second = first;
    second.setId(2);
    overlaps.addItem(second);
    rejects([&] { verifyPacking(overlaps, exact); });
    auto duplicated = exact.bin;
    duplicated.addItem(first);
    first.setPosicion({5,0,0});
    duplicated.addItem(first);
    rejects([&] { verifyPacking(duplicated, exact); });
    for (auto position : {Punto(-1,0,0), Punto(6,0,0)})
    {
        auto outside = exact.bin;
        first.setPosicion(position);
        outside.addItem(first);
        rejects([&] { verifyPacking(outside, exact); });
    }
    auto badDimensions = exact.bin;
    first.setPosicion({0,0,0}).setCurrentDimension({1,1,1});
    badDimensions.addItem(first);
    rejects([&] { verifyPacking(badDimensions, exact); });
    auto badPermissions = exact;
    badPermissions.totalItems[0].setRotations({0,0,0});
    rejects([&] { verifyPacking(packed, badPermissions); });
    auto wrongType = exact;
    wrongType.totalItems[0].setTipo(99);
    rejects([&] { verifyPacking(packed, wrongType); });
}

void testEvolution()
{
    std::mt19937_64 random(123);
    const RandomKeys elite(100000,1), other(100000,0);
    require(biasedCrossover(elite,other,1,random) == elite, "Bias 1 did not copy elite");
    require(biasedCrossover(elite,other,0,random) == other, "Bias 0 did not copy other parent");
    const auto child = biasedCrossover(elite,other,.7,random);
    const double ratio = std::count(child.begin(),child.end(),1.0) / static_cast<double>(child.size());
    require(ratio > .69 && ratio < .71, "Biased inheritance is not approximately 0.7");
    const auto mutants = randomKeys(1000,random);
    for (double key : mutants) require(key >= 0 && key < 1, "Mutant key outside [0,1)");
    rejects([&] { biasedCrossover({0},{0,1},.7,random); });

    std::vector<Population> populations = {
        {{{.1},100},{{.2},90},{{.3},10}},
        {{{.4},80},{{.5},70},{{.6},20}},
        {{{.7},60},{{.8},50},{{.9},30}}
    };
    exchangeElites(populations,2);
    for (const auto &population : populations)
    {
        require(population.size() == 3 && population[0].volume == 100 && population[1].volume >= 90,
                "Global elite migration lost fitness or population size");
        require(std::any_of(population.begin(),population.end(),[](const Individual &individual) {
            return individual.volume == 90 && individual.keys == RandomKeys({.2});
        }), "Migration must snapshot elites before modifying any population");
    }

    auto data = instance(50,50,50);
    for (int id = 1; id <= 4; ++id) data.totalItems.push_back(item(id,id,2,3,4));
    Configuration configuration;
    configuration.populationSize = 20;
    configuration.seconds = 0;
    configuration.maximumGenerations = 16;
    const auto result = search(data,123,configuration);
    const auto repeated = search(data,123,configuration);
    verifyPacking(result.packing,data);
    verifyRepeat(result,repeated);
    require(result.generations == 16 && result.migrationEvents == 1 &&
            result.evaluationsCompleted == 3*20 + 16*3*17 &&
            result.initialized == std::vector<size_t>({20,20,20}), "Elitism, mutants or generation accounting differs");
    auto replay = data.bin;
    decodeLayers(replay,buildBoxTypes(data),result.keys);
    require(replay.getLoadedVolume() == result.packing.getLoadedVolume(), "Best keys did not reconstruct fitness");
    Configuration published;
    published.seconds = 0;
    published.maximumGenerations = 0;
    const auto initialized = search(data,123,published);
    require(initialized.populationSize == 80 && initialized.evaluationsCompleted == 240,
            "Published population must be 20 times M per island");
    auto invalid = configuration;
    invalid.populationSize = 1;
    rejects([&] { search(data,123,invalid); });
    invalid = configuration;
    invalid.eliteFraction = .9;
    rejects([&] { search(data,123,invalid); });

    configuration.seconds = 1;
    configuration.maximumGenerations = 10000;
    for (int cutoff : {10,100,1000,10000})
    {
        int checks = 0;
        const auto partial = search(data,123,configuration,[&] {
            return ExecutionBudget::Clock::time_point{} + std::chrono::seconds(++checks > cutoff ? 2 : 0);
        });
        require(partial.termination == "time_limit", "Controlled time limit did not stop BRKGA");
        require(partial.evaluationsStarted - partial.evaluationsCompleted <= 1, "Partial evaluation count differs");
        verifyPacking(partial.packing,data);
    }
}

void testRandomizedPackings()
{
    std::mt19937_64 random(456);
    for (int trial = 0; trial < 200; ++trial)
    {
        auto data = instance(10,12,8);
        const int types = 1 + random() % 8;
        for (int type = 1; type <= types; ++type)
        {
            auto prototype = item(1,type,1 + random()%9,1 + random()%9,1 + random()%9);
            const int mask = random()%8;
            prototype.setRotations({mask&1,(mask>>1)&1,(mask>>2)&1});
            const int copies = 1 + random()%3;
            for (int copy = 0; copy < copies; ++copy)
                data.totalItems.push_back(prototype.setId(static_cast<int>(data.totalItems.size()+1)));
        }
        auto keys = randomKeys(2*data.totalItems.size(),random);
        if (trial == 0) std::fill(keys.begin(),keys.end(),0);
        if (trial == 1) std::fill(keys.begin(),keys.end(),1);
        const auto grouped = buildBoxTypes(data);
        SearchResult first, second;
        first.packing = data.bin;
        second.packing = data.bin;
        first.keys = second.keys = keys;
        decodeLayers(first.packing,grouped,keys);
        decodeLayers(second.packing,grouped,keys);
        verifyPacking(first.packing,data);
        verifyRepeat(first,second);
        require(data.bin.getLoadedItems().empty(), "Decoder modified source container");
    }
}

void testMenu()
{
    CommandLineOptions defaults;
    defaults.root = ".";
    std::istringstream input("15\n1\n1\n6\n1\nbrkga\n1\n");
    std::ostringstream output;
    const auto selected = BRExperimentMenu().choose(input,output,defaults);
    require(selected.confirmed && selected.options.algorithm == "BRKGA2012", "BRKGA is missing from menu");
    require(output.str().find("Metodo de mutacion") == std::string::npos,
            "BRKGA menu should not request the old mutation operators");
    require(parseCommandLine({"--algorithm","BRKGA"}).algorithm == "BRKGA2012", "BRKGA CLI alias failed");
}
}
int main()
{
    try
    {
        testEncoding();
        testLayers();
        testPackingAndAudit();
        testEvolution();
        testRandomizedPackings();
        testMenu();
        std::cout << "PASS: BRKGA Fig.3 encoding, 36 layers, VLT, permissions, quantities, audit corruption, "
                     "crossover, mutants, migration, exact generations, repeatability, time cuts and menu\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
