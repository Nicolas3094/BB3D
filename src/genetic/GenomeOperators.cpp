#include "GenomeOperators.hpp"
#include <chrono>
#include <unordered_set>
#include "../util/RandomNumbers.hpp"
#include "../util/ExecutionCheckpoint.hpp"

namespace
{
void shuffleIds(Chromosome &ids)
{
    for (size_t i = ids.size(); i > 1; --i)
    {
        if ((i & 255) == 0)
            checkExecutionDeadline();
        std::swap(ids[i - 1], ids[randomInteger(0, static_cast<int>(i - 1))]);
    }
}

void validateIds(const LoadedBins &items)
{
    std::unordered_set<int> ids;

    for (const auto &item : items)
        if (item.getId() <= 0 || !ids.insert(item.getId()).second)
            throw std::invalid_argument("Genome item IDs must be positive and unique.");
}
}

// Function to return the next random number
int getNum(std::vector<int> &v)
{
    if (v.empty())
        throw std::invalid_argument("Cannot draw from an empty vector.");

    int n = static_cast<int>(v.size());
    int index = randomInteger(0, n - 1);

    // Get random number from the vector
    int num = v[index];

    // Remove the number from the vector
    std::swap(v[index], v[n - 1]);
    v.pop_back();

    return num;
}

// Function to generate n non-repeating random numbers
Chromosome generatePermutation(int n)
{
    if (n < 0)
        throw std::invalid_argument("Permutation size must be non-negative.");

    Chromosome chromosome(n);

    for (int i = 0; i < n; i++)
    {
        if ((i & 255) == 0)
            checkExecutionDeadline();
        chromosome[i] = i + 1;
    }

    shuffleIds(chromosome);

    return chromosome;
}

LoadedBins decodeChromosome(Chromosome chromosome, LoadedBins allOrderByIdItems)
{
    validateIds(allOrderByIdItems);

    for (size_t i = 0; i < allOrderByIdItems.size(); ++i)
        if (allOrderByIdItems[i].getId() != static_cast<int>(i + 1))
            throw std::invalid_argument("Decoder data must be indexed by IDs 1..N.");

    std::vector<bool> used(allOrderByIdItems.size(), false);

    for (int id : chromosome)
    {
        if (id <= 0 || static_cast<size_t>(id) > allOrderByIdItems.size() || used[id - 1])
            throw std::invalid_argument("Decoder IDs must be in range and unique.");

        used[id - 1] = true;
    }
    LoadedBins bins(chromosome.size());

    for (int i = 0; i < chromosome.size(); i++)
    {
        bins[i] = allOrderByIdItems[chromosome[i] - 1];
    }
    return bins;
}

Chromosome codeChromosome(LoadedBins idItemsToPacked)
{
    validateIds(idItemsToPacked);
    Chromosome chromosome(idItemsToPacked.size());

    for (int i = 0; i < idItemsToPacked.size(); i++)
    {
        chromosome[i] = idItemsToPacked[i].getId();
    }
    return chromosome;
}

std::vector<Chromosome> buildHeuristicChromosomes(LoadedBins allItemBins, bool reverse = true)
{
    LoadedBins sortedByVolumes = allItemBins;
    LoadedBins sortedByLenght = allItemBins;
    LoadedBins sortedByWidth = allItemBins;
    LoadedBins sortedByHeight = allItemBins;
    std::vector<Chromosome> poblacionHeuristica;

    if (!reverse)
    {
        std::sort(sortedByVolumes.begin(), sortedByVolumes.end(), VolumeLess());
    }
    else
    {
        std::sort(sortedByVolumes.begin(), sortedByVolumes.end(), VolumeGreater());
    }
    poblacionHeuristica.push_back(codeChromosome(sortedByVolumes));

    if (reverse)
    {
        std::sort(sortedByLenght.begin(), sortedByLenght.end(), LengthLess());
    }
    else
    {
        std::sort(sortedByLenght.begin(), sortedByLenght.end(), LengthGreater());
    }
    poblacionHeuristica.push_back(codeChromosome(sortedByLenght));

    if (reverse)
    {
        std::sort(sortedByWidth.begin(), sortedByWidth.end(), WidthLess());
    }
    else
    {
        std::sort(sortedByWidth.begin(), sortedByWidth.end(), WidthGreater());
    }
    poblacionHeuristica.push_back(codeChromosome(sortedByWidth));

    if (reverse)
    {
        std::sort(sortedByHeight.begin(), sortedByHeight.end(), HeightLess());
    }
    else
    {
        std::sort(sortedByHeight.begin(), sortedByHeight.end(), HeighGreater());
    }
    poblacionHeuristica.push_back(codeChromosome(sortedByHeight));

    return poblacionHeuristica;
}

std::vector<Chromosome> buildCompleteHeuristicChromosomes(LoadedBins allBins, int numberOfChromosomes)
{
    if (numberOfChromosomes < 0)
        throw std::invalid_argument("Population size must be non-negative.");

    validateIds(allBins);

    if (numberOfChromosomes == 0)
        return {};

    std::vector<Chromosome> chromosomePoblation;
    std::vector<Chromosome> heursiticChromosomePoblation = buildHeuristicChromosomes(allBins);

    if (heursiticChromosomePoblation.size() > static_cast<size_t>(numberOfChromosomes))
        heursiticChromosomePoblation.resize(numberOfChromosomes);
    std::vector<Chromosome> randomChromosomePoblation = buildChromosomes(
        allBins, numberOfChromosomes - static_cast<int>(heursiticChromosomePoblation.size()));
    chromosomePoblation.reserve(heursiticChromosomePoblation.size() + randomChromosomePoblation.size());

    chromosomePoblation.insert(chromosomePoblation.end(), heursiticChromosomePoblation.begin(),
                               heursiticChromosomePoblation.end());
    chromosomePoblation.insert(chromosomePoblation.end(), randomChromosomePoblation.begin(),
                               randomChromosomePoblation.end());

    return chromosomePoblation;
}

std::vector<Chromosome> buildChromosomes(LoadedBins allBins, int numberOfChromosomes)
{
    if (numberOfChromosomes < 0)
        throw std::invalid_argument("Population size must be non-negative.");

    const Chromosome ids = codeChromosome(allBins);
    std::vector<Chromosome> chromosomePoblation(numberOfChromosomes);

    for (int i = 0; i < numberOfChromosomes; i++)
    {
        checkExecutionDeadline();
        chromosomePoblation[i] = ids;
        shuffleIds(chromosomePoblation[i]);
    }

    return chromosomePoblation;
}

Chromosome generateRandomRepeatedAlalleleChromosome(int NumberCount, int minimum, int maximum)
{
    checkExecutionDeadline();

    if (NumberCount < 0 || maximum < 0 || (maximum != 0 && minimum > maximum))
        throw std::invalid_argument("Invalid allele count or random range.");

    Chromosome values(NumberCount);

    if (maximum == 0)
    {
        for (int i = 0; i < NumberCount; i++)
        {
            values[i] = 0;
        }
        return values;
    }

    std::size_t index = 0;
    std::generate(values.begin(), values.end(),
                  [&]()
                  {
                      if ((index++ & 255) == 0)
                          checkExecutionDeadline();

                      return randomInteger(minimum, maximum);
                  });

    return values;
}
