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
  std::cout << "Select algorithm: GA ABC FFA\n";
  std::cin >> algorithm;
  std::cout << "\nSelect mutationType: InverseMutation C1Mutation C2Mutation\n";
  std::cin >> mutationType;
  std::cout << "\nSelect problem: P1A2 P2A2 P3A2 P4A2 P5A2\n";
  std::cin >> algorithmName;
  std::cout << "\nSelect rotation type: 0 2 6\n";
  std::cin >> rotationType;
  if (algorithm != "GA" && algorithm != "ABC" && algorithm != "FFA")
  {
    std::cout << "\nSelect valid algorithm.\n";
    return main();
  }
  if (mutationType != "InverseMutation" && mutationType != "C1Mutation" && mutationType != "C2Mutation")
  {
    std::cout << "\nSelect valid mutationType.\n";
    return main();
  }
  if (algorithmName != "P1A2" && algorithmName != "P2A2" && algorithmName != "P3A2" && algorithmName != "P4A2" && algorithmName != "P5A2")
  {
    std::cout << "\nSelect valid algorithmName.\n";
    return main();
  }
  if (rotationType != "0" && rotationType != "2" && rotationType != "6")
  {
    std::cout << "\nSelect valid rotationType.\n";
    return main();
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
                                            .setRotationType(getRotationWayFromId(rotationWay));
    Poblacion bestPob = geneticAlgorithm.evolve();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    responses[i] = bestPob[0].getFitness();
    durationResponses[i] = duration.count();
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

void printResults(std::vector<double> result, std::vector<long int> delays, std::string algorithmName, std::string mutationType, std::string alrgorithm, int rotationWay)
{
  std::string rotationType;
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