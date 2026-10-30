<#
.SYNOPSIS
Ejecuta BR1-BR15 con BRKGA-CLP-U en 15 procesos reanudables y publica la tabla conjunta.
#>
param(
    [Parameter(Mandatory)][string]$OutputDirectory,
    [ValidateRange(1, 15)][int]$Workers = 15,
    [ValidateRange(0.01, 3600)][double]$Seconds = 30,
    [ValidateRange(1, 1500)][int]$TaskLimit = 1500,
    [UInt64]$RootSeed = 123,
    [string]$MetaDirectory = 'Results/EqualIterations/BR1-BR15-N100-fifteen-workers',
    [string]$BeamDirectory = 'Results/TreeSearch/BR1-BR15-H30-fifteen-workers',
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
. "$PSScriptRoot/PoolIO.ps1"
. "$PSScriptRoot/BRComparisonAverages.ps1"
$previousPath = $env:PATH
Push-Location $root
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
$OutputDirectory = (Resolve-Path $OutputDirectory).Path

if (Test-Path "$OutputDirectory/STOP") {
    throw 'Remove STOP explicitly before resuming'
}
$lock = [IO.FileStream]::new("$OutputDirectory/manager.lock", [IO.FileMode]::OpenOrCreate,
    [IO.FileAccess]::ReadWrite, [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)
$processes = @{}
$started = [DateTime]::UtcNow
$runId = (Get-Date -Format yyyyMMdd-HHmmss) + '-' + [guid]::NewGuid().ToString('N').Substring(0,
    6)


function Publish([string]$State) {
    $rows = @(Get-ChildItem "$OutputDirectory/tasks" -Directory | ForEach-Object {
            if (Test-Path "$($_.FullName)/result.csv") {
                (Read-PoolText "$($_.FullName)/result.csv") | ConvertFrom-Csv
            }
        })
    if (@($rows | Group-Object dataset, instance | Where-Object Count -gt 1).Count -or
        @($rows | Where-Object {
            $_.valid -ne '1' -or $_.clock -ne 'cpu' -or [double]$_.seconds -ne $Seconds
        }).Count) {
        throw 'Invalid or duplicate BRKGA result'
    }

    if ($rows.Count) {
        Write-PoolProgress "$OutputDirectory/resultados.csv" @($rows | ConvertTo-Csv -NoTypeInformation)
    }
    $live = @(Get-ChildItem "$OutputDirectory/worker-*.csv" -ErrorAction SilentlyContinue | ForEach-Object {
            try {
                (Read-PoolText $_.FullName) | ConvertFrom-Csv
            } catch {
                Write-Warning $_
            }
        })
    $lines = @('# BR1-BR15: GA, ABC, Firefly, SCLP y BRKGA',
        "BRKGA: $State; $($rows.Count)/$TaskLimit casos; $Workers procesos seriales, $Seconds segundos CPU por caso.",
        '',
        'BRKGA-CLP-U es una reimplementacion por capas con parametros publicados: 3 poblaciones de 20M, TOP/BOT 15%, sesgo 0.7, migracion cada 15 generaciones y maximo 500 generaciones.',
        'Se registra si el limite llega durante inicializacion; cero generaciones significa que aun no se completo una ronda evolutiva de las tres poblaciones.',
        'GA/ABC/FFA: 100 iteraciones / 4020 evaluaciones. SCLP y BRKGA: 30 segundos CPU en el lote principal. Los presupuestos de las cinco columnas NO estan igualados.',
        '',
        '| BR | GA (%) | ABC (%) | Firefly (%) | SCLP (%) | BRKGA-U (%) | n/100 BRKGA | Generaciones medias BRKGA |',
        '| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |')
    for ($number = 1; $number -le 15; $number++) {
        $dataset = "BR$number"
        $values = @()

        foreach ($algorithm in @('GA', 'ABC', 'FFA')) {
            $group = @($script:meta | Where-Object {
                    $_.dataset -eq $dataset -and $_.algorithm -eq $algorithm
                })
            $values += if ($group.Count -eq 100) {
                '{0:F3}' -f (($group | ForEach-Object {
                        [double]$_.occupation_percent
                    } | Measure-Object -Average).Average)
            } else {
                'pendiente'
            }
        }
        $beamGroup = @($script:beam | Where-Object dataset -eq $dataset)
        $beamMean = if ($beamGroup.Count -eq 100) {
            '{0:F3}' -f (($beamGroup | ForEach-Object {
                    [double]$_.occupation_percent
                } | Measure-Object -Average).Average)
        } else {
            'pendiente'
        }
        $group = @($rows | Where-Object dataset -eq $dataset)
        $mean = 'pendiente'
        $generations = '-'

        if ($group.Count) {
            $mean = '{0:F3}' -f (($group | ForEach-Object {
                    [double]$_.occupation_percent
                } | Measure-Object -Average).Average)
            $generations = '{0:F2}' -f (($group | ForEach-Object {
                    [double]$_.generations
                } | Measure-Object -Average).Average)
        }
        $lines += '| {0} | {1} | {2} | {3} | {4} | {5} | {6} | {7} |' -f $dataset, $values[0], $values[1], $values[2], $beamMean, $mean, $group.Count, $generations
    }
    $lines += @(Get-BRComparisonAverages -MetaRows $script:meta -BeamRows $script:beam -BRKGARows $rows)
    Write-PoolProgress "$OutputDirectory/tabla.md" $lines
    # The previous raw results remain untouched; this is their combined presentation.
    if ($TaskLimit -eq 1500 -and $Seconds -eq 30) {
        Write-PoolProgress "$BeamDirectory/tabla.md" $lines
    }
    Write-PoolProgress "$OutputDirectory/estado.json" (@{
            state = $State
            completed = $rows.Count
            total = $TaskLimit
            manager_pid = $PID
            updated_utc = [DateTime]::UtcNow.ToString('o')
            elapsed_seconds = ([DateTime]::UtcNow - $started).TotalSeconds
            workers = $live
        } | ConvertTo-Json -Depth 6)
    Write-Host "$State`: $($rows.Count)/$TaskLimit BRKGA completos"

    foreach ($worker in $live | Where-Object status -eq working) {
        Write-Host "  Trabajador $($worker.worker) (PID $($worker.pid)): $($worker.task)"
    }
}

try {
    $env:PATH = 'C:\mingw64\bin;' + $env:PATH

    foreach ($name in @('tasks', 'claims', 'runner')) {
        New-Item -ItemType Directory -Force "$OutputDirectory/$name" | Out-Null
    }

    if (!(Test-Path "$OutputDirectory/manifest.json") -and !$SkipBuild) {
        & "$root/src/brkga/Build-BRKGA2012.ps1" -Test
    }
    $hashes = @{}
    1..15 | ForEach-Object {
        $hashes["BR$_"] = (Get-FileHash "$root/Instance/BR/BR$_.txt").Hash
    }
    $script:meta = @(Import-Csv "$MetaDirectory/resultados.csv")
    $script:beam = @(Import-Csv "$BeamDirectory/resultados.csv")

    if ($script:meta.Count -ne 4500 -or $script:beam.Count -ne 1500 -or
        @($script:meta | Where-Object valid -ne '1').Count -or @($script:beam | Where-Object valid -ne '1').Count -or
        @($script:meta | Group-Object dataset, instance, algorithm | Where-Object Count -ne 1).Count -or
        @($script:beam | Group-Object dataset, instance | Where-Object Count -ne 1).Count) {
        throw 'Invalid reference results'
    }

    foreach ($reference in @($MetaDirectory, $BeamDirectory)) {
        $referenceManifest = Get-Content "$reference/manifest.json" -Raw | ConvertFrom-Json

        foreach ($dataset in $hashes.Keys) {
            if ($referenceManifest.datasets_sha256.$dataset -ne $hashes[$dataset]) {
                throw "Dataset mismatch: $dataset"
            }
        }
    }

    if (Test-Path "$OutputDirectory/manifest.json") {
        $manifest = Get-Content "$OutputDirectory/manifest.json" -Raw | ConvertFrom-Json

        if ($manifest.seconds -ne $Seconds -or $manifest.root_seed -ne $RootSeed -or $manifest.tasks -ne $TaskLimit -or
            $manifest.binary_sha256 -ne (Get-FileHash "$OutputDirectory/brkga2012.exe").Hash) {
            throw 'BRKGA resume mismatch'
        }

        foreach ($dataset in $hashes.Keys) {
            if ($manifest.datasets_sha256.$dataset -ne $hashes[$dataset]) {
                throw "Changed dataset: $dataset"
            }
        }

        foreach ($entry in $manifest.runner_sha256.PSObject.Properties) {
            if ((Get-FileHash "$OutputDirectory/runner/$($entry.Name)").Hash -ne $entry.Value) {
                throw 'Frozen runner changed'
            }
        }
    } else {
        $verification = Get-Content "$root/build/brkga2012/verification.json" -Raw | ConvertFrom-Json

        if (!$verification.new_tests_passed -or
            $verification.binary_sha256 -ne (Get-FileHash "$root/build/brkga2012/brkga2012.exe").Hash) {
            throw 'BRKGA binary has not passed tests; run Build-BRKGA2012.ps1 -Test'
        }
        Copy-Item "$root/build/brkga2012/brkga2012.exe" "$OutputDirectory/brkga2012.exe"
        Copy-Item "$root/build/brkga2012/verification.json" "$OutputDirectory/verification-before-run.json"
        Copy-Item "$PSScriptRoot/BRKGA2012PoolWorker.ps1", "$PSScriptRoot/PoolIO.ps1" "$OutputDirectory/runner"
        $runnerHashes = @{}
        Get-ChildItem "$OutputDirectory/runner" -File | ForEach-Object {
            $runnerHashes[$_.Name] = (Get-FileHash $_.FullName).Hash
        }
        $sourceHashes = @{}
        Get-ChildItem "$root/src" -Recurse -File | Where-Object Extension -in '.cpp', '.hpp', '.cxx' | ForEach-Object {
            $sourceHashes[$_.FullName.Substring($root.Length + 1)] = (Get-FileHash $_.FullName).Hash
        }
        @{
            seconds = $Seconds
            clock = 'process_cpu'
            root_seed = $RootSeed
            tasks = $TaskLimit
            binary_sha256 = (Get-FileHash "$OutputDirectory/brkga2012.exe").Hash
            datasets_sha256 = $hashes
            runner_sha256 = $runnerHashes
            profile = 'BRKGA-CLP-U independent reimplementation; Table3; serial fitness; 3 populations of 20M; TOP/BOT .15; bias .7; migration 15; migrants 2; max generations 500'
            source_sha256 = $sourceHashes
            flags = '-std=c++14 -O3 -DNDEBUG'
            rng = 'mt19937_64'
            meta_reference = $MetaDirectory
            beam_reference = $BeamDirectory
        } | ConvertTo-Json -Depth 8 | Set-Content "$OutputDirectory/manifest.json"
    }
    $topology = @(& "$OutputDirectory/brkga2012.exe" --topology | ForEach-Object {
            $fields = $_ -split ' '
            [pscustomobject]@{
                logical = [UInt64]$fields[1]
                physical = [UInt64]$fields[2]
            }
        })
    if ($LASTEXITCODE) {
        throw 'Cannot query processor topology'
    }
    $masks = @($topology | Select-Object -Skip 1 | ForEach-Object logical)

    foreach ($core in $topology) {
        $remaining = $core.physical -bxor $core.logical

        for ($bit = 0; $bit -lt 64; $bit++) {
            $mask = [UInt64]1 -shl $bit

            if (($remaining -band $mask) -ne 0) {
                $masks += $mask
            }
        }
    }

    if ($masks.Count -lt $Workers) {
        throw 'Not enough logical processors'
    }
    $hostExecutable = (Get-Process -Id $PID).Path

    for ($worker = 1; $worker -le $Workers; $worker++) {
        $arguments = @('-NoProfile', '-NonInteractive', '-File', ('"' + $OutputDirectory + '/runner/BRKGA2012PoolWorker.ps1"'),
            '-Directory', ('"' + $OutputDirectory + '"'), '-DataRoot', ('"' + $root + '"'), '-Worker', $worker,
            '-TaskLimit', $TaskLimit, '-Seconds', $Seconds, '-RootSeed', $RootSeed, '-CpuMask', $masks[$worker - 1])
        $processes[$worker] = Start-Process $hostExecutable -ArgumentList $arguments -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput "$OutputDirectory/worker-$worker-$runId.log" `
        -RedirectStandardError "$OutputDirectory/worker-$worker-$runId-errors.log"
        $null = $processes[$worker].Handle
    }
    do {
        Publish 'ejecutando'

        foreach ($process in $processes.Values) {
            if ($process.HasExited -and $process.ExitCode -ne 0) {
                throw 'BRKGA worker failed; inspect its log'
            }
        }
        Start-Sleep -Seconds 15
    } while (@($processes.Values | Where-Object {
            !$_.HasExited
        }).Count)
    foreach ($process in $processes.Values) {
        $process.WaitForExit()

        if ($process.ExitCode -ne 0) {
            throw 'BRKGA worker failed'
        }
    }
    $state = if (Test-Path "$OutputDirectory/STOP") {
        'detenido'
    } else {
        'completado'
    }
    Publish $state

    if ($state -eq 'completado' -and @(Get-ChildItem "$OutputDirectory/tasks" -Directory | Where-Object {
            Test-Path "$($_.FullName)/result.csv"
        }).Count -ne $TaskLimit) {
        throw 'Incomplete BRKGA batch'
    }
}
catch {
    foreach ($process in $processes.Values) {
        if (!$process.HasExited) {
            $process.Kill()
            $process.WaitForExit()
        }
    }
    Publish ('interrumpido: ' + $_.Exception.Message)
    throw
}
finally {
    $lock.Dispose()
    $env:PATH = $previousPath
    Pop-Location
}
