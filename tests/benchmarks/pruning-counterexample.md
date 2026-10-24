# Contraejemplo de poda de puntos DBLF

Contenedor 8 x 8 x 8, sin rotacion, orden de cajas 1..10. Todas las orientaciones
originales estan permitidas.

| ID | X | Y | Z |
| --- | ---: | ---: | ---: |
| 1 | 2 | 6 | 6 |
| 2 | 2 | 5 | 3 |
| 3 | 6 | 4 | 6 |
| 4 | 1 | 1 | 1 |
| 5 | 3 | 1 | 4 |
| 6 | 6 | 2 | 3 |
| 7 | 1 | 4 | 3 |
| 8 | 1 | 5 | 4 |
| 9 | 1 | 3 | 5 |
| 10 | 5 | 3 | 3 |

La poda antigua carga IDs 1,2,4,5,6,7,8,9: 8 cajas y volumen 198.
Conservar puntos distintos permite cargar tambien ID 10 en (3,5,3): 9 cajas
y volumen 243. En este caso, la ocupacion pasa de 38.67% a 47.46%.

Las posiciones de las primeras ocho cajas coinciden:

| ID | Posicion |
| --- | --- |
| 1 | (0,0,0) |
| 2 | (2,0,0) |
| 4 | (0,6,0) |
| 5 | (0,7,0) |
| 6 | (2,5,0) |
| 7 | (2,0,3) |
| 8 | (3,0,3) |
| 9 | (2,4,3) |

`tests/DBLFInputTests.cpp` fija este caso y exige el resultado corregido tanto
en DBLF como en DBLFQueue. `tests/FindPruningCase.cpp` conserva la busqueda
reproducible que lo encontro, con semilla 20261023 y una implementacion de
referencia de la poda antigua; el caso aparece en el ensayo 29.

Un contraejemplo local tambien comprueba que (0,0,0) y (0,2,0) no son
intercambiables: una caja unitaria cargada en el primer punto bloquea otra
caja unitaria alli, pero no en el segundo. Compartir X y Z no basta para
eliminar el segundo punto.

Ambas colas ahora eliminan solo duplicados exactos. Conservar mas puntos puede
aumentar el trabajo y cambiar las decisiones de la heuristica; este caso no
garantiza mejorar todas las instancias. La generacion sigue siendo de tres
puntos por caja y no constituye una enumeracion completa de huecos utiles.

Para ejecutar las regresiones y validaciones:

```powershell
.\tests\run.ps1
```
