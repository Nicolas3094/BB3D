#include "main.hpp"
#include "src/Individuo.hpp"
#include "src/GenomeOperators.hpp"

int main()
{
  Chromosome genome{1, 2, 3, 4, 6, 7, 8, 9};
  Chromosome genomeRot{1, 1, 1, 1, 1, 1, 1, 1};
  Individuo individuo = Individuo::Build()
                            .setFitness(0.21)
                            .setGenome(
                                DoubleGenome::Build().setGenome(genome).setDGenome(genomeRot));
  Chromosome nwGen{0, 1, 1, 0, 1, 1, 1, 1};
  individuo.getGenome().setDGenome(nwGen);
  int n = 8;
  Chromosome chrs = generatePermutation(n);
  print(chrs);

  system("pause");
}