# Comparación de candidatos DBLF

Medición local del 24 de octubre de 2026, Windows, MinGW GCC 16.2.0,
`-std=c++14 -O2`. La medición final se ejecutó en serie, sin compilaciones
simultáneas. Comando para reproducir:

```powershell
./tests/benchmark-candidates.ps1
```

Se usaron las primeras 10 instancias de cada dataset P1A2–P5A2, las semillas
123 y 456, y rotaciones ZERO/TWO/SIX: 300 casos por ronda. Cada política ejecutó
un calentamiento y tres rondas medidas. Las permutaciones y orientaciones son
idénticas entre políticas. El tiempo incluye únicamente DBLF; excluye lectura,
generación de cromosomas y escritura de CSV. La ocupación es la media aritmética
de las fracciones de volumen ocupado, no la suma de volúmenes de distintos bins.

| Política | Mediana por 300 evaluaciones | Ocupación media | Pico del conjunto de trabajo |
| --- | ---: | ---: | ---: |
| Tres puntos (`legacy`) | 127.58 ms | 43.94 % | 14.79 MiB |
| Proyecciones (`projected`) | 1392.03 ms | 48.47 % | 14.89 MiB |
| Mejor de ambas (`best`) | 1462.38 ms | 49.59 % | 14.82 MiB |

`best` mejoró 159 de los 300 casos, empató en los otros 141 y nunca empeoró el
volumen. Ganó 5.65 puntos porcentuales de ocupación media y tardó aproximadamente
11.46 veces más que `legacy`. Este coste es la razón para conservar `legacy`
como predeterminado y ofrecer `best` como opción explícita. `projected` por sí
solo puede empeorar algunas disposiciones; los candidatos nuevos cambian las
decisiones posteriores del algoritmo voraz.

La memoria corresponde a `PeakWorkingSetSize` medido por Windows para todo el
proceso, incluida la carga de los datasets y el runtime. Las pequeñas diferencias
entre políticas no permiten concluir una reducción de memoria del algoritmo.
Las tres políticas tienen cota O(n³), pero constantes diferentes; las proyecciones
añaden como máximo 54 candidatos intermedios por caja y conservan espacio O(n).
La exploración sigue siendo heurística y no garantiza el óptimo.

## Contraejemplo unitario

En un contenedor 10×10×10, sin rotaciones, las cajas de
`ExperimentIntegrityTests.cpp::checkCandidates`, en su orden de IDs, producen
volumen 364 con `legacy` y 516 con `projected`/`best`: ocupación 36.4 % → 51.6 %.
La prueba también comprueba la selección del mayor volumen en los tres caminos
de DBLF y la validez geométrica de 100 casos aleatorios con seis orientaciones.

## Archivos

- Resumen conservado: `tests/benchmarks/candidates-p1-p5.csv`.
- Casos completos: `build/candidates/policy-{0,1,2}-cases.csv`.
- Rondas y memoria: `build/candidates/policy-{0,1,2}-summary.csv`.
- Resumen regenerable: `build/candidates/comparison.csv`.

Una primera exploración que generaba todas las intersecciones de caras resultó
demasiado costosa y se descartó. Las cifras de esta tabla corresponden únicamente
a la implementación final con proyecciones acotadas.
