#include "FitnessOperators.hpp"

void evaluateFitness(Individuo &ind, LoadedBins itemData, Bin bin)
{
    DBLF(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), itemData);
    ind.setFitness((float)bin.getLoadedVolume() / (float)bin.getDimensions().getVolumen());
    std::string codeSolution = "|";
    for (auto item : bin.getLoadedItems())
    {
        codeSolution += std::to_string(item.getId()) + " " +
                        std::to_string(item.getRotationId()) + "|";
    }
    ind.solution = codeSolution;
}

void evaluateFitnessDecodificated(Individuo &ind, LoadedBins itemData, Bin &bin)
{
    DBLF(bin, decodificateIndividual(ind, itemData, bin));
    ind.setFitness((float)bin.getLoadedVolume() / (float)bin.getDimensions().getVolumen());
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

Poblacion buildHeuristicPoblation(const int numberPoblation, Bin bin, LoadedBins allItemsBin)
{
    std::vector<Chromosome> heuristicPoblationChromosome =
        buildCompleteHeuristicChromosomes(allItemsBin, numberPoblation);
    Poblacion heuristicPoblation(numberPoblation);
    for (int i = 0; i < numberPoblation; i++)
    {
        heuristicPoblation[i] = Individuo::Build()
                                    .setGenome(
                                        DoubleGenome::Build()
                                            .setGenome(heuristicPoblationChromosome[i])
                                            .setDGenome(
                                                generateRandomRepeatedAlalleleChromosome(
                                                    /* NumberCount= */ allItemsBin.size(),
                                                    /* minimum= */ 1,
                                                    /* maximum= */ getIdFromRotationWay(bin.getRotationWay()))));
    }

    return heuristicPoblation;
}

void evaluatePoblation(Poblacion &poblacion, LoadedBins allItemsBin, Bin bin)
{
    for (int i = 0; i < poblacion.size(); i++)
    {
        evaluateFitness(poblacion[i], allItemsBin, bin);
    }
}

Poblacion initialHeuristicPoblation(
    int numberOfIndividuals, Bin bin, LoadedBins orderItemsById, ROTATION_WAY rotationType)
{
    Poblacion heuristicPoblation = buildHeuristicPoblation(numberOfIndividuals,
                                                           bin.setRotationWay(rotationType),
                                                           orderItemsById);
    evaluatePoblation(heuristicPoblation, orderItemsById, bin.setRotationWay(rotationType));
    rankPoblation(heuristicPoblation);
    return heuristicPoblation;
}