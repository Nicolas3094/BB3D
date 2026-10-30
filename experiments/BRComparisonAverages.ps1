# Formatting only: importing this file never starts or modifies an experiment.

function Get-BRComparisonAverages {
    param(
        [object[]]$MetaRows,
        [object[]]$BeamRows,
        [object[]]$BRKGARows
    )

    $lines = @('', '## Promedio general de ocupacion', '',
        '| Algoritmo | Ocupacion media (%) | Casos incluidos | Estado |',
        '| --- | ---: | ---: | --- |')
    foreach ($algorithm in @('GA', 'ABC', 'FFA', 'SCLP', 'BRKGA-U')) {
        $group = switch ($algorithm) {
            'SCLP' {
                @($BeamRows)
                break
            }
            'BRKGA-U' {
                @($BRKGARows)
                break
            }
            default {
                @($MetaRows | Where-Object algorithm -eq $algorithm)
            }
        }
        $group = @($group)
        $mean = 'pendiente'

        if ($group.Count) {
            # Average individual results, not the already rounded class means.
            $mean = '{0:F3}' -f (($group | ForEach-Object {
                    [double]$_.occupation_percent
                } | Measure-Object -Average).Average)
        }
        $state = if ($group.Count -eq 1500) {
            'completo'
        } else {
            'parcial'
        }
        $lines += '| {0} | {1} | {2}/1500 | {3} |' -f $algorithm, $mean, $group.Count, $state
    }
    $lines += @('', 'Medias calculadas con los resultados individuales sin redondear. Los promedios marcados como parciales incluyen solo los casos terminados y no representan todo el conjunto BR1-BR15.')
    return $lines
}
