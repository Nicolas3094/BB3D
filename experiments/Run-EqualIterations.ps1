param(
    [string]$Dataset = 'BR1',
    [int]$Instance = 1,
    [ValidateRange(0, 10000000)][int]$Iterations = 100,
    [string]$OutputDirectory,
    [string]$Binary
)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent

if (!$Binary) {
    $Binary = Join-Path $root 'main.exe'
}

if (!$OutputDirectory) {
    $OutputDirectory = Join-Path $root ('Results/EqualIterations/' + (Get-Date -Format yyyyMMdd-HHmmss))
}
$arguments = @('--root', $root, '--dataset', $Dataset, '--first-instance', $Instance, '--instances',
    '1',
    '--population', '20', '--execution', 'iterations', '--iterations', $Iterations, '--rounds',
    '1', '--seed', '123',
    '--decoder', 'contact', '--candidates', 'legacy', '--mutation', 'c2', '--mutation-probability',
    '1', '--rotation', '6',
    '--rotation-mutation', '0.05', '--replacement', 'false', '--patience', '0', '--cache-entries',
    '0', '--report')
foreach ($algorithm in @('GA', 'ABC', 'FFA')) {
    $extra = if ($algorithm -eq 'GA') {
        @('--ga-matings', '20', '--crossover', '1', '--selection', '0.85')
    }
    elseif ($algorithm -eq 'ABC') {
        @('--sites', '10', '--elite-sites', '5', '--elite-bees', '4', '--non-elite-bees', '2')
    }
    else {
        @('--movements', '2')
    }
    & $Binary @arguments --algorithm $algorithm --output (Join-Path $OutputDirectory $algorithm) @extra

    if ($LASTEXITCODE -ne 0) {
        throw "$algorithm failed"
    }
}
