#include "main.hpp"

#include "src/FitnessOperators.hpp"
#include "metaheuristics/GeneticsOperators.hpp"
#include "metaheuristics/PermutationOperators.hpp"
#include "metaheuristics/GeneticAlgorithm.hpp"
#include "readData/ReadData.hpp"
#include <chrono>
int main()
{
  std::string algorithmName, rotationType, mutationType, algorithm;
  std::cout << "Select algorithm:\n[1] Genetic Algorithm\n[2] Artificial Bee Colony Algorithm \n[3] Firefly Algorithm\n";
  std::cin >> algorithm;
  std::cout << "\nSelect mutationType:\n[1] Inverse Mutation\n[2] Group C1 Mutation\n[3] Group C2 Mutation\n";
  std::cin >> mutationType;
  std::cout << "\nSelect problem:\n[1] P1A2\n[2] P2A2\n[3] P3A2\n[4] P4A2\n[5] P5A2\n";
  std::cin >> algorithmName;
  std::cout << "\nSelect rotation type: [1]ZERO-ROTATION [2]2-ROTATION [3]6-ROTATION\n";
  std::cin >> rotationType;
  if (algorithm != "1" && algorithm != "2" && algorithm != "3")
  {
    std::cout << "\nSelect valid algorithm.\n";
    return main();
  }
  if (mutationType != "1" && mutationType != "2" && mutationType != "3")
  {
    std::cout << "\nSelect valid mutationType.\n";
    return main();
  }
  if (algorithmName != "2" && algorithmName != "3" && algorithmName != "4" && algorithmName != "5" && algorithmName != "1")
  {
    std::cout << "\nSelect valid algorithmName.\n";
    return main();
  }
  if (rotationType != "1" && rotationType != "2" && rotationType != "3")
  {
    std::cout << "\nSelect valid rotationType.\n";
    return main();
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

  iterGeneticAll(algorithmName, mutationType, algorithm, std::stoi(rotationType));
  // system("pause");
}

void iterGeneticAll(std::string algorithmName, std::string mutationType, std::string algorithm, int rotationWay)
{

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = "C:\\Users\\nicoo\\OneDrive\\Documentos\\Progamming\\3DBPP_CPP\\Instance\\" + algorithmName + ".csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  MutationType mutationTyped;
  if (mutationType == "1")
  {
    mutationTyped = MutationType::INVERSE_MUTATION;
  }
  else if (mutationType == "2")
  {
    mutationTyped = MutationType::C1;
  }
  else
  {
    mutationTyped = MutationType::C2;
  }
  std::vector<double> responses(DATASSET.size());
  std::vector<long int> durationResponses(DATASSET.size());
  std::cout << "\nStart evolving.\n";
  for (int i = 0; i < DATASSET.size(); i++)
  {
    auto start = std::chrono::high_resolution_clock::now();
    GeneticAlgorithm geneticAlgorithm = GeneticAlgorithm::Build()
                                            .setProblem(DATASSET[i])
                                            .setMaxIteration(1000)
                                            .setNumberOfIndividuals(50)
                                            .setCrossProbability(0.75)
                                            .setDMutationProbability(0.05)
                                            .setSelectionProbability(0.85)
                                            .setRotationType(getRotationWayFromId(rotationWay))
                                            .setMutationType(mutationTyped);
    Poblacion bestPob = geneticAlgorithm.evolve();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    responses[i] = bestPob[0].getFitness();
    durationResponses[i] = duration.count();
    std::cout << (i + 1) << ": " << responses[i] << "\n";
    printResults(responses, durationResponses, algorithmName, mutationType, algorithm, rotationWay);
  }
}

void genetic()
{

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = "C:\\Users\\nicoo\\OneDrive\\Documentos\\Progamming\\3DBPP_CPP\\Instance\\P1A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  auto start = std::chrono::high_resolution_clock::now();
  GeneticAlgorithm algorithm = GeneticAlgorithm::Build()
                                   .setProblem(DATASSET[0])
                                   .setMaxIteration(1000)
                                   .setNumberOfIndividuals(50)
                                   .setCrossProbability(0.75)
                                   .setDMutationProbability(0.05)
                                   .setSelectionProbability(0.85);
  Poblacion bestPob = algorithm.evolve();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Initial best fit: " << bestPob[0].getFitness() << "\n";
  std::cout << "Initial worst fit: " << bestPob[bestPob.size() - 1].getFitness() << "\n";
  print("Duration: " << duration.count() << "ms");
}

void evaluatePoblationTime()
{
  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = "C:\\Users\\nicoo\\OneDrive\\Documentos\\Progamming\\3DBPP_CPP\\Instance\\P5A2.csv";
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
  const string dataPATH = "C:\\Users\\nicoo\\OneDrive\\Documentos\\Progamming\\3DBPP_CPP\\Results\\" + algorithmName + "\\" + mutationType + "\\" + alrgorithm + "\\" + rotationType + "\\" + alrgorithm + ".csv";
  const string timePATH = "C:\\Users\\nicoo\\OneDrive\\Documentos\\Progamming\\3DBPP_CPP\\Results\\" + algorithmName + "\\" + mutationType + "\\" + alrgorithm + "\\" + rotationType + "\\t.csv";
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