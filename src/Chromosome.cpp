#include "Chromosome.hpp"

DoubleGenome::DoubleGenome() {}

DoubleGenome::~DoubleGenome()
{
}

DoubleGenome &DoubleGenome::Build()
{
    DoubleGenome *newGenome = new DoubleGenome();
    return *newGenome;
}

DoubleGenome &DoubleGenome::setGenome(Chromosome chromosome)
{
    this->genome = chromosome;
    return *this;
}

DoubleGenome &DoubleGenome::setDGenome(Chromosome chromosome)
{
    this->doubleGenome = chromosome;
    return *this;
}

Chromosome DoubleGenome::getGenome()
{
    return this->genome;
}

Chromosome DoubleGenome::getDGenome()
{
    return this->doubleGenome;
}

std::ostream &operator<<(std::ostream &COUT, DoubleGenome &gen)
{
    COUT << "|";
    for (int i = 0; i < gen.getGenome().size(); i++)
    {
        COUT << gen.getGenome()[i];
        if (i != gen.getGenome().size() - 1)
        {
            COUT << "|";
        }
    }
    COUT << "|"
         << "\n"
         << "|";
    for (int i = 0; i < gen.getDGenome().size(); i++)
    {
        COUT << gen.getDGenome()[i];
        if (i != gen.getDGenome().size() - 1)
        {
            COUT << "|";
        }
    }
    COUT << "|"
         << "\n";
    return COUT;
}
std::ostream &operator<<(std::ostream &COUT, Chromosome &chr)
{
    COUT << "|";
    for (int i = 0; i < chr.size(); i++)
    {
        COUT << chr[i];
        if (i != chr.size() - 1)
        {
            COUT << "|";
        }
    }
    return COUT << "|";
}