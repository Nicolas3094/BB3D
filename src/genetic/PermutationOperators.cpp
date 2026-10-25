#include "PermutationOperators.hpp"
#include <map>
#include <unordered_set>
#include <limits>

int hamming(const DoubleGenome &gen1, const DoubleGenome &gen2)
{
    const auto &items1 = gen1.getGenome();
    const auto &items2 = gen2.getGenome();
    const auto &rotations1 = gen1.getDGenome();
    const auto &rotations2 = gen2.getDGenome();
    const bool withRotations = !rotations1.empty();
    if (items1.size() != items2.size() ||
        rotations1.empty() != rotations2.empty() ||
        (!rotations1.empty() && rotations1.size() != items1.size()) ||
        (!rotations2.empty() && rotations2.size() != items2.size()))
    {
        throw std::invalid_argument("Hamming requires compatible chromosome lengths and rotation encoding.");
    }
    int count = 0;
    for (size_t i = 0; i < items1.size(); ++i)
    {
        if((i&255)==0)checkExecutionDeadline();
        if (items1[i] != items2[i] ||
            (withRotations && rotations1[i] != rotations2[i]))
        {
            ++count;
        }
    }
    return count;
}

DoubleGenome crossOx(DoubleGenome gen1, DoubleGenome gen2, int i, int j)
{
    const size_t size = gen1.getGenome().size();
    if (size == 0 || size > static_cast<size_t>(std::numeric_limits<int>::max()) ||
        gen2.getGenome().size() != size || gen1.getDGenome().size() != size || gen2.getDGenome().size() != size)
        throw std::invalid_argument("Crossover requires matching non-empty item and rotation chromosomes.");
    if (i < 0 || j < i || static_cast<size_t>(j) >= size)
        throw std::invalid_argument("Crossover cut indices are outside the chromosome.");
    std::unordered_set<int> firstIds, secondIds;
    for (size_t index = 0; index < size; ++index)
    {
        const int first = gen1.getGenome()[index], second = gen2.getGenome()[index];
        if (first <= 0 || second <= 0 || !firstIds.insert(first).second || !secondIds.insert(second).second)
            throw std::invalid_argument("Crossover parents must contain positive, unique item IDs.");
        const int firstRotation = gen1.getDGenome()[index], secondRotation = gen2.getDGenome()[index];
        if (firstRotation < 0 || firstRotation > 6 || secondRotation < 0 || secondRotation > 6)
            throw std::invalid_argument("Crossover parents contain an invalid rotation code.");
    }
    if (firstIds != secondIds)
        throw std::invalid_argument("Crossover parents must contain the same item IDs.");
    int n = gen1.getGenome().size();
    Chromosome resultant(n);
    Chromosome resultantRot(n);
    std::map<int, bool> m;

    for (int k = i; k < j + 1; k++)
    {
        resultant[k] = gen1.getGenome()[k];
        resultantRot[k] = gen1.getDGenome()[k];
        m[gen1.getGenome()[k]] = true;
    }
    for (int k = j + 1; k < n; k++)
    {
        for (int l = 0; l < n; l++)
        {
            if (!m[gen2.getGenome()[l]])
            {
                m[gen2.getGenome()[l]] = true;
                resultant[k] = gen2.getGenome()[l];
                resultantRot[k] = gen2.getDGenome()[l];
                break;
            }
        }
    }
    for (int k = 0; k < i; k++)
    {
        for (int l = 0; l < n; l++)
        {
            if (!m[gen2.getGenome()[l]])
            {
                m[gen2.getGenome()[l]] = true;
                resultant[k] = gen2.getGenome()[l];
                resultantRot[k] = gen2.getDGenome()[l];
                break;
            }
        }
    }

    return DoubleGenome::Build().setGenome(resultant).setDGenome(resultantRot);
}

void swapPointValue(DoubleGenome &gen, int i, int value)
{
    if (gen.getGenome()[i] == value)
    {
        return;
    }
    int tmp = gen.getGenome()[i];
    int tmpRot = gen.getDGenome()[i];
    for (int j = 0; j < gen.getGenome().size(); j++)
    {
        if (i != j && gen.getGenome()[j] == value)
        {
            gen.getGenome()[i] = value;
            gen.getDGenome()[i] = gen.getDGenome()[j];
            gen.getGenome()[j] = tmp;
            gen.getDGenome()[j] = tmpRot;
            break;
        }
    }
}

