#ifndef BRKGA_AUDIT_HPP
#define BRKGA_AUDIT_HPP
#include "../readData/DataSetBPP.hpp"

namespace brkga2012
{
/**
 * @brief Audita limites, IDs/cantidades, tipos, orientaciones, intersecciones y volumen.
 * @note No llama a la geometria, rotacion ni comprobaciones de factibilidad del decoder.
 * @throws std::runtime_error Si el acomodo no corresponde a una solucion valida de data.
 */
void verifyPacking(const Bin &packing, const DatasetBinBacking &data);
}
#endif
