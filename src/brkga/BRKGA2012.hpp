#ifndef BRKGA_2012_HPP
#define BRKGA_2012_HPP

#include "BRKGALayerDecoder.hpp"
#include "../util/ExecutionBudget.hpp"
#include <random>

namespace brkga2012
{
enum class BudgetClock
{
    ProcessCPU,
    Wall
};

/** @brief Parametros de Tabla 3; un tamano explicito permite pruebas pequenas. */
struct Configuration
{
    size_t populationSize = 0; // 0: 20 * M, independently for each population.
    size_t populations = 3;
    double eliteFraction = 0.15;
    double mutantFraction = 0.15;
    double eliteBias = 0.7;
    size_t migrationInterval = 15;
    size_t migrants = 2;
    size_t maximumGenerations = 500;
    double seconds = 30;
    BudgetClock clock = BudgetClock::ProcessCPU;
};

struct Individual
{
    RandomKeys keys;
    std::int64_t volume = 0;
};

using Population = std::vector<Individual>;

/** @brief Acomodo incumbente y esfuerzo realmente realizado, incluso durante inicializacion. */
struct SearchResult
{
    Bin packing;
    RandomKeys keys;
    size_t evaluationsStarted = 0;
    size_t evaluationsCompleted = 0;
    size_t generations = 0;
    size_t migrationEvents = 0;
    size_t populationSize = 0;
    std::vector<size_t> initialized;
    bool partialEvaluation = false;
    std::string termination = "generation_limit";
    double cpuSeconds = 0;
    double wallSeconds = 0;
};

/** @brief CPU de proceso acumulada, usuario y sistema; no equivale a tiempo transcurrido. */
double processCpuSeconds();

/** @brief Genera claves uniformes en [0,1) usando exclusivamente el RNG de este BRKGA. */
RandomKeys randomKeys(size_t length, std::mt19937_64 &random);

/** @brief Cruce uniforme sesgado: primer progenitor elite, segundo de toda la poblacion. */
RandomKeys biasedCrossover(const RandomKeys &elite, const RandomKeys &other, double bias,
                           std::mt19937_64 &random);

/** @brief Inserta una instantanea de los dos mejores globales reemplazando los peores locales. */
void exchangeElites(std::vector<Population> &populations, size_t migrants);

/**
 * @brief Ejecuta BRKGA-CLP-U independiente de los operadores y cache de GA/ABC/FFA.
 * @param data Instancia BR con cantidades expandidas; no se modifica.
 * @param seed Semilla independiente y reproducible para claves, padres y cruce.
 * @param configuration Tabla 3 por defecto; tiempo 0 desactiva el plazo en tests.
 * @param controlledClock Reloj opcional para probar cortes sin esperar tiempo real.
 * @return Mejor acomodo valido visto, incluyendo una evaluacion parcial interrumpida.
 * @note Un proceso usa un hilo de evaluacion. Las tres poblaciones logicas se
 * actualizan secuencialmente para permitir 15 instancias concurrentes sin anidar hilos.
 */
SearchResult search(const DatasetBinBacking &data, std::uint64_t seed, const Configuration &configuration,
                    ExecutionBudget::Now controlledClock = {});
}
#endif
