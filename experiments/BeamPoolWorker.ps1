param(
    [Parameter(Mandatory)][string]$Directory,
    [Parameter(Mandatory)][string]$DataRoot,
    [int]$Worker,
    [int]$TaskLimit = 1500,
    [double]$Seconds = 30,
    [UInt64]$CpuMask
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/PoolIO.ps1"

try {
    [Diagnostics.Process]::GetCurrentProcess().ProcessorAffinity = [IntPtr]::new([long]$CpuMask)
    $state = "$Directory/worker-$Worker.csv"
    function Announce([string]$Status, [string]$Task) {
        Write-PoolProgress $state @('worker,pid,status,task,cpu_mask', "$Worker,$PID,$Status,$Task,$CpuMask")
    }
    Announce 'starting' ''

    for ($task = 0; $task -lt $TaskLimit; $task++) {
        if (Test-Path "$Directory/STOP") {
            Announce 'stopped' ''
            return
        }
        $dataset = 'BR' + ([int][math]::Floor($task / 100) + 1)
        $instance = $task % 100 + 1
        $name = "$dataset-i$instance-SCLP"
        $folder = "$Directory/tasks/$name"

        if (Test-Path "$folder/result.csv") {
            continue
        }
        $claim = $null

        try {
            $claim = [IO.FileStream]::new("$Directory/claims/$name.lock", [IO.FileMode]::OpenOrCreate, [IO.FileAccess]::ReadWrite,
                [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)
        }
        catch [IO.IOException] {
            continue
        }

        try {
            if (Test-Path "$folder/result.csv") {
                continue
            }
            Announce 'working' $name
            Write-Output "[worker $Worker pid $PID] CLAIM $name"
            [IO.Directory]::CreateDirectory($folder) | Out-Null
            # Keep any interrupted attempt for diagnostics; never delete its data.
            $attempt = Join-Path $folder ('attempt-' + [guid]::NewGuid().ToString('N'))
            & "$Directory/runner/Run-SCLP.ps1" -Dataset $dataset -Instance $instance -Seconds $Seconds -DataRoot $DataRoot -OutputDirectory $attempt -BinaryPath "$Directory/sclp_reference.exe"
            $run = Get-Content "$attempt/run.json" -Raw | ConvertFrom-Json

            if (!$run.verification.valid) {
                throw 'Independent geometry check failed'
            }
            $row = [pscustomobject][ordered]@{
                dataset = $dataset
                instance = $instance
                algorithm = 'SCLP'
                occupation_percent = $run.verification.occupation_percent
                cpu_seconds = $run.execution.cpu_seconds_including_blocks
                wall_seconds = $run.process_wall_seconds
                cpu_limit_seconds = $Seconds
                boxes = $run.verification.boxes
                valid = 1
                worker = $Worker
                attempt = [IO.Path]::GetFileName($attempt)
            }
            Write-PoolText "$folder/result.csv" @($row | ConvertTo-Csv -NoTypeInformation)
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
