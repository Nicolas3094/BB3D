<# .SYNOPSIS
Compila desde fuentes actuales el ejecutor por CPU y verifica el contrato temporal.
#>
param(
    [string]$Compiler = 'C:/mingw64/bin/g++.exe'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$previousPath = $env:PATH
Push-Location $root

try {
    $env:PATH = (Split-Path $Compiler -Parent) + ';' + $env:PATH
    $build = "$root/build/time-comparison"
    New-Item -ItemType Directory -Force "$build/objects" | Out-Null
    $sources = @(Get-ChildItem "$root/src" -Recurse -Filter *.cpp |
        Where-Object Name -ne main.cpp | ForEach-Object FullName)
    Push-Location "$build/objects"

    try {
        & $Compiler -std=c++14 -O3 -DNDEBUG -c $sources

        if ($LASTEXITCODE) {
            throw 'Comparison core compilation failed'
        }
        & (Join-Path (Split-Path $Compiler -Parent) 'ar.exe') rcs core.a @(Get-ChildItem *.o | ForEach-Object FullName)

        if ($LASTEXITCODE) {
            throw 'Comparison core archive failed'
        }
    } finally {
        Pop-Location
    }
    & $Compiler -std=c++14 -O3 -DNDEBUG experiments/MetaTimeBenchmark.cpp "$build/objects/core.a" -o "$build/meta_time.exe"

    if ($LASTEXITCODE) {
        throw 'CPU benchmark compilation failed'
    }

    foreach ($name in @('TimedExecutionTests', 'EqualIterationTests', 'ContactDecoderTests')) {
        & $Compiler -std=c++14 -O2 -D_GLIBCXX_ASSERTIONS "tests/$name.cpp" "$build/objects/core.a" -o "$build/$name.exe"

        if ($LASTEXITCODE) {
            throw "$name compilation failed"
        }
        & "$build/$name.exe"

        if ($LASTEXITCODE) {
            throw "$name failed"
        }
    }
    @{
        tests_passed = $true
        binary_sha256 = (Get-FileHash "$build/meta_time.exe").Hash
        flags = '-std=c++14 -O3 -DNDEBUG'
        tests = @('TimedExecutionTests', 'EqualIterationTests', 'ContactDecoderTests')
    } | ConvertTo-Json | Set-Content "$build/verification.json"
} finally {
    $env:PATH = $previousPath
    Pop-Location
}
