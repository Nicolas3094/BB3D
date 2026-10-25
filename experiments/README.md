# Experimento del decodificador por contacto

## Barrido BR1–BR15 con adición de candidatos

```powershell
./experiments/Run-BRContactAdding.ps1 -OutputDirectory Results/BRContactAdding/nueva-corrida
```

Ejecuta la **primera instancia** de BR1 a BR15 con GA, ABC y Firefly, contacto,
mutación C2 y `replacement=false` (añadir candidatos y seleccionar los mejores;
no crecimiento ilimitado). Población 20, seis orientaciones sujetas a permisos,
semilla raíz 123 y límite de 30 segundos de pared por corrida. Los demás parámetros
existentes se conservan y sus identidades completas se guardan por algoritmo.

En Windows lanza tres procesos de búsqueda, uno por algoritmo, fijados a los
núcleos lógicos 0, 2 y 4 respectivamente; requiere esos núcleos disponibles.
No cambia el número de hilos de las metaheurísticas: cada proceso busca en un hilo.
Compila con `-O3 -DNDEBUG`; no altera el ejecutable del menú.

El resultado incluye una tabla `resumen.md`, `resultados.csv`, CSV por algoritmo,
genomas, secuencias de inserción y acomodos auditados geométricamente. La carpeta
de salida debe ser nueva. Duración aproximada: ocho minutos más compilación.
Es una corrida por combinación, no un promedio de semillas ni de las 100 instancias
de cada archivo. No debe compararse como si fuera el promedio completo de una clase.

Para **todas las instancias** (4500 corridas, aproximadamente 12,5 horas con tres
procesos) se usa una carpeta existente, pudiendo reutilizar el barrido inicial:

```powershell
./experiments/Run-BRContactAll.ps1 -OutputDirectory Results/BRContactAdding/BR1-BR15-contact-C2-add-seed123
```

El gestor publica `estado.json`, `resultados-todas.csv` y `tabla-todas.md` cada
30 segundos. La tabla identifica cuántas de las 100 instancias completó cada celda;
las medias son provisionales hasta completar el lote. Al finalizar marca
`completado` y las 4500 corridas. Conserva los resultados existentes y omite
combinaciones dataset/posición ya registradas y verificadas con la misma configuración.
Para reanudar, ejecutar de nuevo el mismo comando cuando el gestor anterior haya
terminado. No iniciar gestores duplicados; el equipo debe permanecer encendido y
sin suspensión para que avance el trabajo. Los errores se registran y las corridas
completas no se eliminan. El ejecutable por algoritmo acepta `--all` para este modo.

## Referencia confirmada antes del cambio

La ruta codificada usada por las metaheurísticas es `DBLF(Bin&, order, rotations,
items)`. Con candidatos legacy arranca en (0,0,0), mantiene una cola ordenada y
la recorre de atrás hacia delante: menor **X**, después menor **Z**, después menor
**Y**. Conserva la orientación indicada por cada gen. Si ese permiso es inválido,
omite la caja, sin sustituir su orientación.

Para cada caja, el DBLF original elige el **primer punto factible**, lo elimina,
compacta la posición hacia X, Z, Y y coloca la caja. La compactación mueve
directamente hasta pared/cara, sin avanzar unidad por unidad, y conserva la
excepción histórica: no mueve las dos primeras cajas. Genera tres puntos junto
a las caras positivas de la caja compactada, elimina duplicados y sigue con la
siguiente caja. No cambia esa lógica cuando el decodificador es `dblf`.
Los modos projected/best existentes permanecen disponibles como antes.

## Variante independiente

`PackingDecoder::Contact` usa exactamente la generación legacy, su cola y su
compactación; no agrega proyecciones. Recorre **todos** los puntos factibles para
la caja/orientación actual, compacta cada uno en una variable local y puntúa su
contacto. Solo elimina el punto de origen de la alternativa ganadora y actualiza
la cola al colocarla. La orientación del gen no se modifica. Los rechazos de
orientaciones y validaciones de entrada son los mismos.

Puntuación: área total de las caras de la caja que tocan cualquiera de las seis
paredes o las caras de cajas ya colocadas. Para dos caras coplanares en eje a,
el área es la intersección rectangular en los otros dos ejes u,v:

`max(0, min(u1,u2)-max(u0,u0_otro)) * max(0, min(v1,v2)-max(v0,v0_otro))`.

Solo hay contacto si una cara final coincide exactamente con una cara inicial
del vecino, o si la cara coincide con una pared. Coordenadas enteras; sin tolerancia
de proximidad. Se acumula en `long double` para evitar desbordar sumas de áreas
en int64. En las instancias BR de este experimento las áreas son enteros exactos.
El área se mide en unidades cuadradas; dividirla por la superficie de la caja
no cambiaría el ranking entre posiciones de esa misma caja y orientación.

No se suman las dos caras de una misma interfaz: se cuenta exclusivamente la
cara de la caja candidata. En una cara suya, los rectángulos aportados por dos
vecinos no pueden solaparse en área positiva sin que esos vecinos se intersecten
en volumen. Una pared tampoco puede compartir un parche con una caja vecina
contenida en el contenedor. Sus bordes tienen área cero. Por ello la suma equivale
al área de unión, bajo la precondición de un empacado válido. Los tests contrastan
la fórmula con un oráculo independiente que cuenta cuadrados unitarios y prueba
contactos repartidos entre varias cajas.

