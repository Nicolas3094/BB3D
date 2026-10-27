#include "../src/heuristics/DBLF.hpp"
#include "../src/heuristics/MaximalSpacesDecoder.hpp"
#include "../src/readData/BRInstanceReader.hpp"
#include "../tests/PackingAudit.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <random>

namespace
{
/**
 * @brief Exporta el orden y las orientaciones de un genoma en formato CSV.
 * @param path Archivo de destino, dentro de una carpeta existente.
 * @param itemIds Identificadores en orden de evaluacion.
 * @param rotationGenes Orientaciones, con la misma longitud que itemIds.
 */
void saveGenome(const std::string &path, const Chromosome &itemIds, const Chromosome &rotationGenes)
{
    std::ofstream output(path);
    output << "id,rotation\n";
    for (size_t index = 0; index < itemIds.size(); ++index)
        output << itemIds[index] << ',' << rotationGenes[index] << '\n';
}

/**
 * @brief Exporta las cajas cargadas con ID, orientacion, posicion y dimensiones.
 * @param path Archivo CSV de destino, dentro de una carpeta existente.
 * @param bin Acomodo evaluado; solo se exportan las cajas realmente insertadas.
 */
void savePacking(const std::string &path, const Bin &bin)
{
    std::ofstream output(path);
    output << "id,rotation,x,y,z,dx,dy,dz\n";
    for (const auto &item : bin.getLoadedItems())
    {
        const auto position = item.getPunto();
        const auto dimensions = item.getCurrentDimension();
        output << item.getId() << ',' << item.getRotationId() << ','
               << position.x << ',' << position.y << ',' << position.z << ','
               << dimensions.x << ',' << dimensions.y << ',' << dimensions.z << '\n';
    }
}
}

/**
 * @brief Compara DBLF y Espacios Maximos sobre ocho genomas identicos por pares.
 * @param argc Debe ser 2: nombre del programa y carpeta de salida.
 * @param argv argv[1] indica una carpeta de salida existente.
 * @return 0 al completar las evaluaciones y auditorias; 1 ante una excepcion.
 *
 * Usa cuatro genomas de BR1/1 y cuatro de BR15/1, semilla 123 y orientaciones
 * legales. Alterna el orden de medicion, audita cada acomodo y guarda genomas,
 * cajas, ocupacion, tiempo y maximo de espacios retenidos. Debe ejecutarse desde
 * la raiz del proyecto. No ejecuta las metaheuristicas ni la bateria BR completa.
 */
int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw std::runtime_error("Usage: maximal_spaces_comparison OUTPUT_DIRECTORY");

        const std::string outputDirectory = argv[1];
        std::ofstream results(outputDirectory + "/evaluations.csv");
        results << "dataset,instance,genome,decoder,occupation_percent,microseconds,peak_spaces,valid\n"
                << std::setprecision(17);

        for (const std::string dataset : {"BR1", "BR15"})
        {
            const auto data = BRInstanceReader().read("Instance/BR/" + dataset + ".txt").front();
            std::mt19937 random(123);
            Chromosome itemIds(data.totalItems.size());
            std::iota(itemIds.begin(), itemIds.end(), 1);

            for (int genomeIndex = 0; genomeIndex < 4; ++genomeIndex)
            {
                if (genomeIndex != 0)
                    std::shuffle(itemIds.begin(), itemIds.end(), random);

                Chromosome rotationGenes;
                for (int itemId : itemIds)
                {
                    const auto permissions = data.totalItems[itemId - 1].getRotations();
                    std::vector<int> permittedRotations;
                    if (permissions.z)
                    {
                        permittedRotations.push_back(1);
                        permittedRotations.push_back(3);
                    }
                    if (permissions.y)
                    {
                        permittedRotations.push_back(2);
                        permittedRotations.push_back(5);
                    }
                    if (permissions.x)
                    {
                        permittedRotations.push_back(4);
                        permittedRotations.push_back(6);
                    }
                    if (permittedRotations.empty())
                        throw std::runtime_error("No legal orientation");
                    rotationGenes.push_back(permittedRotations[random() % permittedRotations.size()]);
                }

                const auto label = dataset + "-g" + std::to_string(genomeIndex);
                saveGenome(outputDirectory + "/" + label + "-genome.csv", itemIds, rotationGenes);

                // Alternate measurement order; both decoders receive the same genome.
                for (int evaluationOrder = 0; evaluationOrder < 2; ++evaluationOrder)
                {
                    const bool useMaximalSpaces = (genomeIndex + evaluationOrder) % 2 != 0;
                    auto bin = data.bin;
                    bin.setRotationWay(ROTATION_WAY::SIX_WAY);
                    bin.setDecoder(useMaximalSpaces ? PackingDecoder::MaximalSpaces : PackingDecoder::DBLF);

                    MaximalSpacesStats stats;
                    const auto start = std::chrono::steady_clock::now();
                    if (useMaximalSpaces)
                        maximalSpacesDecode(bin, itemIds, rotationGenes, data.totalItems, &stats);
                    else
                        DBLF(bin, itemIds, rotationGenes, data.totalItems);
                    const auto microseconds = std::chrono::duration<double, std::micro>(
                        std::chrono::steady_clock::now() - start).count();

                    auditPacking(bin, data.totalItems, itemIds, rotationGenes);
                    const std::string decoder = packingDecoderName(bin.getDecoder());
                    const double occupationPercent = 100.0 * bin.getLoadedVolume() /
                                                     bin.getDimensions().getVolumen();
                    results << dataset << ",1," << genomeIndex << ',' << decoder << ','
                            << occupationPercent << ',' << microseconds << ',';
                    if (useMaximalSpaces)
                        results << stats.peakRetainedSpaces;
                    results << ",1\n";
                    savePacking(outputDirectory + "/" + label + "-" + decoder + "-packing.csv", bin);
                }
            }
        }
        if (!results)
            throw std::runtime_error("Could not write results");
        std::cout << "PASS: 8 paired genomes, 16 independently audited packings\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
