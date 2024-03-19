#include "main.hpp"

#include "src/FitnessOperators.hpp"
#include "metaheuristics/GeneticsOperators.hpp"
#include "metaheuristics/PermutationOperators.hpp"
#include "metaheuristics/GeneticAlgorithm.hpp"
#include "metaheuristics/ArtificialBeeColonyAlgorithm.hpp"
#include "metaheuristics/FireflyAlgorithm.hpp"
#include "readData/ReadData.hpp"
#include <chrono>
#include <filesystem>
#include "structures/PriorityPointQueue.hpp"
#include "structures/ColaPrioridad.hpp"

std::string LOCAL_PATH = std::__fs::filesystem::current_path();

int main()
{
  // testDblf();
  //  testDblfOneInstance();
  iterGeneticAll();

  // std::cin.get();
  //  system("pause");
}

void iterGeneticAll()
{
  std::string algorithmName, rotationType, mutationType, algorithm;
  std::cout << "Select algorithm:\n[1] Genetic Algorithm\n[2] Artificial Bee Colony Algorithm \n[3] Firefly Algorithm\n";
  std::cin >> algorithm;
  std::cout << "Select problem:\n[1] P1A2\n[2] P2A2\n[3] P3A2\n[4] P4A2\n[5] P5A2\n";
  std::cin >> algorithmName;
  // std::cout << "Select mutationType:\n[1] Inverse Mutation\n[2] Group C1 Mutation\n[3] Group C2 Mutation\n";
  // std::cin >> mutationType;
  std::cout << "Select rotation type:\n[1] ZERO-ROTATION\n[2] 2-ROTATION\n[3] 6-ROTATION\n";
  std::cin >> rotationType;
  if (algorithm != "1" && algorithm != "2" && algorithm != "3")
  {
    std::cout << "\nSelect valid algorithm.\n";
    return iterGeneticAll();
  }
  /*if (mutationType != "1" && mutationType != "2" && mutationType != "3")
  {
    std::cout << "\nSelect valid mutationType.\n";
    return iterGeneticAll();
  }*/
  if (algorithmName != "2" && algorithmName != "3" && algorithmName != "4" && algorithmName != "5" && algorithmName != "1")
  {
    ßßß
    std::cout << "\nSelect valid algorithmName.\n";
    return iterGeneticAll();
  }
  if (rotationType != "1" && rotationType != "2" && rotationType != "3")
  {
    std::cout << "\nSelect valid rotationType.\n";
    return iterGeneticAll();
  }

  if (algorithm == "1")
  {
    algorithm = "GA";
  }
  else if (algorithm == "2")
  {
    algorithm = "ABC";
  }
  else
  {
    algorithm = "FFA";
  }

  if (algorithmName == "1")
  {
    algorithmName = "P1A2";
  }
  else if (algorithmName == "2")
  {
    algorithmName = "P2A2";
  }
  else if (algorithmName == "3")
  {
    algorithmName = "P3A2";
  }
  else if (algorithmName == "4")
  {
    algorithmName = "P4A2";
  }
  else
  {
    algorithmName = "P5A2";
  }

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

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = LOCAL_PATH + "/Instance/" + algorithmName + ".csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  MutationType mutationTyped;
  std::vector<double> responses(DATASSET.size());
  std::vector<long int> durationResponses(DATASSET.size());
  for (int i = 1; i <= 3; i++)
  {
    if (i == 1)
    {
      mutationTyped = MutationType::INVERSE_MUTATION;
      mutationType = "1";
    }
    else if (i == 2)
    {
      mutationTyped = MutationType::C1;
      mutationType = "2";
    }
    else
    {
      mutationTyped = MutationType::C2;
      mutationType = "3";
    }
    std::cout << "Init " << mutationTyped << "\n";
    std::cout << "\nStart evolving.\n";
    for (int i = 0; i < DATASSET.size(); i++)
    {
      std::cout << (i + 1) << "-";
      auto start = std::chrono::high_resolution_clock::now();
      Poblacion bestPob;
      if (algorithm == "GA")
      {
        GeneticAlgorithm geneticAlgorithm = GeneticAlgorithm::Build()
                                                .setMaxIteration(1000)
                                                .setNumberOfIndividuals(50)
                                                .setCrossProbability(0.75)
                                                .setDMutationProbability(0.05)
                                                .setSelectionProbability(0.85)
                                                .setRotationType(getRotationWayFromId(std::stoi(rotationType)))
                                                .setMutationType(mutationTyped)
                                                .setProblem(DATASSET[i]);
        bestPob = geneticAlgorithm.evolve();
      }
      else if (algorithm == "ABC")
      {
        ArtificialBeeColonyAlgorithm abc = ArtificialBeeColonyAlgorithm::Build()
                                               .setMaxIteration(1000)
                                               .setNumberOfIndividuals(20)
                                               .setNumberOfSites(10)
                                               .setNumberOfEliteSites(6)
                                               .setNumberOfEliteBees(3)
                                               .setNumberOfNonEliteBees(2)
                                               .setMutationProbabiliy(1.0)
                                               .setDMutationProbability(0.05)
                                               .setRotationType(getRotationWayFromId(std::stoi(rotationType)))
                                               .setMutationType(mutationTyped)
                                               .setProblem(DATASSET[i]);
        bestPob = abc.search();
      }
      else
      {
        FireflyAlgorithm firefly = FireflyAlgorithm::Build()
                                       .setMaxIteration(1000)
                                       .setNumberOfIndividuals(25)
                                       .setupIndex(4)
                                       .setMutationProbabiliy(1.0)
                                       .setDMutationProbability(0.05)
                                       .setRotationType(getRotationWayFromId(std::stoi(rotationType)))
                                       .setMutationType(mutationTyped)
                                       .setProblem(DATASSET[i]);
        bestPob = firefly.search();
      }

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      responses[i] = bestPob[0].getFitness();
      durationResponses[i] = duration.count();
      printResults(responses, durationResponses, algorithmName, mutationType, algorithm, std::stoi(rotationType));
    }
    std::cout << "\n";
  }

  std::string response;
  std::cout << "Repeat experiment: Y/N\n";
  std::cin >> response;
  if (response == "y" || response == "Y")
  {
    iterGeneticAll();
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
                                   .setDMutationProbability(0.05)
                                   .setSelectionProbability(0.85)
                                   .setMutationType(MutationType::C2);
  Poblacion bestPob = algorithm.evolve();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Initial best fit: " << bestPob[0].getFitness() << "\n";
  std::cout << "Initial worst fit: " << bestPob[bestPob.size() - 1].getFitness() << "\n";
  print("Duration: " << duration.count() << "ms");
}

void printResults(std::vector<double> result, std::vector<long int> delays, std::string algorithmName, std::string mutationTypeNumber, std::string alrgorithm, int rotationWay)
{
  std::string rotationType, mutationType;
  if (rotationWay == 0)
  {
    rotationType = "ZERO";
  }
  else if (rotationWay == 2)
  {
    rotationType = "TWO";
  }
  else
  {
    rotationType = "SIX";
  }
  if (mutationTypeNumber == "1")
  {
    mutationType = "InverseMutation";
  }
  else if (mutationTypeNumber == "2")
  {
    mutationType = "C1Mutation";
  }
  else
  {
    mutationType = "C2Mutation";
  }
  const string dataPATH = LOCAL_PATH + "/Results/" + algorithmName + "/" + mutationType + "/" + alrgorithm + "/" + rotationType + "/" + alrgorithm + ".csv";
  const string timePATH = LOCAL_PATH + "/Results/" + algorithmName + "/" + mutationType + "/" + alrgorithm + "/" + rotationType + "/t.csv";
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