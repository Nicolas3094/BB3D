#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include "../main.hpp"

class DoubleGenome
{
private:
    Chromosome genome;
    Chromosome doubleGenome;

public:
    DoubleGenome();
    ~DoubleGenome();

    static DoubleGenome &Build();

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