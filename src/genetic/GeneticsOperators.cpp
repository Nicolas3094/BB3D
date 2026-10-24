#include "GeneticsOperators.hpp"
#include "PermutationOperators.hpp"
#include <cmath>
#include <limits>

void mutationWithType(DoubleGenome &gen, MutationType type, int step)
{
    int n;
    n = gen.getGenome().size();

    switch (type)
    {
    case C1:
        mutateC1(gen, step);
        break;
    case C2:
        mutateC2(gen, step);
        break;
    case INVERSE_MUTATION:
        mutateInversion(gen, step);
        break;
    }
}
void flipMutation(DoubleGenome &gen, double probability, ROTATION_WAY rotation)
{
    int n;
    n = gen.getGenome().size();
    for (int i = 0; i < n; i++)
    {
        if (uniformUnit() > probability)
        {
            continue;
        }
        int number = randomInteger(1, getIdFromRotationWay(rotation));
        gen.getDGenome()[i] = number;
    }
}

// Independent Bernoulli(p) selection via geometric gaps: expected O(1+n*p)
// time and O(1) auxiliary space. Every selected locus gets a different rotation.
void flipMutationO1(DoubleGenome &gen, double probability, ROTATION_WAY rotation)
{
    if (!std::isfinite(probability) || probability < 0.0 || probability > 1.0)
        throw std::invalid_argument("Mutation probability must be finite and in [0,1].");
    if (gen.getGenome().size() != gen.getDGenome().size())
        throw std::invalid_argument("Rotation mutation requires matching chromosomes.");
    switch (rotation)
    {
    case ROTATION_WAY::ZERO_WAY: return; // No alternative orientations.
    case ROTATION_WAY::TWO_WAY:
    case ROTATION_WAY::SIX_WAY: break;
    default: throw std::invalid_argument("Unsupported rotation mode.");
    }
    if (probability == 0.0) return;
    auto &rotations = gen.getDGenome();
    const int alternatives = getIdFromRotationWay(rotation);
    const double logFailure = probability < 1.0 ? std::log1p(-probability) : 0.0;
    size_t index = 0;
    while (index < rotations.size())
    {
        if (probability < 1.0)
        {
            const double gap = std::floor(std::log1p(-uniformUnit()) / logFailure);
            if (gap >= static_cast<double>(rotations.size() - index)) break;
            index += static_cast<size_t>(gap);
        }
        const int oldRotation = rotations[index];
        if (oldRotation < 1 || oldRotation > alternatives)
            throw std::invalid_argument("Rotation gene is outside the configured mode.");
        int replacement = randomInteger(1, alternatives - 1);
        if (replacement >= oldRotation) ++replacement;
        rotations[index++] = replacement;
    }
}

DoubleGenome crossOver(DoubleGenome gen1, DoubleGenome gen2)
{
    if (gen1.getGenome().empty() || gen1.getGenome().size() > static_cast<size_t>(std::numeric_limits<int>::max()) ||
        gen1.getGenome().size() != gen2.getGenome().size() ||
        gen1.getGenome().size() != gen1.getDGenome().size() || gen2.getGenome().size() != gen2.getDGenome().size())
        throw std::invalid_argument("Crossover requires matching non-empty chromosomes.");
    const int n = static_cast<int>(gen1.getGenome().size());
    const int start = n == 1 ? 0 : randomInteger(0, n - 2);
    const int end = n == 1 ? 0 : randomInteger(start + 1, n - 1);
    return crossOx(gen1, gen2, start, end);
}

int tournament(const Poblacion &poblation, int n, double probability, int excludedIndex)
{
    if (n < 1 || static_cast<size_t>(n) > poblation.size() || excludedIndex < -1 || excludedIndex >= n ||
        !std::isfinite(probability) || probability < 0.0 || probability > 1.0)
        throw std::invalid_argument("Tournament requires a valid population prefix, exclusion and probability.");
    const int eligible = n - (excludedIndex >= 0 ? 1 : 0);
    if (eligible == 0) throw std::invalid_argument("Tournament has no eligible candidates.");
    const auto originalIndex = [excludedIndex](int index) {
        return excludedIndex >= 0 && index >= excludedIndex ? index + 1 : index;
    };
    if (eligible == 1) return originalIndex(0);
    const int first = randomInteger(0, eligible - 1);
    int second = randomInteger(0, eligible - 2);
    if (second >= first) ++second;
    const int i1 = originalIndex(first), i2 = originalIndex(second);
    const bool firstIsBetter = poblation[i1].getFitness() > poblation[i2].getFitness();
    const int better = firstIsBetter ? i1 : i2;
    const int worse = firstIsBetter ? i2 : i1;
    return uniformUnit() < probability ? better : worse;
}

int rouletteWheel(Poblacion poblation)
{
    if (poblation.empty())
    {
        throw std::invalid_argument("Roulette selection requires a non-empty population.");
    }
    double totalFitness = 0.0;
    for (auto &individual : poblation)
    {
        const double fitness = individual.getFitness();
        if (!std::isfinite(fitness) || fitness < 0.0)
        {
            throw std::invalid_argument("Roulette fitness must be finite and non-negative.");
        }
        totalFitness += fitness;
    }
    if (!std::isfinite(totalFitness))
    {
        throw std::invalid_argument("Roulette total fitness must be finite.");
    }
    if (totalFitness == 0.0)
    {
        return randomInteger(0, static_cast<int>(poblation.size()) - 1);
    }

    const double target = uniformUnit() * totalFitness;
    double cumulativeFitness = 0.0;
    int lastPositive = 0;
    for (int i = 0; i < static_cast<int>(poblation.size()); ++i)
    {
        const double fitness = poblation[i].getFitness();
        if (fitness == 0.0)
            continue;
        lastPositive = i;
        cumulativeFitness += fitness;
        if (target < cumulativeFitness)
        {
            return i;
        }
    }
    // Handle floating-point rounding without selecting a zero-weight entry.
    return lastPositive;
}
