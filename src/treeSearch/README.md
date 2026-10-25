# Método independiente SCLP de Guesser, de Queiroz y Miyazawa (2026)

Se agrega junto a `src/metaheuristics`, en un ejecutable separado. No se cambian
GA, ABC, Firefly ni DBLF. En el menú se selecciona con **5=articulo SCLP**;
la opción 4 sigue comparando solo GA/ABC/FFA. No entra en `--algorithm all`.
El ejecutable principal del proyecto sigue funcionando como antes.

El menú SCLP conserva la selección de archivo y problemas, pide rondas y nombre
de sesión y usa 30 segundos CPU por corrida. Las rotaciones siguen todos los
permisos BR; no pide mutación, reemplazo, población ni candidatos DBLF.
Lanza el ejecutable independiente mediante Windows PowerShell incluido en Windows, hereda la terminal y
guarda CSV/resumen de sesión y una subcarpeta por corrida con acomodo y verificación.
Se comprueba que el ejecutable SCLP esté compilado antes de crear la sesión.
Las rondas son repeticiones temporales del mismo método determinista; no semillas
aleatorias independientes. Las sesiones SCLP no usan los checkpoints del GA.

Fuentes: [artículo](https://doi.org/10.1016/j.ejor.2026.01.039),
[depósito solicitado](https://zenodo.org/records/19247237), archivo
`3d_sclp_source_code.zip`. La copia íntegra sin editar está en `third_party/sclp`,
con licencia MIT, NOTICE y licencias de dependencias. `provenance.json` registra
el archivo de origen y sus hashes. No se utiliza una reimplementación de la búsqueda.

## Compilar y ejecutar en Windows

Desde la raíz del proyecto, con PowerShell, MinGW y CMake existentes:

```powershell
./src/treeSearch/Build-SCLP.ps1
./tests/sclp-reference.ps1
./src/treeSearch/Run-SCLP.ps1 -Dataset BR15 -Instance 1 -Seconds 30
```

La primera compilación descarga Boost 1.83 (aproximadamente 145 MB comprimido)
dentro de `build/sclp-original`, verifica SHA256 y extrae solo encabezados y fuentes
necesarias. No instala componentes globales. Permite `-Compiler <ruta>` y `-Jobs 2`.
`Run-SCLP.ps1` crea una carpeta nueva bajo `Results/TreeSearch` y muestra el porcentaje
final verificado en la terminal. `-OutputDirectory` permite elegir una carpeta nueva.
La posición de instancia comienza en 1. La conversión verifica el formato BR completo.

Se generan `input.json`, `packing.json`, `run.json`, `verification.json` y
`terminal.txt`. El acomodo contiene tipo original (índice base 0), orientación,
posición y dimensiones de cada caja. `run.json` contiene argumentos completos,
tiempos, hashes del ejecutable y datos y resultado de verificación.

## Compilación original y adaptación mínima

El README original pide GCC 14, CMake 3.30, make 4.4 y Boost 1.83; usa C++20.
Su CMake admite desde 3.26. `make` compila Debug; para rendimiento los autores
indican `make BUILD_TYPE=Release`. El artículo describe GCC 13.3 sobre Ubuntu 24.04.1,
Xeon E5-2630 v4 a 2.2 GHz y 64 GB; no son el hardware ni compilador de esta prueba.

Aquí: MinGW GCC 16.2.0, CMake 4.4.4, Release `-O3 -DNDEBUG -std=gnu++20`, sin
fast-math, OpenMP ni paralelización de la búsqueda. `--parallel 2` solo paraleliza
la compilación. CMake conserva además el CLI original como `sclp_original.exe`.
El adaptador `sclp_reference.exe` llama a los mismos `Problem::initialize/solve` y
`Logs`, añadiendo únicamente IDs, orientaciones y tiempos al archivo y consola.

Boost timer se compila desde las fuentes oficiales y flat_map/flat_set y las
tablas hash originales siguen siendo Boost; no se sustituyen por contenedores STL.
El CMake independiente evita exigir una instalación de Boost con configuración
CMake y no aplica `-Werror/-pedantic-errors` originales a este compilador distinto.
Las dependencias fmt/json/cxxopts son las incluidas por los autores.

En glibc x86_64 `int_fast16_t` suele ser de 64 bits; en MinGW es de 16 bits.
El original no compila por deducción incompatible en `std::max(short, int)` en
`Packing::add`. Se genera un encabezado de compatibilidad solo en `build`, que
cambia `CoordinateType` a `int64_t`, recuperando el rango de Linux. No se edita el
original. `Quantity` queda como en MinGW: se rechazan cantidades mayores a 32767.
Esto no afecta BR15/1, cuyas cantidades y permisos se contrastaron con el ejemplo
oficial tipo por tipo. Ninguna modificación cambia criterios de fitness o búsqueda.

## Configuración publicada y restricciones

Se usa **H / two-stage-config**, tabla 3 del artículo, no los defaults del CLI
(que corresponden a Araya y desactivan el cambio de etapa):

| Parámetro | Primera etapa | Segunda etapa |
| --- | ---: | ---: |
| alpha | 3.7015 | 2.0934 |
| beta | 2.6631 | 1.4609 |
| gamma | 0.3968 | 0.4732 |
| delta | 0.0139 | 0.0366 |

Umbral `configuration_switch_threshold=0.6740`; `skip_type=strict` habilita
memoización de estados intermedios, como en el script `solve` de los autores.
Ramificación inicial 4, máxima 10000; máximo de bloques 10000; densidad automática:
1.00 con menos de 30 tipos, 0.98 para 30 o más (BR15/1: 100 tipos). Se conserva
el crecimiento original `round(sqrt(2)*branching_factor)` y el manejo de espacios
vacíos máximos, generación guillotina y completado voraz. No existe parámetro de
semilla de búsqueda en este código. Una corrida temporal no garantiza igualdad
bit a bit entre plataformas, compiladores o cargas del equipo.

BR expresa qué dimensión original puede quedar vertical. El original usa Z vertical:
XYZ/YXZ usan permiso Z, ZYX/YZX permiso X, XZY/ZXY permiso Y. No se habilitan
rotaciones prohibidas. Las cantidades se mantienen por tipo, sin expandir ni
duplicar oferta. Se maximiza volumen; el campo `profit` exportado es informativo.
No se agregan requisitos de estabilidad, soporte completo o peso ausentes del modelo.

El original comprueba cantidad en generación y estados, orientaciones al generar
bloques y límites al ajustar bloques/espacios. `AllConstraints` verifica cantidad,
orientación, límites y ausencia de intersecciones sobre cajas individuales finales.
Además, `Verify-Packing.ps1` realiza una comprobación independiente del JSON:
índice y orientación permitida por tipo, dimensiones rotadas exactas, cantidad,
coordenadas enteras y no negativas, contención y todas las parejas de cajas.
Contacto de caras está permitido; intersección con volumen positivo se rechaza.
Se recalcula volumen ocupado y se contrasta con el declarado.

## Temporizador y comparación

`Timer.cpp` usa Boost CPU **usuario + sistema del proceso**, no reloj de pared.
`Problem::initialize` lee el JSON antes de iniciar el presupuesto; después incluye
generación de bloques, inicialización de estructuras y búsqueda. `solve` pasa el
tiempo restante a DoubleEffort y este a BsgSearch. Logs y exportación quedan fuera.
La generación de bloques no consulta el plazo internamente: consume presupuesto
pero no puede interrumpirse por tiempo. Los chequeos de búsqueda son cooperativos,
no en cada operación voraz; puede existir sobrepaso. Se conserva ese comportamiento.

La implementación es serial: un hilo de búsqueda. Windows/Boost mide CPU con
granularidad visible (en esta prueba pasos de 15.625 ms). Se guardan CPU, tiempo
interno de pared y duración externa del proceso, sin presentarlos como equivalentes.
Tu modo de 30 segundos mide pared y excluye preparación; no debe afirmarse una
comparación temporal estricta sin igualar esos alcances y compilar ambos con
optimización. Esta integración prepara una referencia independiente; no altera el
temporizador de los algoritmos existentes ni ejecuta un ranking de calidad con una
sola instancia.

## Prueba realizada

BR15, primera instancia, contenedor 587 x 233 x 220. **Ocupación 95.5059651800 %**,
111 cajas, volumen 28 737 382 / 30 089 620; **30.000000 s CPU** incluyendo
inicialización (0.156250 s) y **30.237874 s externos**. Resultado verificado con
6 105 comprobaciones de parejas y todos los controles del original aprobados.

Archivos: `Results/TreeSearch/BR15-1-20261024-220218-2b93e8/`.
No es una reproducción del promedio del artículo ni de su computadora.
Las pruebas independientes comparan el driver original y el anotado con una única
anchura de beam (para evitar divergencia temporal): producen exactamente el mismo
acomodo. También rechazan ocho variantes de resultados corruptos. Esta prueba
auxiliar usa anchura máxima 4; la corrida principal usa 10000 como se documentó.
