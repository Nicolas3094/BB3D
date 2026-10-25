param([string]$Compiler = 'C:\mingw64\bin\g++.exe')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$binary = Join-Path $PSScriptRoot 'random_numbers_tests.exe'
$previousPath = $env:PATH
try {
    $env:PATH = (Split-Path $Compiler -Parent) + [IO.Path]::PathSeparator + $env:PATH
    & $Compiler -std=c++14 -Wall -Wextra -pedantic (Join-Path $PSScriptRoot 'RandomNumbersTests.cpp') (Join-Path $root 'src/util/RandomNumbers.cpp') -o $binary
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
    & $binary
    if ($LASTEXITCODE -ne 0) { throw "Tests failed: exit code $LASTEXITCODE" }
    # Link the core with controlled RNG inputs; exclude application and search drivers.
    $mutationBinary = Join-Path $PSScriptRoot 'flip_mutation_tests.exe'
    $mutationSources = @(Join-Path $PSScriptRoot 'FlipMutationTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -notin @('main.cpp','RandomNumbers.cpp','ExperimentConfiguration.cpp','ExperimentExecution.cpp','CommandLine.cpp','ExperimentRunner.cpp','BRExperimentMenu.cpp','BRExperimentSession.cpp') -and $_.Directory.Name -ne 'metaheuristics' } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $mutationSources -o $mutationBinary
    if ($LASTEXITCODE -ne 0) { throw 'Mutation test compilation failed.' }
    & $mutationBinary
    if ($LASTEXITCODE -ne 0) { throw "Mutation tests failed: exit code $LASTEXITCODE" }
    $populationBinary = Join-Path $PSScriptRoot 'mutate_poblation_tests.exe'
    $populationSources = @(Join-Path $PSScriptRoot 'MutatePoblationTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' -and $_.Name -ne 'RandomNumbers.cpp' -and $_.Name -ne 'ExperimentRunner.cpp' -and $_.Name -ne 'BRExperimentSession.cpp' } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $populationSources -o $populationBinary
    if ($LASTEXITCODE -ne 0) { throw 'Population test compilation failed.' }
    & $populationBinary
    if ($LASTEXITCODE -ne 0) { throw "Population tests failed: exit code $LASTEXITCODE" }
    $fireflyBinary = Join-Path $PSScriptRoot 'firefly_search_tests.exe'
    $fireflySources = @(Join-Path $PSScriptRoot 'FireflySearchTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -notin @('main.cpp', 'RandomNumbers.cpp', 'FitnessOperators.cpp', 'GeneticsOperators.cpp', 'GeneticAlgorithm.cpp', 'ArtificialBeeColonyAlgorithm.cpp','ExperimentConfiguration.cpp','ExperimentExecution.cpp','CommandLine.cpp','ExperimentRunner.cpp','BRExperimentMenu.cpp','BRExperimentSession.cpp') } | ForEach-Object { $_.FullName })
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
    $regressionBinary = Join-Path $PSScriptRoot 'mutation_regression_tests.exe'
    $regressionSources = @(Join-Path $PSScriptRoot 'MutationRegressionTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -notin @('main.cpp', 'RandomNumbers.cpp','ExperimentRunner.cpp','BRExperimentSession.cpp') } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $regressionSources -o $regressionBinary
    if ($LASTEXITCODE -ne 0) { throw 'Mutation regression compilation failed.' }
    & $regressionBinary
    if ($LASTEXITCODE -ne 0) { throw "Mutation regression tests failed: exit code $LASTEXITCODE" }
    $evolutionBinary = Join-Path $PSScriptRoot 'evolution_regression_tests.exe'
    $evolutionSources = @(Join-Path $PSScriptRoot 'EvolutionRegressionTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -notin @('main.cpp', 'RandomNumbers.cpp','ExperimentRunner.cpp','BRExperimentSession.cpp') } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $evolutionSources -o $evolutionBinary
    if ($LASTEXITCODE -ne 0) { throw 'Evolution regression compilation failed.' }
    & $evolutionBinary
    if ($LASTEXITCODE -ne 0) { throw "Evolution regression tests failed: exit code $LASTEXITCODE" }
    $safetyBinary = Join-Path $PSScriptRoot 'search_safety_tests.exe'
    $safetySources = @(Join-Path $PSScriptRoot 'SearchSafetyTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -notin @('main.cpp','RandomNumbers.cpp','ExperimentRunner.cpp','BRExperimentSession.cpp') } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 $safetySources -o $safetyBinary
    if ($LASTEXITCODE -ne 0) { throw 'Search safety test compilation failed.' }
    & $safetyBinary
    if ($LASTEXITCODE -ne 0) { throw "Search safety tests failed: exit code $LASTEXITCODE" }
    New-Item -ItemType Directory -Force (Join-Path $root 'build/infrastructure-tests') | Out-Null
    $infrastructureBinary = Join-Path $PSScriptRoot 'infrastructure_tests.exe'
    $infrastructureSources = @(Join-Path $PSScriptRoot 'InfrastructureTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -ne 'main.cpp' } | ForEach-Object { $_.FullName })
    & $Compiler -std=c++14 -fno-elide-constructors $infrastructureSources -o $infrastructureBinary
    if ($LASTEXITCODE -ne 0) { throw 'Infrastructure test compilation failed.' }
    Push-Location $root
    try {
        & $infrastructureBinary
        if ($LASTEXITCODE -ne 0) { throw "Infrastructure tests failed: exit code $LASTEXITCODE" }
    } finally { Pop-Location }
    $integrityBinary=Join-Path $PSScriptRoot 'experiment_integrity_tests.exe'
    $integritySources=@(Join-Path $PSScriptRoot 'ExperimentIntegrityTests.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 $integritySources -o $integrityBinary
    if($LASTEXITCODE -ne 0) { throw 'Experiment integrity compilation failed' }
    Push-Location $root
    try { & $integrityBinary; if($LASTEXITCODE -ne 0) { throw 'Experiment integrity tests failed' } }
    finally { Pop-Location }
    $completionBinary=Join-Path $PSScriptRoot 'completion_tests.exe'
    $completionSources=@(Join-Path $PSScriptRoot 'CompletionTests.cpp')+@(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 $completionSources -o $completionBinary
    if($LASTEXITCODE -ne 0) {throw 'Completion test compilation failed'}
    & $completionBinary
    if($LASTEXITCODE -ne 0) {throw 'Completion tests failed'}
    $brBinary=Join-Path $PSScriptRoot 'br_reader_tests.exe'
    $brSources=@(Join-Path $PSScriptRoot 'BRReaderTests.cpp')+@(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 $brSources -o $brBinary
    if($LASTEXITCODE -ne 0) {throw 'BR reader test compilation failed'}
    Push-Location $root
    try {& $brBinary;if($LASTEXITCODE -ne 0) {throw 'BR reader tests failed'}}
    finally {Pop-Location}
    $timedBinary=Join-Path $PSScriptRoot 'timed_execution_tests.exe'
    $timedSources=@(Join-Path $PSScriptRoot 'TimedExecutionTests.cpp')+@(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 $timedSources -o $timedBinary
    if($LASTEXITCODE -ne 0) {throw 'Timed execution compilation failed'}
    Push-Location $root
    try {& $timedBinary;if($LASTEXITCODE -ne 0) {throw 'Timed execution tests failed'}} finally {Pop-Location}
    $application=Join-Path $PSScriptRoot 'project_build_check.exe'
    $menuBinary=Join-Path $PSScriptRoot 'br_menu_tests.exe'
    $menuSources=@(Join-Path $PSScriptRoot 'BRMenuTests.cpp')+@(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 $menuSources -o $menuBinary
    if($LASTEXITCODE -ne 0) {throw 'BR menu compilation failed'}
    Push-Location $root
    try {& $menuBinary;if($LASTEXITCODE -ne 0) {throw 'BR menu tests failed'}} finally {Pop-Location}
    $applicationSources=@(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | ForEach-Object FullName)
    & $Compiler -std=c++14 $applicationSources -o $application
    if($LASTEXITCODE -ne 0) {throw 'Application compilation failed'}
    & (Join-Path $PSScriptRoot 'smoke-cli.ps1') -Binary $application
    & (Join-Path $PSScriptRoot 'smoke-menu.ps1') -Binary $application
}
finally {
    $env:PATH = $previousPath
}
