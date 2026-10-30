# Estilo del proyecto

El formato sigue el codigo existente: cuatro espacios, llaves C++ en una linea
propia y nombres publicos conservados. `.clang-format` recoge las reglas mecanicas.

- Agrupar declaraciones y operaciones que resuelven el mismo paso.
- Separar con una linea en blanco las etapas distintas: lectura, validacion,
  preparacion, busqueda, auditoria y escritura.
- Escribir una instruccion por linea. Evitar cadenas de instrucciones con `;`.
- Dejar las comprobaciones simples y su salida juntas. Separar el bloque de
  validacion del trabajo que viene despues.
- Dividir condiciones, argumentos y expresiones largas respetando sus unidades
  logicas. Conservar los textos de salida completos.
- Mantener juntas las asignaciones de una misma estructura y los datos de una
  misma fila de salida.
- Comentar motivos y restricciones que no resultan evidentes. No describir cada
  asignacion ni crear funciones nuevas solo para reducir el numero de lineas.

En PowerShell se conserva la sintaxis propia del lenguaje: parametros en varias
lineas, bloques legibles y continuaciones explicitas cuando hacen falta. Los
separadores de `for` permanecen en su cabecera.

El formato no justifica modificar algoritmos, interfaces, parametros ni datos de
experimentos. Las copias congeladas en `Results/` y el codigo externo en
`third_party/` quedan fuera de esta limpieza, al igual que los tests.

Referencia: [C++ Core Guidelines, Naming and layout](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-naming).
