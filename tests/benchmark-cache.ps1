param([string]$Compiler='C:\mingw64\bin\g++.exe',[int]$Rounds=3,[switch]$SkipBuild)
$ErrorActionPreference='Stop'
if($Rounds -lt 1) {throw 'Rounds must be positive'}
$root=Split-Path $PSScriptRoot -Parent
$output=Join-Path $root 'build/cache'
New-Item -ItemType Directory -Force $output | Out-Null
$previousPath=$env:PATH
Push-Location $root
try {
    $env:PATH=(Split-Path $Compiler -Parent)+[IO.Path]::PathSeparator+$env:PATH
    $binary=Join-Path $output 'benchmark.exe'
    if(!$SkipBuild) {
        $sources=@(Join-Path $PSScriptRoot 'CacheBenchmark.cpp')+@(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
        & $Compiler -std=c++14 -O2 $sources -o $binary
        if($LASTEXITCODE -ne 0) {throw 'Cache benchmark compilation failed'}
    }
    & $binary (Join-Path $output 'comparison.csv') $Rounds
    if($LASTEXITCODE -ne 0) {throw 'Cache benchmark failed'}
    Import-Csv (Join-Path $output 'comparison.csv') | Format-Table
} finally {Pop-Location;$env:PATH=$previousPath}
