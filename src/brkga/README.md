# BRKGA-CLP-U de Goncalves y Resende (2012)

Implementacion independiente de la variante **sin soporte completo** del articulo
[A parallel multi-population biased random-key genetic algorithm for a container loading problem](https://doi.org/10.1016/j.cor.2011.03.009).
Tambien se consulto el [manuscrito de los autores](https://optimization-online.org/2010/07/2688/).
No es el codigo original ni una reproduccion bit a bit de sus resultados.

## Separacion del proyecto existente

Todo el motor esta en `src/brkga/`, dentro del namespace `brkga2012`. No reutiliza
el cruce, mutacion, seleccion, poblacion ni cache de GA, ABC o FFA. Reutiliza los
datos BR, cajas, contenedor, operaciones geometricas puras de espacios y el
mecanismo cooperativo de plazo. No cambia DBLF ni el decodificador de cajas
individuales Espacios Maximos. El auditor de este metodo no llama a la geometria
ni a la rotacion del decodificador.

## Que esta implementado

- Cromosoma con 2M claves reales, donde M es el numero de cajas, no de tipos.
- Primeras M claves: orden ascendente estable de las copias canonicas agrupadas
  por tipo. Los datos BR ya vienen en ese orden. Produce BTPS con tipos repetidos.
- Ultimas M claves: VLT por **posicion de la secuencia ordenada**, sin volver a
  permutar esta mitad segun el indice original de la primera.
- BBL selecciona el primer espacio con esquina minima menor X/Z/Y donde cabe
  alguna orientacion permitida del tipo seleccionado.
- Hasta 36 capas: seis orientaciones y seis ordenes de llenado en dos ejes.
  El tercer eje tiene una caja de espesor. Se respetan las cantidades restantes.
- La capa elegida por `ceil(VLT * numeroCapas)` se coloca en la esquina minima.
  Las claves 0 y 1 estan definidas expresamente como primera y ultima opcion.
- Se actualizan todos los espacios intersectados y se podan degenerados,
  duplicados y contenidos. Como las capas son rectangulos completos, se puede
  restar su envolvente cubica sin eliminar huecos interiores inexistentes.
- Fitness = volumen real de cajas cargadas / volumen del contenedor.
- Tres poblaciones independientes; elitismo; mutantes completamente aleatorios;
  cruce uniforme sesgado entre un elite y un padre de **toda** la poblacion.
- Migracion simultanea de una instantanea de los dos mejores globales a las tres
  poblaciones, reemplazando los peores y conservando los tamanos.

La configuracion predeterminada de Tabla 3 es 20M individuos **por poblacion**,
15% elite, 15% mutantes, sesgo 0.7, tres poblaciones, intercambio cada 15
generaciones y maximo de 500 generaciones. El contador `generations` registra
rondas completas de las tres poblaciones; una ronda interrumpida no cuenta.

## Decisiones y diferencias documentadas

1. Se implementa **U**, no la variante S. No se exige soporte completo y no se
   ejecuta MaxJoin, que en el articulo se usa cuando se requiere soporte.
2. La Fig. 11 imprime `Skip=False` cuando no cabe una caja. Se usa `True` para
   evitar un bucle sin progreso. En U el volumen libre solo disminuye.
3. El texto no publica un orden numerico completo para las 36 opciones ni todos
   los detalles de cantidades incompletas. Se fija el orden de rotaciones 1..6
   del proyecto y, para cada una, YZ, ZY, YX, XY, ZX, XZ de Fig. 8. Se conservan
   opciones coincidentes. Se llena el primer eje hasta su capacidad o cantidad,
   y despues el segundo con filas/columnas completas; sobrantes se intentan en
   las siguientes apariciones del tipo. Estas convenciones pueden diferir del
   programa original, cuyo codigo especifico no se obtuvo.
4. El articulo paraleliza evaluaciones con OpenMP. Aqui cada ejecucion tiene un
   solo hilo de evaluacion y sus tres poblaciones se procesan secuencialmente.
   El lote usa 15 procesos independientes, no 15 x varios hilos internos.
5. Se usa mt19937_64 local. La semilla derivada por instancia sigue el proyecto;
   no se conoce el RNG ni las semillas concretas del programa del articulo.
6. Al limite publicado de 500 generaciones se agrega el presupuesto solicitado
   de 30 segundos CPU. Se puede detener antes al llenar el 100% del contenedor.
   Una evaluacion interrumpida puede aportar un incumbente parcial valido,
   marcado en los resultados. Su acomodo se guarda directamente, no se sustituye
   por una reconstruccion completa que cambiaria el volumen reportado.

## Temporizador y esfuerzo

Por defecto se miden **segundos CPU de proceso, usuario + sistema**, para usar
el mismo tipo de presupuesto que SCLP. Incluye agrupacion de tipos,
inicializacion, claves, evaluaciones, evolucion y migracion. La lectura del
archivo, auditoria independiente y exportacion quedan fuera. El tiempo
transcurrido se guarda por separado; puede superar 30 segundos con 15 procesos.

El plazo es cooperativo: la contabilizacion CPU de Windows tiene granularidad
gruesa y puede existir un pequeno sobrepaso. Se consulta CPU como maximo una vez
por milisegundo transcurrido; el reloj auxiliar se consulta cada 256 checkpoints
para no efectuar millones de lecturas por evaluacion. El CLI permite `--clock
wall` si se desea otro experimento con 30 segundos transcurridos.

**30 segundos no garantizan llegar a la evolucion.** Inicializar 3 x 20M
individuos puede agotar ese presupuesto, especialmente en BR heterogeneos.
Se guardan `initialized`, evaluaciones iniciadas/completadas, generaciones,
migraciones y si el incumbente es parcial. No se reduce automaticamente la
poblacion para ocultar este efecto. `--population N` crea un experimento con
parametros distintos del articulo y debe informarse como tal.

## Compilar y ejecutar

No se instalan dependencias. Desde la raiz:

```powershell
./src/brkga/Build-BRKGA2012.ps1 -Test
New-Item -ItemType Directory -Force Results/BRKGA2012/manual | Out-Null
./build/brkga2012/brkga2012.exe --input Instance/BR/BR15.txt --instance 1 --output Results/BRKGA2012/manual --seconds 30 --clock cpu --root-seed 123
```

`--root-seed` deriva la semilla usando ID y semilla de la instancia, igual que
los lotes anteriores. `--seed` establece directamente la semilla efectiva.
No se pueden usar ambas a la vez. Para una corrida por generaciones, usar
`Configuration.seconds=0` en la API; el CLI por tiempo exige segundos positivos.

El menu de `main.exe` agrega **6 = BRKGA**. No pregunta mutacion, DBLF ni
parametros de GA/ABC/FFA. El CLI de main tambien acepta `--algorithm BRKGA2012`
o `BRKGA`, con `--dataset BR15 --instances 1 --root .`. Esta ruta usa el perfil
publicado y 30 segundos CPU; para configurar el perfil usar el CLI independiente.
La compilacion manual de main debe incluir `src/brkga/*.cpp`; `.cxx` contiene
el main independiente y no debe agregarse al main del proyecto.

Cada caso guarda `packing.csv`, `keys.csv`, `configuration.txt`, `result.csv` y
`verification.json`. La terminal imprime explicitamente el porcentaje final.

## Lote completo y tabla

```powershell
./experiments/Run-BRKGA2012Pool.ps1 -OutputDirectory Results/BRKGA2012/BR1-BR15-U30-fifteen-workers -Workers 15 -Seconds 30 -RootSeed 123 -SkipBuild
```

Son las 100 instancias de cada BR1-BR15. Sin `-SkipBuild` el primer inicio
compila y ejecuta los tests. Se congela el ejecutable y los scripts, se validan
los hashes de las entradas contra los lotes anteriores, y cada caso se reclama
con bloqueo exclusivo. Solo se publica el checkpoint despues del auditor.
Los intentos incompletos se conservan y se repiten al reanudar el mismo comando.
No cambian resultados de otra configuracion por compartir carpeta.

`tabla.md` muestra las cinco columnas y el n/100 de BRKGA mientras avanza; la
tabla del lote beam incorpora la misma vista conjunta. Sus datos originales
quedan intactos. GA/ABC/FFA usan 100 iteraciones / 4020 evaluaciones, mientras
SCLP y este lote usan 30 segundos CPU: no son cinco presupuestos equivalentes.

## Validacion

`tests/BRKGA2012Tests.cpp` comprueba el ejemplo BTPS de Fig. 3, VLT por posicion,
36 capas, cantidades, permisos, extremos de claves, continuidad tras fallo,
encaje exacto, corrupciones geometricas, sesgo de cruce, mutantes, migracion,
conteos exactos de evaluaciones, reproducibilidad y cortes con reloj controlado.
Incluye 200 packings aleatorios auditados y repetidos, y seleccion en menu/CLI.
Los tests anteriores de DBLF y las tres metaheuristicas permanecen vigentes.

La auditoria valida IDs unicos y cantidades, tipo, dimensiones de la rotacion,
permisos, limites, intersecciones de todas las parejas y volumen acumulado.
Esto corrobora factibilidad, no optimalidad ni igualdad de resultados con el
programa original del articulo.
