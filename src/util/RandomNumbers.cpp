#include "RandomNumbers.hpp"

double uniformUnit()
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distributionDouble(0, 1);

    return distributionDouble(generator);
}

int randomInteger(int init, int end)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine generator(seed);
    std::uniform_int_distribution<> distr(init, end);

    int result = distr(generator);
    if (result < init || result > end)
    {
        std::cerr << "\nERROR: randomInteger";
        std::cerr << "\nresult: " << result << " | "
                  << "min: " << init << " end: " << end << "\n";
        throw std::invalid_argument("Invalid random integer result.");
    }
    return result;
}

int defaultBinomialDistribution(int n)
{
    if (n < 0)
    {
        throw std::invalid_argument("Binomial trial count must be non-negative.");
    }
    if (n == 0)
    {
        return 0;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine generator(seed);
    const double probability = (n - 1.0) / n;
    std::binomial_distribution<> distr(n, probability);

    int result = distr(generator);
    if (result > n)
    {
        std::cerr << "\nERROR: randomInteger";
        std::cerr << "\nresult: " << result << " | "
                  << "n: " << n << "\n";
        throw std::invalid_argument("Invalid random integer result.");
    }
    return result;
}