Desempate: X, Z, Y ascendente sobre la posición **compactada**. Si también coincide
esa posición, conserva el primer origen en el recorrido DBL original. No hay
azar añadido. No se exige soporte completo ni se privilegia exclusivamente el
suelo: cuentan las seis caras. No se promete que más contacto produzca más volumen.

Con O(n) puntos y O(n) trabajo para factibilidad, compactación y puntuación por
punto, se mantiene una cota O(n³) para n cajas, con mayor constante que la referencia.
Ambos decodificadores participan en los chequeos cooperativos del presupuesto.
Una interrupción conserva solo cajas insertadas, nunca una colocación a medio hacer.

## Selección sin cambiar metaheurísticas

En el menú, tras seleccionar GA, ABC o Firefly:

- 1: DBLF legacy (referencia predeterminada).
- 2 y 3: DBLF projected/best, existentes.
- 4: contacto con tres puntos originales.

Por argumentos:

```powershell
./main.exe --root . --dataset BR15 --instances 1 --algorithm GA --population 20 --rotation 6 --mutation c2 --seed 123 --execution 30s --decoder dblf --candidates legacy --report --output Results/BR/referencia
./main.exe --root . --dataset BR15 --instances 1 --algorithm GA --population 20 --rotation 6 --mutation c2 --seed 123 --execution 30s --decoder contact --candidates legacy --report --output Results/BR/contacto
```

Se rechaza explícitamente contacto con projected/best para no mezclar experimentos.
Se separan identidad de checkpoint y contexto de caché. Los informes y la terminal
identifican el decodificador. No cambian los operadores, parámetros ni criterios
de selección de GA/ABC/Firefly. DBLF sigue siendo el predeterminado.

## Repetir la comparación

```powershell
./experiments/Compare-Decoders.ps1 -Mode all -OutputDirectory Results/DecoderComparison/nueva-comparacion
./experiments/Summarize-Decoders.ps1 -Directory Results/DecoderComparison/nueva-comparacion
```

Requiere únicamente el MinGW existente. Compila aparte con `-std=c++14 -O3 -DNDEBUG`.
`-Mode fixed` ejecuta solo evaluaciones pareadas; `-Mode search` solo búsqueda
completa. El modo all tarda aproximadamente 12 minutos de búsqueda más compilación
y validación. No sobreescribe los CSV de una comparación ya existente.

Protocolo fijo: primera instancia de BR0 y BR15; semillas 123 y 456; 16 genomas por
semilla, incluyendo identidad, orden por volumen descendente, ascendente y 13
permutaciones aleatorias. Sus genes de orientación se eligen entre los permisos
válidos de cada caja y se guardan. Se evalúan **los mismos dos cromosomas** con
ambos decodificadores, alternando cuál se mide primero. Son 64 genomas y 128
evaluaciones. Tiempo individual de pared incluye DBLF, validación de entrada y
copias de sus argumentos; excluye exportación y auditoría independiente.

Protocolo de búsqueda: las mismas dos instancias y semillas, los tres algoritmos,
ambos decodificadores: 24 corridas de 30 segundos de pared. Población 20, C2, seis
rotaciones sujetas a permisos, reemplazo activado y caché desactivada. El resto
de parámetros conserva los valores existentes de `CommandLineOptions` y se guarda
en `parameters-all.txt` mediante la identidad completa. Se reinicia el RNG con la
misma semilla derivada de instancia antes de cada variante; no se fuerza la misma
trayectoria posterior porque el fitness cambia las decisiones de búsqueda.
El orden DBLF/contacto se invierte para la segunda semilla.

Se exporta cada genoma y cada acomodo, incluso cuando contacto empeora. Para el
incumbente de búsqueda se guarda además su secuencia de inserción. Se reconstruye
fuera del plazo y se exige que su volumen coincida exactamente con el registrado,
incluidos incumbentes provenientes de evaluaciones parciales.

`tests/PackingAudit.hpp` verifica sin llamar a las funciones de rotación, colisión
ni factibilidad de producción: IDs únicos/oferta, alineación con genes,
permutación de dimensiones, permiso del eje vertical, contención y todas las
parejas de cajas, recalculando volumen. Se permiten caras en contacto y se rechaza
volumen de intersección positivo. Los tests incluyen un empacado alterado para
comprobar que el auditor realmente lo rechaza.

Resultados de esta ejecución: `Results/DecoderComparison/contact-v1-BR0-BR15/`.
`evaluations.csv` contiene ocupación y microsegundos por evaluación;
`search.csv`, ocupación y tiempo de cada búsqueda; `resumen.md` presenta las
comparaciones sin elegir ni activar automáticamente una variante.
Son experimentos piloto en dos instancias y dos semillas, no una prueba general
de superioridad. La carga del sistema y el mayor coste por evaluación afectan
la búsqueda de tiempo limitado; se conservan los números de evaluaciones intentadas.
