# Pure report formatting. Does not start searches or modify source results.

function Get-TimeComparisonReport {
    param(
        [object[]]$Rows,
        [string]$State,
        [int]$Total,
        [double]$Seconds,
        [int]$Workers = 15,
        [bool]$FreshRun = $false
    )

    $algorithms = @('GA', 'ABC', 'FFA', 'SCLP', 'BRKGA-U')
    $lines = @("# Comparacion BR1-BR17: $Seconds segundos CPU",
        "Estado: $State; $($Rows.Count)/$Total casos disponibles. Presupuesto: hasta $Seconds segundos CPU por caso.",
        '',
        "Protocolo: BR1-BR17-CPU30-v1. Cada algoritmo usa un hilo por caso; concurrencia de este arranque: $Workers casos. Consultar execution-*.json para el historial.",
        'GA/ABC/FFA conservan el perfil previo: contacto, C2, seis orientaciones permitidas, sin reemplazo, poblacion 20 y cache desactivada. Cambia la parada de 100 iteraciones a tiempo CPU, sin limite de iteraciones/estancamiento.',
        $(if ($FreshRun) {
            'Lote nuevo: todos los casos se ejecutan desde cero, sin importar resultados anteriores. SCLP y BRKGA conservan sus configuraciones y binarios congelados.'
        } else {
            'SCLP y BRKGA conservan sus configuraciones anteriores. Sus BR1-BR15 a 30 CPU se reutilizan con procedencia registrada; BR16-BR17 se ejecutan con los mismos binarios congelados.'
        }),
        'BRKGA conserva las poblaciones publicadas y el maximo de 500 generaciones; el presupuesto puede agotarse durante inicializacion. Todos los acomodos son auditados. El plazo es cooperativo y puede sobrepasarse ligeramente.',
        'Los resultados anteriores de 100 iteraciones se conservan en Results/EqualIterations; no se mezclan en esta tabla.',
        '',
        'Cada celda muestra ocupacion media (%) y numero de instancias terminadas de las 100 de su BR.',
        '',
        '| BR | GA | ABC | FFA | SCLP | BRKGA-U |',
        '| --- | ---: | ---: | ---: | ---: | ---: |')
    for ($number = 1; $number -le 17; $number++) {
        $cells = @()

        foreach ($algorithm in $algorithms) {
            $group = @($Rows | Where-Object {
                    $_.dataset -eq "BR$number" -and $_.algorithm -eq $algorithm
                })
            $cells += if ($group.Count) {
                $mean = ($group | ForEach-Object {
                        [double]$_.occupation_percent
                    } | Measure-Object -Average).Average
                '{0:F3} ({1}/100)' -f $mean, $group.Count
            } else {
                'pendiente (0/100)'
            }
        }
        $lines += '| BR{0} | {1} | {2} | {3} | {4} | {5} |' -f $number, $cells[0], $cells[1], $cells[2], $cells[3], $cells[4]
    }
    $lines += @('', '## Promedio general de ocupacion', '',
        '| Algoritmo | Ocupacion media (%) | Casos incluidos | Estado |',
        '| --- | ---: | ---: | --- |')
    foreach ($algorithm in $algorithms) {
        $group = @($Rows | Where-Object algorithm -eq $algorithm)
        $mean = if ($group.Count) {
            '{0:F3}' -f (($group | ForEach-Object {
                    [double]$_.occupation_percent
                } | Measure-Object -Average).Average)
        } else {
            'pendiente'
        }
        $status = if ($group.Count -eq 1700) {
            'completo'
        } else {
            'parcial'
        }
        $lines += '| {0} | {1} | {2}/1700 | {3} |' -f $algorithm, $mean, $group.Count, $status
    }
    $lines += @('', 'Promedios calculados desde resultados individuales sin redondear. Los promedios parciales pueden incluir subconjuntos distintos; esperar 1700/1700 por algoritmo para comparar el conjunto completo.')
    $brkga = @($Rows | Where-Object algorithm -eq 'BRKGA-U')

    if ($brkga.Count) {
        $initialOnly = @($brkga | Where-Object {
                [int]$_.generations -eq 0
            }).Count
        $lines += "BRKGA: $initialOnly/$($brkga.Count) casos sin completar una primera generacion de las tres poblaciones."
    }
    return $lines
}
