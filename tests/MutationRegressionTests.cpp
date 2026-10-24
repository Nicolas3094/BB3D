#include "../src/genetic/PermutationOperators.hpp"
#include "../src/genetic/GeneticsOperators.hpp"
#include "../src/genetic/FitnessOperators.hpp"
#include <cmath>
#include <limits>
#include <random>
#include <sstream>

std::mt19937 engine(20261023);
std::vector<double> draws;
size_t drawIndex = 0;
bool controlled = false;
double uniformUnit()
{
    if (!controlled) return std::generate_canonical<double, 53>(engine);
    if (drawIndex == draws.size()) throw std::runtime_error("Unexpected extra random draw");
    return draws[drawIndex++];
}
int randomInteger(int low, int high)
{
    if (low > high) throw std::runtime_error("Invalid random interval");
    return std::uniform_int_distribution<int>(low, high)(engine);
}
int defaultBinomialDistribution(int) { throw std::runtime_error("Unexpected default binomial call"); }
void require(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
DoubleGenome genome(int n)
{
    Chromosome ids, rotations;
    for (int i = 1; i <= n; ++i) { ids.push_back(i); rotations.push_back(i % 6 + 1); }
    return DoubleGenome::Build().setGenome(ids).setDGenome(rotations);
}
void verifyPermutation(DoubleGenome value, int n)
{
    require(value.getGenome().size() == static_cast<size_t>(n) && value.getDGenome().size() == static_cast<size_t>(n), "Mutation changed size");
    auto ids = value.getGenome(); std::sort(ids.begin(), ids.end());
    for (int i = 0; i < n; ++i)
        require(ids[i] == i + 1 && value.getDGenome()[i] == value.getGenome()[i] % 6 + 1, "Mutation lost an item or its rotation");
}
void branch(bool c1, std::initializer_list<double> randomValues, Chromosome expected)
{
    draws = randomValues; drawIndex = 0; controlled = true;
    DoubleGenome value = genome(8);
    if (c1) combine1(value, 0, 1, 4, 5); else combine2(value, 7, 2, 3);
    controlled = false;
    require(value.getGenome() == expected, "Wrong C1/C2 branch or multiple operations applied");
    require(drawIndex == draws.size(), "C1/C2 consumed wrong branch draws");
    verifyPermutation(value, 8);
}
void checkCombinations()
{
    branch(true, {0.1}, {6,2,3,4,5,1,7,8});
    branch(true, {0.5}, {5,6,3,4,1,2,7,8});
    branch(true, {0.9,0.1,0.1}, {6,5,3,4,2,1,7,8});
    branch(false, {0.1}, {1,2,4,5,6,7,3,8});
    branch(false, {0.5}, {1,2,5,6,7,3,4,8});
    branch(false, {0.9,0.9}, {1,2,5,6,7,4,3,8});
    for (int n = 0; n <= 12; ++n)
    for (int repeat = 0; repeat < 20; ++repeat)
    for (int step : {-1, 0, 1, 2, 20})
    for (bool c1 : {false, true})
    {
        DoubleGenome value = genome(n), original = value;
        if (c1) mutateC1(value, step); else mutateC2(value, step);
        verifyPermutation(value, n);
        require(n < 2 || !(value == original), "Default/bounded C1/C2 mutation did nothing");
    }
    std::cout << "PASS: all six C1/C2 branches, sizes 0-12, default/explicit steps and paired genes\n";
}
void checkFlip()
{
    controlled = false;
    double total = 0;
    const int trials = 20000;
    for (int trial = 0; trial < trials; ++trial)
    {
        DoubleGenome value = DoubleGenome::Build().setGenome(Chromosome(151, 1)).setDGenome(Chromosome(151, 1));
        flipMutationO1(value, 0.01, ROTATION_WAY::TWO_WAY);
        for (int rotation : value.getDGenome()) total += rotation != 1;
    }
    require(std::abs(total / trials - 1.51) < 0.06, "Rotation mutation distribution is not Bernoulli per gene");
    for (auto way : {ROTATION_WAY::TWO_WAY, ROTATION_WAY::SIX_WAY})
    for (int n : {0, 1, 2, 151})
    {
        auto value = DoubleGenome::Build().setGenome(Chromosome(n,1)).setDGenome(Chromosome(n,1));
        flipMutationO1(value, 1.0, way);
        for (int rotation : value.getDGenome()) require(rotation > 1 && rotation <= getIdFromRotationWay(way), "p=1 did not change every gene");
        auto original = value;
        flipMutationO1(value, 0.0, way);
        require(value == original, "p=0 changed genes");
    }
    for (double invalid : {-0.1, 1.1, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()})
    {
        bool rejected = false; auto value = genome(3);
        try { flipMutationO1(value, invalid, ROTATION_WAY::SIX_WAY); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected, "Invalid mutation probability accepted");
    }
    // The RNG may return zero. Each iteration must still advance to the next locus.
    draws = {0,0,0}; drawIndex = 0; controlled = true;
    auto value = DoubleGenome::Build().setGenome({1,2,3}).setDGenome({1,1,1});
    flipMutationO1(value, 0.01, ROTATION_WAY::TWO_WAY);
    controlled = false;
    require(value.getDGenome() == Chromosome({2,2,2}), "Zero RNG draws failed to advance");
    auto fixed = DoubleGenome::Build().setGenome({1,2}).setDGenome({0,0});
    auto originalFixed = fixed;
    flipMutationO1(fixed, 1.0, ROTATION_WAY::ZERO_WAY);
    require(fixed == originalFixed, "ZERO_WAY mutation must leave fixed orientations unchanged");
    bool mismatchedRejected = false;
    try { auto malformed = DoubleGenome::Build().setGenome({1,2}).setDGenome({1}); flipMutationO1(malformed, 0.5, ROTATION_WAY::TWO_WAY); }
    catch (const std::invalid_argument &) { mismatchedRejected = true; }
    require(mismatchedRejected, "Mismatched mutation chromosomes accepted");
    std::cout << "PASS: flip mutation mean=" << total / trials << ", endpoints, singleton, empty and invalid probabilities\n";
}
ItemBin box(int id, int size = 1)
{
    return ItemBin::Build().setId(id).setTipo(id).setLargo(size).setAncho(size).setAlto(size).setRotations(Punto::Build(1,1,1));
}
void checkZeroRotation()
{
    auto item = box(1).setLargo(2).setAncho(3).setAlto(4);
    item.rotate(ROTATION_WAY::SIX_WAY, ROTATION_MODE::TWO);
    item.rotate(ROTATION_WAY::ZERO_WAY, ROTATION_MODE::NONE);
    require(item.getCurrentDimension() == Punto::Build(2,3,4), "ZERO_WAY did not restore original dimensions");
    for (int permitted : {0,1})
    {
        item.setRotations(Punto::Build(1,1,permitted));
        for (int variant = 0; variant < 4; ++variant)
        {
            Bin bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10).setRotationWay(ROTATION_WAY::ZERO_WAY);
            Individuo individual = Individuo::Build().setGenome(DoubleGenome::Build().setGenome({1}).setDGenome({0}));
            if (variant == 0) DBLF(bin, {1}, {0}, {item});
            else if (variant == 1) DBLFQueue(bin, {1}, {0}, {item});
            else if (variant == 2) DBLF(bin, LoadedBins{item});
            else evaluateFitnessDecodificated(individual, {item}, bin);
            require(bin.getNumberOfLoadedItems() == permitted, "ZERO_WAY accepted forbidden original orientation");
        }
    }
    item.rotate(ROTATION_WAY::SIX_WAY, ROTATION_MODE::ONE);
    item.rotate(ROTATION_WAY::TWO_WAY, ROTATION_MODE::SIX);
    require(!item.isValidOrientation(), "Unsupported rotation retained old validity");
    std::cout << "PASS: ZERO_WAY dimensions, permissions, decoded route and stale validity\n";
}
void checkVolumes()
{
    require(Dim(2000,2000,2000).getVolumen() == 8000000000LL, "Volume overflowed at 32 bits");
    Bin bin = Bin::Build().setLargo(4000).setAncho(2000).setAlto(2000);
    bin.addItem(box(1,2000)); bin.addItem(box(2,2000));
    require(bin.getLoadedVolume() == 16000000000LL, "Loaded volume storage/return truncated");
    bool rejected = false;
    try { Dim(2147483647,2147483647,2147483647).getVolumen(); }
    catch (const std::overflow_error &) { rejected = true; }
    require(rejected, "Volume beyond int64 must be rejected");
    Bin accumulated;
    accumulated.addItem(box(1,2000000));
    rejected = false;
    try { accumulated.addItem(box(2,2000000)); }
    catch (const std::overflow_error &) { rejected = true; }
    require(rejected && accumulated.getNumberOfLoadedItems() == 1 && accumulated.getLoadedVolume() == 8000000000000000000LL,
            "Accumulated overflow must be rejected before changing the bin");
    Bin packing = Bin::Build().setLargo(4000).setAncho(2000).setAlto(2000).setRotationWay(ROTATION_WAY::ZERO_WAY);
    Individuo individual = Individuo::Build().setGenome(DoubleGenome::Build().setGenome({1,2}).setDGenome({0,0}));
    evaluateFitness(individual, {box(1,2000),box(2,2000)}, packing);
    require(individual.getFitness() == 1.0, "Large-volume fitness is incorrect");
    Bin almostFull = Bin::Build().setLargo(20000000).setAncho(1).setAlto(1).setRotationWay(ROTATION_WAY::ZERO_WAY);
    individual.setGenome(DoubleGenome::Build().setGenome({1}).setDGenome({0}));
    evaluateFitness(individual, {box(1).setLargo(19999999)}, almostFull);
    require(individual.getFitness() < 1.0 && std::abs(individual.getFitness() - 0.99999995) < 1e-12,
            "Fitness rounded incomplete packing to full occupancy");
    std::cout << "PASS: int64 volumes, accumulated volume, overflow rejection and large-box fitness\n";
}
int main(int argc, char **argv)
{
    const int group = argc > 1 ? std::stoi(argv[1]) : 0;
    std::ostringstream diagnostics; auto *previous = std::cout.rdbuf(diagnostics.rdbuf());
    try
    {
        if (group == 0 || group == 1) checkCombinations();
        if (group == 0 || group == 2) checkFlip();
        if (group == 0 || group == 3) checkZeroRotation();
        if (group == 0 || group == 4) checkVolumes();
        std::cout.rdbuf(previous); std::cout << diagnostics.str(); return 0;
    }
    catch (const std::exception &error)
    {
        std::cout.rdbuf(previous); std::cerr << "FAIL: " << error.what() << '\n'; return 1;
    }
}
