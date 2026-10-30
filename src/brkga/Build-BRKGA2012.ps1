<#
.SYNOPSIS
Compila el BRKGA independiente con optimizacion y, opcionalmente, sus tests.
#>
param(
    [string]$Compiler = 'C:/mingw64/bin/g++.exe',
    [switch]$Test
)

$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$previousPath = $env:PATH
Push-Location $root

try {
    $env:PATH = (Split-Path $Compiler -Parent) + ';' + $env:PATH
    New-Item -ItemType Directory -Force build/brkga2012 | Out-Null
    $sources = @(Get-ChildItem src -Recurse -Filter *.cpp |
        Where-Object Name -ne main.cpp | ForEach-Object FullName)
    if ($Test) {
        & $Compiler -std=c++14 -O2 -D_GLIBCXX_ASSERTIONS tests/BRKGA2012Tests.cpp $sources -o build/brkga2012/tests.exe

        if ($LASTEXITCODE) {
            throw 'BRKGA test compilation failed'
        }
        & ./build/brkga2012/tests.exe

        if ($LASTEXITCODE) {
            throw 'BRKGA tests failed; batch must not start'
        }
    }
    & $Compiler -std=c++14 -O3 -DNDEBUG src/brkga/BRKGAStandalone.cxx $sources -o build/brkga2012/brkga2012.exe

    if ($LASTEXITCODE) {
        throw 'BRKGA compilation failed'
    }

    if ($Test) {
        @{
            new_tests_passed = $true
            binary_sha256 = (Get-FileHash build/brkga2012/brkga2012.exe).Hash
            compiler = (& $Compiler --version | Select-Object -First 1)
            flags = '-std=c++14 -O3 -DNDEBUG'
        } | ConvertTo-Json | Set-Content build/brkga2012/verification.json
    }
    Write-Host 'Compiled: build/brkga2012/brkga2012.exe (-O3 -DNDEBUG, one evaluation thread)'
}
finally {
    $env:PATH = $previousPath
    Pop-Location
}
