#include "../src/heuristics/MaximalSpacesDecoder.hpp"
#include "../src/spatial/MaximalSpacesProfile.hpp"
#include "../src/readData/BRInstanceReader.hpp"
#include "../tests/PackingAudit.hpp"
#include <windows.h>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
/** @brief Obtiene CPU acumulada del proceso (usuario y sistema) en microsegundos. */
double processCpuMicroseconds()
{
    FILETIME created, exited, kernel, user;

    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
        throw std::runtime_error("Cannot read process CPU time");

    ULARGE_INTEGER kernelTicks, userTicks;
    kernelTicks.LowPart = kernel.dwLowDateTime;
    kernelTicks.HighPart = kernel.dwHighDateTime;
    userTicks.LowPart = user.dwLowDateTime;
    userTicks.HighPart = user.dwHighDateTime;

    return (kernelTicks.QuadPart + userTicks.QuadPart) / 10.0;
}

/** @brief Lee un genoma guardado para evitar volver a generar sus orientaciones. */
void readGenome(const std::string &path, Chromosome &itemIds, Chromosome &rotationGenes)
{
    std::ifstream input(path);

    if (!input)
        throw std::runtime_error("Cannot open genome: " + path);

    std::string line;
    std::getline(input, line);

    while (std::getline(input, line))
    {
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream row(line);
        int id, rotation;

        if (!(row >> id >> rotation))
            throw std::runtime_error("Malformed saved genome");

        itemIds.push_back(id);
        rotationGenes.push_back(rotation);
    }
}

/** @brief Guarda el acomodo para compararlo byte a byte con la referencia anterior. */
void savePacking(const std::string &path, const Bin &bin)
{
    std::ofstream output(path);
    output << "id,rotation,x,y,z,dx,dy,dz\n";

    for (const auto &item : bin.getLoadedItems())
    {
        const auto position = item.getPunto();
        const auto dimensions = item.getCurrentDimension();
        output << item.getId() << ',' << item.getRotationId() << ',' << position.x << ',' << position.y << ','
               << position.z << ',' << dimensions.x << ',' << dimensions.y << ',' << dimensions.z << '\n';
    }
    if (!output)
        throw std::runtime_error("Cannot save packing");
}
}

/**
 * @brief Mide una repeticion de los ocho genomas guardados, sin ejecutar busquedas.
 * @param argv Carpeta de salida, carpeta de genomas y numero de repeticion.
 * @return 0 si todas las evaluaciones pasan el auditor; 1 ante un error.
 * @note Lectura, auditoria y exportacion quedan fuera de los tiempos de evaluacion.
 */
int main(int argc, char **argv)
{
    try
    {
        if (argc != 4)
            throw std::runtime_error("Usage: profile OUTPUT GENOMES REPETITION");

        const std::string outputDirectory = argv[1], genomesDirectory = argv[2];
        const int repetition = std::stoi(argv[3]);
#ifdef BB3D_PROFILE_MAXIMAL_SPACES
        const std::string mode = "profiled";
#else
        const std::string mode = "normal";
#endif
        const auto runLabel = mode + "-r" + std::to_string(repetition);
        std::ofstream results(outputDirectory + "/" + runLabel + ".csv");
        results
            << "mode,repetition,dataset,genome,occupation_percent,total_us,cpu_us,placement_us,split_us,"
               "pruning_us,sorting_us,other_us,candidate_spaces,split_visits,affected_spaces,pruning_pair_visits,"
               "containment_checks,duplicates_removed,contained_removed,peak_before_pruning,peak_retained,valid\n"
            << std::setprecision(17);

        for (const std::string dataset : {"BR1", "BR15"})
        {
            const auto data = BRInstanceReader().read("Instance/BR/" + dataset + ".txt").front();

            for (int genomeIndex = 0; genomeIndex < 4; ++genomeIndex)
            {
                const auto genomeLabel = dataset + "-g" + std::to_string(genomeIndex);
                Chromosome itemIds, rotationGenes;
                readGenome(genomesDirectory + "/" + genomeLabel + "-genome.csv", itemIds, rotationGenes);
                auto bin = data.bin;
                bin.setRotationWay(ROTATION_WAY::SIX_WAY).setDecoder(PackingDecoder::MaximalSpaces);
                MaximalSpacesStats stats;
                maximalSpacesProfile() = {};
                const double cpuStart = processCpuMicroseconds();
                const auto start = std::chrono::steady_clock::now();
                maximalSpacesDecode(bin, itemIds, rotationGenes, data.totalItems, &stats);
                const double total =
                    std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start)
                        .count();
                const double cpu = processCpuMicroseconds() - cpuStart;
                const auto &profile = maximalSpacesProfile();
                const double other = total - profile.placementMicroseconds - profile.splitMicroseconds -
                                     profile.pruningMicroseconds - profile.sortingMicroseconds;

                auditPacking(bin, data.totalItems, itemIds, rotationGenes);
                savePacking(outputDirectory + "/" + runLabel + "-" + genomeLabel + "-packing.csv", bin);
                results << mode << ',' << repetition << ',' << dataset << ',' << genomeIndex << ','
                        << 100.0 * bin.getLoadedVolume() / bin.getDimensions().getVolumen() << ',' << total
                        << ',' << cpu << ',' << profile.placementMicroseconds << ','
                        << profile.splitMicroseconds << ',' << profile.pruningMicroseconds << ','
                        << profile.sortingMicroseconds << ',' << other << ',' << profile.candidateSpaces
                        << ',' << profile.spacesVisitedForSplit << ',' << profile.affectedSpaces << ','
                        << profile.pruningPairVisits << ',' << profile.containmentChecks << ','
                        << profile.duplicatesRemoved << ',' << profile.containedSpacesRemoved << ','
                        << profile.peakSpacesBeforePruning << ',' << stats.peakRetainedSpaces << ",1\n";
            }
        }
        if (!results)
            throw std::runtime_error("Cannot save profile");

        std::cout << "PASS: " << runLabel << ", eight audited evaluations\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';

        return 1;
    }
}
