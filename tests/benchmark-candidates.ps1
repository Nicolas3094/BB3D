param([string]$Compiler='C:\mingw64\bin\g++.exe',[int]$Rounds=3,[switch]$SkipBuild)
$ErrorActionPreference='Stop'
if($Rounds -lt 1) { throw 'Rounds must be positive' }
$root=Split-Path $PSScriptRoot -Parent
$output=Join-Path $root 'build/candidates'
New-Item -ItemType Directory -Force $output | Out-Null
$previousPath=$env:PATH
Push-Location $root
try {
    $env:PATH=(Split-Path $Compiler -Parent)+';'+$env:PATH
    $binary=Join-Path $output 'benchmark.exe'
    if(!$SkipBuild) {
        $sources=@(Join-Path $PSScriptRoot 'CandidateBenchmark.cpp')+@(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
        & $Compiler -std=c++14 -O2 $sources -o $binary
        if($LASTEXITCODE -ne 0) { throw 'Benchmark compilation failed' }
    }
    foreach($policy in @(0,1,2)) {
        & $binary $policy (Join-Path $output "policy-$policy") $Rounds
        if($LASTEXITCODE -ne 0) { throw "Policy $policy benchmark failed" }
    }
    $invariant=[Globalization.CultureInfo]::InvariantCulture
    $legacy=@(Import-Csv (Join-Path $output 'policy-0-cases.csv'))
    $projected=@(Import-Csv (Join-Path $output 'policy-1-cases.csv'))
    $best=@(Import-Csv (Join-Path $output 'policy-2-cases.csv'))
    if($legacy.Count -ne $best.Count -or $legacy.Count -ne $projected.Count) { throw 'Case counts differ' }
    $wins=0
    for($i=0;$i -lt $legacy.Count;++$i) {
        foreach($column in @('round','dataset','instance','rotation','seed')) {
            if($legacy[$i].$column -ne $projected[$i].$column -or $legacy[$i].$column -ne $best[$i].$column) { throw 'Benchmark cases differ' }
        }
        $expected=[Math]::Max([long]$legacy[$i].volume,[long]$projected[$i].volume)
        if([long]$best[$i].volume -ne $expected) { throw 'BestOfBoth quality guarantee failed' }
        if($legacy[$i].round -eq '0' -and [long]$best[$i].volume -gt [long]$legacy[$i].volume) { ++$wins }
    }
    $report=@()
    foreach($policy in @(0,1,2)) {
        $samples=@(Import-Csv (Join-Path $output "policy-$policy-summary.csv"))
        $times=@($samples | ForEach-Object { [double]::Parse($_.milliseconds,$invariant) } | Sort-Object)
        $middle=[int][Math]::Floor($times.Count/2)
        $median=if($times.Count%2) { $times[$middle] } else { ($times[$middle-1]+$times[$middle])/2 }
        $peak=($samples | ForEach-Object { [long]$_.peak_working_set_bytes } | Measure-Object -Maximum).Maximum
        $name=@('legacy','projected','best')[$policy]
        $report += [pscustomobject]@{policy=$name; evaluations_per_round=$samples[0].evaluations; rounds=$Rounds; median_ms=$median.ToString('R',$invariant); mean_occupancy=$samples[0].mean_occupancy; peak_working_set_bytes=$peak; improved_cases=if($policy -eq 2){$wins}else{''}}
    }
    $report | Export-Csv (Join-Path $output 'comparison.csv') -NoTypeInformation
    $report | Format-Table
    Write-Output "PASS: identical seeded cases; best matches max(legacy, projected) in every case. Improved: $wins/300"
} finally { Pop-Location; $env:PATH=$previousPath }
