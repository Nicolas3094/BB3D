<#
.SYNOPSIS
Compara cinco algoritmos a 30 segundos CPU en BR1-BR17, conservando el lote N100.
.PARAMETER TaskLimit
0: protocolo completo con reutilizacion validada; un valor positivo crea un lote
corto sin importar casos antiguos y no modifica las tablas principales.
#>
param(
    [Parameter(Mandatory)][string]$OutputDirectory,
    [ValidateRange(1, 15)][int]$Workers = 15,
    [ValidateRange(1, 3600)][double]$Seconds = 30,
    [ValidateRange(0, 8500)][int]$TaskLimit = 0,
    [UInt64]$RootSeed = 123,
    [string]$MetaDirectory = 'Results/EqualIterations/BR1-BR15-N100-fifteen-workers',
    [string]$BeamDirectory = 'Results/TreeSearch/BR1-BR15-H30-fifteen-workers',
    [string]$BRKGADirectory = 'Results/BRKGA2012/BR1-BR15-U30-fifteen-workers',
    [switch]$SkipBuild,
    [switch]$FreshRun,
    [switch]$PhysicalCoresOnly
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
. "$PSScriptRoot/PoolIO.ps1"
. "$PSScriptRoot/TimeComparisonReport.ps1"
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
$total = 0


function Publish([string]$State) {
    $rows = @(Get-ChildItem "$OutputDirectory/tasks" -Directory | ForEach-Object {
            if (Test-Path "$($_.FullName)/result.csv") {
                (Read-PoolText "$($_.FullName)/result.csv") | ConvertFrom-Csv
            }
        })
    if (@($rows | Group-Object dataset, instance, algorithm | Where-Object Count -gt 1).Count -or
        @($rows | Where-Object {
            $_.valid -ne '1' -or $_.clock -ne 'cpu' -or [double]$_.cpu_limit_seconds -ne $Seconds
        }).Count) {
        throw 'Invalid or duplicate time-comparison result'
    }

    if ($rows.Count) {
        Write-PoolProgress "$OutputDirectory/resultados.csv" @($rows | ConvertTo-Csv -NoTypeInformation)
    }
    $lines = @(Get-TimeComparisonReport -Rows $rows -State $State -Total $total -Seconds $Seconds -Workers $Workers -FreshRun ([bool]$FreshRun))
    Write-PoolProgress "$OutputDirectory/tabla.md" $lines

    if ($TaskLimit -eq 0 -and $Seconds -eq 30) {
        Write-PoolProgress "$BeamDirectory/tabla.md" $lines
        Write-PoolProgress "$BRKGADirectory/tabla.md" $lines
    }
    $live = @(Get-ChildItem "$OutputDirectory/worker-*.csv" -ErrorAction SilentlyContinue | ForEach-Object {
            try {
                (Read-PoolText $_.FullName) | ConvertFrom-Csv
            } catch {
                Write-Warning $_
            }
        })
    $reused = @($rows | Where-Object reused -eq '1').Count
    Write-PoolProgress "$OutputDirectory/estado.json" (@{
            state = $State
            completed = $rows.Count
            total = $total
            reused = $reused
            new_completed = $rows.Count - $reused
            manager_pid = $PID
            updated_utc = [DateTime]::UtcNow.ToString('o')
            elapsed_seconds = ([DateTime]::UtcNow - $started).TotalSeconds
            workers = $live
        } | ConvertTo-Json -Depth 6)
    Write-Host "$State`: $($rows.Count)/$total disponibles; $reused reutilizados; $($rows.Count - $reused) nuevos"

    foreach ($worker in $live | Where-Object status -eq working) {
        Write-Host "  Trabajador $($worker.worker) (PID $($worker.pid)): $($worker.task)"
    }
}


function Import-Reference([string]$Directory, [object[]]$Rows, [string]$Algorithm, [string]$TaskSuffix) {
    foreach ($source in $Rows) {
        $sourceDirectory = "$Directory/tasks/$($source.dataset)-i$($source.instance)-$TaskSuffix/$($source.attempt)"
        $audit = (Read-PoolText "$sourceDirectory/verification.json") | ConvertFrom-Json

        if (!$audit.valid) {
            throw "Invalid reference audit: $sourceDirectory"
        }
        $row = [ordered]@{
            dataset = $source.dataset
            instance = $source.instance
            algorithm = $Algorithm
            occupation_percent = $source.occupation_percent
            cpu_seconds = $source.cpu_seconds
            wall_seconds = $source.wall_seconds
            cpu_limit_seconds = 30
            clock = 'cpu'
            actual_seed = ''
            evaluations_started = ''
            generations = ''
            partial_evaluation = ''
            termination = 'reused_verified_reference'
            valid = 1
            reused = 1
            worker = 0
            source_directory = (Resolve-Path $sourceDirectory).Path
        }

        if ($Algorithm -eq 'BRKGA-U') {
            $row.actual_seed = $source.seed

            foreach ($field in @('evaluations_started', 'generations', 'partial_evaluation', 'termination')) {
                $row[$field] = $source.$field
            }
        }
        $folder = "$OutputDirectory/tasks/$($source.dataset)-i$($source.instance)-$Algorithm"
        [IO.Directory]::CreateDirectory($folder) | Out-Null
        Write-PoolText "$folder/result.csv" @([pscustomobject]$row | ConvertTo-Csv -NoTypeInformation)
    }
}

try {
    $env:PATH = 'C:\mingw64\bin;' + $env:PATH

    foreach ($name in @('tasks', 'claims', 'runner', 'reference-before-cpu30')) {
        New-Item -ItemType Directory -Force "$OutputDirectory/$name" | Out-Null
    }
    $hashes = @{}
    1..17 | ForEach-Object {
        $hashes["BR$_"] = (Get-FileHash "$root/Instance/BR/BR$_.txt").Hash
    }
    $beamManifest = Get-Content "$BeamDirectory/manifest.json" -Raw | ConvertFrom-Json
    $brkgaManifest = Get-Content "$BRKGADirectory/manifest.json" -Raw | ConvertFrom-Json
    $metaManifest = Get-Content "$MetaDirectory/manifest.json" -Raw | ConvertFrom-Json

    foreach ($reference in @($beamManifest, $brkgaManifest, $metaManifest)) {
        1..15 | ForEach-Object {
            if ($reference.datasets_sha256."BR$_" -ne $hashes["BR$_"]) {
                throw "Reference dataset mismatch: BR$_"
            }
        }
    }
    $beamRows = @(Import-Csv "$BeamDirectory/resultados.csv")
    $brkgaRows = @(Import-Csv "$BRKGADirectory/resultados.csv")

    foreach ($group in @(@{
            rows = $beamRows
            manifest = $beamManifest
        }, @{
            rows = $brkgaRows
            manifest = $brkgaManifest
        })) {
        if ($group.rows.Count -ne 1500 -or @($group.rows | Where-Object valid -ne '1').Count -or
            @($group.rows | Group-Object dataset, instance | Where-Object Count -ne 1).Count) {
            throw 'Incomplete SCLP/BRKGA reference'
        }
    }

    if ($beamManifest.cpu_seconds -ne 30 -or $brkgaManifest.seconds -ne 30 -or $brkgaManifest.clock -ne 'process_cpu') {
        throw 'Reference time-budget mismatch'
    }

    if ($beamManifest.binary_sha256 -ne (Get-FileHash "$BeamDirectory/sclp_reference.exe").Hash -or
        $brkgaManifest.binary_sha256 -ne (Get-FileHash "$BRKGADirectory/brkga2012.exe").Hash) {
        throw 'Reference binary mismatch'
    }

    if (Test-Path "$OutputDirectory/manifest.json") {
        $manifest = Get-Content "$OutputDirectory/manifest.json" -Raw | ConvertFrom-Json

        if ($manifest.seconds -ne $Seconds -or $manifest.root_seed -ne $RootSeed -or $manifest.task_limit -ne $TaskLimit -or
            [bool]$manifest.fresh_run -ne [bool]$FreshRun) {
            throw 'Time-comparison resume configuration mismatch'
        }

        if ($manifest.previous_meta_csv_sha256 -ne (Get-FileHash "$MetaDirectory/resultados.csv").Hash -or
            $manifest.beam_reference_csv_sha256 -ne (Get-FileHash "$BeamDirectory/resultados.csv").Hash -or
            $manifest.brkga_reference_csv_sha256 -ne (Get-FileHash "$BRKGADirectory/resultados.csv").Hash) {
            throw 'Reference results changed after the comparison was created'
        }

        foreach ($dataset in $hashes.Keys) {
            if ($manifest.datasets_sha256.$dataset -ne $hashes[$dataset]) {
                throw "Changed input $dataset"
            }
        }

        foreach ($entry in $manifest.frozen_sha256.PSObject.Properties) {
            if ((Get-FileHash "$OutputDirectory/$($entry.Name)").Hash -ne $entry.Value) {
                throw "Frozen file changed: $($entry.Name)"
            }
        }
        $total = $manifest.total
    } else {
        if (!$SkipBuild) {
            & "$PSScriptRoot/Build-TimeComparison.ps1"
        }
        $verification = Get-Content "$root/build/time-comparison/verification.json" -Raw | ConvertFrom-Json

        if (!$verification.tests_passed -or $verification.binary_sha256 -ne (Get-FileHash "$root/build/time-comparison/meta_time.exe").Hash) {
            throw 'Run Build-TimeComparison.ps1 before starting the batch'
        }
        Copy-Item "$root/build/time-comparison/meta_time.exe" "$OutputDirectory/meta_time.exe"
        Copy-Item "$root/build/time-comparison/verification.json" "$OutputDirectory/verification-before-run.json"
        Copy-Item "$BeamDirectory/sclp_reference.exe", "$BRKGADirectory/brkga2012.exe" $OutputDirectory
        Copy-Item "$PSScriptRoot/TimeComparisonWorker.ps1", "$PSScriptRoot/PoolIO.ps1" "$OutputDirectory/runner"
        New-Item -ItemType Directory -Force "$OutputDirectory/runner/sclp" | Out-Null

        foreach ($name in @('Run-SCLP.ps1', 'Convert-BR.ps1', 'Verify-Packing.ps1')) {
            Copy-Item "$BeamDirectory/runner/$name" "$OutputDirectory/runner/sclp/$name"
        }
        Copy-Item "$root/experiments/equal-iterations.json" "$OutputDirectory/profile-before-time-change.json"
        Copy-Item "$MetaDirectory/resultados.csv" "$OutputDirectory/reference-before-cpu30/meta-N100.csv"

        foreach ($reference in @(@{
                dir = $BeamDirectory
                name = 'beam'
            }, @{
                dir = $BRKGADirectory
                name = 'brkga'
            })) {
            Copy-Item "$($reference.dir)/tabla.md" "$OutputDirectory/reference-before-cpu30/$($reference.name)-tabla.md"
        }
        $reuse = !$FreshRun -and $TaskLimit -eq 0 -and $Seconds -eq 30

        if ($reuse) {
            Import-Reference $BeamDirectory $beamRows 'SCLP' 'SCLP'

            if ($brkgaManifest.root_seed -eq $RootSeed) {
                Import-Reference $BRKGADirectory $brkgaRows 'BRKGA-U' 'BRKGA2012'
            }
        }
        $plan = @(
            for ($instance = 1; $instance -le 100; $instance++) {
                foreach ($number in @(16, 17) + @(1..15)) {
                    foreach ($algorithm in @('GA', 'ABC', 'FFA', 'SCLP', 'BRKGA-U')) {
                        $name = "BR$number-i$instance-$algorithm"

                        if (!(Test-Path "$OutputDirectory/tasks/$name/result.csv")) {
                            [pscustomobject]@{
                                dataset = "BR$number"
                                instance = $instance
                                algorithm = $algorithm
                            }
                        }
                    }
                }
            }
        )

        if ($TaskLimit) {
            $plan = @($plan | Select-Object -First $TaskLimit)
        }
        $plan | ConvertTo-Csv -NoTypeInformation | Set-Content "$OutputDirectory/plan.csv"
        $total = if ($TaskLimit) {
            $plan.Count
        } else {
            8500
        }
        $frozen = @{}

        foreach ($path in @('meta_time.exe', 'sclp_reference.exe', 'brkga2012.exe', 'plan.csv',
            'runner/TimeComparisonWorker.ps1', 'runner/PoolIO.ps1', 'runner/sclp/Run-SCLP.ps1',
            'runner/sclp/Convert-BR.ps1', 'runner/sclp/Verify-Packing.ps1')) {
            $frozen[$path] = (Get-FileHash "$OutputDirectory/$path").Hash
        }
        @{
            protocol = 'BR1-BR17-CPU30-v1'
            seconds = $Seconds
            clock = 'process_cpu'
            root_seed = $RootSeed
            task_limit = $TaskLimit
            total = $total
            pending_at_creation = $plan.Count
            fresh_run = [bool]$FreshRun
            datasets_sha256 = $hashes
            frozen_sha256 = $frozen
            meta_profile = 'equal-iterations.json; only stopping rule changes'
            previous_meta_csv_sha256 = (Get-FileHash "$MetaDirectory/resultados.csv").Hash
            beam_reference_csv_sha256 = (Get-FileHash "$BeamDirectory/resultados.csv").Hash
            brkga_reference_csv_sha256 = (Get-FileHash "$BRKGADirectory/resultados.csv").Hash
            preserved_meta_directory = $MetaDirectory
            beam_reference = $BeamDirectory
            brkga_reference = $BRKGADirectory
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
        throw 'Cannot query topology'
    }
    # A physical-core run uses one logical processor from each distinct core.
    $masks = if ($PhysicalCoresOnly) {
        @($topology | ForEach-Object logical)
    } else {
        @($topology | Select-Object -Skip 1 | ForEach-Object logical)
    }

    foreach ($core in $topology) {
        if ($PhysicalCoresOnly) {
            continue
        }
        $remaining = $core.physical -bxor $core.logical

        for ($bit = 0; $bit -lt 64; $bit++) {
            $mask = [UInt64]1 -shl $bit

            if (($remaining -band $mask) -ne 0) {
                $masks += $mask
            }
        }
    }

    if ($masks.Count -lt $Workers) {
        throw 'Not enough logical CPUs'
    }
    @{
        workers = $Workers
        physical_cores_only = [bool]$PhysicalCoresOnly
        cpu_masks = @($masks | Select-Object -First $Workers)
        started_utc = [DateTime]::UtcNow.ToString('o')
    } | ConvertTo-Json -Depth 4 | Set-Content "$OutputDirectory/execution-$runId.json"
    $hostExecutable = (Get-Process -Id $PID).Path

    for ($worker = 1; $worker -le $Workers; $worker++) {
        $arguments = @('-NoProfile', '-NonInteractive', '-File', ('"' + $OutputDirectory + '/runner/TimeComparisonWorker.ps1"'),
            '-Directory', ('"' + $OutputDirectory + '"'), '-DataRoot', ('"' + $root + '"'), '-Worker', $worker,
            '-Seconds', $Seconds, '-RootSeed', $RootSeed, '-CpuMask', $masks[$worker - 1])
        $processes[$worker] = Start-Process $hostExecutable -ArgumentList $arguments -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput "$OutputDirectory/worker-$worker-$runId.log" `
        -RedirectStandardError "$OutputDirectory/worker-$worker-$runId-errors.log"
        $null = $processes[$worker].Handle
    }
    do {
        Publish 'ejecutando'

        foreach ($process in $processes.Values) {
            if ($process.HasExited -and $process.ExitCode -ne 0) {
                throw 'Time-comparison worker failed; inspect its log'
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
    Publish $state

    if ($state -eq 'completado' -and @(Get-ChildItem "$OutputDirectory/tasks" -Directory | Where-Object {
            Test-Path "$($_.FullName)/result.csv"
        }).Count -ne $total) {
        throw 'Incomplete comparison'
    }
} catch {
    foreach ($process in $processes.Values) {
        if (!$process.HasExited) {
            $process.Kill()
            $process.WaitForExit()
        }
    }
    Publish ('interrumpido: ' + $_.Exception.Message)
    throw
} finally {
    $lock.Dispose()
    $env:PATH = $previousPath
    Pop-Location
}
