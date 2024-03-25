#include "GenomeOperators.hpp"
#include <chrono>

// Function to return the next random number
int getNum(std::vector<int> &v)
{
    // Size of the vector
    int n = v.size();

    // Generate a random number
    srand(time(NULL));

    // Make sure the number is within
    // the index range
    int index = rand() % n;

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
    Chromosome chromosome(n);
    static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine gen(seed);

    for (int i = 0; i < n; i++)
        chromosome[i] = i + 1;

    std::shuffle(chromosome.begin(), chromosome.end(), gen);

    return chromosome;
}

LoadedBins decodeChromosome(Chromosome chromosome, LoadedBins allOrderByIdItems)
{
    LoadedBins bins(chromosome.size());
    for (int i = 0; i < chromosome.size(); i++)
    {
        bins[i] = allOrderByIdItems[chromosome[i]];
    }
    return bins;
}

Chromosome codeChromosome(LoadedBins idItemsToPacked)
{
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
    std::vector<Chromosome> chromosomePoblation;
    std::vector<Chromosome> heursiticChromosomePoblation = buildHeuristicChromosomes(allBins);
    std::vector<Chromosome> randomChromosomePoblation =
        buildChromosomes(allBins, numberOfChromosomes - heursiticChromosomePoblation.size());
    chromosomePoblation.reserve(heursiticChromosomePoblation.size() +
                                randomChromosomePoblation.size());

    chromosomePoblation.insert(chromosomePoblation.end(),
                               heursiticChromosomePoblation.begin(),
                               heursiticChromosomePoblation.end());
    chromosomePoblation.insert(chromosomePoblation.end(),
                               randomChromosomePoblation.begin(),
                               randomChromosomePoblation.end());

    return chromosomePoblation;
}

std::vector<Chromosome> buildChromosomes(LoadedBins allBins, int numberOfChromosomes)
{
    std::vector<Chromosome> chromosomePoblation(numberOfChromosomes);
    int longOfChromosome = allBins.size();
    for (int i = 0; i < numberOfChromosomes; i++)
    {
        chromosomePoblation[i] = generatePermutation(longOfChromosome);
    }

    return chromosomePoblation;
}

Chromosome generateRandomRepeatedAlalleleChromosome(int NumberCount, int minimum, int maximum)
{
    Chromosome values(NumberCount);
    if (maximum == 0)
    {
        for (int i = 0; i < NumberCount; i++)
        {
            values[i] = 0;
        }
        return values;
    }
    static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine gen(seed);
    std::uniform_int_distribution<> dis(minimum, maximum);

    std::generate(values.begin(), values.end(), [&]()
                  { return dis(gen); });
    return values;
}
