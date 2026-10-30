#include "BRKGARunner.hpp"
#include "../readData/BRInstanceReader.hpp"
#include "../readData/ExperimentConfiguration.hpp"
#include <map>
#include <iomanip>
#include <fstream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
/** @brief Enumera mascaras de nucleos fisicos y su primer hilo para repartir el lote. */
void printTopology()
{
#ifdef _WIN32
    DWORD bytes = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bytes);
    std::vector<char> buffer(bytes);

    if (!GetLogicalProcessorInformationEx(
            RelationProcessorCore, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
            &bytes))
        throw std::runtime_error("Cannot query BRKGA CPU topology");

    unsigned index = 0;

    for (DWORD offset = 0; offset < bytes;)
    {
        auto entry = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data() + offset);

        if (entry->Relationship == RelationProcessorCore && entry->Processor.GroupCount == 1 &&
            entry->Processor.GroupMask[0].Group == 0)
        {
            const auto mask = static_cast<unsigned long long>(entry->Processor.GroupMask[0].Mask);
            std::cout << index++ << ' ' << (mask & (~mask + 1)) << ' ' << mask << '\n';
        }
        offset += entry->Size;
    }
#else
    throw std::runtime_error("Process pool topology requires Windows");
#endif
}
}

/** @brief CLI independiente para un caso BRKGA, usado tambien por el lote de 15 procesos. */
int main(int argc, char **argv)
{
    try
    {
        if (argc == 2 && std::string(argv[1]) == "--topology")
        {
            printTopology();

            return 0;
        }
        if (argc == 2 && std::string(argv[1]) == "--help")
        {
            std::cout
                << "brkga2012 --input BR.txt --instance 1 --output EXISTING_DIRECTORY "
                   "[--seconds 30 --clock cpu|wall --seed 123|--root-seed 123 --population N --generations 500]\n";

            return 0;
        }
        std::map<std::string, std::string> options;

        for (int argument = 1; argument < argc; argument += 2)
        {
            const std::string name = argv[argument];

            if (argument + 1 == argc || options.count(name))
                throw std::invalid_argument("Missing or duplicate BRKGA option");

            if (name != "--input" && name != "--instance" && name != "--output" && name != "--seconds" &&
                name != "--clock" && name != "--seed" && name != "--root-seed" && name != "--population" &&
                name != "--generations")
                throw std::invalid_argument("Unknown BRKGA option: " + name);

            options[name] = argv[argument + 1];
        }
        if (!options.count("--input") || !options.count("--output"))
            throw std::invalid_argument("BRKGA requires --input and --output");

        if (std::ifstream(options["--output"] + "/result.csv"))
            throw std::invalid_argument("BRKGA output already contains a result; use a new directory");

        brkga2012::Configuration configuration;

        if (options.count("--seconds"))
        {
            size_t parsed = 0;
            configuration.seconds = std::stod(options["--seconds"], &parsed);

            if (parsed != options["--seconds"].size() || configuration.seconds <= 0 ||
                configuration.seconds > 3600)
                throw std::invalid_argument("BRKGA seconds must be in (0,3600]");
        }
        if (options.count("--population"))
            configuration.populationSize = parseExperimentSeed(options["--population"]);
        if (options.count("--generations"))
            configuration.maximumGenerations = parseExperimentSeed(options["--generations"]);
        if (options.count("--clock"))
        {
            if (options["--clock"] == "wall")
                configuration.clock = brkga2012::BudgetClock::Wall;
            else if (options["--clock"] != "cpu")
                throw std::invalid_argument("BRKGA clock must be cpu or wall");
        }
        if (options.count("--seed") && options.count("--root-seed"))
            throw std::invalid_argument("Choose --seed or --root-seed, not both");

        auto seed = options.count("--seed") ? parseExperimentSeed(options["--seed"]) : 123;
        const auto instance = options.count("--instance") ? parseExperimentSeed(options["--instance"]) : 1;
        const auto data = BRInstanceReader().read(options["--input"]);

        if (!instance || instance > data.size())
            throw std::invalid_argument("BRKGA instance out of range");

        if (options.count("--root-seed"))
            seed = instanceSeed(parseExperimentSeed(options["--root-seed"]), data[instance - 1].id,
                                data[instance - 1].PSeed);
        const auto result = brkga2012::search(data[instance - 1], seed, configuration);
        brkga2012::saveResult(options["--output"], data[instance - 1], result, configuration, seed);
        std::cout << std::fixed << std::setprecision(3) << "BRKGA2012-U | OCUPACION FINAL: "
                  << 100.0 * result.packing.getLoadedVolume() /
                         data[instance - 1].bin.getDimensions().getVolumen()
                  << " % | CPU: " << result.cpuSeconds << " s | tiempo transcurrido: " << result.wallSeconds
                  << " s | generaciones: " << result.generations
                  << " | evaluaciones: " << result.evaluationsCompleted << " | " << result.termination
                  << '\n';
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';

        return 1;
    }
}
