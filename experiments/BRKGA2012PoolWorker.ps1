<# .SYNOPSIS
Reclama casos BR con bloqueo exclusivo y ejecuta el BRKGA congelado del lote.
#>
param(
    [Parameter(Mandatory)][string]$Directory,
    [Parameter(Mandatory)][string]$DataRoot,
    [int]$Worker,
    [int]$TaskLimit = 1500,
    [double]$Seconds = 30,
    [UInt64]$RootSeed = 123,
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
        $name = "$dataset-i$instance-BRKGA2012"
        $folder = "$Directory/tasks/$name"

        if (Test-Path "$folder/result.csv") {
            continue
        }
        $claim = $null

        try {
            $claim = [IO.FileStream]::new("$Directory/claims/$name.lock", [IO.FileMode]::OpenOrCreate,
                [IO.FileAccess]::ReadWrite, [IO.FileShare]::None, 4096, [IO.FileOptions]::DeleteOnClose)
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
            $attempt = "$folder/attempt-" + [guid]::NewGuid().ToString('N')
            [IO.Directory]::CreateDirectory($attempt) | Out-Null
            & "$Directory/brkga2012.exe" --input "$DataRoot/Instance/BR/$dataset.txt" --instance $instance `
            --output $attempt --seconds $Seconds --clock cpu --root-seed $RootSeed

            if ($LASTEXITCODE) {
                throw "BRKGA failed: $name"
            }
            $verification = Get-Content "$attempt/verification.json" -Raw | ConvertFrom-Json
            $run = @(Import-Csv "$attempt/result.csv")[0]

            if (!$verification.valid -or !$verification.independent_geometry_audit -or $run.valid -ne '1' -or
                $run.clock -ne 'cpu' -or [double]$run.seconds -ne $Seconds) {
                throw "Invalid BRKGA result: $name"
            }
            $row = [ordered]@{
                dataset = $dataset
                instance = $instance
            }

            foreach ($property in $run.PSObject.Properties) {
                $row[$property.Name] = $property.Value
            }
            $row['worker'] = $Worker
            $row['attempt'] = [IO.Path]::GetFileName($attempt)
            Write-PoolText "$folder/result.csv" @([pscustomobject]$row | ConvertTo-Csv -NoTypeInformation)
            Write-Output "[worker $Worker] DONE $name occupation=$($run.occupation_percent)% generations=$($run.generations)"
            Announce 'idle' $name
        }
        finally {
            if ($claim) {
                $claim.Dispose()
            }
        }
    }
    Announce 'finished' ''
}
catch {
    Write-Error $_
    exit 1
}
