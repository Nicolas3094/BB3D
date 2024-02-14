#include "PermutationOperators.hpp"
#include <map>

int hamming(DoubleGenome gen1, DoubleGenome gen2)
{
    int count = 0;
    for (int i = 0; i < gen1.getGenome().size(); i++)
    {
        if (gen1.getGenome()[i] != gen2.getGenome()[i] && gen1.getDGenome()[i] != gen2.getDGenome()[i])
        {
            count += 1;
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
            }
            break;
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
            gen.getGenome()[j] = tmpRot;
            break;
        }
    }
}

void inverseMutation(DoubleGenome &gen, int i, int j)
{
    DoubleGenome genTmp = gen;
    for (int k = i; k <= j; k++)
    {
        gen.getGenome()[k] = genTmp.getGenome()[j - k + i];
    }
}

void randomInsertion(DoubleGenome &gen, int indexToInsert, int indexValue)
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
        return;
    }
}

DoubleGenome randomInsertionOfSubsequence(DoubleGenome gen, int indexToInsert, int i, int j)
{
    int n = gen.getGenome().size();
    DoubleGenome newGenome;
    DoubleGenome subSequenceGenome;

    int valueIndex = gen.getGenome()[indexToInsert];
    int valueIndexRot = gen.getDGenome()[indexToInsert];

    addRangeToGenome(subSequenceGenome, gen, i, j + 1);
    int nSub = subSequenceGenome.getGenome().size();

    if (indexToInsert < i)
    {
        addRangeToGenome(newGenome, gen, 0, indexToInsert);
        addRangeToGenome(newGenome, subSequenceGenome, 0, nSub);
        addToGenome(newGenome, valueIndex, valueIndexRot);

        if (indexToInsert < i - 1)
        {

            addRangeToGenome(newGenome, gen, indexToInsert + 1, i);
        }
        if (j < n - 1)
        {
            addRangeToGenome(newGenome, gen, j + 1, n);
        }
        else if (j < indexToInsert)
        {
            addRangeToGenome(newGenome, gen, 0, i);

            if (j + 1 < indexToInsert)
            {
                addRangeToGenome(newGenome, gen, j + 1, indexToInsert);
            }
            if (indexToInsert < n - 1)
            {
                addRangeToGenome(newGenome, subSequenceGenome, 0, nSub);
                addToGenome(newGenome, valueIndex, valueIndexRot);
            }
            else
            {
                addToGenome(newGenome, valueIndex, valueIndexRot);
                addRangeToGenome(newGenome, subSequenceGenome, 0, nSub);
            }
            addRangeToGenome(newGenome, gen, indexToInsert + 1, n);
        }
        if (newGenome.getGenome().size() != n)
        {
            throw std::invalid_argument("Genes are not equal.");
        }
        return newGenome;
    }
}

DoubleGenome randomReversingInsertionOfSubsequence(DoubleGenome gen, int indexToInsert, int i, int j)
{
    DoubleGenome newGenome;
    DoubleGenome subSequenceGenome;
    int n = gen.getGenome().size();
    int value = gen.getGenome()[indexToInsert];
    int valueRot = gen.getDGenome()[indexToInsert];
    double r = ((double)rand() / (RAND_MAX)) + 1;

    addRangeToGenome(subSequenceGenome, gen, i, j + 1);
    int nSub = subSequenceGenome.getGenome().size();

    if (r > 0.5)
    {
        // it is inverted
        for (int k = 0; k < nSub; k++)
        {
            subSequenceGenome.getGenome()[k] = gen.getGenome()[nSub - k - 1];
        }
    }
    if (indexToInsert < i)
    {
        addRangeToGenome(newGenome, gen, 0, indexToInsert);
        addRangeToGenome(newGenome, gen, 0, nSub);
        addToGenome(newGenome, value, valueRot);

        if (indexToInsert < i - 1)
        {
            addRangeToGenome(newGenome, gen, indexToInsert + 1, i);
        }
        if (j < n - 1)
        {
            addRangeToGenome(newGenome, gen, j + 1, n);
        }
    }
    else if (indexToInsert > j)
    {
        addRangeToGenome(newGenome, gen, 0, i);

        if (j + 1 < indexToInsert)
        {
            addRangeToGenome(newGenome, gen, j + 1, indexToInsert);
        }
        if (indexToInsert < n - 1 || r > 0.5)
        {

            addRangeToGenome(newGenome, gen, 0, nSub);
        }
        else
        {
            addToGenome(newGenome, value, valueRot);
            addRangeToGenome(newGenome, gen, 0, nSub);
        }
        addRangeToGenome(newGenome, gen, indexToInsert + 1, n);
    }

    if (newGenome.getGenome().size() != n)
    {
        throw std::invalid_argument("Chromosomes are not equal.");
    }
    return newGenome;
}