void inverseMutation(DoubleGenome &gen, int i, int j)
{
    if (gen.getGenome().size() != gen.getDGenome().size() || i < 0 || j < i ||
        static_cast<size_t>(j) > gen.getGenome().size())
        throw std::invalid_argument("Inversion requires matching chromosomes and a valid half-open range.");
    std::reverse(gen.getGenome().begin() + i, gen.getGenome().begin() + j);
    std::reverse(gen.getDGenome().begin() + i, gen.getDGenome().begin() + j);
}

// O(1)
void insertion(DoubleGenome &gen, int indexToInsert, int indexValue)
{
    int n, valueToInsert, valueRotToInsert, tmpValue, tmpValueRot;
    n = gen.getGenome().size();
    valueToInsert = gen.getGenome()[indexValue];
    valueRotToInsert = gen.getDGenome()[indexValue];

    if (indexValue < indexToInsert)
    {
        gen.getGenome().insert(gen.getGenome().begin() + indexToInsert, valueToInsert);
        gen.getDGenome().insert(gen.getDGenome().begin() + indexToInsert, valueRotToInsert);

        gen.getGenome().erase(gen.getGenome().begin() + indexValue);
        gen.getDGenome().erase(gen.getDGenome().begin() + indexValue);
    }
    else if (indexValue > indexToInsert)
    {
        gen.getGenome().insert(gen.getGenome().begin() + indexToInsert, valueToInsert);
        gen.getDGenome().insert(gen.getDGenome().begin() + indexToInsert, valueRotToInsert);

        gen.getGenome().erase(gen.getGenome().begin() + indexValue + 1);
        gen.getDGenome().erase(gen.getDGenome().begin() + indexValue + 1);
    }
    else
    {
        throw std::invalid_argument("RI: index inside the interval [init, end]");
    }
}

// O(n)
void insertionOfSubsequence(DoubleGenome &gen, int indexToInsert, int init, int end)
{
    DoubleGenome subSequence = buildSubsequenceGenome(gen, init, end);

    addSubsequence(gen, subSequence, indexToInsert, init, end);
}

// O(n)
void randomReversingInsertionOfSubsequence(DoubleGenome &gen, int indexToInsert, int init, int end)
{
    double r;
    DoubleGenome subSequenceGenome = buildSubsequenceGenome(gen, init, end);
    r = uniformUnit();

    if (r > 0.5)
    {
        reverseGenome(subSequenceGenome);
    }

    addSubsequence(gen, subSequenceGenome, indexToInsert, init, end);
}

void swap(DoubleGenome &gen, int i, int j)
{
    int valueSwap, valueSwapRot, valueSwap2, valueSwapRot2;

    valueSwap = gen.getGenome()[i];
    valueSwapRot = gen.getDGenome()[i];
    valueSwap2 = gen.getGenome()[j];
    valueSwapRot2 = gen.getDGenome()[j];

    gen.getGenome()[i] = valueSwap2;
    gen.getDGenome()[i] = valueSwapRot2;
    gen.getGenome()[j] = valueSwap;
    gen.getDGenome()[j] = valueSwapRot;
}

void swapSubsequences(DoubleGenome &gen, int init1, int end1, int init2, int end2)
{
    if (end1 - init1 != end2 - init2)
    {
        throw std::invalid_argument("Subsequences most have with same lenght.");
    }
    const DoubleGenome first = buildSubsequence(gen, init1, end1);
    const DoubleGenome second = buildSubsequence(gen, init2, end2);
    copySubsequence(gen, second, init1, end1);
    copySubsequence(gen, first, init2, end2);
}

void randomReversingSwapOfSubsequences(
    DoubleGenome &gen, int init1, int end1, int init2, int end2)
{
    if (end1 - init1 != end2 - init2)
    {
        throw std::invalid_argument("Subsequences most have with same lenght.");
    }
    double r1, r2;
    r1 = uniformUnit();
    r2 = uniformUnit();
    DoubleGenome subSequenceGenome = buildSubsequence(gen, init2, end2);
    DoubleGenome secondSubSequenceGenome = buildSubsequence(gen, init1, end1);

    if (r1 <= 0.5)
    {
        reverseGenome(subSequenceGenome);
    }
    if (r2 <= 0.5)
    {
        reverseGenome(secondSubSequenceGenome);
    }
    copySubsequence(gen, subSequenceGenome, init1, end1);
    copySubsequence(gen, secondSubSequenceGenome, init2, end2);
}

void combine1(DoubleGenome &gen, int i1, int j1, int i2, int j2)
{
    double r = uniformUnit();

    if (r < 1.0 / 3.0)
    {
        swap(gen, i1, j2);
    }
    else if (r < 2.0 / 3.0)
    {
        swapSubsequences(gen, i1, j1, i2, j2);
    }
    else
    {
        randomReversingSwapOfSubsequences(gen, i1, j1, i2, j2);
    }
}

