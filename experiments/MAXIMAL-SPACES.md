# Decodificador por Espacios Máximos con prioridad DBL

Esta variante empaca **cajas individuales** en el orden y orientación del genoma.
No reproduce el BRKGA publicado ni el generador de bloques del método SCLP.
DBLF sigue siendo el valor predeterminado; su recorrido y compactación no cambian.
Los operadores, parámetros y paradas de GA, ABC y FFA no se modifican.

## Ejecución

Desde la raíz del proyecto, ejecutar `./main.exe` y elegir el algoritmo y la
opción de decodificador **5 = Espacios Máximos (DBL)**. Las opciones 1–3 siguen
siendo DBLF; la 4 conserva el experimento previo de contacto.

Estos ejemplos ejecutan la primera instancia de BR15, 100 iteraciones, semilla
123, seis orientaciones según permisos, mutación C2 y sin reemplazo:

```powershell
./main.exe --root . --algorithm GA --dataset BR15 --instances 1 --decoder maximal-spaces --rotation 6 --mutation c2 --replacement false --population 20 --iterations 100 --seed 123 --output Results/MS-GA
./main.exe --root . --algorithm ABC --dataset BR15 --instances 1 --decoder maximal-spaces --rotation 6 --mutation c2 --replacement false --population 20 --iterations 100 --seed 123 --output Results/MS-ABC
./main.exe --root . --algorithm FFA --dataset BR15 --instances 1 --decoder maximal-spaces --rotation 6 --mutation c2 --replacement false --population 20 --iterations 100 --seed 123 --output Results/MS-FFA
```

Para DBLF, cambiar únicamente `--decoder maximal-spaces` por `--decoder dblf`.
Se acepta también `--decoder MAXIMAL_SPACES`. `--execution 30s` conserva el
presupuesto habitual de búsqueda. `--candidates` pertenece a DBLF y no interviene
en Espacios Máximos: no genera proyecciones, puntos DBLF ni compactación adicional.
Los resultados indican el decodificador. Caché y checkpoints lo distinguen,
además de los datos y la configuración que ya identificaban.

## Geometría y selección

Un espacio es un cuboide semiabierto `[mínima, máxima)`. Inicialmente ocupa todo
el contenedor. La caja orientada produce un candidato en la esquina mínima de
cada espacio donde cabe; se elige menor X, luego menor Z y luego menor Y.
Los espacios se ordenan además por esquina máxima X/Z/Y para un orden estable;
dos candidatos con la misma esquina producen el mismo acomodo.

Cada inserción resta la caja de **todos** los espacios intersectados. Cada
espacio genera hasta seis losas, una a cada lado de las caras de la caja,
conservando el alcance original en los otros dos ejes. Se eliminan losas de
extensión no positiva, duplicados y espacios contenidos completamente en otro.
Las losas pueden solaparse entre sí; esto permite conservar posiciones útiles
que una partición disjunta eliminaría. No se suman sus volúmenes.

La aptitud sigue siendo volumen de cajas cargadas / volumen del contenedor.
Una caja inviable se omite y continúa el genoma. No se añade soporte, peso ni
rotación automática. Se reutilizan validaciones de entrada y permisos existentes.
La selección compartida se realiza en las tres entradas actuales de DBLF,
incluyendo evaluación y reconstrucción desde cajas ya orientadas.

La poda cuesta O(m²) para m espacios antes de podar y tiene comprobaciones del
plazo de ejecución. No hay tope de espacios: su crecimiento puede resultar caro
en instancias fragmentadas. Una interrupción conserva las cajas ya insertadas,
que siguen siendo válidas. No se promete mayor ocupación ni menor tiempo.

## Pruebas y comparación corta

`./tests/run.ps1` incluye `MaximalSpacesTests.cpp`: seis divisiones y unión
comprobada por una cuadrícula independiente, contención, duplicados, múltiples
espacios solapados, encaje exacto, continuidad tras fallo, orientación, orden DBL,
determinismo, caché, interrupciones y ejecución de las tres metaheurísticas.
`PackingAudit.hpp` comprueba dimensiones/orientaciones, cantidades por ID,
límites, intersecciones y volumen sin usar la geometría del decodificador.

```powershell
./experiments/Compare-MaximalSpaces.ps1 -OutputDirectory Results/MaximalSpaces/otra-comparacion
```

Compila con C++14, `-O3 -DNDEBUG`, sin instalar dependencias. Evalúa únicamente
cuatro genomas idénticos por método para BR1/1 y BR15/1, semilla 123, orientación
legal fija por gen. Alterna el orden de medición. Guarda genomas y acomodos,
ocupación, microsegundos por evaluación y máximo número de espacios **retenidos
después de podar**, incluyendo el inicial. Para DBLF ese campo no aplica.
No mide el máximo transitorio durante la división. Son mediciones individuales
de tiempo transcurrido, sensibles a otras cargas; no una comparación estadística
de búsquedas ni la batería completa BR.
