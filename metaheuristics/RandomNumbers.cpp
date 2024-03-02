#include "RandomNumbers.hpp"

double uniformUnit()
{
    static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine generator(seed);
    static std::uniform_real_distribution<double> distributionDouble(0, 1);

    return distributionDouble(generator);
}