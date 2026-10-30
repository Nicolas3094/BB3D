#ifndef BRKGA_RUNNER_HPP
#define BRKGA_RUNNER_HPP
#include "BRKGA2012.hpp"
#include "../BRExperimentMenu.hpp"

namespace brkga2012
{
/** @brief Audita y exporta acomodo, claves, parametros, esfuerzo y ocupacion a una carpeta existente. */
void saveResult(const std::string &directory, const DatasetBinBacking &data, const SearchResult &result,
                const Configuration &configuration, std::uint64_t seed);
/** @brief Ejecuta las instancias/rondas elegidas en el menu, sin parametros de GA/ABC/FFA. */
std::string executeMenu(const BRMenuSelection &selection);
}
#endif
