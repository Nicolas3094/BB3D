# Compactacion por contacto: comparacion P5A2

La compactacion conserva el orden X, Z, Y. En cada eje recorre las cajas
cargadas y encuentra la cara mas cercana que bloquea el movimiento hacia cero.
Solo considera cajas detras de la posicion actual cuyas proyecciones se
intersectan estrictamente en los otros dos ejes. El contacto entre caras esta
permitido. Si no hay obstaculos, la coordenada final es cero.

Para una posicion inicial valida, cada eje se resuelve con un recorrido de las
k cajas: O(k) tiempo y O(1) espacio auxiliar, sin depender de la distancia D.
La implementacion anterior hacia O(k * D) trabajo. Se conserva la regla
existente de omitir compactacion cuando hay como maximo una caja cargada.

DBLF sobre un contenedor inicialmente vacio tiene una cota O(n^3): hasta O(n)
puntos por caja y O(n) comprobaciones de colision por punto. La compactacion
se realiza una vez por caja insertada. La antigua cota considerando el recorrido
unitario era O(n^3 + n^2 * D). Estas son cotas, no tiempos medidos.

## Medicion

- Dataset: `Instance/P5A2.csv`, las 100 instancias, 151 cajas por instancia.
- Cada ronda: 300 evaluaciones DBLF, una por instancia y modo de 0, 2 y 6 rotaciones.
- Cinco rondas por version, alternando antes/despues y despues/antes.
- Cada proceso realiza primero una ronda de calentamiento no medida.
- Ordenes y rotaciones generados con `std::mt19937`, semilla 20261023.
- Compilador: MinGW-w64 g++ 16.2.0; opciones `-std=c++14 -O2`.
- Base: commit `07efb60410e41a81e572ca22e3a4c86c4f9114a7`. Ya incluye la
  eliminacion de copias del contenedor. Se sustituye solo la unidad de
  compilacion `ItemBinOperators.cpp` entre los dos ejecutables.
- Lectura del dataset, generacion de casos y escritura de resultados fuera
  del intervalo medido. Se mide DBLF y la retencion de sus contenedores resultado.

| Ronda | Antes (ms) | Despues (ms) |
| --- | ---: | ---: |
| 0 | 339.6192 | 281.9574 |
| 1 | 310.7592 | 242.1946 |
| 2 | 309.7558 | 236.7537 |
| 3 | 309.6681 | 248.0618 |
| 4 | 328.3524 | 254.6556 |
| Mediana | 310.7592 | 248.0618 |

Reduccion de tiempo de la mediana: **20.18 %**. Factor antes/despues: **1.25x**.
Promedio por evaluacion derivado de la mediana: 1.036 ms antes, 0.827 ms despues.
Estos valores dependen del equipo y su carga; no miden una ejecucion completa
de GA, ABC o Firefly.

En todas las rondas coinciden los IDs, rotaciones, posiciones y dimensiones
de las cajas, en el mismo orden. Cada ronda inserta 42443 cajas en total.
SHA256 del archivo de posiciones:
`EA0A855645B56F73768169CA30A985A8D424460773335A329971878C00C9B8A0`.

Ademas pasan 1986 casos sinteticos reproducibles que comparan la nueva
compactacion contra la implementacion unitaria, las pruebas de geometria
y DBLF, un recorrido de 100000000 unidades, la suite previa y la compilacion
completa del proyecto.

## Reproducir

Desde la raiz del repositorio:

```powershell
.\tests\run.ps1
.\tests\benchmark-compaction.ps1
```

El benchmark escribe `build/compaction/comparison.csv` y los archivos de
posiciones de cada ronda. Verifica sus hashes y falla si alguna posicion cambia.
Los resultados originales estan en `tests/benchmarks/compaction-p5a2.csv`.
