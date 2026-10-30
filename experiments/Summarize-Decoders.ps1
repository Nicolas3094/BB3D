param(
    [Parameter(Mandatory)][string]$Directory
)

$ErrorActionPreference = 'Stop'
$fixed = @(Import-Csv (Join-Path $Directory 'evaluations.csv'))
$search = @(Import-Csv (Join-Path $Directory 'search.csv'))
$report = [Collections.Generic.List[string]]::new()

foreach ($line in @('# Comparacion DBLF y contacto', '', 'BR0/1 y BR15/1; semillas 123 y 456. Compilacion -O3 -DNDEBUG; un hilo; cache desactivada.',
    'Mismos puntos legacy y genes de orientacion en la evaluacion pareada. Contacto evalua todos los candidatos factibles tras compactarlos.',
    '', '## Mismos genomas', '',
    '| Dataset | Genomas pareados | Contacto mejora / igual / empeora | Cambio medio (pp) | Tiempo medio DBLF (us) | Tiempo medio contacto (us) |',
    '| --- | ---: | --- | ---: | ---: | ---: |')) {
    $report.Add($line)
}

foreach ($dataset in @('BR0', 'BR15')) {
    $pairs = @($fixed | Where-Object dataset -eq $dataset | Group-Object seed, genome)
    $better = 0
    $equal = 0
    $worse = 0
    $delta = 0.0
    $legacyTime = 0.0
    $contactTime = 0.0

    foreach ($pair in $pairs) {
        $a = @($pair.Group | Where-Object decoder -eq 'dblf')
        $b = @($pair.Group | Where-Object decoder -eq 'contact')

        if ($a.Count -ne 1 -or $b.Count -ne 1) {
            throw 'Incomplete paired evaluation'
        }
        $difference = [double]$b[0].occupation_percent - [double]$a[0].occupation_percent

        if ($difference -gt 1e-9) {
            $better++
        } elseif ($difference -lt -1e-9) {
            $worse++
        } else {
            $equal++
        }
        $delta += $difference
        $legacyTime += [double]$a[0].microseconds
        $contactTime += [double]$b[0].microseconds
    }
    $n = $pairs.Count

    if ($n) {
        $report.Add(('| {0} | {1} | {2} / {3} / {4} | {5:F4} | {6:F1} | {7:F1} |' -f $dataset, $n, $better,
            $equal, $worse, ($delta / $n), ($legacyTime / $n), ($contactTime / $n)))
    }
}

foreach ($line in @('', '## Busqueda completa: 30 segundos por corrida', '',
    '| Dataset | Algoritmo | Semillas | Ocupacion media DBLF (%) | Ocupacion media contacto (%) | Cambio (pp) | Evaluaciones medias DBLF / contacto |',
    '| --- | --- | ---: | ---: | ---: | ---: | --- |')) {
    $report.Add($line)
}

foreach ($dataset in @('BR0', 'BR15')) {
    foreach ($algorithm in @('GA', 'ABC', 'FFA')) {
        $a = @($search | Where-Object {
                $_.dataset -eq $dataset -and $_.algorithm -eq $algorithm -and $_.decoder -eq 'dblf'
            })
        $b = @($search | Where-Object {
                $_.dataset -eq $dataset -and $_.algorithm -eq $algorithm -and $_.decoder -eq 'contact'
            })
        if ($a.Count -ne 2 -or $b.Count -ne 2) {
            throw "Incomplete search comparison $dataset $algorithm"
        }
        $meanA = ($a | ForEach-Object {
                [double]$_.occupation_percent
            } | Measure-Object -Average).Average
        $meanB = ($b | ForEach-Object {
                [double]$_.occupation_percent
            } | Measure-Object -Average).Average
        $evalA = ($a | ForEach-Object {
                [double]$_.evaluations
            } | Measure-Object -Average).Average
        $evalB = ($b | ForEach-Object {
                [double]$_.evaluations
            } | Measure-Object -Average).Average
        $report.Add(('| {0} | {1} | 2 | {2:F4} | {3:F4} | {4:F4} | {5:F0} / {6:F0} |' -f $dataset, $algorithm,
            $meanA, $meanB, ($meanB - $meanA), $evalA, $evalB))
    }
}

foreach ($line in @('', '## Validacion y alcance', '',
    'Cada acomodo fue auditado sin usar las funciones de colision, rotacion ni factibilidad del decodificador: IDs/cantidades, genes y permisos, dimensiones rotadas, limites y todas las parejas. Se recalculo el volumen.',
    'Se conservaron los genomas completos, las secuencias de insercion de incumbentes y los acomodos en CSV. Las evaluaciones interrumpidas se identifican como parciales. La reconstruccion del incumbente coincide exactamente con el fitness registrado.',
    'Las evaluaciones por corrida incluyen la evaluacion intentada al vencer el plazo. La comprobacion y exportacion quedan fuera del tiempo de busqueda.',
    'Son dos instancias y dos semillas: un experimento piloto, sin significancia estadistica ni superioridad general demostrada. No se eligio el mejor decodificador automaticamente.',
    'Los tiempos individuales son mediciones locales de pared; alternar el orden reduce sesgos, pero no elimina ruido del sistema. Las primeras corridas coincidieron con compilaciones de pruebas en otros procesos; las cifras de rendimiento son orientativas.',
    'Contacto no impone soporte completo, no genera proyecciones y no elige orientaciones nuevas. Un mayor contacto inmediato puede cerrar huecos y empeorar el acomodo final.')) {
    $report.Add($line)
}
$report | Set-Content (Join-Path $Directory 'resumen.md') -Encoding utf8
$report
