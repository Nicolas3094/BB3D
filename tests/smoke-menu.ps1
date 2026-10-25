param([Parameter(Mandatory=$true)][string]$Binary)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$executable=(Resolve-Path -LiteralPath $Binary).Path
$output=Join-Path $root ('build/menu-smoke-'+[guid]::NewGuid().ToString('N'))
Push-Location $root
try {
    $first=@('999','bad','0','1','1','4','4','1','1','1','2','1','0','1','123','smoke','0','1','1')
    $second=@('0','1','1','1','1','1','1','1','2','1','0','1','123','otra','0','1','0')
    $console=@(($first+$second -join "`n") | & $executable --menu --root $root --output $output)
    if($LASTEXITCODE -ne 0 -or ($console -join "`n") -notmatch 'Pruebas BR') {throw 'Interactive BR menu failed'}
    if(($console -join "`n") -notmatch 'Valor invalido') {throw 'Menu did not recover from invalid input'}
    $sessions=@(Get-ChildItem $output -Directory)
    if($sessions.Count -ne 2) {throw 'Menu did not isolate two sessions'}
    $session=($sessions | Where-Object Name -like 'smoke-*' | Select-Object -First 1).FullName
    $rows=@(Import-Csv (Join-Path $session 'resultados.csv'))
    if($rows.Count -ne 9 -or @($rows.algoritmo | Sort-Object -Unique).Count -ne 3 -or @($rows.mutacion | Sort-Object -Unique).Count -ne 3) {throw 'Menu did not compare all algorithms and mutations'}
    $report=Get-Content (Join-Path $session 'resumen.md') -Raw
    if(@([regex]::Matches($report,'### BR0 /')).Count -ne 9) {throw 'Missing report sections'}
    $resume=@(& $executable --report --root $root --output $session --dataset BR0 --algorithm all --mutation all --rotation 0 --replacement true --population 2 --iterations 0 --rounds 1 --instances 1 --seed 123)
    if($LASTEXITCODE -ne 0 -or @($resume | Where-Object {$_ -match 'resumed=complete'}).Count -ne 9) {throw 'Report command did not resume completed menu tests'}
    if(@(Import-Csv (Join-Path $session 'resultados.csv') | Where-Object reanudado -ne '1').Count -ne 0) {throw 'Resumed summary was not regenerated'}
    $cancelPath=Join-Path $output 'cancelled'
    $null='salir' | & $executable --menu --root $root --output $cancelPath
    if($LASTEXITCODE -ne 0 -or (Test-Path -LiteralPath $cancelPath)) {throw 'Cancelled menu wrote results'}
    $default=@('salir' | & $executable)
    if($LASTEXITCODE -ne 0 -or ($default -join "`n") -notmatch 'Pruebas BR') {throw 'No-argument launch did not open BR menu'}
    Write-Output 'PASS: interactive BR menu, repeated sessions, comparisons, reports, resume, input validation and cancellation'
} finally {Pop-Location}
