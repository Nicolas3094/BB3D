#include "../src/metaheuristics/GeneticAlgorithm.hpp"
#include <stdexcept>

// Controlled RNG: always enter the mutation branch, with valid fixed indices.
double uniformUnit() { return 0.0; }
bool chooseUpperBound = false;
int randomInteger(int minimum, int maximum)
{
    if (minimum > maximum) throw std::runtime_error("Invalid random range");
    return chooseUpperBound ? maximum : minimum;
}
int defaultBinomialDistribution(int) { throw std::runtime_error("Unexpected binomial call"); }

struct GeneticAlgorithmTestAccess
{
    static Individuo crossover(GeneticAlgorithm &algorithm, Individuo first, Individuo second)
    {
        return algorithm.crossover(first, second);
    }
    static void replace(GeneticAlgorithm &algorithm, Poblacion &population,
                        Individuo child, Individuo secondChild,
                        bool firstVisited, bool secondVisited)
    {
        algorithm.visitedIndividuals[child.solution] = firstVisited;
        algorithm.visitedIndividuals[secondChild.solution] = secondVisited;
        algorithm.replacePoblationWithChildren(population, child, secondChild, 1);
    }
    static void run(GeneticAlgorithm &algorithm, Poblacion &population, int first, int last)
    {
        DatasetBinBacking data{};
        data.bin=Bin::Build().setLargo(10).setAncho(10).setAlto(10);
        for(int id=1;id<=6;++id) data.totalItems.push_back(ItemBin::Build().setId(id).setLargo(1).setAncho(1).setAlto(1).setRotations(Punto::Build(1,1,1)));
        algorithm.setProblem(data);
        algorithm.visitedIndividuals["seen"] = true;
        algorithm.mutatePoblation(population, first, last);
    }
};

void checkCrossover()
{
    GeneticAlgorithm algorithm;
    for (bool upper : {false, true})
    {
        chooseUpperBound = upper;
        for (int n : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 50})
        {
            Chromosome ids, rotations;
            for (int id = 1; id <= n; ++id)
            {
                ids.push_back(id);
                rotations.push_back(id % 6 + 1);
            }
            Individuo first = Individuo::Build().setGenome(
                DoubleGenome::Build().setGenome(ids).setDGenome(rotations));
            std::reverse(ids.begin(), ids.end());
            std::reverse(rotations.begin(), rotations.end());
            Individuo second = Individuo::Build().setGenome(
                DoubleGenome::Build().setGenome(ids).setDGenome(rotations));
            const DoubleGenome originalFirst = first.getGenome();
            const DoubleGenome originalSecond = second.getGenome();
            Individuo child = GeneticAlgorithmTestAccess::crossover(algorithm, first, second);
            auto &genome = child.getGenome();
            if (genome.getGenome().size() != static_cast<size_t>(n) ||
                genome.getDGenome().size() != static_cast<size_t>(n))
                throw std::runtime_error("Crossover changed chromosome size");
            Chromosome sorted = genome.getGenome();
            std::sort(sorted.begin(), sorted.end());
            for (int i = 0; i < n; ++i)
                if (sorted[i] != i + 1 || genome.getDGenome()[i] != genome.getGenome()[i] % 6 + 1)
                    throw std::runtime_error("Crossover lost a box or its rotation");
            if (!(first.getGenome() == originalFirst) || !(second.getGenome() == originalSecond) ||
                child.getFitness() != 0.0)
                throw std::runtime_error("Crossover changed parents or child fitness");
        }
    }
    chooseUpperBound = false;
    bool rejected = false;
    try { GeneticAlgorithmTestAccess::crossover(algorithm, Individuo(), Individuo()); }
    catch (const std::invalid_argument &) { rejected = true; }
    if (!rejected) throw std::runtime_error("Empty parents must be rejected");
    std::cout << "PASS: crossover sizes 1-10 and 50 at both random boundaries; empty rejected\n";
}

