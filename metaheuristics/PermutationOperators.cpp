#include "PermutationOperators.hpp"
#include <map>

int hamming(DoubleGenome gen1, DoubleGenome gen2)
{
    int count = 0;
    for (int i = 0; i < gen1.getGenome().size(); i++)
    {
        if (gen1.getDGenome().size() > 0)
        {
            if (gen1.getGenome()[i] == gen2.getGenome()[i] && gen1.getDGenome()[i] == gen2.getDGenome()[i])
            {
                count += 1;
            }
        }
        else
        {

            if (gen1.getGenome()[i] == gen2.getGenome()[i])
            {
                count += 1;
            }
        }
    }
    return count;
}

DoubleGenome crossOx(DoubleGenome gen1, DoubleGenome gen2, int i, int j)
{
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
    std::reverse(gen.getGenome().begin() + i, gen.getGenome().begin() + j);
    std::reverse(gen.getDGenome().begin() + i, gen.getDGenome().begin() + j);
}

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

void insertionOfSubsequence(DoubleGenome &gen, int indexToInsert, int init, int end)
{
    int n = gen.getGenome().size();
    int nSub = end - init + 1;
    Chromosome subSequence(nSub);
    Chromosome subSequenceRot(nSub);

    for (int i = init; i < end + 1; i++)
    {
        subSequence[i - init] = gen.getGenome()[i];
        subSequenceRot[i - init] = gen.getDGenome()[i];
    }
    if (indexToInsert < init)
    {
        gen.getGenome().erase(gen.getGenome().begin() + init, gen.getGenome().begin() + end + 1);
        gen.getDGenome().erase(gen.getDGenome().begin() + init, gen.getDGenome().begin() + end + 1);
        gen.getGenome().insert(gen.getGenome().begin() + indexToInsert, subSequence.begin(), subSequence.end());
        gen.getDGenome().insert(gen.getDGenome().begin() + indexToInsert, subSequenceRot.begin(), subSequenceRot.end());
    }
    else if (indexToInsert > end)
    {
        gen.getGenome().insert(gen.getGenome().begin() + indexToInsert, subSequence.begin(), subSequence.end());
        gen.getDGenome().insert(gen.getDGenome().begin() + indexToInsert, subSequenceRot.begin(), subSequenceRot.end());

        gen.getGenome().erase(gen.getGenome().begin() + init, gen.getGenome().begin() + end + 1);
        gen.getDGenome().erase(gen.getDGenome().begin() + init, gen.getDGenome().begin() + end + 1);
    }
    else
    {
        throw std::invalid_argument("RIS: index inside the interval [init, end]");
    }
    if (gen.getGenome().size() != n || gen.getDGenome().size() != n)
    {
        throw std::invalid_argument("RIS: mutation got different size.");
    }
}

