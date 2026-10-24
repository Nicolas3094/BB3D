#include "../src/metaheuristics/FireflyAlgorithm.hpp"
#include <stdexcept>

// Test doubles isolate search control flow from packing, mutation and crossover.
Poblacion fixture;
int crossCalls = 0;
int movementCalls = 0;
int fitnessCalls = 0;
struct FireflyAlgorithmTestAccess
{
    static Poblacion searchD(FireflyAlgorithm &algorithm) { return algorithm.searchD(); }
};
Poblacion initialHeuristicPoblation(int, Bin, LoadedBins, ROTATION_WAY) { return fixture; }
void mutationWithType(DoubleGenome &, MutationType, int) { ++movementCalls; }
void flipMutation(DoubleGenome &, double, ROTATION_WAY) {}
DoubleGenome crossOver(DoubleGenome first, DoubleGenome second)
{
    ++crossCalls;
    // Rotation values identify the selected target without changing distances.
    if (second.getDGenome()[0] != 2)
        throw std::runtime_error("Search did not approach the strongest firefly");
    return first;
}
void evaluateFitness(Individuo &individual, LoadedBins, Bin)
{
    ++fitnessCalls;
    individual.setFitness(0); // Keep the fixture stable during the iteration.
    individual.solution = "new-" + std::to_string(fitnessCalls);
}
double uniformUnit() { return 0.5; }
int randomInteger(int minimum, int) { return minimum; }

void checkSearch(std::initializer_list<double> fitnesses, int expectedCrosses,
                 bool directed = false, bool replacement = true, int movements = 1)
{
    fixture.clear();
    int marker = 1;
    for (double fitness : fitnesses)
    {
        Chromosome rotations(8, 1);
        rotations[0] = marker;
        Individuo individual = Individuo::Build().setGenome(DoubleGenome::Build()
            .setGenome({1, 2, 3, 4, 5, 6, 7, 8})
            .setDGenome(rotations)).setFitness(fitness);
        individual.solution = std::to_string(marker++);
        fixture.push_back(individual);
    }
    crossCalls = movementCalls = fitnessCalls = 0;
    FireflyAlgorithm algorithm;
    DatasetBinBacking problem{};
    problem.bin = Bin::Build().setLargo(10).setAncho(10).setAlto(10);
    problem.totalItems = {ItemBin::Build().setId(1).setLargo(1).setAncho(1).setAlto(1).setRotations(Punto::Build(1,1,1))};
    algorithm.setProblem(problem);
    algorithm.setNumberOfIndividuals(static_cast<int>(fixture.size()))
        .setMaxIteration(1).setupIndex(movements).setIsWithReplacement(replacement)
        .setRotationType(ROTATION_WAY::ZERO_WAY)
        .setMutationType(MutationType::INVERSE_MUTATION);
    Poblacion result = directed ? FireflyAlgorithmTestAccess::searchD(algorithm) : algorithm.evolve();
    if (crossCalls != expectedCrosses)
        throw std::runtime_error("Wrong approach count: expected " +
            std::to_string(expectedCrosses) + ", got " + std::to_string(crossCalls));
    if (movementCalls != static_cast<int>(fixture.size()) * movements ||
        fitnessCalls != static_cast<int>(fixture.size()) * movements || result.size() != fixture.size())
        throw std::runtime_error("Search skipped movement/evaluation or resized population");
}

int main()
{
    try
    {
        // Strongest candidate comes before a weaker candidate: detects truncation.
        checkSearch({0.2, 0.9, 0.5}, 2);
        checkSearch({0.5, 0.5, 0.5}, 0);
        checkSearch({0.5}, 0);
        checkSearch({0.2, 0.9, 0.5}, 2, true);
        checkSearch({0.5, 0.5, 0.5}, 0, true);
        checkSearch({0.5}, 0, true);
        // Appending children can reallocate the population during the inner loop.
        checkSearch({0.2, 0.9, 0.5}, 6, true, false, 3);
        std::cout << "PASS: approach strongest target, equal fitness and singleton fallback\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
