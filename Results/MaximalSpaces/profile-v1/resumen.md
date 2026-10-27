# Perfil de Espacios Maximos: BR1/1 y BR15/1

Se reutilizaron exactamente los ocho genomas de paired-smoke: cuatro por instancia,
con el orden y las orientaciones ya guardados. C++14, GCC 16.2.0, -O3 -DNDEBUG.
Una pasada de calentamiento por version, excluida, seguida de cinco repeticiones.
Se alterno el orden normal/instrumentada: 20 evaluaciones medidas por instancia y
version, 80 en total. Lectura, auditoria y exportacion estan fuera de los tiempos.

## Tiempo por fase

Medias por evaluacion de la version instrumentada. Los porcentajes son la suma
del tiempo de la fase dividida por la suma del tiempo total, no medias de porcentajes.

| Fase | BR1/1 (ms) | BR1/1 (%) | BR15/1 (ms) | BR15/1 (%) |
| --- | ---: | ---: | ---: | ---: |
| Buscar posicion DBL | 0.424 | 0.599 | 0.814 | 0.321 |
| Dividir y ensamblar espacios | 1.398 | 1.977 | 2.484 | 0.980 |
| Poda: contencion y duplicados | 68.315 | 96.609 | 249.036 | 98.287 |
| Ordenar espacios retenidos | 0.271 | 0.384 | 0.696 | 0.275 |
| Resto: validacion, orientacion, insercion y costes no incluidos en las fases | 0.305 | 0.431 | 0.346 | 0.136 |
| Total instrumentado | 70.713 | 100 | 253.376 | 100 |

La poda usa el mismo bucle para contencion y duplicados. Medirlos por separado
requeriria alterar ese bucle o cronometrar cada comparacion, distorsionando la
medicion. Se reportan juntos y se distinguen las eliminaciones con contadores.
La poda incluye las comprobaciones cooperativas de plazo dentro de ese bucle;
no se ha separado su coste del de los predicados geometricos.

## Trabajo realizado

| Indicador | BR1/1 | BR15/1 |
| --- | ---: | ---: |
| Comprobaciones de contencion medias por evaluacion | 3,771,663.75 | 14,182,997.25 |
| Espacios eliminados por contencion, media | 1,915 | 2,660.5 |
| Duplicados eliminados directamente, media | 0 | 0 |
| Maximo de espacios antes de podar | 493 | 788 |
| Maximo retenido despues de podar | 445 | 679 |
| Espacios intersectados / espacios visitados al actualizar | 7.900 % | 5.218 % |

Cada llamada al predicado containsSpace cuenta como una comprobacion, incluida
la comprobacion inversa usada para distinguir igualdad y contencion estricta.
Cero duplicados eliminados directamente no demuestra que nunca se generaran
duplicados: una region puede eliminarse antes por contencion en otra mayor.

## Coste de instrumentacion y limites

| Version | BR1/1 media (ms) | BR15/1 media (ms) |
| --- | ---: | ---: |
| Normal, sin instrumentacion | 67.266 | 242.436 |
| Instrumentada | 70.713 | 253.376 |
| Diferencia observada | +5.12 % | +4.51 % |

El lote beam continuo activo; no se suspendieron trabajos del usuario. Estas
mediciones describen este muestreo, no rendimiento aislado. La diferencia incluye
instrumentacion y variacion de carga. Los tiempos absolutos difieren de la primera
comparacion (154/633 ms), sin haber optimizado el algoritmo: no son una mejora de
rendimiento atribuible al codigo. El dato robusto para elegir donde trabajar es
que la poda domina las fases instrumentadas y realiza millones de comparaciones.

CPU de proceso usuario+sistema tambien se guarda en measurements.csv, como dato
complementario; su contabilizacion en Windows tiene granularidad gruesa. Los
tiempos por fase usan steady_clock y son excluyentes. La division incluye las
asignaciones/copia de regiones; la liberacion de temporales que quede fuera de los
alcances se incluye en el resto. No se cambio la regla de poda ni el orden DBL.

## Verificacion y reproduccion

Los 80 acomodos medidos pasaron el auditor independiente y coinciden byte a byte
con los archivos anteriores; tambien coinciden ocupacion y pico de espacios
retenidos. Pasaron los tests MaximalSpacesTests con instrumentacion activada,
incluidos cortes por tiempo, permisos y las tres metaheuristicas.

```powershell
./experiments/Profile-MaximalSpaces.ps1 -OutputDirectory Results/MaximalSpaces/otro-perfil
```

El script compila normal e instrumentada con el mismo compilador y opciones;
solo la segunda define BB3D_PROFILE_MAXIMAL_SPACES. Las compilaciones normales
de la aplicacion no incluyen contadores ni relojes de perfil. No se lanza la
bateria BR completa ni busquedas de metaheuristicas. measurements.csv conserva
las muestras individuales, summary.csv los agregados de esta corrida y
manifest.json los hashes de los ejecutables y entradas.
