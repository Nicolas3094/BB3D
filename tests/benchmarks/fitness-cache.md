# Caché de evaluaciones de fitness

Medición local del 24 de octubre de 2026, Windows, MinGW GCC 16.2.0,
`-std=c++14 -O2`. No hubo compilaciones simultáneas durante la medición.
Comando para reproducir:

```powershell
./tests/benchmark-cache.ps1
```

Se utilizó la primera instancia de P5A2 (151 cajas), con seis orientaciones y
DBLF `legacy`. La caché se limitó a 256 entradas y 8 MiB de payload. Cada carga
tuvo calentamiento y tres rondas medidas; se alternó el orden con/sin caché.
Los tiempos incluyen evaluación y almacenamiento de los individuos resultantes;
excluyen lectura del dataset, generación de los casos, escritura y comparación
de resultados. La comprobación compara exactamente fitness, solución y ambos
cromosomas; no utiliza una suma aproximada ni solo un hash.

| Carga | Solicitudes | Mediana sin caché | Mediana con caché | Aciertos | Evaluaciones DBLF con caché |
| --- | ---: | ---: | ---: | ---: | ---: |
| Repetida: 20 cromosomas únicos | 1000 | 1128.85 ms | 70.43 ms | 980 | 20 |
| Única: 1000 cromosomas distintos | 1000 | 1138.61 ms | 1191.28 ms | 0 | 1000 |
| GA: población 20, límite 30 iteraciones | 884 | 1127.26 ms | 1183.15 ms | 1 | 883 |

La carga repetida fue 16.03 veces más rápida. Sin repeticiones, la caché añadió
4.63 % de tiempo; en el GA medido añadió 4.96 % y solo reutilizó el 0.11 % de las
solicitudes. Por eso la aplicación deja la caché desactivada por defecto y permite
activarla con `--cache-entries 256 --cache-mib 8`. No es una mejora universal.

Para GA se utilizó C2 adaptativa, semilla 456, selección 0.85, cruce 0.75,
mutación de orientación 0.05 y `patience=0`. Los casos repetidos y únicos se
generaron con semilla 123. Los resultados exactos coincidieron en todas las
rondas, tanto con caché como sin ella.

En la carga repetida se almacenaron 20 entradas con un pico de 127.56 KiB de
payload. Las cargas única y GA alcanzaron las 256 entradas; en la única hubo
744 expulsiones y un pico de 1.375 MiB de payload, por debajo del presupuesto.
Payload incluye los bytes del contexto, las dos copias de cada clave y las
soluciones. El allocator, los nodos y los buckets requieren memoria adicional;
estas cifras no representan el RSS completo del proceso.

La caché se reinicia al comenzar cada experimento y al cambiar datos, dimensiones,
permisos u opciones de empaquetado. Un contexto o una entrada que no cabe se
evalúa normalmente sin almacenarse. Los datos se validan antes de devolver un
acierto. Las pruebas cubren invalidación, límites, expulsión LRU, fitness cero y
equivalencia de búsquedas completas de GA, ABC y Firefly.

Datos de las 18 mediciones: `tests/benchmarks/fitness-cache.csv`.
Salida regenerable: `build/cache/comparison.csv`.
