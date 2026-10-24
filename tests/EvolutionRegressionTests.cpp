#include "../src/heuristics/DBLF.hpp"
#include "../src/genetic/GeneticsOperators.hpp"
#include "../src/genetic/PermutationOperators.hpp"
#include "../src/metaheuristics/GeneticAlgorithm.hpp"
#include "../src/metaheuristics/FireflyAlgorithm.hpp"
#include <cmath>
#include <limits>

bool upper = false;
int randomCalls = 0, unitCalls = 0, maximumBound = -1, binomialResult = 1;
double unitValue = 0.5;
double uniformUnit() { ++unitCalls; return unitValue; }
int randomInteger(int low, int high)
{
    if (++randomCalls > 10000) throw std::runtime_error("Random retry loop did not terminate");
    if (low > high) throw std::runtime_error("Invalid random interval");
    maximumBound = std::max(maximumBound, high);
    return upper ? high : low;
}
int defaultBinomialDistribution(int) { return binomialResult; }
void require(bool condition, const char *message) { if (!condition) throw std::runtime_error(message); }
template<typename Operation> void rejects(Operation operation)
{
    bool caught = false;
    try { operation(); } catch (const std::invalid_argument &) { caught = true; }
    require(caught, "Malformed input was accepted");
}
DoubleGenome genome(int size)
{
    Chromosome ids, rotations;
    for (int i = 1; i <= size; ++i) { ids.push_back(i); rotations.push_back(i % 6 + 1); }
    return DoubleGenome::Build().setGenome(ids).setDGenome(rotations);
}
ItemBin box(int id, int x = 1, int y = 1, int z = 1)
{
    return ItemBin::Build().setId(id).setTipo(id).setLargo(x).setAncho(y).setAlto(z)
        .setRotations(Punto::Build(1,1,1));
}
Bin container() { return Bin::Build().setLargo(8).setAncho(8).setAlto(8).setRotationWay(ROTATION_WAY::ZERO_WAY); }
void checkPackingState()
{
    auto fake = box(1,2,2,2).setCurrentDimension(Punto::Build(1,1,1));
    for (int variant = 0; variant < 3; ++variant)
    {
        Bin bin = container();
        rejects([&] {
            if (variant == 0) DBLF(bin, {1}, {0}, {fake});
            else if (variant == 1) DBLFQueue(bin, {1}, {0}, {fake});
            else DBLF(bin, LoadedBins{fake});
        });
        require(bin.getLoadedItems().empty() && bin.getLoadedVolume() == 0, "Invalid dimensions changed packing");
    }
    for (int variant = 0; variant < 3; ++variant)
    {
        Bin bin = container(); bin.addItem(box(1).setPosicion(Punto::Build(7,7,7)));
        rejects([&] {
            if (variant == 0) DBLF(bin, {1}, {0}, {box(1)});
            else if (variant == 1) DBLFQueue(bin, {1}, {0}, {box(1)});
            else DBLF(bin, LoadedBins{box(1)});
        });
        require(bin.getLoadedItems().size() == 1 && bin.getLoadedVolume() == 1 &&
                bin.getLoadedItems()[0].getPunto() == Punto::Build(7,7,7), "Preloaded bin was modified before rejection");
    }
    Bin legal = container(); legal.setRotationWay(ROTATION_WAY::SIX_WAY);
    auto rotated = box(1,2,3,4); rotated.rotate(ROTATION_WAY::SIX_WAY, ROTATION_MODE::TWO);
    DBLF(legal, LoadedBins{rotated});
    require(legal.getLoadedVolume() == 24 && legal.getLoadedItems()[0].getCurrentDimension() == Punto::Build(2,4,3), "Valid rotated dimensions rejected");
    std::cout << "PASS: coherent dimensions, legal permutation and fresh-bin contract\n";
}
void verify(DoubleGenome value, int size)
{
    auto ids = value.getGenome(); std::sort(ids.begin(), ids.end());
    require(ids.size() == static_cast<size_t>(size) && value.getDGenome().size() == ids.size(), "Wrong chromosome size");
    for (int i = 0; i < size; ++i)
        require(ids[i] == i + 1 && value.getDGenome()[i] == value.getGenome()[i] % 6 + 1, "Operator corrupted paired genes");
}
void checkSmallOperators()
{
    for (bool boundary : {false, true})
    {
        upper = boundary;
        for (int size = 1; size <= 10; ++size)
        {
            randomCalls = 0;
            auto first = genome(size), second = first;
            reverseGenome(second);
            verify(crossOver(first, second), size);
            auto mutated = first;
            mutateInversion(mutated);
            verify(mutated, size);
            require(size < 2 || !(mutated == first), "Inversion of multiple genes did nothing");
        }
    }
    upper = false; randomCalls = 0;
    auto empty = genome(0); mutateInversion(empty); verify(empty, 0);
    rejects([&] { crossOver(empty, empty); });
    auto valid = genome(3);
    rejects([&] { crossOver(valid, genome(2)); });
    auto shortRotations = valid; shortRotations.setDGenome({1});
    rejects([&] { crossOver(valid, shortRotations); });
    auto duplicate = valid; duplicate.setGenome({1,1,3});
    rejects([&] { crossOver(valid, duplicate); });
    auto different = valid; different.setGenome({1,2,4});
    rejects([&] { crossOver(valid, different); });
    rejects([&] { crossOx(valid, valid, -1, 1); });
    rejects([&] { crossOx(valid, valid, 0, 3); });
    rejects([&] { mutateInversion(shortRotations); });
    rejects([&] { inverseMutation(valid, -1, 2); });
    std::cout << "PASS: small crossover/inversion, paired genes and malformed parents\n";
}
Individuo firefly(double fitness, Chromosome rotations)
{
    return Individuo::Build().setFitness(fitness).setGenome(DoubleGenome::Build()
        .setGenome({1,2,3,4,5,6,7,8}).setDGenome(rotations));
}
void checkAttraction()
{
    FireflyAlgorithm algorithm;
    Poblacion dark{firefly(0,Chromosome(8,1)),firefly(0.8,Chromosome(8,2))};
    require(algorithm.findMostAttractiveFirefly(dark, dark[0], 0.125) == &dark[1], "Zero fitness ignored a better target");
    Chromosome close(8,1); close[0] = 2;
    Poblacion candidates{firefly(0.1,Chromosome(8,1)),firefly(0.8,close),firefly(0.9,Chromosome(8,2))};
    require(algorithm.findMostAttractiveFirefly(candidates, candidates[0], 0.125) == &candidates[1], "Target comparison reused another candidate's distance");
    require(algorithm.findMostAttractiveFirefly(candidates, candidates[2], 0.125) == nullptr, "No better target should return nullptr");
    std::cout << "PASS: zero fitness attraction and distinct candidate distances\n";
}
struct GeneticAlgorithmTestAccess
{
    static double diversity(GeneticAlgorithm &ga, Poblacion population) { return ga.diversityIndex(population); }
    static Poblacion parents(GeneticAlgorithm &ga, Poblacion population, int count) { return ga.getParents(population, count); }
    static void next(GeneticAlgorithm &ga, Poblacion &population, bool added)
    {
        if (added) ga.nextGenerationByAdding(population); else ga.nextGeneration(population);
    }
};
Poblacion population(int count, double fitness = 0)
{
    Poblacion values;
    for (int i = 0; i < count; ++i)
    {
        auto value = Individuo::Build().setGenome(genome(8)).setFitness(fitness);
        value.solution = "member-" + std::to_string(i); values.push_back(value);
    }
    return values;
}
void checkGA()
{
    GeneticAlgorithm ga;
    ga.setNumberOfIndividuals(4);
    const double diversity = GeneticAlgorithmTestAccess::diversity(ga, population(4));
    require(std::isfinite(diversity) && diversity == 0, "All-zero fitness has non-finite diversity");
    rejects([&] { GeneticAlgorithmTestAccess::diversity(ga, {}); });
    auto varied = population(2); varied.front().setFitness(0.8); varied.back().setFitness(0.4);
    require(std::abs(GeneticAlgorithmTestAccess::diversity(ga, varied) - 0.625) < 1e-12,
            "Positive-fitness diversity formula changed");
    ga.setSelectionProbability(1);
    auto two = population(2); two[0].setFitness(0.4); two[1].setFitness(0.9);
    for (bool boundary : {false,true})
    {
        upper = boundary; randomCalls = 0;
        auto parents = GeneticAlgorithmTestAccess::parents(ga, two, 2);
        require(parents[0].solution == "member-1" && parents[1].solution == "member-0", "Parents must be distinct and include the last member");
        require(tournament(two, 1, 1) == 0, "Singleton tournament must terminate");
        rejects([&] { tournament(two, 0, 1); });
        rejects([&] { tournament(two, 3, 1); });
        rejects([&] { tournament(two, 2, -0.1); });
    }
    upper = false;
    ga.setNumberOfIndividuals(6).setCrossProbability(0).setSelectionProbability(0.8);
    for (bool added : {false,true})
    for (int count : {0,1,6})
    {
        auto values = population(6,0.125);
        binomialResult = count; unitCalls = randomCalls = 0; maximumBound = -1;
        GeneticAlgorithmTestAccess::next(ga, values, added);
        require(unitCalls == count * 3, "Generation processed wrong offspring/replacement count");
        require(values.size() == 6, "Generation changed population size");
        require(count == 0 || maximumBound == 5, "Generation excluded members or selected parents from offspring count");
    }
    // Probability zero must also exclude the event when the RNG returns zero.
    unitValue = 0.0;
    for (bool added : {false,true})
    {
        auto values = population(6,0.125);
        binomialResult = 1; randomCalls = unitCalls = 0;
        GeneticAlgorithmTestAccess::next(ga, values, added);
        require(unitCalls == 3, "Zero crossover probability produced children");
    }
    unitValue = 0.5;
    for (int count : {0,1})
    {
        GeneticAlgorithm invalid;
        invalid.setNumberOfIndividuals(count);
        rejects([&] { invalid.evolve(); });
    }
    for (int size : {1,2,3,4})
    for (int count : {2,3,4})
    {
        DatasetBinBacking problem{}; problem.bin = container();
        for (int id = 1; id <= size; ++id) problem.totalItems.push_back(box(id));
        for (bool replacement : {false,true})
        {
            GeneticAlgorithm small;
            small.setProblem(problem).setNumberOfIndividuals(count).setMaxIteration(1)
                .setCrossProbability(1).setSelectionProbability(1).setIsWithReplacement(replacement);
            binomialResult = 1; randomCalls = 0;
            auto result = small.evolve();
            require(result.size() == static_cast<size_t>(count) && std::abs(result[0].getFitness() - size / 512.0) < 1e-12,
                    "GA failed with small chromosomes or addition/replacement mode");
        }
    }
    DatasetBinBacking validProblem{}; validProblem.bin = container(); validProblem.totalItems = {box(1)};
    for (int option = 0; option < 5; ++option)
    {
        GeneticAlgorithm invalid;
        invalid.setProblem(validProblem).setNumberOfIndividuals(4);
        if (option == 0) invalid.setSelectionProbability(-0.1);
        if (option == 1) invalid.setCrossProbability(1.1);
        if (option == 2) invalid.setDMutationProbability(std::numeric_limits<double>::quiet_NaN());
        if (option == 3) invalid.setMaxIteration(-1);
        if (option == 4) invalid.setMaxDiversityIndex(-0.1);
        rejects([&] { invalid.evolve(); });
    }
    std::cout << "PASS: finite diversity, bounded distinct parents, full parent pool and exact generation counts\n";
}
int main(int argc, char **argv)
{
    try
    {
        const int group = argc > 1 ? std::stoi(argv[1]) : 0;
        if (group == 0 || group == 5) checkPackingState();
        if (group == 0 || group == 6) checkSmallOperators();
        if (group == 0 || group == 7) checkAttraction();
        if (group == 0 || group == 8) checkGA();
        return 0;
    }
    catch (const std::exception &error) { std::cerr << "FAIL: " << error.what() << '\n'; return 1; }
}