void randomSwap(DoubleGenome &gen, int i, int j)
{

    int valueSwap = gen.getGenome()[i];
    int valueSwapRot = gen.getDGenome()[i];

    gen.getGenome()[i] = newGen.getGenome()[j];
    gen.getDGenome()[i] = newGen.getDGenome()[j];
    newGen.getGenome()[j] = valueSwap;
    newGen.getDGenome()[j] = valueSwapRot;
}

void randomSwapSubsequences(DoubleGenome &gen, int init1, int end1, int init2, int end2)
{
    if (end1 - init1 != end2 - init2)
    {
        throw std::invalid_argument("Subsequences most have with same lenght.");
    }
    int n;
    n = end2 - init2 + 1;
    DoubleGenome subSequenceGenome;
    for (int i = init2; i < n; i++)
    {
        subSequenceGenome.getGenome().push_back(gen.getGenome()[i]);
        subSequenceGenome.getDGenome().push_back(gen.getDGenome()[i]);
    }

    copyEqualRangeToGenome(gen, subSequenceGenome, init1, end1, 0, n - 1);
    subSequenceGenome.getGenome().clear();
    subSequenceGenome.getDGenome().clear();
    for (int i = init1; i < n; i++)
    {
        subSequenceGenome.getGenome().push_back(gen.getGenome()[i]);
        subSequenceGenome.getDGenome().push_back(gen.getDGenome()[i]);
    }
    copyEqualRangeToGenome(gen, subSequenceGenome, init2, end2, 0, n - 1);
}

void randomReversingSwapOfSubsequences(
    DoubleGenome gen &, int init1, int end1, int init2, int end2)
{
    double r1 = ((double)rand() / (RAND_MAX)) + 1;
    double r2 = ((double)rand() / (RAND_MAX)) + 1;
    if (end1 - init1 != end2 - init2)
    {
        throw std::invalid_argument("Subsequences most have with same lenght.");
    }
    DoubleGenome subSequenceGenome;
    int n;
    n = end2 - init2 + 1;
    if (r1 > 0.5)
    {
        for (int i = init2; i < n; i++)
        {
            subSequenceGenome.getGenome().push_back(gen.getGenome()[i]);
            subSequenceGenome.getDGenome().push_back(gen.getDGenome()[i]);
        }
        copyEqualRangeToGenome(gen, subSequenceGenome, init1, end1, 0, n - 1);
        subSequenceGenome.getGenome().clear();
        subSequenceGenome.getDGenome().clear();
    }
    else
    {

        for (int i = init2; i < n; i++)
        {
            subSequenceGenome.getGenome().push_back(gen.getGenome()[i]);
            subSequenceGenome.getDGenome().push_back(gen.getDGenome()[i]);
        }
        copyReversedEqualRangeToGenome(gen, subSequenceGenome, init1, end1, init2, end2);
        subSequenceGenome.getGenome().clear();
        subSequenceGenome.getDGenome().clear();
    }
    if (r2 > 0.5)
    {
        for (int i = init1; i < n; i++)
        {
            subSequenceGenome.getGenome().push_back(gen.getGenome()[i]);
            subSequenceGenome.getDGenome().push_back(gen.getDGenome()[i]);
        }
        copyEqualRangeToGenome(gen, subSequenceGenome, init2, end2, 0, n - 1);
    }
    else
    {
        for (int i = init1; i < n; i++)
        {
            subSequenceGenome.getGenome().push_back(gen.getGenome()[i]);
            subSequenceGenome.getDGenome().push_back(gen.getDGenome()[i]);
        }
        copyReversedEqualRangeToGenome(gen, gen, init2, end2, init1, end1);
    }
}

