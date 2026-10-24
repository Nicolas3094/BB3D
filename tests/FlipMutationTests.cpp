#include "../src/genetic/GeneticsOperators.hpp"
#include <stdexcept>
#include <limits>

// Controlled random inputs for roulette and geometric-gap rotation selection.
double randomUnit = 0.5;
double uniformUnit() { return randomUnit; }
int randomInteger(int minimum, int maximum)
{
    if (minimum > maximum)
        throw std::runtime_error("Invalid random range");
    return maximum;
}

void require(bool condition, const char *message)
{
    if (!condition)
        throw std::runtime_error(message);
}

Poblacion populationWithFitness(std::initializer_list<double> fitnesses)
{
    Poblacion result;
    for (double fitness : fitnesses)
        result.push_back(Individuo::Build().setFitness(fitness));
    return result;
}

void checkRouletteWheel()
{
    const Poblacion population = populationWithFitness({0.125, 0.375, 0.5});
    randomUnit = 0.0;
    require(rouletteWheel(population) == 0, "Roulette must select first positive weight at zero");
    randomUnit = 0.125;
    require(rouletteWheel(population) == 1, "Roulette cumulative boundary is incorrect");
    randomUnit = 0.5;
    require(rouletteWheel(population) == 2, "Roulette must include the last individual");
    int counts[3] = {0, 0, 0};
    for (int i = 0; i < 1000; ++i)
    {
        randomUnit = (i + 0.5) / 1000.0;
        int selected = rouletteWheel(population);
        require(selected >= 0 && selected < 3, "Roulette returned invalid index");
        ++counts[selected];
    }
    require(counts[0] == 125 && counts[1] == 375 && counts[2] == 500,
            "Roulette selection must be proportional to fractional fitness");
    for (double unit : {0.0, 0.5, 0.999999})
    {
        randomUnit = unit;
        require(rouletteWheel(populationWithFitness({0.0, 0.25, 0.0})) == 1,
                "Zero-weight individuals must not be selected");
        require(rouletteWheel(populationWithFitness({0.3})) == 0, "Singleton index must be zero");
    }
    require(rouletteWheel(populationWithFitness({0.0, 0.0, 0.0})) == 2,
            "Zero total fitness must use uniform integer fallback");
    for (Poblacion invalid : {Poblacion(), populationWithFitness({-0.1, 0.5}),
             populationWithFitness({std::numeric_limits<double>::quiet_NaN()}),
             populationWithFitness({std::numeric_limits<double>::infinity()})})
    {
        bool rejected = false;
        try { rouletteWheel(invalid); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected, "Empty population or invalid fitness must be rejected");
    }
    randomUnit = 0.5;
    std::cout << "PASS: roulette boundaries, 1000 proportional samples, zeros and invalid inputs\n";
}

int main()
{
    try
    {
        checkRouletteWheel();
        // floor(log(1-0.9)/log(1-0.6)) = 2 skipped loci, then mutate index 2.
        randomUnit = 0.9;
        for (ROTATION_WAY rotation : {ROTATION_WAY::TWO_WAY, ROTATION_WAY::SIX_WAY})
        {
            DoubleGenome gen = DoubleGenome::Build()
                .setGenome({3, 1, 2}).setDGenome({1, 1, 1});
            flipMutationO1(gen, 0.6, rotation);
            require(gen.getDGenome()[2] == getIdFromRotationWay(rotation),
                    "Selected rotation was not written to chromosome");
            require(gen.getDGenome()[0] == 1 && gen.getDGenome()[1] == 1,
                    "Unselected rotations changed");
            require(gen.getGenome() == Chromosome({3, 1, 2}),
                    "Item order changed");
            require(gen.getDGenome().size() == 3, "Rotation chromosome resized");
        }
        DoubleGenome unchanged = DoubleGenome::Build()
            .setGenome({1, 2}).setDGenome({1, 2});
        flipMutationO1(unchanged, 0.0, ROTATION_WAY::TWO_WAY);
        require(unchanged.getDGenome() == Chromosome({1, 2}),
                "Zero probability must preserve rotations");
        std::cout << "PASS: two/six rotations, selected allele, unchanged order and zero probability\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
