<#
.SYNOPSIS
Agrega promedios al informe sin interrumpir un gestor de pruebas ya iniciado.
.PARAMETER Watch
Mantiene el pie del informe en un gestor antiguo hasta que termine. Los nuevos
gestores ya generan los promedios directamente desde Run-BRKGA2012Pool.ps1.
#>
param(
    [Parameter(Mandatory)][string]$Directory,
    [string]$MetaDirectory = 'Results/EqualIterations/BR1-BR15-N100-fifteen-workers',
    [string]$BeamDirectory = 'Results/TreeSearch/BR1-BR15-H30-fifteen-workers',
    [switch]$Watch
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/PoolIO.ps1"
. "$PSScriptRoot/BRComparisonAverages.ps1"
$meta = @(Import-Csv "$MetaDirectory/resultados.csv")
$beam = @(Import-Csv "$BeamDirectory/resultados.csv")
do {
    foreach ($path in @("$Directory/tabla.md", "$BeamDirectory/tabla.md")) {
        if (!(Test-Path $path)) {
            continue
        }
        $text = Read-PoolText $path

        if ($text.Contains('## Promedio general de ocupacion')) {
            continue
        }
        $brkga = @()

        if (Test-Path "$Directory/resultados.csv") {
            $brkga = @((Read-PoolText "$Directory/resultados.csv") | ConvertFrom-Csv)
        }
        $footer = @(Get-BRComparisonAverages -MetaRows $meta -BeamRows $beam -BRKGARows $brkga)
        Write-PoolText $path (@($text.TrimEnd()) + $footer)
    }

    if (!$Watch) {
        break
    }
    $state = (Read-PoolText "$Directory/estado.json") | ConvertFrom-Json

    if ($state.state -eq 'completado' -or $state.state -eq 'detenido' -or $state.state -like 'interrumpido*') {
        break
    }
    Start-Sleep -Seconds 2
} while ($true)
