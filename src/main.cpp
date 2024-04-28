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
    iterGeneticAll();
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
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

  string rotationType, algorithmOption, problem, mut, name, isReplace, generalPath, fitnessDataPath, timePATH;
  bool isWithReplacement;

  std::cout << "Select algorithm: [1] Genetic Algorithm [2] Artificial Bee Colony Algorithm [3] Firefly Algorithm\n\n";
  std::cin >> algorithmOption;
  // std::cout << "Select rotation type:\n[1] ZERO-ROTATION\n[2] 2-ROTATION\n[3] 6-ROTATION\n";
  // std::cin >> rotationType;
  // std::cout << "Select problem:\n[1] P4A2 \n[2] P5A2\n\n";
  // std::cin >> problem;
  std::cout << "Select mutation: [1] Inverse Mutation [2] Group 1 [3] Group 2\n\n";
  std::cin >> mut;

  std::cout << "With Replace: [1] Yes [2] No\n\n";
  std::cin >> isReplace;
  rotationType = "1";
  handleUserInputs(
      /* algorithmOption= */ algorithmOption,
      /* rotationType= */ rotationType,
      /* algorithnName= */ name,
      /* replace= */ isReplace,
      /* isWithReplacement= */ isWithReplacement);

  vector<string> ALGORITHMS{getAlgorithmNameFromNumber("1"), getAlgorithmNameFromNumber("2"), getAlgorithmNameFromNumber("3")};
  vector<string> PROBLEMS{"P1A2", "P2A2", "P3A2", "P4A2", "P5A2"};
  vector<MutationType> MUTATIONS{
      getMutationTypeFromUser("1"), getMutationTypeFromUser("2"), getMutationTypeFromUser("3")};
  vector<ROTATION_WAY> ROTATIONS{ROTATION_WAY::ZERO_WAY, ROTATION_WAY::TWO_WAY, ROTATION_WAY::SIX_WAY};

  for (string algorithm : ALGORITHMS)
  {
    cout << "\nMetaheuristics: " << algorithm << "\n";
    for (ROTATION_WAY rotation : ROTATIONS)
    {
      cout << "\nRotation: " << rotation << "\n";
      for (auto problemName : PROBLEMS)
      {
        vector<DatasetBinBacking> DATASSET = readData(LOCAL_PATH + "/Instance/" + problemName + ".csv");
        for (MutationType mutationTyped : MUTATIONS)
        {
          cout << "\n\nAlgorithm: " << problemName << " - " << mutationTyped << "\n";

          handlePaths(
              isWithReplacement,
              /* algorithm= */ algorithm,
              /*problemName= */ problemName,
              mutationTyped,
              rotation,
              /* fitnessDataPath= */ fitnessDataPath,
              /* timePATH= */ timePATH);

          vector<double> responses = getFloatNumberListFile(fitnessDataPath);
          vector<long int> durationResponses = getIntegerNumberListFile(timePATH);

          for (int i = 0; i < DATASSET.size(); i++)
          {
            if (durationResponses[i] != 0 && responses[i] != 0)
            {
              continue;
            }

            cout << (i + 1) << "|";

            Poblacion bestPob;

            // Set -1 to inform it is being attended
            responses[i] = -1;
            durationResponses[i] = -1;

            // Print it
            printResults(responses, durationResponses, fitnessDataPath, timePATH);

            auto start = chrono::high_resolution_clock::now();
            bestPob = getEvolutiveAlgorithm(algorithm, rotation, mutationTyped, DATASSET[i], isWithReplacement);
            auto stop = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

            // update data
            responses = getFloatNumberListFile(fitnessDataPath);
            durationResponses = getIntegerNumberListFile(timePATH);

            responses[i] = bestPob[0].getFitness();
            durationResponses[i] = duration.count();

            printResults(responses, durationResponses, fitnessDataPath, timePATH);
          }
        }
      }
    }
  }
}

