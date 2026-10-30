#include "BRKGA2012.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>
#include <memory>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace brkga2012
{
namespace
{
/** @brief Ordena por volumen descendente y conserva el orden previo en los empates. */
void sortPopulation(Population &population)
{
    std::stable_sort(population.begin(), population.end(),
                     [](const Individual &first, const Individual &second)
                     {
                         return first.volume > second.volume;
                     });
}

/** @brief Rechaza configuraciones sin elites, mutantes o descendientes antes de reservar memoria. */
void validateConfiguration(const Configuration &configuration, size_t populationSize)
{
    if (populationSize < 2 || populationSize > Population().max_size() ||
        configuration.populations > std::vector<Population>().max_size() || !configuration.populations ||
        !configuration.migrationInterval || configuration.migrants > populationSize ||
        !std::isfinite(configuration.seconds) || configuration.seconds < 0 ||
        configuration.seconds > static_cast<double>(std::numeric_limits<std::int64_t>::max() / 1000))
        throw std::invalid_argument("Invalid BRKGA population, migration or time configuration");

    for (double probability :
         {configuration.eliteFraction, configuration.mutantFraction, configuration.eliteBias})
        if (!std::isfinite(probability) || probability < 0 || probability > 1)
            throw std::invalid_argument("BRKGA probabilities must be finite and in [0,1]");

    const auto elites = static_cast<size_t>(populationSize * configuration.eliteFraction);
    const auto mutants = static_cast<size_t>(populationSize * configuration.mutantFraction);

    if (!elites || !mutants || elites + mutants >= populationSize)
        throw std::invalid_argument("BRKGA needs positive elite, mutant and offspring counts");
}
}

double processCpuSeconds()
{
#ifdef _WIN32
    FILETIME created, exited, kernel, user;

    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
        throw std::runtime_error("Cannot query BRKGA process CPU time");

    ULARGE_INTEGER kernelTicks, userTicks;
    kernelTicks.LowPart = kernel.dwLowDateTime;
    kernelTicks.HighPart = kernel.dwHighDateTime;
    userTicks.LowPart = user.dwLowDateTime;
    userTicks.HighPart = user.dwHighDateTime;

    return (kernelTicks.QuadPart + userTicks.QuadPart) / 10000000.0;
#else
    return static_cast<double>(std::clock()) / CLOCKS_PER_SEC;
#endif
}

RandomKeys randomKeys(size_t length, std::mt19937_64 &random)
{
    RandomKeys keys;
    keys.reserve(length);

    for (size_t index = 0; index < length; ++index)
    {
        checkExecutionDeadline();
        keys.push_back(std::generate_canonical<double, 53>(random));
    }
    return keys;
}

RandomKeys biasedCrossover(const RandomKeys &elite, const RandomKeys &other, double bias,
                           std::mt19937_64 &random)
{
    if (elite.size() != other.size() || !std::isfinite(bias) || bias < 0 || bias > 1)
        throw std::invalid_argument("Invalid BRKGA crossover parents or bias");

    RandomKeys child;
    child.reserve(elite.size());

    for (size_t gene = 0; gene < elite.size(); ++gene)
    {
        checkExecutionDeadline();
        child.push_back(std::generate_canonical<double, 53>(random) < bias ? elite[gene] : other[gene]);
    }
    return child;
}

void exchangeElites(std::vector<Population> &populations, size_t migrants)
{
    if (!migrants)
        return;

    std::vector<const Individual *> ranked;

    for (const auto &population : populations)
    {
        if (population.size() < migrants)
            throw std::invalid_argument("Too many BRKGA migrants for a population");

        for (const auto &individual : population)
            ranked.push_back(&individual);
    }
    if (ranked.size() < migrants)
        throw std::invalid_argument("Not enough BRKGA individuals to exchange");

    std::stable_sort(ranked.begin(), ranked.end(),
                     [](const Individual *first, const Individual *second)
                     {
                         return first->volume > second->volume;
                     });
    Population snapshot;

    for (size_t index = 0; index < migrants; ++index)
        snapshot.push_back(*ranked[index]);
    for (auto &population : populations)
    {
        sortPopulation(population);
        std::copy(snapshot.begin(), snapshot.end(), population.end() - migrants);
        sortPopulation(population);
    }
}

SearchResult search(const DatasetBinBacking &data, std::uint64_t seed, const Configuration &configuration,
                    ExecutionBudget::Now controlledClock)
{
    using Clock = ExecutionBudget::Clock;
    const auto wallStart = Clock::now();
    const double cpuStart = processCpuSeconds();
    SearchResult result;
    result.packing = data.bin;
    result.packing.setRotationWay(ROTATION_WAY::SIX_WAY);
    const size_t itemCount = data.totalItems.size();

    if (itemCount > std::numeric_limits<size_t>::max() / 20)
        throw std::overflow_error("BRKGA population size overflow");

    result.populationSize = configuration.populationSize ? configuration.populationSize : 20 * itemCount;
    validateConfiguration(configuration, result.populationSize);
    result.initialized.assign(configuration.populations, 0);

    // Geometry checkpoints share the existing cooperative mechanism. CPU queries
    // are cached for 1ms wall time to avoid a syscall per containment comparison.
    auto lastCpuPoll = wallStart - std::chrono::milliseconds(1);
    double consumedCpu = 0;
    std::uint64_t checkpointCalls = 0;
    ExecutionBudget::Now budgetClock = controlledClock;

    if (!budgetClock && configuration.clock == BudgetClock::Wall)
        budgetClock = []
        {
            return Clock::now();
        };
    if (!budgetClock)
        budgetClock = [&]
        {
            // Sample the wall clock every 256 checkpoints; reading a clock for
            // every pair in quadratic pruning would dominate the actual search.
            if (checkpointCalls++ % 256 == 0)
            {
                const auto now = Clock::now();

                if (now - lastCpuPoll >= std::chrono::milliseconds(1))
                {
                    consumedCpu = processCpuSeconds() - cpuStart;
                    lastCpuPoll = now;
                }
            }
            return Clock::time_point{} +
                   std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(consumedCpu));
        };
    std::unique_ptr<ExecutionBudget> budget;

    if (configuration.seconds > 0)
        budget.reset(new ExecutionBudget(std::chrono::milliseconds(std::max<std::int64_t>(
                                             1, static_cast<std::int64_t>(configuration.seconds * 1000))),
                                         Individuo::Build(), budgetClock));

    try
    {
        const auto types = buildBoxTypes(data);
        std::mt19937_64 random(seed);
        auto evaluate = [&](RandomKeys keys)
        {
            checkExecutionDeadline();
            ++result.evaluationsStarted;
            auto packing = data.bin;
            packing.setRotationWay(ROTATION_WAY::SIX_WAY);
            auto observe = [&](bool partial)
            {
                if (packing.getLoadedVolume() > result.packing.getLoadedVolume() || result.keys.empty())
                {
                    result.packing = packing;
                    result.keys = keys;
                    result.partialEvaluation = partial;
                }
            };
            try
            {
                decodeLayers(packing, types, keys);
            }
            catch (const ExecutionTimeLimit &)
            {
                observe(true);
                throw;
            }
            observe(false);
            ++result.evaluationsCompleted;

            return Individual{std::move(keys), packing.getLoadedVolume()};
        };
        std::vector<Population> populations(configuration.populations);

        for (size_t populationIndex = 0; populationIndex < populations.size(); ++populationIndex)
        {
            auto &population = populations[populationIndex];
            population.reserve(result.populationSize);

            for (size_t individual = 0; individual < result.populationSize; ++individual)
            {
                population.push_back(evaluate(randomKeys(2 * itemCount, random)));
                ++result.initialized[populationIndex];
            }
            sortPopulation(population);
        }
        const size_t elites = result.populationSize * configuration.eliteFraction;
        const size_t mutants = result.populationSize * configuration.mutantFraction;

        while (result.generations < configuration.maximumGenerations)
        {
            checkExecutionDeadline();

            if (result.packing.getLoadedVolume() == result.packing.getDimensions().getVolumen())
            {
                result.termination = "full_container";
                break;
            }
            for (auto &population : populations)
            {
                Population next(population.begin(), population.begin() + elites);
                next.reserve(result.populationSize);
                std::uniform_int_distribution<size_t> eliteParent(0, elites - 1);
                std::uniform_int_distribution<size_t> otherParent(0, population.size() - 1);

                while (next.size() < result.populationSize - mutants)
                {
                    const auto first = eliteParent(random), second = otherParent(random);
                    next.push_back(evaluate(biasedCrossover(population[first].keys, population[second].keys,
                                                            configuration.eliteBias, random)));
                }
                while (next.size() < result.populationSize)
                    next.push_back(evaluate(randomKeys(2 * itemCount, random)));
                sortPopulation(next);
                population.swap(next);
            }
            ++result.generations;

            if (result.generations % configuration.migrationInterval == 0)
            {
                exchangeElites(populations, configuration.migrants);
                ++result.migrationEvents;
            }
        }
    }
    catch (const ExecutionTimeLimit &)
    {
        result.termination = "time_limit";
    }
    result.cpuSeconds = processCpuSeconds() - cpuStart;
    result.wallSeconds = std::chrono::duration<double>(Clock::now() - wallStart).count();

    return result;
}
}
