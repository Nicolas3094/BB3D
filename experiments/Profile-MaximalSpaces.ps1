<#
.SYNOPSIS
Mide las fases de Espacios Maximos con los ocho genomas de la comparacion previa.
.DESCRIPTION
Compila versiones normal e instrumentada con las mismas opciones. Realiza un
calentamiento y alterna el orden de las repeticiones. Comprueba que cada acomodo
coincida con la referencia y guarda tiempos, contadores y hashes.
.PARAMETER SkipBuild
Reutiliza los dos ejecutables ya compilados en build/maximal-spaces-profile.
#>
param(
    [string]$OutputDirectory = 'Results/MaximalSpaces/profile-v1',
    [string]$ReferenceDirectory = 'Results/MaximalSpaces/paired-smoke',
    [int]$Repetitions = 5,
    [string]$Compiler = 'C:/mingw64/bin/g++.exe',
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$previousPath = $env:PATH
Push-Location $root

try {
    if ($Repetitions -lt 1) {
        throw 'Repetitions must be positive'
    }

    if (Test-Path $OutputDirectory) {
        throw 'Use a new output directory'
    }
    $env:PATH = (Split-Path $Compiler -Parent) + ';' + $env:PATH
    $buildDirectory = 'build/maximal-spaces-profile'
    New-Item -ItemType Directory -Force $buildDirectory, $OutputDirectory | Out-Null

    if (!$SkipBuild) {
        $sources = @(Get-ChildItem src -Recurse -Filter *.cpp |
            Where-Object Name -ne main.cpp | ForEach-Object FullName)
        & $Compiler -std=c++14 -O3 -DNDEBUG experiments/Profile-MaximalSpaces.cpp $sources -o "$buildDirectory/normal.exe"

        if ($LASTEXITCODE) {
            throw 'Normal benchmark compilation failed'
        }
        & $Compiler -std=c++14 -O3 -DNDEBUG -DBB3D_PROFILE_MAXIMAL_SPACES experiments/Profile-MaximalSpaces.cpp $sources -o "$buildDirectory/profiled.exe"

        if ($LASTEXITCODE) {
            throw 'Profiled benchmark compilation failed'
        }
    }

    # Repetition zero warms both versions and is excluded from the aggregate.
    for ($repetition = 0; $repetition -le $Repetitions; $repetition++) {
        $modes = if ($repetition % 2) {
            @('profiled', 'normal')
        } else {
            @('normal', 'profiled')
        }

        foreach ($mode in $modes) {
            & "./$buildDirectory/$mode.exe" $OutputDirectory $ReferenceDirectory $repetition

            if ($LASTEXITCODE) {
                throw "$mode repetition $repetition failed"
            }
        }
    }

    $rows = @(Get-ChildItem "$OutputDirectory/*.csv" | ForEach-Object {
            Import-Csv $_.FullName | Where-Object {
                [int]$_.repetition -gt 0
            }
        })
    $referenceRows = @(Import-Csv "$ReferenceDirectory/evaluations.csv" |
        Where-Object decoder -eq 'maximal-spaces')
    foreach ($row in $rows) {
        $label = "$($row.dataset)-g$($row.genome)"
        $reference = Join-Path $ReferenceDirectory "$label-maximal-spaces-packing.csv"
        $packing = Join-Path $OutputDirectory "$($row.mode)-r$($row.repetition)-$label-packing.csv"

        if ((Get-FileHash $reference).Hash -ne (Get-FileHash $packing).Hash) {
            throw "Packing changed: $packing"
        }
        $expected = @($referenceRows | Where-Object {
                $_.dataset -eq $row.dataset -and $_.genome -eq $row.genome
            })
        if ($expected.Count -ne 1 -or $row.valid -ne '1' -or
            [double]$row.occupation_percent -ne [double]$expected[0].occupation_percent -or
            [int]$row.peak_retained -ne [int]$expected[0].peak_spaces) {
            throw "Evaluation differs from reference: $label"
        }
    }
    $rows | ConvertTo-Csv -NoTypeInformation | Set-Content "$OutputDirectory/measurements.csv"
    $inputHashes = @{}
    Get-ChildItem "$ReferenceDirectory/*-genome.csv" | ForEach-Object {
        $inputHashes[$_.Name] = (Get-FileHash $_.FullName).Hash
    }

    foreach ($dataset in @('BR1', 'BR15')) {
        $inputHashes[$dataset] = (Get-FileHash "Instance/BR/$dataset.txt").Hash
    }
    @{
        repetitions = $Repetitions
        warmup_repetitions_excluded = 1
        evaluations = $rows.Count
        identical_to_reference = $rows.Count
        flags = '-std=c++14 -O3 -DNDEBUG; instrumented adds -DBB3D_PROFILE_MAXIMAL_SPACES'
        normal_sha256 = (Get-FileHash "$buildDirectory/normal.exe").Hash
        profiled_sha256 = (Get-FileHash "$buildDirectory/profiled.exe").Hash
        input_sha256 = $inputHashes
        clock = 'steady_clock wall time per phase; GetProcessTimes total user+kernel CPU'
        note = 'Pruning combines containment and duplicate removal. Phase timers exclude input reading, audit and export. Counters and extra intersection checks add instrumentation overhead. Other is total minus the four phase times.'
        workload_note = 'Other workloads are not stopped by this script. Wall times and instrumentation overhead are descriptive, not isolated performance estimates.'
    } | ConvertTo-Json | Set-Content "$OutputDirectory/manifest.json"
    Write-Host "PASS: $($rows.Count) measured evaluations, identical packings, occupancy and peak retained spaces"
}
finally {
    $env:PATH = $previousPath
    Pop-Location
}