void combine2(DoubleGenome &gen, int indexToInsert, int i, int j)
{
    double ran = uniformUnit();

    if (ran < 1.0 / 3.0)
    {
        insertion(gen, indexToInsert, i);
    }
    else if (ran < 2.0 / 3.0)
    {
        insertionOfSubsequence(gen, indexToInsert, i, j);
    }
    else
    {
        randomReversingInsertionOfSubsequence(gen, indexToInsert, i, j);
    }
}

void mutateC1(DoubleGenome &gen, int randomStep)
{
    if (gen.getGenome().size() != gen.getDGenome().size() || randomStep < -1)
        throw std::invalid_argument("C1 requires matching chromosomes and step >= -1.");
    const int n = static_cast<int>(gen.getGenome().size());
    if (n < 2) return;
    const int half = n / 2;
    const int length = randomStep == -1 ? randomInteger(1, half)
        : std::min(half, std::max(1, randomStep / 2));
    // Equal, non-empty segments in disjoint halves; explicit steps are bounded.
    const int first = randomInteger(0, half - length);
    const int second = randomInteger(half, n - length);
    combine1(gen, first, first + length - 1, second, second + length - 1);
}

void mutateC2(DoubleGenome &gen, int randomStep)
{
    if (gen.getGenome().size() != gen.getDGenome().size() || randomStep < -1)
        throw std::invalid_argument("C2 requires matching chromosomes and step >= -1.");
    const int n = static_cast<int>(gen.getGenome().size());
    if (n < 2) return;
    const int length = randomStep == -1 ? randomInteger(1, n - 1)
        : std::min(n - 1, std::max(1, randomStep));
    const int init = randomInteger(0, n - length);
    const int end = init + length - 1;
    // Insertion uses the original vector boundary. Skip the immediately
    // adjacent boundary on the right, which would leave the sequence unchanged.
    const int index = init > 0 && (end == n - 1 || uniformUnit() < 0.5)
        ? randomInteger(0, init - 1) : randomInteger(end + 2, n);
    combine2(gen, index, init, end);
}

void mutateInversion(DoubleGenome &gen, int randomStep)
{
    if (gen.getGenome().size() != gen.getDGenome().size() || randomStep < -1)
        throw std::invalid_argument("Inversion requires matching chromosomes and step >= -1.");
    const int n = static_cast<int>(gen.getGenome().size());
    if (n < 2) return;
    const int length = randomStep == -1 ? randomInteger(2, n)
        : std::min(n, std::max(2, randomStep));
    const int start = randomInteger(0, n - length);
    inverseMutation(gen, start, start + length); // End iterator is exclusive.
}

void addRangeToChromosome(Chromosome &chromosome, Chromosome chromosmeToAdd, int init, int end)
{
    for (int i = init; i < end; i++)
    {
        chromosome.push_back(chromosmeToAdd[i]);
    }
}

void addRangeToGenome(DoubleGenome &gen, DoubleGenome genToAdd, int init, int end)
{

    for (int i = init; i < end; i++)
    {
        gen.getGenome().push_back(genToAdd.getGenome()[i]);
        gen.getDGenome().push_back(genToAdd.getDGenome()[i]);
    }
}

void addToGenome(DoubleGenome &gen, int value, int valueRot)
{
    gen.getGenome().push_back(value);
    gen.getDGenome().push_back(valueRot);
}

void copyEqualRangeToGenome(
    DoubleGenome &gen, DoubleGenome genToAdd, int initGen, int endGen, int initCopy, int endCopy)
{
    if(gen.getGenome().size()!=gen.getDGenome().size() || genToAdd.getGenome().size()!=genToAdd.getDGenome().size() ||
       initGen<0 || initCopy<0 || endGen<initGen || endCopy<initCopy ||
       static_cast<size_t>(endGen)>=gen.getGenome().size() || static_cast<size_t>(endCopy)>=genToAdd.getGenome().size() ||
       endGen-initGen!=endCopy-initCopy)
        throw std::invalid_argument("Copy requires valid inclusive intervals of equal length and paired chromosomes.");
    for (size_t offset=0;offset<=static_cast<size_t>(endGen-initGen);++offset)
    {
        gen.getGenome()[static_cast<size_t>(initGen)+offset] = genToAdd.getGenome()[static_cast<size_t>(initCopy)+offset];
        gen.getDGenome()[static_cast<size_t>(initGen)+offset] = genToAdd.getDGenome()[static_cast<size_t>(initCopy)+offset];
    }
}

