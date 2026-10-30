<#
.SYNOPSIS
Compara DBLF y Espacios Maximos sobre los mismos ocho genomas.
.DESCRIPTION
Compila el comparador con optimizacion, evalua BR1/1 y BR15/1 y exporta
acomodos, genomas, mediciones y un manifiesto de ejecucion. Usa el auditor
geometrico independiente y muestra un resumen por instancia y decodificador.
No ejecuta las metaheuristicas ni la bateria completa BR.
.PARAMETER OutputDirectory
Carpeta de resultados. Rechaza una carpeta con evaluations.csv para no
sobrescribir una comparacion anterior.
.PARAMETER Compiler
Ruta al compilador C++ existente; no instala dependencias.
.EXAMPLE
./experiments/Compare-MaximalSpaces.ps1 -OutputDirectory Results/MaximalSpaces/nueva-comparacion
#>
param(
    [string]$OutputDirectory = 'Results/MaximalSpaces/paired-smoke',
    [string]$Compiler = 'C:/mingw64/bin/g++.exe'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$previousPath = $env:PATH
Push-Location $root

try {
    $env:PATH = (Split-Path $Compiler -Parent) + ';' + $env:PATH

    if (Test-Path "$OutputDirectory/evaluations.csv") {
        throw 'Use a new output directory to preserve previous results'
    }
    New-Item -ItemType Directory -Force $OutputDirectory, build | Out-Null
    $sources = @(Get-ChildItem src -Recurse -Filter *.cpp |
        Where-Object Name -ne main.cpp | ForEach-Object FullName)
    & $Compiler -std=c++14 -O3 -DNDEBUG experiments/MaximalSpacesComparison.cpp $sources -o build/maximal_spaces_comparison.exe

    if ($LASTEXITCODE) {
        throw 'Compilation failed'
    }
    & ./build/maximal_spaces_comparison.exe $OutputDirectory

    if ($LASTEXITCODE) {
        throw 'Comparison failed'
    }

    $manifest = @{
        compiler = (& $Compiler --version | Select-Object -First 1)
        flags = '-std=c++14 -O3 -DNDEBUG'
        seed = 123
        genomes_per_instance = 4
        instances = @('BR1/1', 'BR15/1')
        geometry_audits = 16
        binary_sha256 = (Get-FileHash build/maximal_spaces_comparison.exe).Hash
        note = 'Single wall-time measurement per evaluation; may overlap other workloads. peak_spaces is the maximum retained free-space count after pruning (initial space included). DBLF has no free-space list.'
    }
    $manifest | ConvertTo-Json | Set-Content "$OutputDirectory/manifest.json"

    Import-Csv "$OutputDirectory/evaluations.csv" | Group-Object dataset, decoder | ForEach-Object {
        $evaluations = $_.Group
        $meanOccupation = ($evaluations | ForEach-Object {
                [double]$_.occupation_percent
            } | Measure-Object -Average).Average
        $meanMilliseconds = ($evaluations | ForEach-Object {
                [double]$_.microseconds / 1000
            } | Measure-Object -Average).Average
        $maximumSpaces = ($evaluations | ForEach-Object {
                [int]$_.peak_spaces
            } | Measure-Object -Maximum).Maximum
        [pscustomobject]@{
            group = $_.Name
            occupation = $meanOccupation
            mean_ms = $meanMilliseconds
            peak_spaces = $maximumSpaces
        }
    } | Format-Table
}
finally {
    $env:PATH = $previousPath
    Pop-Location
}
