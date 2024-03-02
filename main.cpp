#include "main.hpp"
#include "src/FitnessOperators.hpp"
#include "metaheuristics/GeneticsOperators.hpp"
#include "metaheuristics/PermutationOperators.hpp"
#include "readData/ReadData.hpp"
#include <chrono>

int main()
{
  srand(time(NULL));

  Chromosome chromosome1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  Chromosome chromosome2{2, 8, 5, 1, 9, 6, 3, 4, 7, 10};
  DoubleGenome gen = DoubleGenome::Build().setGenome(chromosome1);
  DoubleGenome gen2 = DoubleGenome::Build().setGenome(chromosome2);
  DoubleGenome mutGen2 = gen2;
  // DoubleGenome child = crossOx(gen, gen2, 2, 4);
  cout << gen2 << "\n";
  // swapPointValue(gen2, 2, 4);
  // inverseMutation(gen2, 2, 4);
  // randomReversingInsertionOfSubsequence(gen2, 2, 3, 6);
  // randomSwap(gen2, 2, 4);
  // swapSubsequences(gen2, 2, 4, 6, 8);

  double promHamming = 0.0;
  try
  {
    mutateC2(mutGen2, 2);
  }

  catch (const std::exception &ex)
  {
    cerr << ex.what() << "\n";
  }
  promHamming += hamming(gen2, mutGen2);
  mutGen2 = gen2;
  cout << "Diff: " << promHamming << "\n";

  system("pause");
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