param(
    [Parameter(Mandatory)][string]$Directory,
    [switch]$Watch
)

$ErrorActionPreference = 'Stop'
do {
    $state = Get-Content (Join-Path $Directory 'estado.json') -Raw | ConvertFrom-Json
    Write-Host ("{0}: {1}/{2} casos completos; actualizado (UTC): {3}" -f $state.state, $state.completed,
        $state.total, $state.updated_utc)
    $state.workers | Select-Object @{
        Name = 'Trabajador'
        Expression = {
            $_.worker
        }
    }, pid, status, @{
        Name = 'Caso'
        Expression = {
            $_.task
        }
    } | Format-Table -AutoSize | Out-Host

    if (!$Watch -or $state.state -ne 'ejecutando') {
        break
    }
    Start-Sleep -Seconds 15
} while ($true)
