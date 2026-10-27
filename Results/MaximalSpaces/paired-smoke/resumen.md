# Comparacion breve: DBLF y Espacios Maximos

Cuatro genomas identicos por decodificador en la primera instancia de BR1 y BR15. Semilla 123, seis orientaciones legales, C++14 -O3 -DNDEBUG. Todos los 16 acomodos pasaron la auditoria independiente.

| Instancia | Decodificador | Ocupacion media (%) | Tiempo medio por evaluacion (ms) | Maximo de espacios retenidos |
| --- | --- | ---: | ---: | ---: |
| BR1/1 | dblf | 65.407 | 6.887 | no aplica |
| BR1/1 | maximal-spaces | 68.632 | 154.107 | 445 |
| BR15/1 | dblf | 69.568 | 11.314 | no aplica |
| BR15/1 | maximal-spaces | 71.803 | 633.051 | 679 |

Espacios Maximos mejoro la ocupacion en los ocho pares de este muestreo, pero tuvo un costo de evaluacion mayor. Esto no demuestra superioridad en busqueda completa ni generaliza a otras instancias o semillas.

Tiempos de una sola muestra por evaluacion y con el lote beam ejecutandose simultaneamente: son descriptivos y no sirven para comparar rendimiento aislado. No se ejecuto la bateria BR completa ni una busqueda de 30 segundos adicional.

El maximo contabiliza espacios retenidos despues de podar, incluido el inicial; no suma volumen ni mide el pico transitorio de losas.

Regresion: los ocho archivos de acomodo DBLF son identicos byte a byte a una compilacion del DBLF anterior de HEAD (commit registrado en manifest.json). Ver dblf-baseline-verification.json. Los tests de geometria, entradas, contacto y evaluacion tambien se ejecutaron.
