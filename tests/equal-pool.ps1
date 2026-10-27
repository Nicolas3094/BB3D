# Windows integration test for the process queue, separate from the C++ unit suite.
param([switch]$SkipBuild)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$base=Join-Path $root ('build/equal-pool-tests-'+[guid]::NewGuid().ToString('N'))
$parallel=Join-Path $base 'parallel';$serial=Join-Path $base 'serial'
& "$root/experiments/Run-EqualPool.ps1" -OutputDirectory $parallel -Workers 7 -Iterations 1 -TaskLimit 14 -SkipBuild:$SkipBuild
& "$root/experiments/Run-EqualPool.ps1" -OutputDirectory $serial -Workers 1 -Iterations 1 -TaskLimit 14 -SkipBuild
$a=@(Import-Csv "$parallel/resultados.csv");$b=@(Import-Csv "$serial/resultados.csv")
if($a.Count -ne 14 -or $b.Count -ne 14){throw 'Missing results'}
foreach($row in $a) {
    $other=@($b | Where-Object {$_.dataset -eq $row.dataset -and $_.instance -eq $row.instance -and $_.algorithm -eq $row.algorithm})
    if($other.Count -ne 1 -or $row.occupation_percent -ne $other[0].occupation_percent -or $row.evaluations -ne '60' -or $other[0].evaluations -ne '60'){throw 'Serial/parallel result mismatch'}
    $case=$row.dataset+'-i'+$row.instance+'-'+$row.algorithm
    foreach($file in @('packing.csv','genome.csv','insertion.csv')){if((Get-FileHash "$parallel/tasks/$case/$file").Hash -ne (Get-FileHash "$serial/tasks/$case/$file").Hash){throw 'Serial/parallel artifact mismatch'}}
}
$claims=@(Get-Content "$parallel/worker-*.log" | Where-Object {$_ -match '\] CLAIM '} | ForEach-Object {($_ -split '\] CLAIM ')[1]})
if($claims.Count -ne 14 -or @($claims | Sort-Object -Unique).Count -ne 14){throw 'Duplicate task claim'}
& "$root/experiments/Run-EqualPool.ps1" -OutputDirectory $parallel -Workers 7 -Iterations 1 -TaskLimit 14 -SkipBuild
if(@(Import-Csv "$parallel/resultados.csv").Count -ne 14){throw 'Resume duplicated results'}
Write-Output 'PASS: seven-worker queue, 14 exclusive claims, exact serial parity, geometry/evaluation checks, and resume'
