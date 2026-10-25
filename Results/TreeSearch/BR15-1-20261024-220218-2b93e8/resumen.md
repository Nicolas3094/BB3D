# SCLP H — BR15, instancia 1

- Ocupación final verificada: **95.50596518001889 %**.
- Cajas: **111**; volumen ocupado **28 737 382**; contenedor **30 089 620**.
- Límite original: **30 segundos CPU de proceso (usuario + sistema)**.
- Tiempo CPU: **30.000000000000004 s**, incluye bloques e inicialización.
- Inicialización: **0.156250 s**; búsqueda DoubleEffort: **29.843750 s**.
- Tiempo interno transcurrido: **30.195902 s**.
- Tiempo externo del proceso: **30.237874 s**.
- Hilos del solver: **1**; compilación: **MinGW GCC 16.2.0, C++20, -O3 -DNDEBUG**.

Configuración H/two-stage, tabla 3 del artículo:
primera etapa `(3.7015, 2.6631, 0.3968, 0.0139)`;
segunda `(2.0934, 1.4609, 0.4732, 0.0366)`; umbral **0.6740**;
memoización **strict**; ramificación inicial **4**, máxima **10000**;
bloques máximos **10000**; densidad automática **0.98** para 100 tipos.

Verificación geométrica independiente aprobada: **6 105 parejas**, sin
intersecciones, límites respetados, cantidades y orientaciones permitidas por tipo.
También aprobaron todos los controles de restricciones de la implementación original.
Los 100 tipos de entrada coinciden con el ejemplo oficial BR15/1.

El original incluye generación de bloques en el presupuesto, pero no comprueba el
plazo dentro de la generación; su límite es cooperativo. No se cambió el reloj por
pared ni se equiparó con el temporizador de las metaheurísticas del proyecto.
Se aplicó únicamente la compatibilidad Windows de coordenadas a int64 mediante
un encabezado generado; fuentes originales intactas. Véase la auditoría en
`src/treeSearch/README.md` para alcance y diferencias de compilación.

- [Acomodo con posición, dimensiones, tipo y orientación](packing.json)
- [Datos convertidos conservando permisos](input.json)
- [Argumentos completos, hashes y tiempos](run.json)
- [Verificación independiente](verification.json)
- [Salida del solver](terminal.txt)
- [Artículo](https://doi.org/10.1016/j.ejor.2026.01.039)
- [Código original](https://zenodo.org/records/19247237)

Una instancia y otra plataforma no reproducen el promedio de calidad publicado.
