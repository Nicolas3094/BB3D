#include "../src/readData/BRInstanceReader.hpp"
#include "../src/readData/CommandLine.hpp"
#include "../src/readData/ExperimentResults.hpp"
#include "../src/genetic/FitnessCache.hpp"
#include "../src/heuristics/DBLF.hpp"
#include "../src/brkga/BRKGA2012.hpp"
#include "../src/brkga/BRKGAAudit.hpp"
#include "../src/util/RandomNumbers.hpp"
#include "../tests/PackingAudit.hpp"
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

/** @brief Construye el perfil de 40 evaluaciones por iteracion, cambiando solo la parada a tiempo. */
ExperimentConfiguration comparisonProfile(const std::string &algorithm)
{
    CommandLineOptions options;
    options.population = 20;
    options.decoder = PackingDecoder::Contact;
    options.execution = ExecutionMode::ThirtySeconds;
    options.iterations = 100; // Retained in identity; time-only execution ignores this cap.
    options.patience = 0;
    options.gaMatings = 20;
    options.crossover = 1;
    options.selection = .85;
    options.mutationProbability = 1;
    options.rotationMutation = .05;
    options.sites = 10;
    options.eliteSites = 5;
    options.eliteBees = 4;
    options.nonEliteBees = 2;
    options.movements = 2;
    options.cacheEntries = 0;

    return options.configuration(algorithm, ROTATION_WAY::SIX_WAY, MutationType::C2, false);
}

/**
 * @brief Evalua un caso GA/ABC/FFA con CPU de proceso y audita el incumbente final.
 * @param argv Entrada BR, posicion, algoritmo, carpeta nueva, segundos CPU y semilla raiz.
 * @return 0 cuando se guardan resultado y acomodo validos; 1 ante error.
 */
int main(int argc, char **argv)
{
    try
    {
        if (argc != 7)
            throw std::invalid_argument(
                "Usage: meta_time INPUT INSTANCE GA|ABC|FFA OUTPUT SECONDS ROOT_SEED");
        const std::string input = argv[1], algorithm = argv[3], directory = argv[4];
        const auto instance = parseExperimentSeed(argv[2]);
        const auto rootSeed = parseExperimentSeed(argv[6]);
        size_t parsed = 0;
        const double seconds = std::stod(argv[5], &parsed);

        if (parsed != std::string(argv[5]).size() || !std::isfinite(seconds) || seconds < .001 ||
            seconds > 3600)
            throw std::invalid_argument("CPU limit must be in [0.001,3600] seconds");

        if (std::ifstream(directory + "/result.csv"))
            throw std::invalid_argument("Use a new meta_time output directory");

        const auto datasets = BRInstanceReader().read(input);

        if (!instance || instance > datasets.size())
            throw std::invalid_argument("Instance out of range");

        const auto &data = datasets[instance - 1];
        const auto configuration = comparisonProfile(algorithm);
        const auto seed = instanceSeed(rootSeed, data.id, data.PSeed);
        seedRandomNumbers(seed);

        using Clock = ExecutionBudget::Clock;
        const auto wallStart = Clock::now();
        const auto cpuStart = brkga2012::processCpuSeconds();
        auto lastPoll = wallStart - std::chrono::milliseconds(1);
        double cpuElapsed = 0;
        std::uint64_t calls = 0;
        auto cpuClock = [&]
        {
            // Match the CPU polling policy used by the BRKGA benchmark; do not
            // issue a system call at every geometry/containment checkpoint.
            if (calls++ % 256 == 0)
            {
                const auto now = Clock::now();

                if (now - lastPoll >= std::chrono::milliseconds(1))
                {
                    cpuElapsed = brkga2012::processCpuSeconds() - cpuStart;
                    lastPoll = now;
                }
            }
            return Clock::time_point{} +
                   std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(cpuElapsed));
        };
        const auto limit = std::chrono::milliseconds(static_cast<std::int64_t>(seconds * 1000));
        const auto result = ThirtySecondExecution(cpuClock, limit).execute(configuration, data);
        const double cpuSeconds = brkga2012::processCpuSeconds() - cpuStart;
        const double wallSeconds = std::chrono::duration<double>(Clock::now() - wallStart).count();

        if (result.population.empty())
            throw std::runtime_error("Missing timed incumbent");

        const auto &best = result.population.front();
        const auto evaluations = fitnessCacheStats().misses;

        auto code = best.solution;
        std::replace(code.begin(), code.end(), '|', ' ');
        std::istringstream decoded(code);
        Chromosome ids, rotations;
        int id, rotation;

        while (decoded >> id >> rotation)
        {
            ids.push_back(id);
            rotations.push_back(rotation);
        }
        auto packing = data.bin;
        packing.setRotationWay(configuration.rotation)
            .setDecoder(configuration.decoder)
            .setCandidatePolicy(configuration.candidates);
        DBLF(packing, ids, rotations, data.totalItems);
        auditPacking(packing, data.totalItems, best.getGenome().getGenome(), best.getGenome().getDGenome());
        brkga2012::verifyPacking(packing, data);

        if (static_cast<double>(packing.getLoadedVolume()) / packing.getDimensions().getVolumen() !=
            best.getFitness())
            throw std::runtime_error("Timed incumbent replay changed fitness");

        std::ostringstream boxes, genome, insertion, record;
        boxes << "id,type,rotation,x,y,z,dx,dy,dz\n";

        for (const auto &item : packing.getLoadedItems())
        {
            const auto p = item.getPunto(), d = item.getCurrentDimension();
            boxes << item.getId() << ',' << item.getTipo() << ',' << item.getRotationId() << ',' << p.x << ','
                  << p.y << ',' << p.z << ',' << d.x << ',' << d.y << ',' << d.z << '\n';
        }
        genome << "position,id,rotation\n";
        const auto &order = best.getGenome().getGenome(), &genes = best.getGenome().getDGenome();

        for (size_t index = 0; index < order.size(); ++index)
            genome << index << ',' << order[index] << ',' << genes[index] << '\n';
        insertion << "position,id,rotation\n";

        for (size_t index = 0; index < ids.size(); ++index)
            insertion << index << ',' << ids[index] << ',' << rotations[index] << '\n';
        record << std::setprecision(17)
               << "algorithm,occupation_percent,cpu_seconds,wall_seconds,seconds,clock,actual_seed,"
                  "evaluations_started,partial_evaluation,termination,valid\n"
               << algorithm << ',' << 100 * best.getFitness() << ',' << cpuSeconds << ',' << wallSeconds
               << ',' << seconds << ",cpu," << seed << ',' << evaluations << ',' << result.partialEvaluation
               << ',' << result.termination << ",1\n";
        saveResultText(directory + "/packing.csv", boxes.str());
        saveResultText(directory + "/genome.csv", genome.str());
        saveResultText(directory + "/insertion.csv", insertion.str());
        saveResultText(directory + "/configuration.txt",
                       configuration.identity("BR-source", seed) +
                           ";protocol=br1-br17-cpu-time-v1;budget_clock=process_cpu;actual_limit_seconds=" +
                           std::to_string(seconds) + "\n");
        saveResultText(directory + "/verification.json", "{\"valid\":true,\"independent_audits\":2}\n");
        saveResultText(directory + "/result.csv", record.str());
        std::cout << algorithm << " OCUPACION FINAL: " << 100 * best.getFitness() << "% CPU=" << cpuSeconds
                  << "s evaluations_started=" << evaluations << '\n';
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';

        return 1;
    }
}
