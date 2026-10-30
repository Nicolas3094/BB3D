# Comparacion BR1-BR17 con 30 segundos CPU

Este protocolo reemplaza la **vista** de `tabla.md` por una comparacion con el
mismo presupuesto CPU para GA, ABC, FFA, SCLP y BRKGA-U. No borra resultados de
100 iteraciones ni los usa como si fueran corridas de 30 segundos.

## Configuracion conservada

GA/ABC/FFA mantienen exactamente el perfil de `equal-iterations.json`:

| Parametro | Valor |
| --- | --- |
| Decodificador | Contacto, tres puntos originales, sin proyecciones |
| Poblacion | 20 |
| Mutacion | C2, probabilidad fija 1 |
| Orientaciones | Seis, respetando permisos de cada tipo |
| Mutacion de orientacion | 0.05 |
| Reemplazo | false: modalidad por adicion |
| Cache | Desactivada |
| Semilla raiz | 123, derivada por ID y semilla de la instancia |
| GA | 20 intentos de apareamiento, cruce 1, seleccion 0.85 |
| ABC | 10 sitios, 5 elites, 4 abejas elite, 2 no elite, 10 sitios globales |
| FFA | 2 movimientos por luciernaga |

Se conserva el esfuerzo de 40 evaluaciones por iteracion completa de las tres
metaheuristicas, pero **no** se fuerza que completen las mismas iteraciones:
cada una trabaja hasta alcanzar el plazo CPU o el 100% de ocupacion. No se
aplican el tope de 100 iteraciones ni la parada por estancamiento.

SCLP y BRKGA-U conservan los mismos ejecutables congelados de los lotes anteriores.
Esto preserva sus configuraciones publicadas y las diferencias ya documentadas.
BRKGA mantiene tres poblaciones de 20M y el maximo de 500 generaciones; puede
consumir el presupuesto inicializando poblaciones sin llegar al cruce.

## Presupuesto y comparabilidad

Todos reciben hasta **30 segundos CPU de proceso, usuario + sistema**, incluyendo
inicializacion y busqueda; SCLP incluye generacion de bloques. Lectura de archivos,
auditoria independiente y exportacion quedan fuera. Se guarda tiempo transcurrido
aparte. No se instala ninguna dependencia ni se cambian los operadores.

Las comprobaciones del plazo son cooperativas. Puede haber pequenos sobrepasos
por granularidad CPU de Windows o fases no interrumpibles del solver original.
La politica de consulta CPU de las metaheuristicas coincide con la de BRKGA:
reloj auxiliar cada 256 checkpoints y CPU como maximo una vez por milisegundo.

La opcion `--execution 30s` del main mantiene su reloj de pared habitual. Esta
comparacion usa el ejecutor especifico `MetaTimeBenchmark.cpp`, que inyecta el
reloj CPU en la estrategia temporal compartida. El plazo predeterminado de esa
estrategia sigue siendo 30000 ms; el plazo configurable permite smoke tests cortos.

Es un presupuesto comun, no una garantia de igual numero de evaluaciones ni de
reproduccion de los resultados publicados. Cada caso usa un hilo; hay 15 casos
simultaneos con afinidades consultadas a Windows, como en los lotes anteriores.
Los resultados pueden variar con la carga del sistema aunque se conserve semilla.

## Alcance y preservacion

- 17 clases, 100 instancias distintas por clase y cinco algoritmos: **8500 casos**.
- Se reutilizan los 1500 casos BR1-BR15 de SCLP y los 1500 de BRKGA-U que ya se
  ejecutaron a 30 segundos CPU. Se comprueban datos, binarios, auditorias y semillas.
- Se ejecutan **5100 casos nuevos** de GA/ABC/FFA y **400** de SCLP/BRKGA para
  BR16-BR17: **5500 corridas nuevas** con la configuracion predeterminada.
- Las referencias antiguas y sus acomodos no se modifican. Cada fila reutilizada
  indica `reused=1` y apunta a la carpeta de su acomodo original, que debe conservarse.
- El CSV N100 se copia y sus hashes se guardan en `manifest.json`. Las tablas previas
  quedan en `reference-before-cpu30/`; el lote N100 original permanece intacto.
