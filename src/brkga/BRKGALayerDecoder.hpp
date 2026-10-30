#ifndef BRKGA_LAYER_DECODER_HPP
#define BRKGA_LAYER_DECODER_HPP

#include "../readData/DataSetBPP.hpp"
#include "../spatial/MaximalSpace.hpp"
#include <array>
#include <vector>

namespace brkga2012
{
using RandomKeys = std::vector<double>;

/** @brief Tipo homogeneo y copias disponibles, sin perder sus IDs originales. */
struct BoxType
{
    ItemBin prototype;
    std::vector<int> itemIds;
};

/** @brief Capa rectangular completa: un eje de espesor y dos ejes de llenado. */
struct Layer
{
    int rotation = 1;
    Punto itemDimensions;
    std::array<std::int64_t, 3> counts{{1, 1, 1}};
    int firstAxis = 1;
    int secondAxis = 2;
    std::size_t boxCount = 0;
};

/** @brief Valida los datos comunes y agrupa copias de dimensiones y permisos identicos. */
std::vector<BoxType> buildBoxTypes(const DatasetBinBacking &data);

/** @brief Ordena las primeras M claves y devuelve el tipo de cada copia en BTPS. */
std::vector<std::size_t> decodeTypeSequence(const RandomKeys &keys, const std::vector<BoxType> &types);

/**
 * @brief Enumera hasta 36 capas factibles respetando orientaciones y cantidad.
 * @note Orden de planos/llenado de Fig. 8: YZ, ZY, YX, XY, ZX, XZ;
 * orientaciones en el orden 1..6 existente. No elimina capas coincidentes.
 */
std::vector<Layer> enumerateLayers(const BoxType &type, const MaximalSpace &space, std::size_t remaining);

/** @brief Aplica ceil(clave * numeroCapas), con extremos 0/1 definidos y indices desde cero. */
std::size_t layerIndex(double key, std::size_t layerCount);

/**
 * @brief Decodifica BRKGA-CLP-U: BTPS, VLT por posicion ordenada, capas y prioridad BBL.
 * @param[in,out] bin Contenedor inicialmente vacio, recibe solo cajas realmente colocadas.
 * @param types Tipos validados, en el orden devuelto por buildBoxTypes.
 * @param keys Vector de 2M claves finitas en [0,1]. No son genes de orientacion fija.
 * @throws ExecutionTimeLimit Conserva las cajas ya colocadas como acomodo parcial valido.
 * @note No exige soporte completo: MaxJoin pertenece a la variante S, no a esta U.
 */
void decodeLayers(Bin &bin, const std::vector<BoxType> &types, const RandomKeys &keys);
}
#endif