void handleUserInputs(
    string &algorithmOption, string &rotationType, string algorithnName, string replace, bool &isWithReplacement)
{
  isWithReplacement = replace == "1";

  if (algorithmOption != "1" && algorithmOption != "2" && algorithmOption != "3")
  {
    cout << "\nSelect valid algorithm.\n";
    return iterGeneticAll();
  }
  if (rotationType != "1" && rotationType != "2" && rotationType != "3")
  {
    cout << "\nSelect valid rotationType.\n";
    return iterGeneticAll();
  }
  if (algorithmOption == "1")
  {
    algorithnName = "Genetic Algoritm";
  }
  else if (algorithmOption == "2")
  {
    algorithnName = "Aritifical Bee Colony Algorithm";
  }
  else
  {
    algorithnName = "Firefly Algorithm";
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
}

void handlePaths(
    bool isWithReplacement,
    string algorithm,
    string problemName,
    MutationType mutationTyped,
    ROTATION_WAY rotation,
    string &fitnessDataPath,
    string &timePATH)
{
  string generalPath;

  generalPath = LOCAL_PATH + "/Results/" + problemName + "/" + getMutationName(mutationTyped) +
                "/" + algorithm + "/" + getNameFromRotationWay(rotation) + "/";

  if (isWithReplacement)
  {
    if (algorithm == "GA")
    {
      fitnessDataPath = generalPath + algorithm + "Bin.csv";
      timePATH = generalPath + "tBin.csv";
    }
    else
    {
      fitnessDataPath = generalPath + algorithm + ".csv";
      timePATH = generalPath + "t.csv";
    }
  }
  else
  {

    if (algorithm == "GA")
    {
      fitnessDataPath = generalPath + algorithm + "2Bin.csv";
      timePATH = generalPath + "t2Bin.csv";
    }
    else
    {
      fitnessDataPath = generalPath + algorithm + "2.csv";
      timePATH = generalPath + "t2.csv";
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

  catch (const exception &ex)
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

  cout << ind.getFitness() << " | " << indQ.getFitness() << "\n";
}

void testRotationsGA(string name)
{
  print("GA with " + name + " evolving. ");

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = LOCAL_PATH + "/Instance/P1A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }

  catch (const exception &ex)
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
                                          .setIsWithReplacement(true)
                                          .setMaxIteration(maxIteration)
                                          .setNumberOfIndividuals(population)
                                          .setRotationType(rotation)
                                          .setMutationType(mutationType)
                                          .setProblem(data)
                                          .setCrossProbability(0.75)
                                          .setSelectionProbability(0.85);
  for (int i = 0; i < 1000; i++)
  {
    cout << i << " ";

    auto start = chrono::high_resolution_clock::now();
    bestPobRot = geneticAlgorithm.evolve();
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

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
                                   .setIsWithReplacement(true)
                                   .setProblem(DATASSET[0])
                                   .setMaxIteration(1000)
                                   .setNumberOfIndividuals(50)
                                   .setCrossProbability(0.75)
                                   .setSelectionProbability(0.85)
                                   .setDMutationProbability(0.01)
                                   .setMutationType(MutationType::C2);
  Poblacion bestPob = algorithm.evolve();
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
Poblacion getEvolutiveAlgorithm(std::string algorithm, ROTATION_WAY rotation, MutationType mutation, DatasetBinBacking data, bool isWithReplacement)
{
  if (algorithm == "GA")
  {
    GeneticAlgorithm genetic;
    genetic.setMaxIteration(1000)
        .setNumberOfIndividuals(50)
        .setCrossProbability(0.75)
        .setSelectionProbability(0.85)
        .setDMutationProbability(0.05)
        .setIsWithReplacement(isWithReplacement)
        .setRotationType(rotation)
        .setMutationType(mutation)
        .setProblem(data);

    return genetic.evolve();
  }
  else if (algorithm == "ABC")
  {
    ArtificialBeeColonyAlgorithm abc;
    abc.setMaxIteration(1000)
        .setNumberOfIndividuals(20)
        .setNumberOfSites(11)
        .setNumberOfEliteSites(8)
        .setNumberOfEliteBees(4)
        .setNumberOfNonEliteBees(3)
        .setIsWithReplacement(isWithReplacement)
        .setRotationType(rotation)
        .setMutationType(mutation)
        .setProblem(data);
    return abc.evolve();
  }

  FireflyAlgorithm ffa;
  ffa.setMaxIteration(1000)
      .setNumberOfIndividuals(25)
      .setupIndex(2)
      .setIsWithReplacement(isWithReplacement)
      .setRotationType(rotation)
      .setMutationType(mutation)
      .setProblem(data);
  return ffa.evolve();
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