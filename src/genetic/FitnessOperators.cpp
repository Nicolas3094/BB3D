#include "FitnessOperators.hpp"
#include "FitnessCache.hpp"
#include "../util/ExecutionBudget.hpp"

namespace {
void updateSolution(Individuo &ind,const Bin &bin) {
    std::string code="|";
    for(const auto &item:bin.getLoadedItems())
        code+=std::to_string(item.getId())+" "+std::to_string(item.getRotationId())+"|";
    ind.solution=std::move(code);
}
}

void evaluateFitness(Individuo &ind, LoadedBins itemData, Bin bin)
{
    try {
        checkExecutionDeadline();
        std::string cacheKey;
        if(lookupFitness(ind,itemData,bin,cacheKey)) {
            recordExecutionCandidate(ind);checkExecutionDeadline();return;
        }
        DBLF(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), itemData);
        ind.setFitness(static_cast<double>(bin.getLoadedVolume()) / static_cast<double>(bin.getDimensions().getVolumen()));
        updateSolution(ind,bin);recordExecutionCandidate(ind);
        rememberFitness(ind,cacheKey);checkExecutionDeadline();
    } catch(const ExecutionTimeLimit &) {
        if(bin.getDimensions().getVolumen()>0) {
            ind.setFitness(static_cast<double>(bin.getLoadedVolume())/bin.getDimensions().getVolumen());
            updateSolution(ind,bin);recordExecutionCandidate(ind,true);
        }
        throw;
    }
}

void evaluateFitnessDecodificated(Individuo &ind, LoadedBins itemData, Bin &bin)
{
    try {
        checkExecutionDeadline();DBLF(bin, decodificateIndividual(ind, itemData, bin));
        ind.setFitness(static_cast<double>(bin.getLoadedVolume()) / static_cast<double>(bin.getDimensions().getVolumen()));
        updateSolution(ind,bin);recordExecutionCandidate(ind);checkExecutionDeadline();
    } catch(const ExecutionTimeLimit &) {
        if(bin.getDimensions().getVolumen()>0) {
            ind.setFitness(static_cast<double>(bin.getLoadedVolume())/bin.getDimensions().getVolumen());
            updateSolution(ind,bin);recordExecutionCandidate(ind,true);
        }
        throw;
    }
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
        checkExecutionDeadline();
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
        checkExecutionDeadline();
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
