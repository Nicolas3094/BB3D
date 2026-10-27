#ifndef MAXIMAL_SPACES_DECODER_HPP
#define MAXIMAL_SPACES_DECODER_HPP

#include "../binPacking/Bin.hpp"

/** @brief Estadisticas locales de una evaluacion del decodificador. */
struct MaximalSpacesStats
{
    /// Maximo de espacios retenidos tras podar, incluido el espacio inicial.
    std::size_t peakRetainedSpaces = 0;
};

/**
 * @brief Empaca cajas individuales ya orientadas usando Espacios Maximos y DBL.
 * @param[in,out] bin Contenedor inicialmente vacio; recibe las cajas y su volumen.
 * @param orientedItems Copia de las cajas en el orden de insercion solicitado.
 * @param[out] stats Estadisticas opcionales; nullptr desactiva su recopilacion.
 *
 * Prueba las esquinas minimas de los espacios donde cabe cada caja y elige la
 * menor X, despues Z y despues Y. Omite cajas inviables o con orientacion
 * prohibida y continua con las siguientes. No exige soporte ni cambia la
 * orientacion recibida, salvo normalizarla al configurar ZERO_WAY.
 * @note La ocupacion se obtiene solo de las cajas cargadas en bin. No se suman
 * volumenes de espacios ni se aplica la compactacion de DBLF.
 * @throws std::invalid_argument Si las entradas incumplen las validaciones comunes.
 * @throws ExecutionTimeLimit Si vence el presupuesto activo. Las cajas que ya
 * contiene bin siguen formando un acomodo valido; stats puede ser parcial.
 */
void maximalSpacesDecode(Bin &bin, LoadedBins orientedItems, MaximalSpacesStats *stats = nullptr);

/**
 * @brief Interpreta el genoma y delega el empacado en la variante de cajas orientadas.
 * @param[in,out] bin Contenedor vacio con el modo de rotacion configurado.
 * @param itemIds Identificadores de cajas en el orden del genoma, sin repetidos.
 * @param rotationGenes Orientacion de cada posicion; misma longitud que itemIds.
 * @param itemsById Datos indexados por identificadores consecutivos desde 1.
 * @param[out] stats Estadisticas opcionales de la evaluacion.
 * @note Valida ambos cromosomas antes de acceder por ID y trabaja con copias de
 * las cajas. No selecciona orientaciones distintas de las indicadas por el genoma.
 * @throws std::invalid_argument Si las entradas incumplen las validaciones comunes.
 * @throws ExecutionTimeLimit Si vence el presupuesto activo.
 * @see maximalSpacesDecode(Bin &, LoadedBins, MaximalSpacesStats *)
 */
void maximalSpacesDecode(Bin &bin, const Chromosome &itemIds, const Chromosome &rotationGenes,
                         const LoadedBins &itemsById, MaximalSpacesStats *stats = nullptr);

#endif
