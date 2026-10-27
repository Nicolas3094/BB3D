# BR1–BR15 con el método SCLP H de árbol/beam

```powershell
./experiments/Run-BeamPool.ps1 -OutputDirectory Results/TreeSearch/BR1-BR15-H30-fifteen-workers -Workers 15 -Seconds 30 -MetaDirectory Results/EqualIterations/BR1-BR15-N100-fifteen-workers
```

Son 1500 casos: 100 instancias de cada clase BR1–BR15. No se repiten las
metaheurísticas: su lote de 4500 casos ya está completado y auditado.
Se comprueba que los datos tengan los mismos hashes que esa referencia.

El algoritmo es el ejecutable independiente original `sclp_reference.exe`,
compilado Release -O3 -DNDEBUG, con parámetros H/two-stage de la tabla 3,
memoización strict, máximo de 10000 bloques, ramificación inicial 4 y máxima
10000, densidad de bloque automática y todos los permisos de orientación BR.
No se cambian búsqueda, decisiones geométricas ni temporizador originales.

Cada caso recibe **30 segundos CPU del proceso**, incluyendo generación de bloques
y búsqueda, con el sobrepaso cooperativo de la implementación original.
Lectura/conversión de BR, auditoría y exportación no son parte de ese límite.
Se guardan por separado CPU y tiempo transcurrido del proceso.
Las metaheurísticas recibieron 100 iteraciones (4020 evaluaciones): **no es el mismo
presupuesto**, por lo que la tabla conjunta no es una comparación con esfuerzo
igualado ni una reproducción del rendimiento de la computadora del artículo.

Quince trabajadores PowerShell persistentes reclaman casos de una cola mediante
un archivo con bloqueo exclusivo y eliminación al cerrar. Cada uno lanza un solo
solver serial por vez, heredando su afinidad a un hilo lógico distinto. Se usa
la topología consultada por el ejecutable del lote anterior, repartiendo primero
entre núcleos físicos y después SMT; el hilo lógico 0 no se asigna a la búsqueda.
No son 15 procesos de solver más otros 15 calculando en segundo plano: los gestores
esperan mientras su solver realiza la búsqueda.

La carpeta contiene una copia congelada del ejecutable y de los scripts de
ejecución, conversión y auditoría. `manifest.json` registra sus hashes, el límite,
las afinidades y hashes de las 15 entradas BR. No se mezcla con pruebas anteriores.

Cada `tasks/BRx-iy-SCLP/` conserva intentos, con:

- `input.json`: dimensiones, cantidades y seis permisos de orientación.
- `packing.json`: cajas con posición, dimensión, orientación y tipo.
- `run.json`: todos los parámetros, hashes, tiempos y resultado del auditor.
- `verification.json`: límites, cantidades, orientaciones y todas las parejas.
- `terminal.txt`: salida del solver.

Solo después de pasar los controles originales y la auditoría independiente se
publica `result.csv` mediante renombrado con reintentos. Un intento interrumpido
no se borra ni cuenta como terminado; al reanudar se crea otro intento en la misma
carpeta y se omiten exclusivamente los casos ya comprometidos.

`tabla.md`, `resultados.csv` y `estado.json` se actualizan cada 15 segundos.
La tabla muestra las medias completas de GA/ABC/Firefly junto a la media SCLP
parcial, con su n/100 explícito. Los fallos transitorios de publicación de progreso
no cancelan búsquedas válidas. Se usa la misma recuperación de archivos aplicada
tras la interrupción del lote de metaheurísticas en BR6.

Para ver qué trabajador atiende cada caso:

```powershell
./experiments/Show-EqualPool.ps1 -Directory Results/TreeSearch/BR1-BR15-H30-fifteen-workers -Watch
```

Para detener ordenadamente, crear un archivo `STOP` en la carpeta del lote.
Los trabajadores terminan el caso actual y salen. Para reanudar, quitar ese
marcador y ejecutar el mismo comando; se verifica el ejecutable, scripts y datos
congelados. No iniciar dos gestores en la misma carpeta: existe un bloqueo del gestor.

Prueba del ejecutor: tres casos con tres trabajadores y dos segundos CPU por caso,
todos reclamados una vez y auditados. También se comprueba que la reanudación no
reescriba sus resultados completos. Esta prueba corta no sustituye la corrida
principal de 30 segundos.
