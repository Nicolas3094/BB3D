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

Chromosome &DoubleGenome::getGenome()
{
    return this->genome;
}

Chromosome &DoubleGenome::getDGenome()
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

bool operator==(Chromosome chr1, Chromosome chr2)
{
    if (chr1.size() != chr2.size())
    {
        throw std::invalid_argument("Chromosomes have different lenght.");
    }
    for (int i = 0; i < chr1.size(); i++)
    {
        if (chr1[i] != chr2[i])
        {
            return false;
        }
    }
    return true;
}

bool operator==(DoubleGenome gen1, DoubleGenome gen2)
{
    return gen1.getGenome() == gen2.getGenome() && gen1.getDGenome() == gen2.getDGenome();
}