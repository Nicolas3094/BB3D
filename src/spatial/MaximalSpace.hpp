#ifndef MAXIMAL_SPACE_HPP
#define MAXIMAL_SPACE_HPP

#include "Punto.hpp"
#include <vector>

/**
 * @brief Cuboide libre delimitado por esquinas minima y maxima.
 *
 * Representa el intervalo semiabierto [minimum, maximum): tocar una cara no
 * constituye interseccion de volumen. Los espacios pueden solaparse entre si;
 * sus volumenes no se suman para calcular la ocupacion del contenedor.
 */
struct MaximalSpace
{
    Punto minimum;
    Punto maximum;
};

/** @brief Indica si las tres dimensiones del espacio son estrictamente positivas. */
bool hasPositiveDimensions(const MaximalSpace &space);

/**
 * @brief Comprueba si dos cuboides comparten volumen interior.
 * @return false cuando solo se tocan por una cara, arista o vertice.
 */
bool spacesIntersect(const MaximalSpace &first, const MaximalSpace &second);

/**
 * @brief Comprueba si outer contiene completamente a inner, incluida la igualdad.
 * @param outer Espacio que podria contener al otro.
 * @param inner Espacio cuya contencion se comprueba.
 */
bool containsSpace(const MaximalSpace &outer, const MaximalSpace &inner);

/**
 * @brief Resta un cuboide ocupado de un espacio libre sin modificar las entradas.
 * @param space Espacio libre original, de dimensiones positivas.
 * @param occupied Cuboide ocupado que se desea excluir, de dimensiones positivas.
 * @return Hasta seis losas positivas, o el espacio original si no hay interseccion.
 * @note Las losas conservan el alcance original en los otros ejes y pueden
 * solaparse. La union representa la parte libre; no es una particion disjunta.
 */
std::vector<MaximalSpace> splitFreeSpace(const MaximalSpace &space, const MaximalSpace &occupied);

/**
 * @brief Elimina espacios degenerados, duplicados y contenidos completamente.
 * @param[in,out] spaces Lista que se sustituye por los espacios retenidos.
 * @note Conserva la primera copia de cada duplicado. Ordena por esquina minima
 * X/Z/Y y despues maxima X/Z/Y para mantener desempates deterministas.
 * @throws ExecutionTimeLimit Si vence el presupuesto activo; la lista de entrada
 * conserva su estado anterior porque la sustitucion se realiza al terminar.
 */
void removeRedundantSpaces(std::vector<MaximalSpace> &spaces);

/**
 * @brief Excluye una caja de todos los espacios libres y poda el resultado.
 * @param[in,out] spaces Lista completa de espacios libres, que puede tener solapes.
 * @param occupied Cuboide ocupado por la caja recien colocada.
 * @note Actualiza todos los espacios intersectados, no solo el usado al colocar.
 * @throws ExecutionTimeLimit Si vence el presupuesto activo; no publica una lista
 * parcialmente actualizada.
 */
void subtractOccupiedSpace(std::vector<MaximalSpace> &spaces, const MaximalSpace &occupied);

#endif
