#include "RandomNumbers.hpp"
namespace {
std::mt19937_64 &engine() { static std::mt19937_64 generator(5489); return generator; }
}
void seedRandomNumbers(std::uint64_t seed) { engine().seed(seed); }
double uniformUnit() { return std::generate_canonical<double,53>(engine()); }
int randomInteger(int init,int end) {
    if(init>end) throw std::invalid_argument("Random interval must satisfy minimum <= maximum.");
    return std::uniform_int_distribution<int>(init,end)(engine());
}
int defaultBinomialDistribution(int n) {
    if(n<0) throw std::invalid_argument("Binomial trial count must be non-negative.");
    if(n==0) return 0;
    return std::binomial_distribution<int>(n,(n-1.0)/n)(engine());
}
