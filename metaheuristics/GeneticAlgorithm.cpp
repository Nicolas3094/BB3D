#include "GeneticAlgorithm.hpp"

GeneticAlgorithm::GeneticAlgorithm()
{
    this->maxIteration = 0;
    this->selectionProbability = 0.0;
    this->crossProability = 0.0;
    this->mutationProbaility = 0.0;
    this->dMutationProbability = 0.0;
    this->mutationType = MutationType::INVERSE_MUTATION;
    this->rotationType = ROTATION_WAY::ZERO_WAY;
}
GeneticAlgorithm::~GeneticAlgorithm()
{
}

GeneticAlgorithm GeneticAlgorithm::Build()
{
    GeneticAlgorithm g;
    return g;
}

GeneticAlgorithm &GeneticAlgorithm::setSelectionProbability(double selectionProbability)
{
    this->selectionProbability = selectionProbability;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setCrossProbability(double crossProability)
{
    this->crossProability = crossProability;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setMutationProbabiliy(double mutationProbaility)
{
    this->mutationProbaility = mutationProbaility;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setDMutationProbability(double dMutationProbability)
{
    this->dMutationProbability = dMutationProbability;
    return *this;
}

GeneticAlgorithm &GeneticAlgorithm::setMutationType(MutationType mutationType)
{
    this->mutationType = mutationType;
    return *this;
}

GeneticAlgorithm &GeneticAlgorithm::setRotationType(ROTATION_WAY rotationType)
{
    this->rotationType = rotationType;
    this->dataSet.bin.setRotationWay(rotationType);
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setMaxIteration(long maxIteration)
{
    this->maxIteration = maxIteration;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setProblem(DatasetBinBacking dataset)
{
    this->dataSet = dataset;
    return *this;
}
GeneticAlgorithm &GeneticAlgorithm::setNumberOfIndividuals(long numberOfIndividuals)
{
    this->numberOfIndividuals = numberOfIndividuals;
    return *this;
}

void GeneticAlgorithm::mutate(DoubleGenome &genome, int step)
{
    mutationWithType(genome, this->mutationType, this->mutationProbaility, step);
}

Individuo GeneticAlgorithm::crossover(Individuo parent1, Individuo parent2)
{
    int n, init, end;

    n = parent1.getGenome().getGenome().size();
    init = std::experimental::randint(3, n / 2);
    end = std::experimental::randint(init + 1, n);

    return Individuo::Build()
        .setGenome(crossOx(parent1.getGenome(), parent2.getGenome(), init, end))
        .setFitness(0.0);
}

Individuo GeneticAlgorithm::instantiateChild(Individuo parent1, Individuo parent2)
{
    // std::cout << "init crossoved \n";
    Individuo child = crossover(parent1, parent2);
    // std::cout << "enter " << child.getGenome() << "\n";
    // std::cout << "end crossoved \n";
    if (rotationType != ROTATION_WAY::ZERO_WAY)
    {
        flipMutation(child.getGenome(), this->dMutationProbability, this->rotationType);
    }
    // std::cout << "entered fliped" << child.getGenome() << "\n";
    mutate(child.getGenome());
    // std::cout << "enter to evaluated " << child.getGenome() << "\n";
    evaluateFitness(child, this->dataSet.totalItems, this->dataSet.bin);
    return child;
}

void GeneticAlgorithm::nextGeneration(Poblacion &poblation)
{
    // std::cout << "next gen fun\n";
    int pobSize, numberOfDeletedIndividual, id1, id2;
    double adaptiveProbability;
    Poblacion oldPoblation = poblation;

    pobSize = oldPoblation.size();
    numberOfDeletedIndividual = randomInteger(pobSize / 4, pobSize / 2);
    // std::cout << "DONE\n";

    for (int i = numberOfDeletedIndividual; i < pobSize; i++)
    {
        // std::cout << "iter\n";
        id1 = tournament(oldPoblation, this->selectionProbability);
        id2 = tournament(oldPoblation, this->selectionProbability);
        while (id1 == id2)
        {
            id2 = tournament(oldPoblation, selectionProbability);
        }
        if (uniformUnit() <= crossProability)
        {
            adaptiveProbability = 1.0 - (oldPoblation[id1].getFitness() + oldPoblation[id2].getFitness()) / 2;
            this->mutationProbaility = adaptiveProbability;
            // std::cout << "Childer\n";
            Individuo child1 = instantiateChild(oldPoblation[id1], oldPoblation[id2]);
            // std::cout << "Child1" << child1.getGenome() << "\n";
            Individuo child2 = instantiateChild(oldPoblation[id2], oldPoblation[id1]);
            // std::cout << "Child2" << child2.getGenome() << "\n";
            if (child1.getFitness() > child2.getFitness())
            {
                // std::cout << "change" << poblation[i].getGenome() << "\n";
                poblation[i] = child1;
                // std::cout << "change11" << poblation[i].getGenome() << "\n";
                // std::cout << "changed" << child1.getGenome() << "\n";
            }
            else
            {
                poblation[i] = child2;
            }
        }
    }
}

Poblacion GeneticAlgorithm::evolve()
{
    int maxPoblation;
    Poblacion poblation = initialPoblation();

    maxPoblation = poblation.size();
    for (int i = 0; i < maxIteration; i++)
    {
        // std::cout << "next gen\n";
        nextGeneration(poblation);
        // std::cout << "order\n";
        rankPoblation(poblation);
        if (poblation[0].getFitness() == 1 || (poblation[0].getFitness() - poblation[maxPoblation - 1].getFitness()) / (poblation[0].getFitness() * poblation[0].getFitness()) <= 0.01)
        {
            break;
        }
    }
    return poblation;
}

Poblacion GeneticAlgorithm::initialPoblation()
{
    Poblacion heuristicPoblation = buildHeuristicPoblation(
        this->numberOfIndividuals, this->dataSet.bin, this->dataSet.totalItems);
    // std::cout << "Bin" << this->dataSet.bin << "\n";
    // std::cout << "total of items " << this->dataSet.totalItems.size() << "\n";
    evaluatePoblation(heuristicPoblation, this->dataSet.totalItems, this->dataSet.bin);
    rankPoblation(heuristicPoblation);
    return heuristicPoblation;
}