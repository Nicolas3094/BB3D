#include "../src/genetic/GeneticsOperators.hpp"
#include <stdexcept>

// Controlled random inputs: p=0.6 selects exactly one allele because
// 0.5 <= 0.6 and 0.5 > 0.6*0.6. Select the last index and rotation.
double uniformUnit() { return 0.5; }
int randomInteger(int minimum, int maximum)
{
    if (minimum > maximum)
        throw std::runtime_error("Invalid random range");
    return maximum;
}

void require(bool condition, const char *message)
{
    if (!condition)
        throw std::runtime_error(message);
}

int main()
{
    try
    {
        for (ROTATION_WAY rotation : {ROTATION_WAY::TWO_WAY, ROTATION_WAY::SIX_WAY})
        {
            DoubleGenome gen = DoubleGenome::Build()
                .setGenome({3, 1, 2}).setDGenome({1, 1, 1});
            flipMutationO1(gen, 0.6, rotation);
            require(gen.getDGenome()[2] == getIdFromRotationWay(rotation),
                    "Selected rotation was not written to chromosome");
            require(gen.getDGenome()[0] == 1 && gen.getDGenome()[1] == 1,
                    "Unselected rotations changed");
            require(gen.getGenome() == Chromosome({3, 1, 2}),
                    "Item order changed");
            require(gen.getDGenome().size() == 3, "Rotation chromosome resized");
        }
        DoubleGenome unchanged = DoubleGenome::Build()
            .setGenome({1, 2}).setDGenome({1, 2});
        flipMutationO1(unchanged, 0.0, ROTATION_WAY::TWO_WAY);
        require(unchanged.getDGenome() == Chromosome({1, 2}),
                "Zero probability must preserve rotations");
        std::cout << "PASS: two/six rotations, selected allele, unchanged order and zero probability\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