void checkHamming()
{
    auto genome = [](Chromosome ids, Chromosome rotations = Chromosome()) {
        return DoubleGenome::Build().setGenome(ids).setDGenome(rotations);
    };
    auto expect = [](DoubleGenome first, DoubleGenome second, int distance) {
        if (hamming(first, second) != distance || hamming(second, first) != distance)
            throw std::runtime_error("Incorrect or asymmetric Hamming distance");
    };
    expect(genome({1, 2, 3}), genome({1, 2, 3}), 0);
    expect(genome({1, 2, 3}), genome({2, 1, 3}), 2);
    expect(genome({1, 2, 3}), genome({2, 3, 1}), 3);
    expect(genome({}, {}), genome({}, {}), 0);
    expect(genome({1}, {1}), genome({1}, {2}), 1);
    // An (item,rotation) pair is one locus: changing both counts once.
    expect(genome({1, 2, 3}, {1, 1, 1}), genome({2, 1, 3}, {2, 2, 1}), 2);
    expect(genome({1, 2, 3}, {1, 1, 1}), genome({1, 2, 3}, {1, 2, 1}), 1);
    expect(genome({1, 2, 3}, {1, 1, 1}), genome({1, 2, 3}, {1, 1, 1}), 0);

    const std::vector<DoubleGenome> samples{
        genome({1, 2, 3}, {1, 1, 1}), genome({2, 1, 3}, {2, 1, 1}),
        genome({2, 3, 1}, {1, 2, 3}), genome({1, 2, 3}, {2, 2, 2})};
    for (auto first : samples)
    for (auto second : samples)
    {
        int distance = hamming(first, second);
        if (distance < 0 || distance > 3 || distance != hamming(second, first))
            throw std::runtime_error("Hamming bounds/symmetry violated");
        for (auto third : samples)
            if (distance > hamming(first, third) + hamming(third, second))
                throw std::runtime_error("Hamming triangle inequality violated");
    }
    const DoubleGenome valid = genome({1, 2, 3}, {1, 1, 1});
    for (auto invalid : {genome({1, 2}, {1, 1}), genome({1, 2, 3}, {1}),
                         genome({1, 2, 3}), genome({1, 2, 3}, {1, 1, 1, 1})})
    {
        for (bool reverse : {false, true})
        {
            bool rejected = false;
            try { reverse ? hamming(invalid, valid) : hamming(valid, invalid); }
            catch (const std::invalid_argument &) { rejected = true; }
            if (!rejected) throw std::runtime_error("Incompatible Hamming genomes must be rejected");
        }
    }
    std::cout << "PASS: Hamming differences, rotations, symmetry, bounds, triangle and invalid lengths\n";
}

Individuo individual(const std::string &solution, double fitness)
{
    Individuo result;
    result.solution = solution;
    result.setFitness(fitness);
    return result;
}

void checkReplacement()
{
    int cases = 0;
    for (bool firstVisited : {false, true})
    for (bool secondVisited : {false, true})
    for (double firstFitness : {0.4, 0.6, 0.8})
    for (double secondFitness : {0.4, 0.6, 0.8})
    {
        GeneticAlgorithm algorithm;
        Poblacion population{individual("left", 0.9), individual("current", 0.6),
                             individual("right", 0.1)};
        Individuo first = individual("first", firstFitness);
        Individuo second = individual("second", secondFitness);
        std::string expected = "current";
        double expectedFitness = 0.6;
        // Only unseen children that strictly improve the current fitness qualify.
        // Preserve the existing tie rule: the second child wins a tie.
        if (!firstVisited && firstFitness > expectedFitness)
        {
            expected = "first";
            expectedFitness = firstFitness;
        }
        if (!secondVisited && secondFitness > 0.6 && secondFitness >= expectedFitness)
        {
            expected = "second";
            expectedFitness = secondFitness;
        }
        GeneticAlgorithmTestAccess::replace(algorithm, population, first, second,
                                           firstVisited, secondVisited);
        if (population[1].solution != expected || population[1].getFitness() != expectedFitness)
            throw std::runtime_error("Wrong child replacement: visited=" +
                std::to_string(firstVisited) + "," + std::to_string(secondVisited) +
                " fitness=" + std::to_string(firstFitness) + "," +
                std::to_string(secondFitness) + " expected=" + expected);
        if (population.size() != 3 || population[0].solution != "left" ||
            population[0].getFitness() != 0.9 || population[2].solution != "right" ||
            population[2].getFitness() != 0.1)
            throw std::runtime_error("Replacement modified another population member");
        ++cases;
    }
    std::cout << "PASS: " << cases << " child replacement cases\n";
}

void checkRange(int first, int last)
{
    GeneticAlgorithm algorithm;
    Poblacion population(5);
    for (auto &individual : population)
    {
        individual.setGenome(DoubleGenome::Build()
            .setGenome({1, 2, 3, 4, 5, 6}).setDGenome({0, 0, 0, 0, 0, 0}));
        individual.setFitness(0.25);
        individual.solution = "seen";
    }
    GeneticAlgorithmTestAccess::run(algorithm, population, first, last);
    for (int i = 0; i < 5; ++i)
    {
        const bool selected=i>=first && i<=last;
        // Identical newly evaluated packings may be penalized as duplicates.
        if ((selected && (population[i].solution=="seen" ||
                (population[i].getFitness()!=0 && population[i].getFitness()!=6.0/1000.0))) ||
            (!selected && (population[i].getFitness()!=0.25 || population[i].solution!="seen")))
            throw std::runtime_error("Incorrect inclusive mutation interval");
    }
}

int main()
{
    try
    {
        checkHamming();
        checkCrossover();
        checkReplacement();
        checkRange(1, 3);
        checkRange(2, 2);
        checkRange(0, 4);
        checkRange(3, 1);
        std::cout << "PASS: multiple, singleton, full and reversed intervals\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