void combine1(DoubleGenome &gen, int i1, int j1, int i2, int j2)
{
    double r = ((double)rand() / (RAND_MAX)) + 1;

    if (r <= 1 / 3)
    {
        randomSwap(gen, i1, j2);
    }
    else if (r > 1 / 3 && r <= 2 / 3)
    {
        randomSwapSubsequences(gen, i1, j1, i2, j2);
    }
    else
    {
        randomReversingSwapOfSubsequences(gen, i1, j1, i2, j2);
    }
}

void combine2(DoubleGenome &gen, int indexToInsert, int i, int j)
{
    double ran = ((double)rand() / (RAND_MAX)) + 1;

    if (ran <= 1 / 3)
    {
        randomInsertion(gen, i, j);
    }
    if (ran > 1 / 3 && ran <= 2 / 3)
    {
        gen = randomInsertionOfSubsequence(gen, indexToInsert, i, j);
    }
    else
    {
        gen = randomReversingInsertionOfSubsequence(gen, indexToInsert, i, j);
    }
}

void mutateC1(DoubleGenome &gen, int randomStep = -1)
{
    int n, step, i, j, i2, j2, low, high;
    n = gen.getDGenome().size();
    step = randomStep;

    if (randomStep == 1)
    {
        step = rand() % (n - 2) + 1;
    }
    else if (randomStep < 2 && randomStep >= 0)
    {
        step = 2;
    }
    low = 1;
    high = std::floor(n / 2) - static_cast<int>(std::floor(step / 2)) + 1;
    i = std::experimental::randint(low, high);
    j = i + static_cast<int>(std::floor(step / 2)) - 1;
    low = static_cast<int>(std::floor(n / 2));
    high = n - static_cast<int>(std::floor(step / 2));
    i2 = std::experimental::randint(low, high);
    j2 = i2 + static_cast<int>(std::floor(step / 2)) - 1;

    combine1(gen, i, j, i2, j2);
}

void mutateC2(DoubleGenome &gen, int randomStep)
{
    int n, step, init, end, index;
    double r, r1;
    n = gen.getDGenome().size();
    step = randomStep;

    if (randomStep == -1)
    {
        step = std::experimental::randint(1, n - 2);
    }
    else if (randomStep < 2 && randomStep >= 0)
    {
        step = 2;
    }
    init = std::experimental::randint(1, static_cast<int>(std::floor(n / 2)));
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
        index = std::experimental::randint(0, init);
    }
    else
    {
        index = std::experimental::randint(end + 1, n);
    }

    combine2(gen, index, init, end);
}

void mutateInversion(DoubleGenome &gen, int randomStep = -1)
{
    int n, step, i, j;
    double r, r1;
    n = gen.getDGenome().size();
    step = randomStep;
    if (randomStep == -1)
    {
        step = std::experimental::randint(1, n - 2);
    }
    else if (randomStep < 2)
    {
        step = 2;
    }
    i = std::experimental::randint(1, static_cast<int>(std::floor(n / 2)));
    j = i + step + 1;
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
        addRangeToChromosome(gen.getGenome(), genToAdd.getGenome(), init, end);
        addRangeToChromosome(gen.getDGenome(), genToAdd.getDGenome(), init, end);
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