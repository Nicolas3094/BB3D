#include "main.hpp"

#include <chrono>
#include "genetic/FitnessOperators.hpp"
#include "genetic/GeneticsOperators.hpp"
#include "metaheuristics/GeneticAlgorithm.hpp"
#include "metaheuristics/ArtificialBeeColonyAlgorithm.hpp"
#include "metaheuristics/FireflyAlgorithm.hpp"
#include "structures/PriorityPointQueue.hpp"
#include "structures/ColaPrioridad.hpp"
#include "readData/Env.hpp"

std::string LOCAL_PATH = getEnv("LOCAL_PATH");

int main()
{

  try
  {
    // testRotationsGA("replacement");
    // testRotationsGA("added");
    iterGeneticAll();
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  // testInverseMutation();
  //
  std::cin.get();
  // system("pause");
}

void testInverseMutation()
{
  DoubleGenome zerosChromosome;
  flipMutation(zerosChromosome, 0.01, ROTATION_WAY::SIX_WAY);
  std::cout << zerosChromosome << "\n";
}

void iterGeneticAll()
{
  std::string rotationType, algorithm;

  std::cout << "Select algorithm:\n[1] Genetic Algorithm\n[2] Artificial Bee Colony Algorithm \n[3] Firefly Algorithm\n\n";
  std::cin >> algorithm;
  std::cout << "Select rotation type:\n[1] ZERO-ROTATION\n[2] 2-ROTATION\n[3] 6-ROTATION\n";
  std::cin >> rotationType;
  if (algorithm != "1" && algorithm != "2" && algorithm != "3")
  {
    std::cout << "\nSelect valid algorithm.\n";
    return iterGeneticAll();
  }
  if (rotationType != "1" && rotationType != "2" && rotationType != "3")
  {
    std::cout << "\nSelect valid rotationType.\n";
    return iterGeneticAll();
  }

  algorithm = getAlgorithmNameFromNumber(algorithm);

  if (rotationType == "1")
  {
    rotationType = "0";
  }
  else if (rotationType == "2")
  {
    rotationType = "2";
  }
  else
  {
    rotationType = "6";
  }

  string generalPath, fitnessDataPath, timePATH;
  std::vector<std::string> algorithmsProblems{"P1A2", "P2A2", "P3A2", "P4A2", "P5A2"};
  std::vector<std::string> mutationNumber{"1", "2", "3"};
  ROTATION_WAY rotation = getRotationWayFromId(std::stoi(rotationType));
  bool isWithReplacement = false;

  for (auto algorithmName : algorithmsProblems)
  {
    std::cout << "\nAlgorithm: " << algorithmName << "\n";
    const string dataPATH = LOCAL_PATH + "/Instance/" + algorithmName + ".csv";
    MutationType mutationTyped;
    vector<DatasetBinBacking> DATASSET = readData(dataPATH);

    for (auto mutationType : mutationNumber)
    {
      std::cout << "\nInit " << mutationTyped << "\n";

      mutationTyped = getMutationTypeFromUser(mutationType);

      generalPath = LOCAL_PATH + "/Results/" + algorithmName + "/" + getMutationName(mutationTyped) +
                    "/" + algorithm + "/" + getNameFromRotationWay(rotation) + "/";
      if (isWithReplacement)
      {
        fitnessDataPath = generalPath + algorithm + ".csv";
        timePATH = generalPath + "t.csv";
      }
      else
      {
        fitnessDataPath = generalPath + algorithm + "2.csv";
        timePATH = generalPath + "t2.csv";
      }

      vector<double> responses = getFloatNumberListFile(fitnessDataPath);
      vector<long int> durationResponses = getIntegerNumberListFile(timePATH);

      for (int i = 0; i < DATASSET.size(); i++)
      {
        if (durationResponses[i] != 0 && responses[i] != 0)
        {
          continue;
        }
        std::cout << (i + 1) << "|";
        Poblacion bestPob;
        EvolutiveAlgorithm *evolutive;

        evolutive = getEvolutiveAlgorithm(algorithm, rotation, mutationTyped, DATASSET[i]);

        auto start = std::chrono::high_resolution_clock::now();
        if (isWithReplacement)
        {
          bestPob = evolutive->evolveWithReplacement();
        }
        else
        {
          bestPob = evolutive->evolveWithAdded();
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        responses[i] = bestPob[0].getFitness();
        durationResponses[i] = duration.count();

        printResults(responses, durationResponses, fitnessDataPath, timePATH);

        delete evolutive;
      }
    }
  }
}
void testDblfOneInstance()
{

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = LOCAL_PATH + "/Instance/P1A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  DatasetBinBacking data = DATASSET[0];
  Chromosome chromosome{8, 9, 6, 12, 4, 15, 10, 11, 5, 16, 1, 14, 3, 13, 7, 2};
  Chromosome chromosomeRot{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  DoubleGenome genome = DoubleGenome::Build().setGenome(chromosome).setDGenome(chromosomeRot);
  Individuo ind = Individuo::Build().setGenome(genome);
  Individuo indQ = Individuo::Build().setGenome(genome);
  Bin bin = DATASSET[0].bin;
  Bin binQ = DATASSET[0].bin;

  DBLF(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), data.totalItems);
  ind.setFitness((double)bin.getLoadedVolume() / (double)bin.getDimensions().getVolumen());

  DBLFQueue(binQ, indQ.getGenome().getGenome(), indQ.getGenome().getDGenome(), data.totalItems);
  indQ.setFitness((double)binQ.getLoadedVolume() / (double)binQ.getDimensions().getVolumen());

  std::cout << ind.getFitness() << " | " << indQ.getFitness() << "\n";
}

void testRotationsGA(std::string name)
{
  print("GA with " + name + " evolving. ");

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = LOCAL_PATH + "/Instance/P1A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }

  int maxIteration, population;
  double fitnessGAReplace, fitnessGAAdd, timeGAReplace, timeGAAdd;

  Poblacion bestPobRot;

  ROTATION_WAY rotation = ROTATION_WAY::SIX_WAY;
  MutationType mutationType = MutationType::C1;
  DatasetBinBacking data = DATASSET[0];

  maxIteration = 1000;
  population = 50;

  fitnessGAReplace = 0.0;
  fitnessGAAdd = 0.0;
  timeGAReplace = 0.0;
  timeGAAdd = 0.0;

  GeneticAlgorithm geneticAlgorithm = GeneticAlgorithm::Build()
                                          .setMaxIteration(maxIteration)
                                          .setNumberOfIndividuals(population)
                                          .setRotationType(rotation)
                                          .setMutationType(mutationType)
                                          .setProblem(data)
                                          .setCrossProbability(0.75)
                                          .setSelectionProbability(0.85);
  for (int i = 0; i < 1000; i++)
  {
    std::cout << i << " ";

    auto start = std::chrono::high_resolution_clock::now();
    if (name == "replacement")
    {
      bestPobRot = geneticAlgorithm.evolveWithAdded();
    }
    else
    {
      bestPobRot = geneticAlgorithm.evolveWithAdded();
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    fitnessGAReplace += bestPobRot[0].getFitness() / 1000;
    timeGAReplace += duration.count() / 1000;
  }

  std::cout << "\nfi: " << fitnessGAReplace << "\n";
  std::cout << "t: " << (timeGAReplace / 1000) << " s\n";
}

void testDblf()
{

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = LOCAL_PATH + "/Instance/P5A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  DatasetBinBacking data = DATASSET[0];
  Poblacion heuristicPoblation = buildHeuristicPoblation(1000, data.bin, data.totalItems);
  Poblacion secondPob = heuristicPoblation;
  auto start = std::chrono::high_resolution_clock::now();
  for (Individuo &ind : heuristicPoblation)
  {
    Bin bin = data.bin;
    DBLF(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), data.totalItems);
    ind.setFitness((double)bin.getLoadedVolume() / (double)bin.getDimensions().getVolumen());
  }
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  auto start2 = std::chrono::high_resolution_clock::now();
  for (Individuo &ind : secondPob)
  {
    Bin bin = data.bin;
    DBLFQueue(bin, ind.getGenome().getGenome(), ind.getGenome().getDGenome(), data.totalItems);
    ind.setFitness((double)bin.getLoadedVolume() / (double)bin.getDimensions().getVolumen());
  }
  auto stop2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2);
  for (int i = 0; i < secondPob.size(); i++)
  {
    if (heuristicPoblation[i].getFitness() != secondPob[i].getFitness())
    {
      std::cout << "not equal\n";
      std::cout << heuristicPoblation[i].getFitness() << "\n"
                << secondPob[i].getFitness() << "\n";
      std::cout << heuristicPoblation[i].getGenome() << "\n"
                << secondPob[i].getGenome() << "\n\n";
    }
  }
  std::cout << "DBLF: " << duration.count() << " | DBLFQueue: " << duration2.count() << "\n";
}

void evaluatePoblationTime()
{
  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = LOCAL_PATH + "/Instance/P5A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  int numberOfIndividuals = 100;
  DatasetBinBacking dataset = DATASSET[10];
  dataset.bin.setRotationWay(ROTATION_WAY::ZERO_WAY);
  // Get items ID list.
  LoadedBins totalItems = dataset.totalItems;
  // Builds an Heuristic poblation.
  Poblacion heuristicPoblation = buildHeuristicPoblation(
      /* numberPoblation= */ numberOfIndividuals,
      /* bin= */ dataset.bin,
      /* allItemsBin= */ totalItems);
  auto start = std::chrono::high_resolution_clock::now();
  evaluatePoblation(heuristicPoblation, totalItems, dataset.bin);
  //  DBLF(dataset.bin, individual.getGenome().getGenome(), individual.getGenome().getDGenome(), totalItems);
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  print("Duration: " << duration.count() << "ms");
  print("Item places: " << dataset.bin.getLoadedItems());
  print("FI: " << heuristicPoblation[0].getFitness());
  print("Genome: " << heuristicPoblation[0].getGenome());
}

void genetic()
{

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = LOCAL_PATH + "/Instance/P1A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  auto start = std::chrono::high_resolution_clock::now();
  std::cout << "Init\n";
  GeneticAlgorithm algorithm = GeneticAlgorithm::Build()
                                   .setProblem(DATASSET[0])
                                   .setMaxIteration(1000)
                                   .setNumberOfIndividuals(50)
                                   .setCrossProbability(0.75)
                                   .setSelectionProbability(0.85)
                                   .setDMutationProbability(0.01)
                                   .setMutationType(MutationType::C2);
  Poblacion bestPob = algorithm.evolveWithAdded();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Initial best fit: " << bestPob[0].getFitness() << "\n";
  std::cout << "Initial worst fit: " << bestPob[bestPob.size() - 1].getFitness() << "\n";
  print("Duration: " << duration.count() << "ms");
}

void printResults(std::vector<double> result, std::vector<long int> delays, const string dataPATH, const string timePATH)
{
  ofstream MyFile(dataPATH);

  for (auto res : result)
  {
    MyFile << res << "\n";
  }

  MyFile.close();

  ofstream MyFile2(timePATH);

  for (auto res : delays)
  {
    MyFile2 << res << "\n";
  }

  MyFile2.close();
}

std::string getAlgorithmNameFromNumber(std::string number)
{
  if (number == "1")
  {
    return "GA";
  }
  else if (number == "2")
  {
    return "ABC";
  }
  else
  {
    return "FFA";
  }
}

/** Gets evolutive algorithm based on the algorithm's name.*/
EvolutiveAlgorithm *getEvolutiveAlgorithm(std::string algorithm, ROTATION_WAY rotation, MutationType mutation, DatasetBinBacking data)
{
  if (algorithm == "GA")
  {
    GeneticAlgorithm *genetic = new GeneticAlgorithm();
    genetic->setMaxIteration(1000)
        .setNumberOfIndividuals(50)
        .setCrossProbability(0.75)
        .setSelectionProbability(0.85)
        .setDMutationProbability(0.05)
        .setRotationType(rotation)
        .setMutationType(mutation)
        .setProblem(data);
    return genetic;
  }
  else if (algorithm == "ABC")
  {
    ArtificialBeeColonyAlgorithm *abc = new ArtificialBeeColonyAlgorithm();
    abc->setMaxIteration(1000)
        .setNumberOfIndividuals(20)
        .setNumberOfSites(11)
        .setNumberOfEliteSites(8)
        .setNumberOfEliteBees(4)
        .setNumberOfNonEliteBees(3)
        .setRotationType(rotation)
        .setMutationType(mutation)
        .setProblem(data);
    return abc;
  }
  else
  {
    FireflyAlgorithm *ffa = new FireflyAlgorithm();
    ffa->setMaxIteration(1000)
        .setNumberOfIndividuals(25)
        .setupIndex(2)
        .setRotationType(rotation)
        .setMutationType(mutation)
        .setProblem(data);
    return ffa;
  }
}

/** Gets the mutation type by the user's input.*/
MutationType getMutationTypeFromUser(std::string mutationType)
{
  if (mutationType == "1")
  {
    return MutationType::INVERSE_MUTATION;
  }
  else if (mutationType == "2")
  {
    return MutationType::C1;
  }
  else
  {
    return MutationType::C2;
  }
}