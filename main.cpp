#include "main.hpp"
#include "src/FitnessOperators.hpp"
#include "src/Chromosome.hpp"
#include "readData/ReadData.hpp"
#include <chrono>

int main()
{
  srand(time(NULL));

  vector<DatasetBinBacking> DATASSET;
  const string dataPATH = "C:\\Users\\nicoo\\OneDrive\\Documentos\\Progamming\\3DBPP_CPP\\Instance\\P5A2.csv";
  try
  {
    DATASSET = readData(dataPATH);
  }
  catch (const std::exception &ex)
  {
    cerr
        << ex.what() << "\n";
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

  system("pause");
}