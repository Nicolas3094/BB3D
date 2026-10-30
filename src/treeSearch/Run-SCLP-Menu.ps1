param(
    [Parameter(Mandatory)][string]$DataRoot,
    [string]$Dataset = 'BR15',
    [int]$FirstInstance = 1,
    [int]$Count = 1,
    [int]$Rounds = 1,
    [Parameter(Mandatory)][string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
$rows = [Collections.Generic.List[object]]::new()
$report = Join-Path $OutputDirectory 'resumen.md'
$csv = Join-Path $OutputDirectory 'resultados.csv'
@('SCLP H: parametros publicados de dos etapas; memoizacion strict; 1 hilo.',
    'Limite: 30 segundos CPU por problema/ronda, incluye bloques.',
    "Dataset: $Dataset; desde $FirstInstance; cantidad $Count (0=todos); rondas $Rounds.",
    'Acomodo, parametros completos y verificacion geometrica en cada subcarpeta.') | Set-Content (Join-Path $OutputDirectory 'configuracion.txt')
try {
    '# SCLP: sesion en curso' | Set-Content $report
    $datasets = if ($Dataset -eq 'BR') {
        @(0..18 | ForEach-Object {
                "BR$_"
            })
    } else {
        @($Dataset)
    }

    foreach ($name in $datasets) {
        $header = Get-Content (Join-Path $DataRoot "Instance/BR/$name.txt") | Where-Object {
            $_.Trim()
        } | Select-Object -First 1
        $total = [int]$header.Trim()
        $last = if ($Count -eq 0) {
            $total
        } else {
            $FirstInstance + $Count - 1
        }

        if ($FirstInstance -lt 1 -or $last -gt $total -or $Rounds -lt 1) {
            throw 'Seleccion SCLP fuera de rango'
        }

        for ($instance = $FirstInstance; $instance -le $last; $instance++) {
            for ($round = 1; $round -le $Rounds; $round++) {
                $runDirectory = Join-Path $OutputDirectory "$name-problema-$instance-ronda-$round"
                & "$PSScriptRoot/Run-SCLP.ps1" -Dataset $name -Instance $instance -Seconds 30 -DataRoot $DataRoot -OutputDirectory $runDirectory
                $run = Get-Content (Join-Path $runDirectory 'run.json') -Raw | ConvertFrom-Json
                $rows.Add([pscustomobject]@{
                        dataset = $name
                        problema = $instance
                        algoritmo = 'SCLP'
                        ronda = $round
                        ocupacion_porcentaje = $run.verification.occupation_percent
                        tiempo_cpu_segundos = $run.execution.cpu_seconds_including_blocks
                        tiempo_total_segundos = $run.process_wall_seconds
                        verificado = $run.verification.valid
                        acomodo = "$runDirectory/packing.json"
                    })
                $rows | Export-Csv -LiteralPath $csv -NoTypeInformation -Encoding utf8
            }
        }
    }
    $lines = @('# Resultados SCLP H', '', 'Estado: completada', '',
        'Limite por corrida: 30 segundos CPU incluyendo bloques. Un hilo. Parametros publicados de dos etapas.',
        '', '| Dataset | Problema | Ronda | Ocupacion (%) | CPU (s) | Tiempo total (s) | Verificado |',
        '| --- | ---: | ---: | ---: | ---: | ---: | --- |')
    Write-Output "`n=== PORCENTAJE DE OCUPACION FINAL: SCLP ==="

    foreach ($row in $rows) {
        $lines += '| {0} | {1} | {2} | {3:F6} | {4:F6} | {5:F6} | {6} |' -f $row.dataset, $row.problema, $row.ronda, $row.ocupacion_porcentaje, $row.tiempo_cpu_segundos, $row.tiempo_total_segundos, $row.verificado
        Write-Output ('{0} | problema {1} | ronda {2} -> OCUPACION FINAL: {3:F6} %' -f $row.dataset,
            $row.problema, $row.ronda, $row.ocupacion_porcentaje)
    }
    $lines | Set-Content $report -Encoding utf8
} catch {
    @('# SCLP: sesion interrumpida', '', "$($_.Exception.Message)", '', 'Los resultados completados se conservan en resultados.csv y sus subcarpetas.') | Set-Content $report
    throw
}
