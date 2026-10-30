param(
    [Parameter(Mandatory)][string]$OutputDirectory,
    [ValidateRange(1, 15)][int]$Workers = 15,
    [ValidateRange(1, 3600)][double]$Seconds = 30,
    [ValidateRange(1, 1500)][int]$TaskLimit = 1500,
    [string]$MetaDirectory
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
. "$PSScriptRoot/PoolIO.ps1"
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
$OutputDirectory = (Resolve-Path $OutputDirectory).Path

foreach ($name in @('tasks', 'claims', 'runner')) {
    New-Item -ItemType Directory -Force "$OutputDirectory/$name" | Out-Null
}

if (Test-Path "$OutputDirectory/STOP") {
    throw 'Remove STOP explicitly before resuming'
}
$managerLock = [IO.FileStream]::new("$OutputDirectory/manager.lock", [IO.FileMode]::OpenOrCreate,
    [IO.FileAccess]::ReadWrite, [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)
$processes = @{}
$previousPath = $env:PATH
$started = [DateTime]::UtcNow
$runId = (Get-Date -Format yyyyMMdd-HHmmss) + '-' + [guid]::NewGuid().ToString('N').Substring(0,
    6)

function Publish([string]$State) {
    $rows = @(Get-ChildItem "$OutputDirectory/tasks" -Directory | ForEach-Object {
            if (Test-Path "$($_.FullName)/result.csv") {
                (Read-PoolText "$($_.FullName)/result.csv") | ConvertFrom-Csv
            }
        })
    if (@($rows | Where-Object {
            $_.valid -ne '1' -or [double]$_.cpu_limit_seconds -ne $Seconds
        }).Count -or @($rows | Group-Object dataset, instance | Where-Object Count -gt 1).Count) {
        throw 'Invalid/duplicate beam result'
    }

    if ($rows.Count) {
        try {
            Write-PoolText "$OutputDirectory/resultados.csv" @($rows | ConvertTo-Csv -NoTypeInformation)
        } catch {
            Write-Warning $_
        }
    }
    $live = @(Get-ChildItem "$OutputDirectory/worker-*.csv" -ErrorAction SilentlyContinue | ForEach-Object {
            try {
                (Read-PoolText $_.FullName) | ConvertFrom-Csv
            } catch {
                Write-Warning $_
            }
        })
    $meta = @()

    if ($MetaDirectory) {
        $meta = @(Import-Csv "$MetaDirectory/resultados.csv")
    }
    $lines = @('# BR1-BR15: SCLP tree/beam H', "Estado: $State; completadas $($rows.Count)/$TaskLimit.",
        '',
        "SCLP: $Seconds segundos CPU por caso incluyendo bloques; parametros publicados de dos etapas y memoizacion strict; $Workers procesos, un hilo de busqueda por proceso.",
        'Las metaheuristicas de referencia usan 100 iteraciones / 4020 evaluaciones. Los presupuestos NO son equivalentes; esta tabla no demuestra superioridad con esfuerzo igualado.',
        'Media parcial de beam hasta completar 100 instancias por BR. Cada acomodo pasa los controles originales y un auditor geometrico independiente.',
        '',
        '| BR | GA (%) | ABC (%) | Firefly (%) | SCLP beam (%) | n/100 beam | CPU media beam (s) |',
        '| --- | ---: | ---: | ---: | ---: | ---: | ---: |')
    for ($i = 1; $i -le 15; $i++) {
        $values = @()

        foreach ($a in @('GA', 'ABC', 'FFA')) {
            $group = @($meta | Where-Object {
                    $_.dataset -eq "BR$i" -and $_.algorithm -eq $a
                })
            $values += if ($group.Count -eq 100) {
                '{0:F3}' -f (($group | ForEach-Object {
                        [double]$_.occupation_percent
                    } | Measure-Object -Average).Average)
            } else {
                'pendiente'
            }
        }
        $group = @($rows | Where-Object dataset -eq "BR$i")
        $mean = 'pendiente'
        $cpu = '-'

        if ($group.Count) {
            $mean = '{0:F3}' -f (($group | ForEach-Object {
                    [double]$_.occupation_percent
                } | Measure-Object -Average).Average)
            $cpu = '{0:F3}' -f (($group | ForEach-Object {
                    [double]$_.cpu_seconds
                } | Measure-Object -Average).Average)
        }
        $lines += '| BR{0} | {1} | {2} | {3} | {4} | {5} | {6} |' -f $i, $values[0], $values[1], $values[2], $mean, $group.Count, $cpu
    }
    $lines += @('', '## Trabajadores', '', '| Trabajador | PID | Estado | Caso |', '| ---: | ---: | --- | --- |')

    foreach ($w in $live) {
        $lines += '| {0} | {1} | {2} | {3} |' -f $w.worker, $w.pid, $w.status, $w.task
    }
    Write-PoolProgress "$OutputDirectory/tabla.md" $lines
    Write-PoolProgress "$OutputDirectory/estado.json" (@{
            state = $State
            completed = $rows.Count
            total = $TaskLimit
            manager_pid = $PID
            updated_utc = [DateTime]::UtcNow.ToString('o')
            elapsed_seconds = ([DateTime]::UtcNow - $started).TotalSeconds
            workers = $live
        } | ConvertTo-Json -Depth 6)
    Write-Host "$State`: $($rows.Count)/$TaskLimit beam completos"

    foreach ($w in $live | Where-Object status -eq working) {
        Write-Host "  Trabajador $($w.worker) (PID $($w.pid)): $($w.task)"
    }
}

try {
    $env:PATH = 'C:\mingw64\bin;' + $env:PATH
    $topology = @(& "$root/build/equal_iteration_worker.exe" --topology | ForEach-Object {
            $p = $_ -split ' '
            [pscustomobject]@{
                logical_mask = [UInt64]$p[1]
                full_mask = [UInt64]$p[2]
            }
        })
    if ($LASTEXITCODE -ne 0) {
        throw 'Cannot query topology'
    }
    $masks = @($topology | Select-Object -Skip 1 | ForEach-Object {
            $_.logical_mask
        })
    foreach ($core in $topology) {
        $rest = $core.full_mask -bxor $core.logical_mask

        for ($bit = 0; $bit -lt 64; $bit++) {
            $mask = [UInt64]1 -shl $bit

            if (($rest -band $mask) -ne 0) {
                $masks += $mask
            }
        }
    }

    if ($masks.Count -lt $Workers) {
        throw 'Not enough logical CPUs'
    }
    $hashes = @{}
    1..15 | ForEach-Object {
        $hashes["BR$_"] = (Get-FileHash "$root/Instance/BR/BR$_.txt").Hash
    }

    if ($MetaDirectory) {
        $metaManifest = Get-Content "$MetaDirectory/manifest.json" -Raw | ConvertFrom-Json

        foreach ($name in $hashes.Keys) {
            if ($metaManifest.datasets_sha256.$name -ne $hashes[$name]) {
                throw "Meta/beam dataset mismatch: $name"
            }
        }
        $reference = @(Import-Csv "$MetaDirectory/resultados.csv")

        if ($reference.Count -ne 4500 -or @($reference | Group-Object dataset, instance, algorithm | Where-Object Count -ne 1).Count -or @($reference | Where-Object valid -ne '1').Count) {
            throw 'Metaheuristic reference is incomplete/invalid'
        }
    }

    if (Test-Path "$OutputDirectory/manifest.json") {
        $manifest = Get-Content "$OutputDirectory/manifest.json" -Raw | ConvertFrom-Json

        if ($manifest.cpu_seconds -ne $Seconds -or $manifest.tasks -ne $TaskLimit -or $manifest.binary_sha256 -ne (Get-FileHash "$OutputDirectory/sclp_reference.exe").Hash) {
            throw 'Resume mismatch'
        }

        foreach ($name in $hashes.Keys) {
            if ($manifest.datasets_sha256.$name -ne $hashes[$name]) {
                throw "Changed dataset $name"
            }
        }

        foreach ($entry in $manifest.runner_sha256.PSObject.Properties) {
            if ((Get-FileHash "$OutputDirectory/runner/$($entry.Name)").Hash -ne $entry.Value) {
                throw "Frozen runner changed: $($entry.Name)"
            }
        }
    } else {
        Copy-Item "$root/build/sclp-release/sclp_reference.exe" "$OutputDirectory/sclp_reference.exe"

        foreach ($file in @('Run-SCLP.ps1', 'Convert-BR.ps1', 'Verify-Packing.ps1')) {
            Copy-Item "$root/src/treeSearch/$file" "$OutputDirectory/runner/$file"
        }
        Copy-Item "$PSScriptRoot/BeamPoolWorker.ps1", "$PSScriptRoot/PoolIO.ps1" "$OutputDirectory/runner"
        $scriptHashes = @{}
        Get-ChildItem "$OutputDirectory/runner" -File | ForEach-Object {
            $scriptHashes[$_.Name] = (Get-FileHash $_.FullName).Hash
        }
        @{
            tasks = $TaskLimit
            cpu_seconds = $Seconds
            workers = $Workers
            binary_sha256 = (Get-FileHash "$OutputDirectory/sclp_reference.exe").Hash
            datasets_sha256 = $hashes
            runner_sha256 = $scriptHashes
            worker_cpu_masks = @($masks | Select-Object -First $Workers)
            profile = 'H two-stage; strict memoization; blocks 10000; branching 4..10000; automatic density'
            meta_reference = $MetaDirectory
            note = 'Metaheuristics: 100 iterations, different budget'
        } | ConvertTo-Json -Depth 8 | Set-Content "$OutputDirectory/manifest.json"
    }
    $hostExe = (Get-Process -Id $PID).Path

    for ($worker = 1; $worker -le $Workers; $worker++) {
        $args = @('-NoProfile', '-NonInteractive', '-File', ('"' + $OutputDirectory + '/runner/BeamPoolWorker.ps1"'),
            '-Directory', ('"' + $OutputDirectory + '"'), '-DataRoot', ('"' + $root + '"'), '-Worker', $worker,
            '-TaskLimit', $TaskLimit, '-Seconds', $Seconds, '-CpuMask', $masks[$worker - 1])
        $processes[$worker] = Start-Process $hostExe -ArgumentList $args -WindowStyle Hidden -PassThru -RedirectStandardOutput "$OutputDirectory/worker-$worker-$runId.log" -RedirectStandardError "$OutputDirectory/worker-$worker-$runId-errors.log"
        $null = $processes[$worker].Handle
    }
    do {
        Publish 'ejecutando'

        foreach ($p in $processes.Values) {
            if ($p.HasExited -and $p.ExitCode -ne 0) {
                throw 'Beam worker failed; inspect its log'
            }
        }
        Start-Sleep -Seconds 15
    } while (@($processes.Values | Where-Object {
            !$_.HasExited
        }).Count)
    foreach ($p in $processes.Values) {
        $p.WaitForExit()

        if ($p.ExitCode -ne 0) {
            throw 'Beam worker failed'
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
        throw 'Incomplete batch'
    }
} catch {
    foreach ($p in $processes.Values) {
        if (!$p.HasExited) {
            $p.Kill()
            $p.WaitForExit()
        }
    }
    Publish ('interrumpido: ' + $_.Exception.Message)
    throw
}
finally {
    $managerLock.Dispose()
    $env:PATH = $previousPath
}
