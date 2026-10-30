<# .SYNOPSIS
Piloto pareado exploratorio: mismos ocho casos a 30 s CPU, solos y concurrentes.
No modifica las tablas principales ni demuestra equivalencia estadistica.
#>
param(
    [string]$OutputDirectory = 'Results/TimeComparison/concurrency-pilot',
    [switch]$Resume
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$source = Join-Path $root 'Results/TimeComparison/BR1-BR17-CPU30-eight-workers-fresh'
$output = Join-Path $root $OutputDirectory

if ((Test-Path $output) -and !$Resume) {
    throw 'Use a new pilot directory or -Resume'
}
New-Item -ItemType Directory -Force $output | Out-Null
$cases = @(
    foreach ($dataset in @('BR1', 'BR15')) {
        foreach ($algorithm in @('GA', 'ABC', 'FFA')) {
            [pscustomobject]@{
                dataset = $dataset
                instance = 1
                algorithm = $algorithm
            }
        }
    }
    [pscustomobject]@{
        dataset = 'BR15'
        instance = 1
        algorithm = 'BRKGA-U'
    }
    [pscustomobject]@{
        dataset = 'BR15'
        instance = 1
        algorithm = 'SCLP'
    }
)

$hostExecutable = (Get-Process -Id $PID).Path
$masks = @(1, 4, 16, 64, 256, 1024, 4096, 16384)

foreach ($mode in @('serial', 'parallel')) {
    $directory = Join-Path $output $mode

    if (!(Test-Path "$directory/plan.csv")) {
        New-Item -ItemType Directory -Force $directory | Out-Null

        foreach ($name in @('tasks', 'claims')) {
            New-Item -ItemType Directory -Force "$directory/$name" | Out-Null
        }
        Copy-Item "$source/runner" "$directory/runner" -Recurse
        Copy-Item "$source/meta_time.exe", "$source/brkga2012.exe", "$source/sclp_reference.exe" $directory
        $cases | Export-Csv "$directory/plan.csv" -NoTypeInformation
    }
    $workers = if ($mode -eq 'serial') {
        1
    } else {
        8
    }
    $processes = @()
    Write-Output "Starting $mode ($workers workers)"

    for ($worker = 1; $worker -le $workers; $worker++) {
        $arguments = @('-NoProfile', '-NonInteractive', '-File', ('"' + $directory + '/runner/TimeComparisonWorker.ps1"'),
            '-Directory', ('"' + $directory + '"'), '-DataRoot', ('"' + $root + '"'), '-Worker', $worker,
            '-Seconds', 30, '-RootSeed', 123, '-CpuMask', $masks[$worker - 1])
        $processes += Start-Process $hostExecutable -ArgumentList $arguments -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput "$directory/worker-$worker.log" -RedirectStandardError "$directory/worker-$worker-errors.log"
    }
    do {
        Start-Sleep -Seconds 15
        $completed = @(Get-ChildItem "$directory/tasks" -Directory | Where-Object {
                Test-Path "$($_.FullName)/result.csv"
            })
        $count = $completed.Count
        Write-Output "$mode completed $count/8"
    } while (@($processes | Where-Object {
            !$_.HasExited
        }).Count)
    foreach ($process in $processes) {
        $process.WaitForExit()

        if ($process.ExitCode -ne 0) {
            throw 'Pilot worker failed'
        }
    }

    if ($count -ne 8) {
        throw 'Incomplete pilot'
    }
    $rows = @($completed | ForEach-Object {
            Import-Csv "$($_.FullName)/result.csv"
        })
    $rows | Export-Csv "$directory/results.csv" -NoTypeInformation
}
$serial = Import-Csv "$output/serial/results.csv"
$parallel = Import-Csv "$output/parallel/results.csv"
$comparison = foreach ($a in $serial) {
    $b = @($parallel | Where-Object {
            $_.dataset -eq $a.dataset -and $_.algorithm -eq $a.algorithm
        })[0]
    [pscustomobject]@{
        dataset = $a.dataset
        algorithm = $a.algorithm
        serial_occupation = $a.occupation_percent
        parallel_occupation = $b.occupation_percent
        difference_pp = ([double]$b.occupation_percent - [double]$a.occupation_percent)
        serial_evaluations = $a.evaluations_started
        parallel_evaluations = $b.evaluations_started
        evaluation_change_percent = if ([double]$a.evaluations_started -gt 0) {
            100 * ([double]$b.evaluations_started / [double]$a.evaluations_started - 1)
        } else {
            ''
        }
        serial_cpu = $a.cpu_seconds
        parallel_cpu = $b.cpu_seconds
        valid = ($a.valid -eq '1' -and $b.valid -eq '1')
    }
}
$comparison | Export-Csv "$output/comparison.csv" -NoTypeInformation
$comparison | Format-Table -AutoSize