void randomReversingInsertionOfSubsequence(DoubleGenome &gen, int indexToInsert, int init, int end)
{
    DoubleGenome newGenome, subSequenceGenome;
    int n, value, valueRot, nSub;
    double r;

    n = gen.getGenome().size();
    value = gen.getGenome()[indexToInsert];
    valueRot = gen.getDGenome()[indexToInsert];
    r = uniformUnit();
    addRangeToGenome(subSequenceGenome, gen, init, end + 1);
    nSub = subSequenceGenome.getGenome().size();

    if (r > 0.5)
    {
        std::reverse(subSequenceGenome.getGenome().begin(), subSequenceGenome.getGenome().end());
        std::reverse(subSequenceGenome.getDGenome().begin(), subSequenceGenome.getDGenome().end());
    }
    if (indexToInsert < init)
    {
        gen.getGenome().erase(gen.getGenome().begin() + init, gen.getGenome().begin() + end + 1);
        gen.getDGenome().erase(gen.getDGenome().begin() + init, gen.getDGenome().begin() + end + 1);

        gen.getGenome().insert(gen.getGenome().begin() + indexToInsert, subSequenceGenome.getGenome().begin(), subSequenceGenome.getGenome().end());
        gen.getDGenome().insert(gen.getDGenome().begin() + indexToInsert, subSequenceGenome.getDGenome().begin(), subSequenceGenome.getDGenome().end());
    }
    else if (indexToInsert > end)
    {
        gen.getGenome().insert(gen.getGenome().begin() + indexToInsert, subSequenceGenome.getGenome().begin(), subSequenceGenome.getGenome().end());
        gen.getDGenome().insert(gen.getDGenome().begin() + indexToInsert, subSequenceGenome.getDGenome().begin(), subSequenceGenome.getDGenome().end());

        gen.getGenome().erase(gen.getGenome().begin() + init, gen.getGenome().begin() + end + 1);
        gen.getDGenome().erase(gen.getDGenome().begin() + init, gen.getDGenome().begin() + end + 1);
    }
    else
    {
        throw std::invalid_argument("RRIS: index inside the interval [init, end]");
    }
    if (gen.getGenome().size() != n || gen.getDGenome().size() != n)
    {
        throw std::invalid_argument("RRIS: mutation got different size.");
    }
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
    copySubsequence(gen, buildSubsequence(gen, init2, end2), init1, end1);
    copySubsequence(gen, buildSubsequence(gen, init1, end1), init2, end2);
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
    ;

    if (r <= 1 / 3)
    {
        swap(gen, i1, j2);
    }
    else if (r > 1 / 3 && r <= 2 / 3)
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

    if (ran <= 1 / 3)
    {
        insertion(gen, i, j);
    }
    if (ran > 1 / 3 && ran <= 2 / 3)
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
    int n, step, i, j, i2, j2, low, high;
    n = gen.getDGenome().size();
    step = randomStep;

    if (randomStep == 1)
    {
        step = randomInteger(1, n - 2);
    }
    else if (randomStep <= 2 && randomStep >= 0)
    {
        step = 2;
    }
    low = 1;
    high = std::floor(n / 2) - static_cast<int>(std::floor(step / 2)) + 1;
    i = randomInteger(low, high);
    j = i + static_cast<int>(std::floor(step / 2)) - 1;
    low = static_cast<int>(std::floor(n / 2));
    high = n - static_cast<int>(std::floor(step / 2));
    i2 = randomInteger(low, high);
    j2 = i2 + static_cast<int>(std::floor(step / 2)) - 1;

    combine1(gen, i, j, i2, j2);
}

void mutateC2(DoubleGenome &gen, int randomStep)
{
    int n, step, init, end, index;
    double r;
    r = uniformUnit();
    n = gen.getDGenome().size();
    step = randomStep;

    if (randomStep == -1)
    {
        step = randomInteger(1, n - 2);
    }
    else if (randomStep < 2 && randomStep >= 0)
    {
        step = 2;
    }
    init = randomInteger(1, static_cast<int>(std::floor(n / 2)));
    if (init + step > n - 2)
    {
        end = n - 2;
    }
    else
    {
        end = init + step - 1;
    }
    if (r < 0.5)
    {
        index = randomInteger(0, init - 1);
    }
    else
    {
        index = randomInteger(end + 1, n);
    }
    combine2(gen, index, init, end);
}

void mutateInversion(DoubleGenome &gen, int randomStep)
{
    // std::cout << "Mutate inversion\n";
    int n, step, i, j;
    n = gen.getDGenome().size();
    step = randomStep;
    if (randomStep == -1)
    {
        step = randomInteger(1, n - 2);
    }
    else if (randomStep <= 2)
    {
        step = 2;
    }
    i = randomInteger(1, static_cast<int>(std::floor(n / 2)));
    // std::cout << "i " << i << ", j " << j << "\n";

    if (i + step > n - 1)
    {
        j = n - 1;
    }
    else
    {
        j = i + step - 1;
    }
    inverseMutation(gen, i, j);
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
    int initRange = initCopy;
    for (int i = initGen; i <= endGen + 1; i++)
    {
        gen.getGenome()[i] = genToAdd.getGenome()[initRange];
        gen.getDGenome()[i] = genToAdd.getDGenome()[initRange];
        initRange++;
    }
    if (initCopy > endCopy)
    {
        throw std::invalid_argument(" CopyEqualRangeToGenome ERROR");
    }
}

void copyReversedEqualRangeToGenome(
    DoubleGenome &gen, DoubleGenome genToAdd, int initGen, int endGen, int initCopy, int endCopy)
{
    int initRange = endCopy;
    for (int i = initGen; i <= endGen + 1; i++)
    {
        gen.getGenome()[i] = genToAdd.getGenome()[initRange];
        gen.getDGenome()[i] = genToAdd.getDGenome()[initRange];
        initRange--;
    }
    if (initCopy < endCopy)
    {
        throw std::invalid_argument(" CopyReversedEqualRangeToGenome ERROR");
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