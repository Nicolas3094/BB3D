param([string]$Compiler = 'C:\mingw64\bin\g++.exe')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$binary = Join-Path $PSScriptRoot 'random_numbers_tests.exe'
$previousPath = $env:PATH
try {
    $env:PATH = (Split-Path $Compiler -Parent) + ';' + $env:PATH
    & $Compiler -std=c++14 -Wall -Wextra -pedantic (Join-Path $PSScriptRoot 'RandomNumbersTests.cpp') (Join-Path $root 'src/util/RandomNumbers.cpp') -o $binary
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
    & $binary
    if ($LASTEXITCODE -ne 0) { throw "Tests failed: exit code $LASTEXITCODE" }
    # Link the core with controlled RNG inputs; exclude application and search drivers.
    $mutationBinary = Join-Path $PSScriptRoot 'flip_mutation_tests.exe'
    $mutationSources = @(Join-Path $PSScriptRoot 'FlipMutationTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' -and $_.Name -ne 'RandomNumbers.cpp' -and $_.Directory.Name -ne 'metaheuristics' } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $mutationSources -o $mutationBinary
    if ($LASTEXITCODE -ne 0) { throw 'Mutation test compilation failed.' }
    & $mutationBinary
    if ($LASTEXITCODE -ne 0) { throw "Mutation tests failed: exit code $LASTEXITCODE" }
    $populationBinary = Join-Path $PSScriptRoot 'mutate_poblation_tests.exe'
    $populationSources = @(Join-Path $PSScriptRoot 'MutatePoblationTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' -and $_.Name -ne 'RandomNumbers.cpp' } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $populationSources -o $populationBinary
    if ($LASTEXITCODE -ne 0) { throw 'Population test compilation failed.' }
    & $populationBinary
    if ($LASTEXITCODE -ne 0) { throw "Population tests failed: exit code $LASTEXITCODE" }
    $fireflyBinary = Join-Path $PSScriptRoot 'firefly_search_tests.exe'
    $fireflySources = @(Join-Path $PSScriptRoot 'FireflySearchTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -notin @('main.cpp', 'RandomNumbers.cpp', 'FitnessOperators.cpp', 'GeneticsOperators.cpp', 'GeneticAlgorithm.cpp', 'ArtificialBeeColonyAlgorithm.cpp') } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $fireflySources -o $fireflyBinary
    if ($LASTEXITCODE -ne 0) { throw 'Firefly test compilation failed.' }
    & $fireflyBinary
    if ($LASTEXITCODE -ne 0) { throw "Firefly tests failed: exit code $LASTEXITCODE" }
    $geometryBinary = Join-Path $PSScriptRoot 'geometry_tests.exe'
    $geometrySources = @(Join-Path $PSScriptRoot 'GeometryTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $geometrySources -o $geometryBinary
    if ($LASTEXITCODE -ne 0) { throw 'Geometry test compilation failed.' }
    & $geometryBinary
    if ($LASTEXITCODE -ne 0) { throw "Geometry tests failed: exit code $LASTEXITCODE" }
    $inputBinary = Join-Path $PSScriptRoot 'dblf_input_tests.exe'
    $inputSources = @(Join-Path $PSScriptRoot 'DBLFInputTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $inputSources -o $inputBinary
    if ($LASTEXITCODE -ne 0) { throw 'DBLF input test compilation failed.' }
    & $inputBinary
    if ($LASTEXITCODE -ne 0) { throw "DBLF input tests failed: exit code $LASTEXITCODE" }
}
finally {
    $env:PATH = $previousPath
}
