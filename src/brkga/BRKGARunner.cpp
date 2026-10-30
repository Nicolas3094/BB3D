#include "BRKGARunner.hpp"
#include "BRKGAAudit.hpp"
#include "../readData/BRInstanceReader.hpp"
#include "../readData/ExperimentConfiguration.hpp"
#include "../readData/ExperimentResults.hpp"
#include <fstream>
#include <iomanip>

namespace brkga2012
{
void saveResult(const std::string &directory, const DatasetBinBacking &data, const SearchResult &result,
                const Configuration &configuration, std::uint64_t seed)
{
    verifyPacking(result.packing, data);

    std::ostringstream packing, keys, parameters, record;
    packing << "id,type,rotation,x,y,z,dx,dy,dz\n";

    for (const auto &item : result.packing.getLoadedItems())
    {
        const auto p = item.getPunto(), d = item.getCurrentDimension();
        packing << item.getId() << ',' << item.getTipo() << ',' << item.getRotationId() << ',' << p.x << ','
                << p.y << ',' << p.z << ',' << d.x << ',' << d.y << ',' << d.z << '\n';
    }

    keys << std::setprecision(17) << "gene,key\n";

    for (size_t index = 0; index < result.keys.size(); ++index)
        keys << index << ',' << result.keys[index] << '\n';

    parameters << std::setprecision(17) << "algorithm=BRKGA-CLP-U-2012-reimplementation-v1\nseed=" << seed
               << "\npopulation_size=" << result.populationSize
               << "\npopulations=" << configuration.populations
               << "\nelite_fraction=" << configuration.eliteFraction
               << "\nmutant_fraction=" << configuration.mutantFraction
               << "\nelite_bias=" << configuration.eliteBias
               << "\nmigration_interval=" << configuration.migrationInterval
               << "\nmigrants=" << configuration.migrants
               << "\nmaximum_generations=" << configuration.maximumGenerations
               << "\nseconds=" << configuration.seconds
               << "\nclock=" << (configuration.clock == BudgetClock::ProcessCPU ? "process_cpu" : "wall")
               << "\nthreads_per_process=1\nsupport_required=false\ncompiler=" << __VERSION__
               << "\nlayer_order=YZ,ZY,YX,XY,ZX,XZ; rotations=1..6\n"
               << "differences=independent implementation; serial fitness; deterministic layer enumeration; "
                  "Skip=False typo corrected; cooperative time cap added to published 500-generation cap\n";

    record << std::setprecision(17)
           << "algorithm,occupation_percent,loaded_volume,boxes,cpu_seconds,wall_seconds,seconds,clock,seed,"
              "population_size,populations,generations,evaluations_started,evaluations_completed,migrations,"
              "partial_evaluation,termination,initialized,valid\n"
           << "BRKGA2012-U,"
           << 100.0 * result.packing.getLoadedVolume() / data.bin.getDimensions().getVolumen() << ','
           << result.packing.getLoadedVolume() << ',' << result.packing.getNumberOfLoadedItems() << ','
           << result.cpuSeconds << ',' << result.wallSeconds << ',' << configuration.seconds << ','
           << (configuration.clock == BudgetClock::ProcessCPU ? "cpu" : "wall") << ',' << seed << ','
           << result.populationSize << ',' << configuration.populations << ',' << result.generations << ','
           << result.evaluationsStarted << ',' << result.evaluationsCompleted << ',' << result.migrationEvents
           << ',' << result.partialEvaluation << ',' << result.termination << ',';
    for (size_t index = 0; index < result.initialized.size(); ++index)
    {
        if (index)
            record << ';';
        record << result.initialized[index];
    }
    record << ",1\n";

    saveResultText(directory + "/packing.csv", packing.str());
    saveResultText(directory + "/keys.csv", keys.str());
    saveResultText(directory + "/configuration.txt", parameters.str());
    saveResultText(directory + "/verification.json",
                   "{\"valid\":true,\"independent_geometry_audit\":true}\n");
    saveResultText(directory + "/result.csv", record.str());
}

std::string executeMenu(const BRMenuSelection &selection)
{
    const auto &options = selection.options;
    const auto directory = createResultSessionDirectory(
        options.output.empty() ? options.root + "/Results/BR" : options.output, selection.sessionName);
    std::vector<std::string> datasets;

    if (options.dataset == "BR")
        for (int file = 0; file <= 18; ++file)
            datasets.push_back("BR" + std::to_string(file));
    else
        datasets.push_back(options.dataset);
    Configuration configuration;
    std::ostringstream report, combined;
    report << "# BRKGA-CLP-U (Goncalves y Resende 2012)\n\n"
           << "Reimplementacion independiente; 30 segundos CPU, un hilo por ejecucion.\n"
           << "Configuracion publicada, sin soporte completo. Ver configuration.txt en cada caso.\n\n"
           << "| Instancia | Ronda | Ocupacion (%) | CPU (s) | Generaciones | Evaluaciones |\n"
           << "| --- | ---: | ---: | ---: | ---: | ---: |\n";
    combined
        << "dataset,instance,round,occupation_percent,cpu_seconds,wall_seconds,generations,evaluations,valid\n";
    for (const auto &dataset : datasets)
    {
        const auto instances = BRInstanceReader().read(options.root + "/Instance/BR/" + dataset + ".txt");
        const size_t first = options.firstInstance - 1;

        if (first >= instances.size())
            throw std::invalid_argument("BRKGA first instance out of range");

        const size_t count = options.instances ? std::min<size_t>(options.instances, instances.size() - first)
                                               : instances.size() - first;
        for (size_t index = first; index < first + count; ++index)
            for (int round = 0; round < options.rounds; ++round)
            {
                const auto &data = instances[index];
                const auto roundSeed = round == 0 ? options.seed : instanceSeed(options.seed, round, 0);
                const auto seed = instanceSeed(roundSeed, data.id, data.PSeed);
                const auto result = search(data, seed, configuration);
                const auto caseDirectory = createResultSessionDirectory(
                    directory, dataset + "-i" + std::to_string(index + 1) + "-r" + std::to_string(round + 1));
                saveResult(caseDirectory, data, result, configuration, seed);
                const double occupation =
                    100.0 * result.packing.getLoadedVolume() / data.bin.getDimensions().getVolumen();
                std::cout << std::fixed << std::setprecision(3) << dataset << " problema " << index + 1
                          << " | BRKGA2012-U | OCUPACION FINAL: " << occupation
                          << " % | CPU: " << result.cpuSeconds << " s | generaciones: " << result.generations
                          << " | evaluaciones: " << result.evaluationsCompleted << '\n';
                report << "| " << dataset << '/' << index + 1 << " | " << round + 1 << " | " << occupation
                       << " | " << result.cpuSeconds << " | " << result.generations << " | "
                       << result.evaluationsCompleted << " |\n";
                combined << dataset << ',' << index + 1 << ',' << round + 1 << ',' << occupation << ','
                         << result.cpuSeconds << ',' << result.wallSeconds << ',' << result.generations << ','
                         << result.evaluationsCompleted << ",1\n";
                saveResultText(directory + "/resumen.md", report.str());
                saveResultText(directory + "/resultados.csv", combined.str());
            }
    }
    return directory;
}
}
