#include "../src/heuristics/DBLF.hpp"
#include "../src/readData/ReadData.hpp"
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <random>

struct PackingCase
{
    size_t instance;
    ROTATION_WAY mode;
    Chromosome order;
    Chromosome rotations;
};

int main(int argc, char **argv)
{
    try
    {
        if (argc != 4) throw std::invalid_argument("Usage: benchmark dataset output-prefix rounds");
        const int rounds = std::stoi(argv[3]);
        if (rounds < 1) throw std::invalid_argument("Rounds must be positive");
        auto datasets = readData(argv[1]);
        std::mt19937 generator(20261023);
        std::vector<PackingCase> cases;
        for (size_t i = 0; i < datasets.size(); ++i)
        for (ROTATION_WAY mode : {ROTATION_WAY::ZERO_WAY, ROTATION_WAY::TWO_WAY, ROTATION_WAY::SIX_WAY})
        {
            const size_t n = datasets[i].totalItems.size();
            PackingCase sample{i, mode, Chromosome(n), Chromosome(n, 0)};
            std::iota(sample.order.begin(), sample.order.end(), 1);
            std::shuffle(sample.order.begin(), sample.order.end(), generator);
            if (mode != ROTATION_WAY::ZERO_WAY)
            {
                std::uniform_int_distribution<int> rotation(1, getIdFromRotationWay(mode));
                for (int &value : sample.rotations) value = rotation(generator);
            }
            cases.push_back(sample);
        }
        std::ofstream timings(std::string(argv[2]) + "-timings.csv");
        std::ofstream placements(std::string(argv[2]) + "-placements.txt");
        if (!timings || !placements) throw std::runtime_error("Cannot open benchmark output");
        timings << "round,evaluations,milliseconds,loaded_items,loaded_volume\n" << std::setprecision(12);
        // Round -1 warms caches. I/O and result serialization are outside timed regions.
        for (int round = -1; round < rounds; ++round)
        {
            std::vector<Bin> results;
            results.reserve(cases.size());
            const auto start = std::chrono::steady_clock::now();
            for (const auto &sample : cases)
            {
                Bin bin = datasets[sample.instance].bin;
                bin.setRotationWay(sample.mode);
                DBLF(bin, sample.order, sample.rotations, datasets[sample.instance].totalItems);
                results.push_back(std::move(bin));
            }
            const double ms = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start).count();
            long long count = 0, volume = 0;
            for (size_t i = 0; i < results.size(); ++i)
            {
                count += results[i].getNumberOfLoadedItems();
                volume += results[i].getLoadedVolume();
                if (round != 0) continue;
                placements << cases[i].instance << ',' << getIdFromRotationWay(cases[i].mode) << ':';
                for (const auto &item : results[i].getLoadedItems())
                {
                    const auto p = item.getPunto();
                    const auto d = item.getCurrentDimension();
                    placements << item.getId() << ',' << item.getRotationId() << ','
                        << p.x << ',' << p.y << ',' << p.z << ',' << d.x << ',' << d.y << ',' << d.z << ';';
                }
                placements << '\n';
            }
            if (round < 0) continue;
            timings << round << ',' << cases.size() << ',' << ms << ',' << count << ',' << volume << '\n';
            std::cout << "round=" << round << " evaluations=" << cases.size() << " ms=" << ms << '\n';
        }
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
