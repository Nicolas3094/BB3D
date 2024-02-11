#include "FitnessOperators.hpp"

void evaluateFitness(Individuo &ind, LoadedBins itemData, Bin &bin)
{
    DBLF(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), itemData);
    ind.setFitness(bin.getLoadedVolume() / bin.getDimensions().getVolumen());
}

DoubleGenome codificateItemsToPacked(LoadedBins &allBinsToLoad)
{
    DoubleGenome genome;

    for (int i = 0; i < allBinsToLoad.size(); i++)
    {
        genome.getGenome().push_back(allBinsToLoad[i].getId());
    }

    return genome;
}

Poblacion buildHeuristicPoblation(const uint numberPoblation, Bin &bin, LoadedBins allItemsBin, ROTATION_WAY rotationWay)
{
    std::vector<Chromosome> heuristicPoblationChromosome = buildCompleteHeuristicChromosomes(allItemsBin, numberPoblation);
    Poblacion heuristicPoblation(numberPoblation);
    bin.setRotationWay(rotationWay);
    for (uint i = 0; i < numberPoblation; i++)
    {
        heuristicPoblation[i] = Individuo::Build()
                                    .setGenome(
                                        DoubleGenome::Build()
                                            .setGenome(heuristicPoblationChromosome[i])
                                            .setDGenome(generateRandomRepeatedAlalleleChromosome(
                                                /* NumberCount= */ allItemsBin.size(),
                                                /* minimum= */ 0,
                                                /* maximum= */ getIdFromRotationWay(rotationWay))));
        evaluateFitness(heuristicPoblation[i], allItemsBin, bin);
    }

    return heuristicPoblation;
}

void evaluatePoblation(Poblacion &poblacion, LoadedBins allItemsBin, Bin bin, ROTATION_WAY rotatiomWay)
{
    bin.setRotationWay(rotatiomWay);

    for (uint i = 0; i < poblacion.size(); i++)
    {
        evaluateFitness(heuristicPoblation[i], allItemsBin, bin);
    }
}