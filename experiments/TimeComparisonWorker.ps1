<# .SYNOPSIS
Ejecuta tareas pendientes del protocolo comun por CPU, con claims exclusivos.
#>
param(
    [Parameter(Mandatory)][string]$Directory,
    [Parameter(Mandatory)][string]$DataRoot,
    [int]$Worker,
    [double]$Seconds = 30,
    [UInt64]$RootSeed = 123,
    [UInt64]$CpuMask
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/PoolIO.ps1"

try {
    [Diagnostics.Process]::GetCurrentProcess().ProcessorAffinity = [IntPtr]::new([long]$CpuMask)
    function Announce([string]$Status, [string]$Task) {
        Write-PoolProgress "$Directory/worker-$Worker.csv" @('worker,pid,status,task,cpu_mask', "$Worker,$PID,$Status,$Task,$CpuMask")
    }
    Announce 'starting' ''

    foreach ($task in @(Import-Csv "$Directory/plan.csv")) {
        if (Test-Path "$Directory/STOP") {
            Announce 'stopped' ''
            return
        }
        $name = "$($task.dataset)-i$($task.instance)-$($task.algorithm)"
        $folder = "$Directory/tasks/$name"

        if (Test-Path "$folder/result.csv") {
            continue
        }
        $claim = $null

        try {
            $claim = [IO.FileStream]::new("$Directory/claims/$name.lock", [IO.FileMode]::OpenOrCreate,
                [IO.FileAccess]::ReadWrite, [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)
        } catch [IO.IOException] {
            continue
        }

        try {
            if (Test-Path "$folder/result.csv") {
                continue
            }
            Announce 'working' $name
            Write-Output "[worker $Worker pid $PID] CLAIM $name"
            $attempt = "$folder/attempt-" + [guid]::NewGuid().ToString('N')
            $row = [ordered]@{
                dataset = $task.dataset
                instance = $task.instance
                algorithm = $task.algorithm
                occupation_percent = ''
                cpu_seconds = ''
                wall_seconds = ''
                cpu_limit_seconds = $Seconds
                clock = 'cpu'
                actual_seed = ''
                evaluations_started = ''
                generations = ''
                partial_evaluation = ''
                termination = ''
                valid = 1
                reused = 0
                worker = $Worker
                source_directory = $attempt
            }

            if ($task.algorithm -eq 'SCLP') {
                & "$Directory/runner/sclp/Run-SCLP.ps1" -Dataset $task.dataset -Instance $task.instance `
                -Seconds $Seconds -DataRoot $DataRoot -OutputDirectory $attempt -BinaryPath "$Directory/sclp_reference.exe"
                $run = Get-Content "$attempt/run.json" -Raw | ConvertFrom-Json

                if (!$run.verification.valid) {
                    throw 'SCLP audit failed'
                }
                $row.occupation_percent = $run.verification.occupation_percent
                $row.cpu_seconds = $run.execution.cpu_seconds_including_blocks
                $row.wall_seconds = $run.process_wall_seconds
                $row.termination = 'cooperative_solver_limit'
            } else {
                [IO.Directory]::CreateDirectory($attempt) | Out-Null

                if ($task.algorithm -eq 'BRKGA-U') {
                    & "$Directory/brkga2012.exe" --input "$DataRoot/Instance/BR/$($task.dataset).txt" `
                    --instance $task.instance --output $attempt --seconds $Seconds --clock cpu --root-seed $RootSeed
                } else {
                    & "$Directory/meta_time.exe" "$DataRoot/Instance/BR/$($task.dataset).txt" `
                    $task.instance $task.algorithm $attempt $Seconds $RootSeed
                }

                if ($LASTEXITCODE) {
                    throw "Solver failed: $name"
                }
                $verification = Get-Content "$attempt/verification.json" -Raw | ConvertFrom-Json
                $run = @(Import-Csv "$attempt/result.csv")[0]

                if (!$verification.valid -or $run.valid -ne '1' -or $run.clock -ne 'cpu' -or [double]$run.seconds -ne $Seconds) {
                    throw "Invalid result: $name"
                }

                foreach ($field in @('occupation_percent', 'cpu_seconds', 'wall_seconds', 'evaluations_started',
                    'generations', 'partial_evaluation', 'termination')) {
                    if ($run.PSObject.Properties[$field]) {
                        $row[$field] = $run.$field
                    }
                }
                $row.actual_seed = if ($task.algorithm -eq 'BRKGA-U') {
                    $run.seed
                } else {
                    $run.actual_seed
                }
            }
            Write-PoolText "$folder/result.csv" @([pscustomobject]$row | ConvertTo-Csv -NoTypeInformation)
            Write-Output "[worker $Worker] DONE $name occupation=$($row.occupation_percent)% CPU=$($row.cpu_seconds)s"
            Announce 'idle' $name
        } finally {
            if ($claim) {
                $claim.Dispose()
            }
        }
    }
    Announce 'finished' ''
} catch {
    Write-Error $_
    exit 1
}
