#include "../src/util/RandomNumbers.hpp"
#include <cmath>
#include <stdexcept>
#include <string>

void require(bool condition, const char *message)
{
    if (!condition)
        throw std::runtime_error(message);
}

int main(int argc, char **argv)
{
    try
    {
        // This case is safe to run against the original implementation.
        require(defaultBinomialDistribution(1) == 0, "n=1: p=0 must always return 0");
        if (argc > 1 && std::string(argv[1]) == "--singleton-only")
        {
            std::cout << "PASS: singleton\n";
            return 0;
        }

        require(defaultBinomialDistribution(0) == 0, "n=0 must return 0");
        bool rejected = false;
        try { defaultBinomialDistribution(-1); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected, "negative n must throw invalid_argument");

        for (int n : {2, 10, 50, 1000})
        {
            const int samples = 20000;
            double total = 0;
            bool sawBelowN = false;
            bool sawN = false;
            for (int i = 0; i < samples; ++i)
            {
                const int result = defaultBinomialDistribution(n);
                require(result >= 0 && result <= n, "sample outside [0,n]");
                total += result;
                sawBelowN |= result < n;
                sawN |= result == n;
            }
            // E[X] = n*p = n-1; Var[X] = (n-1)/n < 1.
            // Tolerance exceeds 14 standard errors for 20000 samples.
            require(std::abs(total / samples - (n - 1.0)) < 0.1,
                    "sample mean inconsistent with p=(n-1)/n");
            require(sawBelowN && sawN, "distribution must not be constant");
            std::cout << "PASS: n=" << n << ", mean=" << total / samples << '\n';
        }
        std::cout << "PASS: boundary cases and 80000 samples\n";
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
