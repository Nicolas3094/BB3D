# Comparacion DBLF y contacto

BR0/1 y BR15/1; semillas 123 y 456. Compilacion -O3 -DNDEBUG; un hilo; cache desactivada.
Mismos puntos legacy y genes de orientacion en la evaluacion pareada. Contacto evalua todos los candidatos factibles tras compactarlos.

## Mismos genomas

| Dataset | Genomas pareados | Contacto mejora / igual / empeora | Cambio medio (pp) | Tiempo medio DBLF (us) | Tiempo medio contacto (us) |
| --- | ---: | --- | ---: | ---: | ---: |
| BR0 | 32 | 22 / 1 / 9 | 0.9718 | 7189.6 | 14792.0 |
| BR15 | 32 | 13 / 0 / 19 | -0.2152 | 5141.2 | 13133.2 |

## Busqueda completa: 30 segundos por corrida

| Dataset | Algoritmo | Semillas | Ocupacion media DBLF (%) | Ocupacion media contacto (%) | Cambio (pp) | Evaluaciones medias DBLF / contacto |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| BR0 | GA | 2 | 79.3805 | 78.5621 | -0.8184 | 4198 / 1473 |
| BR0 | ABC | 2 | 78.9713 | 63.8317 | -15.1396 | 7376 / 2468 |
| BR0 | FFA | 2 | 81.4264 | 81.0172 | -0.4092 | 3846 / 1303 |
| BR15 | GA | 2 | 78.3633 | 79.5081 | 1.1448 | 5860 / 1708 |
| BR15 | ABC | 2 | 79.2895 | 79.1255 | -0.1640 | 5862 / 1874 |
| BR15 | FFA | 2 | 78.9488 | 79.6873 | 0.7385 | 5970 / 1723 |

## Validacion y alcance

Cada acomodo fue auditado sin usar las funciones de colision, rotacion ni factibilidad del decodificador: IDs/cantidades, genes y permisos, dimensiones rotadas, limites y todas las parejas. Se recalculo el volumen.
Se conservaron los genomas completos, las secuencias de insercion de incumbentes y los acomodos en CSV. Las evaluaciones interrumpidas se identifican como parciales. La reconstruccion del incumbente coincide exactamente con el fitness registrado.
Las evaluaciones por corrida incluyen la evaluacion intentada al vencer el plazo. La comprobacion y exportacion quedan fuera del tiempo de busqueda.
Son dos instancias y dos semillas: un experimento piloto, sin significancia estadistica ni superioridad general demostrada. No se eligio el mejor decodificador automaticamente.
Los tiempos individuales son mediciones locales de pared; alternar el orden reduce sesgos, pero no elimina ruido del sistema. Las primeras corridas coincidieron con compilaciones de pruebas en otros procesos; las cifras de rendimiento son orientativas.
Contacto no impone soporte completo, no genera proyecciones y no elige orientaciones nuevas. Un mayor contacto inmediato puede cerrar huecos y empeorar el acomodo final.
