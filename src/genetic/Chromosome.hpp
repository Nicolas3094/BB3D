#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include <iostream>
#include <vector>

// TODO : change this to int[] or int*
typedef std::vector<int> Chromosome;

class DoubleGenome
{
private:
    Chromosome genome;
    Chromosome doubleGenome;
    int size;

public:
    DoubleGenome();

    ~DoubleGenome();

    static DoubleGenome Build();

    DoubleGenome &setGenome(Chromosome);
    DoubleGenome &setDGenome(Chromosome);

    Chromosome &getGenome();
    Chromosome &getDGenome();
};

bool operator==(DoubleGenome, DoubleGenome);
std::ostream &operator<<(std::ostream &, DoubleGenome &);
std::ostream &operator<<(std::ostream &, Chromosome &);
bool operator==(Chromosome, Chromosome);

#endif