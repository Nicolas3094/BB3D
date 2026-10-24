#include "../src/metaheuristics/FireflyAlgorithm.hpp"
#include <stdexcept>

// Test doubles isolate search control flow from packing, mutation and crossover.
Poblacion fixture;
int crossCalls = 0;
int movementCalls = 0;
int fitnessCalls = 0;
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
}
double uniformUnit() { return 0.5; }
int randomInteger(int minimum, int) { return minimum; }

void checkSearch(std::initializer_list<double> fitnesses, int expectedCrosses)
{
    fixture.clear();
    int marker = 1;
    for (double fitness : fitnesses)
    {
        Individuo individual = Individuo::Build().setGenome(DoubleGenome::Build()
            .setGenome({1, 2, 3, 4, 5, 6, 7, 8})
            .setDGenome(Chromosome(8, marker))).setFitness(fitness);
        individual.solution = std::to_string(marker++);
        fixture.push_back(individual);
    }
    crossCalls = movementCalls = fitnessCalls = 0;
    FireflyAlgorithm algorithm;
    algorithm.setNumberOfIndividuals(static_cast<int>(fixture.size()))
        .setMaxIteration(1).setupIndex(1).setIsWithReplacement(true)
        .setRotationType(ROTATION_WAY::ZERO_WAY)
        .setMutationType(MutationType::INVERSE_MUTATION);
    Poblacion result = algorithm.evolve();
    if (crossCalls != expectedCrosses)
        throw std::runtime_error("Wrong approach count: expected " +
            std::to_string(expectedCrosses) + ", got " + std::to_string(crossCalls));
    if (movementCalls != static_cast<int>(fixture.size()) ||
        fitnessCalls != static_cast<int>(fixture.size()) || result.size() != fixture.size())
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
        std::cout << "PASS: approach strongest target, equal fitness and singleton fallback\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