void copyReversedEqualRangeToGenome(
    DoubleGenome &gen, DoubleGenome genToAdd, int initGen, int endGen, int initCopy, int endCopy)
{
    if(gen.getGenome().size()!=gen.getDGenome().size() || genToAdd.getGenome().size()!=genToAdd.getDGenome().size() ||
       initGen<0 || initCopy<0 || endGen<initGen || endCopy<initCopy ||
       static_cast<size_t>(endGen)>=gen.getGenome().size() || static_cast<size_t>(endCopy)>=genToAdd.getGenome().size() ||
       endGen-initGen!=endCopy-initCopy)
        throw std::invalid_argument("Reverse copy requires valid inclusive intervals of equal length and paired chromosomes.");
    for (size_t offset=0;offset<=static_cast<size_t>(endGen-initGen);++offset)
    {
        gen.getGenome()[static_cast<size_t>(initGen)+offset] = genToAdd.getGenome()[static_cast<size_t>(endCopy)-offset];
        gen.getDGenome()[static_cast<size_t>(initGen)+offset] = genToAdd.getDGenome()[static_cast<size_t>(endCopy)-offset];
    }
}

DoubleGenome buildSubsequence(DoubleGenome gen, int init, int end)
{
    DoubleGenome subSequence;
    for (int i = init; i <= end; i++)
    {
        subSequence.getGenome().push_back(gen.getGenome()[i]);
        subSequence.getDGenome().push_back(gen.getDGenome()[i]);
    }
    return subSequence;
}

void copySubsequence(DoubleGenome &gen, DoubleGenome subSequenceGenome, int init, int end)
{
    for (int i = init; i <= end; i++)
    {
        gen.getGenome()[i] = subSequenceGenome.getGenome()[i - init];
        gen.getDGenome()[i] = subSequenceGenome.getDGenome()[i - init];
    }
}

void reverseGenome(DoubleGenome &subSequenceGenome)
{
    std::reverse(subSequenceGenome.getGenome().begin(), subSequenceGenome.getGenome().end());
    std::reverse(subSequenceGenome.getDGenome().begin(), subSequenceGenome.getDGenome().end());
}

DoubleGenome buildSubsequenceGenome(DoubleGenome fullSequence, int init, int end)
{
    DoubleGenome subSequence;
    for (int i = init; i < end + 1; i++)
    {
        subSequence.getGenome().push_back(fullSequence.getGenome()[i]);
        subSequence.getDGenome().push_back(fullSequence.getDGenome()[i]);
    }
    return subSequence;
}

void addSubsequence(
    DoubleGenome &fullSequence, DoubleGenome subSequence, int indexToInsert, int init, int end)
{
    int n;

    n = fullSequence.getGenome().size();

    if (indexToInsert < init)
    {
        fullSequence.getGenome().erase(fullSequence.getGenome().begin() + init,
                                       fullSequence.getGenome().begin() + end + 1);
        fullSequence.getDGenome().erase(fullSequence.getDGenome().begin() + init,
                                        fullSequence.getDGenome().begin() + end + 1);

        fullSequence.getGenome().insert(fullSequence.getGenome().begin() + indexToInsert,
                                        subSequence.getGenome().begin(),
                                        subSequence.getGenome().end());
        fullSequence.getDGenome().insert(fullSequence.getDGenome().begin() + indexToInsert,
                                         subSequence.getDGenome().begin(),
                                         subSequence.getDGenome().end());
    }
    else if (indexToInsert > end)
    {
        fullSequence.getGenome().insert(fullSequence.getGenome().begin() + indexToInsert,
                                        subSequence.getGenome().begin(),
                                        subSequence.getGenome().end());
        fullSequence.getDGenome().insert(fullSequence.getDGenome().begin() + indexToInsert,
                                         subSequence.getDGenome().begin(),
                                         subSequence.getDGenome().end());

        fullSequence.getGenome().erase(fullSequence.getGenome().begin() + init,
                                       fullSequence.getGenome().begin() + end + 1);
        fullSequence.getDGenome().erase(fullSequence.getDGenome().begin() + init,
                                        fullSequence.getDGenome().begin() + end + 1);
    }
    else
    {
        throw std::invalid_argument("RIS/RRIS: index inside the interval [init, end]");
    }
    if (fullSequence.getGenome().size() != n || fullSequence.getDGenome().size() != n)
    {
        std::cout << "result " << fullSequence.getGenome() << "\n";
        throw std::invalid_argument("RIS/RRIS: mutation got different size.");
    }
}