- El nuevo lote tiene carpeta, manifiesto, binarios, workers y plan propios. No
  reutiliza checkpoints por iteraciones ni por reloj de pared.

## Ejecutar y reanudar

Desde la raiz del proyecto:

```powershell
./experiments/Build-TimeComparison.ps1
./experiments/Run-TimeComparison.ps1 -OutputDirectory Results/TimeComparison/BR1-BR17-CPU30 -Workers 15 -Seconds 30 -RootSeed 123 -SkipBuild
```

Repetir el segundo comando reanuda omitiendo casos completos y verifica que no
cambiaron entradas, referencias, plan, binarios ni scripts congelados. Los intentos
incompletos se conservan. El archivo `STOP` detiene ordenadamente tras cada caso;
debe retirarse expresamente antes de reanudar.

```powershell
./experiments/Show-EqualPool.ps1 -Directory Results/TimeComparison/BR1-BR17-CPU30 -Watch
```

`tabla.md` muestra ocupacion y n/100 por celda, y al final los promedios sobre
resultados individuales y n/1700 por algoritmo. Solo entran casos con auditoria
valida. Los promedios parciales pueden corresponder a subconjuntos distintos.
La misma vista se publica en las rutas de tabla de SCLP y BRKGA usadas anteriormente.

## Verificacion previa

El build recompila el nucleo completo y comprueba el plazo configurable, los
30000 ms por defecto, los cortes parciales, la igualdad anterior de evaluaciones
por iteracion y los resultados del decodificador de contacto.

El smoke test del pool ejecuto los cinco algoritmos en BR16/1 y BR17/1 con un
segundo CPU por caso. Los diez acomodos fueron auditados y la reanudacion no
reescribio sus resultados. Estos resultados cortos quedan en `build/` y no entran
en la tabla principal. Para reproducir un smoke test, usar una carpeta nueva:

```powershell
./experiments/Run-TimeComparison.ps1 -OutputDirectory build/time-comparison/otro-smoke -Workers 5 -Seconds 1 -TaskLimit 10 -SkipBuild
```

Las metaheuristicas verifican el acomodo reconstruido con dos auditores
independientes y exigen que su volumen corresponda al fitness incumbente. Guardan
genoma completo e inserciones efectivas para soportar evaluaciones parciales.
SCLP y BRKGA conservan sus auditores anteriores. Las evaluaciones iniciadas se
registran como tales: no se presentan como iteraciones o evaluaciones completas.

## Reinicio completo con ocho procesos

El lote `Results/TimeComparison/BR1-BR17-CPU30-eight-workers-fresh` ejecuta los
8500 casos de nuevo, sin importar resultados anteriores. Usa un procesador logico
por cada uno de los ocho nucleos fisicos. Los lotes anteriores se conservan.
La menor concurrencia no garantiza una temperatura determinada; debe vigilarse.
El archivo `execution-*.json` registra afinidades y concurrencia de cada arranque.
Para reanudar este lote conservando su protocolo:

```powershell
./experiments/Run-TimeComparison.ps1 -OutputDirectory Results/TimeComparison/BR1-BR17-CPU30-eight-workers-fresh -Workers 8 -Seconds 30 -RootSeed 123 -SkipBuild -FreshRun -PhysicalCoresOnly
```

`-FreshRun` excluye la importacion de referencias al crear el lote; al reanudar,
omite los casos completos de este mismo lote. No elimina ningun resultado.

## Referencia temporal individual

Tras el piloto pareado `Results/TimeComparison/concurrency-pilot`, se inicia un
lote independiente sin reutilizar datos paralelos. Para reanudar:

```powershell
./experiments/Run-TimeComparison.ps1 -OutputDirectory Results/TimeComparison/BR1-BR17-CPU30-serial-fresh -Workers 1 -Seconds 30 -RootSeed 123 -SkipBuild -FreshRun -PhysicalCoresOnly
```

Una sola corrida esta activa a la vez. Mantener iguales parametros, datos y
binarios; evitar cargas externas y conservar el historial execution-*.json.
Los lotes paralelos anteriores permanecen como resultados exploratorios separados.
