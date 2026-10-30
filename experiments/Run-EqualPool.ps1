param(
    [Parameter(Mandatory)][string]$OutputDirectory,
    [ValidateRange(1, 32)][int]$Workers = 7,
    [ValidateRange(0, 10000000)][int]$Iterations = 100,
    [ValidateRange(1, 4500)][int]$TaskLimit = 4500,
    [switch]$SkipBuild,
    [string]$Compiler = 'C:/mingw64/bin/g++.exe'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
$OutputDirectory = (Resolve-Path $OutputDirectory).Path
New-Item -ItemType Directory -Force "$OutputDirectory/tasks" | Out-Null
$binary = Join-Path $root 'build/equal_iteration_worker.exe'
$statusFile = "$OutputDirectory/estado.json"

if (Test-Path "$OutputDirectory/STOP") {
    throw 'Remove the STOP marker explicitly before resuming'
}
$managerLock = [IO.FileStream]::new("$OutputDirectory/manager.lock", [IO.FileMode]::OpenOrCreate,
    [IO.FileAccess]::ReadWrite, [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)
$processes = @{}
$previousPath = $env:PATH
$started = [DateTime]::UtcNow
$runId = (Get-Date -Format yyyyMMdd-HHmmss) + '-' + [guid]::NewGuid().ToString('N').Substring(0,
    6)

function Commit-Temporary([string]$Temporary, [string]$Path) {
    for ($attempt = 0; $attempt -lt 100; $attempt++) {
        try {
            Move-Item -LiteralPath $Temporary -Destination $Path -Force
            return
        }
        catch {
            if ($attempt -eq 99) {
                throw
            }
            Start-Sleep -Milliseconds 50
        }
    }
}

function Atomic-Text([string]$Path, [string[]]$Lines) {
    $temporary = $Path + '.tmp-' + [guid]::NewGuid().ToString('N')
    $Lines | Set-Content $temporary -Encoding utf8
    Commit-Temporary $temporary $Path
}

function Read-SharedCsv([string]$Path) {
    $stream = [IO.FileStream]::new($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read, ([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
    $reader = [IO.StreamReader]::new($stream)

    try {
        $text = $reader.ReadToEnd()
    } finally {
        $reader.Dispose()
        $stream.Dispose()
    }
    $text | ConvertFrom-Csv
}

function Publish([string]$State) {
    $rows = @(Get-ChildItem "$OutputDirectory/tasks" -Directory | ForEach-Object {
            if (Test-Path "$($_.FullName)/result.csv") {
                Import-Csv "$($_.FullName)/result.csv"
            }
        })
    if (@($rows | Where-Object {
            $_.valid -ne '1' -or [int]$_.iterations_requested -ne $Iterations
        }).Count) {
        throw 'Invalid completed task'
    }

    if (@($rows | Group-Object dataset, instance, algorithm | Where-Object Count -gt 1).Count) {
        throw 'Duplicate completed task'
    }

    if ($rows.Count) {
        try {
            $temporary = "$OutputDirectory/resultados.csv.tmp-" + [guid]::NewGuid().ToString('N')
            $rows | Export-Csv $temporary -NoTypeInformation -Encoding utf8
            Commit-Temporary $temporary "$OutputDirectory/resultados.csv"
        } catch {
            Write-Warning "Cannot update aggregate CSV: $($_.Exception.Message)"
        }
    }
    $live = @(Get-ChildItem "$OutputDirectory/worker-*.csv" -ErrorAction SilentlyContinue | ForEach-Object {
            try {
                Read-SharedCsv $_.FullName
            } catch {
                Write-Warning "Cannot read progress file: $($_.Exception.Message)"
            }
        })
    $lines = @('# Perfil igualado: BR1-BR15', "Estado: $State; completadas $($rows.Count)/$TaskLimit.",
        '',
        "$Workers procesos, un hilo por proceso, cola dinamica sin duplicados. $Iterations iteraciones, 20 iniciales + 40 evaluaciones por iteracion.",
        'Contacto, C2, adicion de candidatos, poblacion 20, semilla 123, seis orientaciones con permisos; sin limite de 30 segundos.',
        'Medias parciales hasta completar 100 instancias por celda. Cada acomodo se audita geometricamente.',
        '', '| BR | GA (%) | n/100 | ABC (%) | n/100 | Firefly (%) | n/100 |', '| --- | ---: | ---: | ---: | ---: | ---: | ---: |')
    for ($i = 1; $i -le 15; $i++) {
        $cells = @()

        foreach ($a in @('GA', 'ABC', 'FFA')) {
            $subset = @($rows | Where-Object {
                    $_.dataset -eq "BR$i" -and $_.algorithm -eq $a
                })
            $cells += if ($subset.Count) {
                '{0:F3}' -f (($subset | ForEach-Object {
                        [double]$_.occupation_percent
                    } | Measure-Object -Average).Average)
            } else {
                'pendiente'
            }
            $cells += [string]$subset.Count
        }
        $lines += '| BR{0} | {1} | {2} | {3} | {4} | {5} | {6} |' -f $i, $cells[0], $cells[1], $cells[2], $cells[3], $cells[4], $cells[5]
    }
    $lines += @('', '## Trabajadores activos', '', '| Proceso | PID | Estado | Caso |', '| ---: | ---: | --- | --- |')

    foreach ($worker in $live) {
        $lines += '| {0} | {1} | {2} | {3} |' -f $worker.worker, $worker.pid, $worker.status, $worker.task
    }

    try {
        Atomic-Text "$OutputDirectory/tabla.md" $lines
    } catch {
        Write-Warning "Cannot update progress table: $($_.Exception.Message)"
    }
    $elapsed = ([DateTime]::UtcNow - $started).TotalSeconds

    try {
        Atomic-Text $statusFile (@{
                state = $State
                completed = $rows.Count
                total = $TaskLimit
                manager_pid = $PID
                updated_utc = [DateTime]::UtcNow.ToString('o')
                elapsed_seconds = $elapsed
                workers = $live
            } | ConvertTo-Json -Depth 6)
    } catch {
        Write-Warning "Cannot update progress status: $($_.Exception.Message)"
    }
    Write-Host ("{0}: {1}/{2} completos" -f $State, $rows.Count, $TaskLimit)

    foreach ($worker in $live | Where-Object status -eq 'working') {
        Write-Host ("  Trabajador {0} (PID {1}) trabajando en {2}" -f $worker.worker, $worker.pid, $worker.task)
    }
    return $rows.Count
}

try {
    $env:PATH = (Split-Path $Compiler -Parent) + ';' + $env:PATH
    $resuming = Test-Path "$OutputDirectory/manifest.json"

    if ($resuming) {
        $binary = Join-Path $OutputDirectory 'equal_iteration_worker.exe'
    }

    if (!$SkipBuild -and !$resuming) {
        $sources = @(Join-Path $PSScriptRoot 'EqualIterationWorker.cpp') + @(Get-ChildItem "$root/src" -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
        & $Compiler -std=c++14 -O3 -DNDEBUG $sources -o $binary

        if ($LASTEXITCODE -ne 0) {
            throw 'Worker compilation failed'
        }
    }
    $topology = @(& $binary --topology | ForEach-Object {
            $parts = $_ -split ' '
            [pscustomobject]@{
                core = [int]$parts[0]
                logical_mask = $parts[1]
                full_mask = $parts[2]
            }
        })
    if ($LASTEXITCODE -ne 0 -or !$topology.Count) {
        throw 'Cannot read CPU topology'
    }
    # First spread across physical cores, then use their other SMT siblings.
    # Keep logical CPU 0 unassigned to the search in either mode.
    $cpuMasks = @()

    foreach ($core in $topology | Select-Object -Skip 1) {
        $cpuMasks += [UInt64]$core.logical_mask
    }

    foreach ($core in $topology) {
        $remaining = [UInt64]$core.full_mask -bxor [UInt64]$core.logical_mask

        for ($bit = 0; $bit -lt 64; $bit++) {
            $mask = [UInt64]1 -shl $bit

            if (($remaining -band $mask) -ne 0) {
                $cpuMasks += $mask
            }
        }
    }

    if ($Workers -gt $cpuMasks.Count) {
        throw 'Not enough logical CPUs after reserving logical CPU 0'
    }
    $hashes = @{}
    1..15 | ForEach-Object {
        $hashes["BR$_"] = (Get-FileHash "$root/Instance/BR/BR$_.txt").Hash
    }
    $manifest = @{
        iterations = $Iterations
        tasks = $TaskLimit
        workers = $Workers
        worker_cpu_masks = @($cpuMasks | Select-Object -First $Workers)
        reserved_logical_cpu = 0
        profile = (Get-Content "$PSScriptRoot/equal-iterations.json" -Raw | ConvertFrom-Json)
        binary_sha256 = (Get-FileHash $binary).Hash
        datasets_sha256 = $hashes
        topology = $topology
        flags = '-O3 -DNDEBUG -std=c++14'
        compiler = (& $Compiler --version | Select-Object -First 1)
    }

    if (Test-Path "$OutputDirectory/manifest.json") {
        $old = Get-Content "$OutputDirectory/manifest.json" -Raw | ConvertFrom-Json

        if ($old.iterations -ne $Iterations -or $old.tasks -ne $TaskLimit -or $old.binary_sha256 -ne $manifest.binary_sha256) {
            throw 'Resume configuration/binary changed; use a new output folder'
        }

        foreach ($name in $hashes.Keys) {
            if ($old.datasets_sha256.$name -ne $hashes[$name]) {
                throw "Dataset changed: $name"
            }
        }
    } else {
        $manifest | ConvertTo-Json -Depth 12 | Set-Content "$OutputDirectory/manifest.json"
        Copy-Item $binary "$OutputDirectory/equal_iteration_worker.exe"
    }

    for ($worker = 1; $worker -le $Workers; $worker++) {
        $mask = $cpuMasks[$worker - 1]
        $processes[$worker] = Start-Process -FilePath $binary -ArgumentList @(('"' + $OutputDirectory + '"'),
            ('"' + $root + '"'), $worker, $Iterations, $TaskLimit, $mask) -WindowStyle Hidden -PassThru -RedirectStandardOutput "$OutputDirectory/worker-$worker-$runId.log" -RedirectStandardError "$OutputDirectory/worker-$worker-$runId-errors.log"
        $null = $processes[$worker].Handle
    }
    do {
        $null = Publish 'ejecutando'

        foreach ($process in $processes.Values) {
            if ($process.HasExited -and $process.ExitCode -ne 0) {
                throw 'Worker failed; inspect its error log'
            }
        }
        Start-Sleep -Seconds 15
    } while (@($processes.Values | Where-Object {
            !$_.HasExited
        }).Count)
    foreach ($process in $processes.Values) {
        $process.WaitForExit()

        if ($process.ExitCode -ne 0) {
            throw 'Worker failed'
        }
    }
    $state = if (Test-Path "$OutputDirectory/STOP") {
        'detenido'
    } else {
        'completado'
    }
    $null = Publish $state
    $finishedCount = @(Get-ChildItem "$OutputDirectory/tasks" -Directory | Where-Object {
            Test-Path "$($_.FullName)/result.csv"
        }).Count
    if ($state -eq 'completado' -and $finishedCount -ne $TaskLimit) {
        throw 'Incomplete batch'
    }
} catch {
    foreach ($process in $processes.Values) {
        if (!$process.HasExited) {
            $process.Kill()
            $process.WaitForExit()
        }
    }
    $null = Publish ('interrumpido: ' + $_.Exception.Message)
    throw
} finally {
    $env:PATH = $previousPath
    $managerLock.Dispose()
}
