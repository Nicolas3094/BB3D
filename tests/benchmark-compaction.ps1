param(
    [string]$Compiler = 'C:\mingw64\bin\g++.exe',
    [string]$BaselineRef = '07efb60410e41a81e572ca22e3a4c86c4f9114a7',
    [int]$Rounds = 5
)
$ErrorActionPreference = 'Stop'
if ($Rounds -lt 1) { throw 'Rounds must be positive.' }
$root = Split-Path $PSScriptRoot -Parent
$output = Join-Path $root 'build/compaction'
New-Item -ItemType Directory -Force $output | Out-Null
$previousPath = $env:PATH
Push-Location $root
try {
    $env:PATH = (Split-Path $Compiler -Parent) + ';' + $env:PATH
    # Replace only the compaction translation unit, leaving all other code and
    # compiler options identical between the two executables.
    $baseline = & git show "${BaselineRef}:src/binPacking/ItemBinOperators.cpp"
    if ($LASTEXITCODE -ne 0) { throw 'Cannot read baseline source.' }
    $baselineSource = Join-Path $output 'baseline-ItemBinOperators.cpp'
    [System.IO.File]::WriteAllText($baselineSource, ($baseline -join "`n"))
    $common = @(Join-Path $PSScriptRoot 'CompactionBenchmark.cpp') + @(Get-ChildItem (Join-Path $root 'src') -Recurse -Filter *.cpp | Where-Object { $_.Name -notin @('main.cpp', 'ItemBinOperators.cpp') } | ForEach-Object { $_.FullName })
    foreach ($variant in @('before', 'after')) {
        $source = if ($variant -eq 'before') { $baselineSource } else { Join-Path $root 'src/binPacking/ItemBinOperators.cpp' }
        & $Compiler -std=c++14 -O2 '-I' (Join-Path $root 'src/binPacking') $common $source -o (Join-Path $output "$variant.exe")
        if ($LASTEXITCODE -ne 0) { throw "$variant compilation failed." }
    }
    $rows = @()
    $referenceHash = $null
    for ($round = 0; $round -lt $Rounds; ++$round) {
        $order = if ($round % 2 -eq 0) { @('before', 'after') } else { @('after', 'before') }
        foreach ($variant in $order) {
            $prefix = Join-Path $output "$variant-$round"
            # Each process performs an untimed warm-up pass, then a timed pass.
            & (Join-Path $output "$variant.exe") (Join-Path $root 'Instance/P5A2.csv') $prefix 1
            if ($LASTEXITCODE -ne 0) { throw "$variant benchmark failed." }
            $hash = (Get-FileHash "$prefix-placements.txt" -Algorithm SHA256).Hash
            if ($null -eq $referenceHash) { $referenceHash = $hash }
            if ($hash -ne $referenceHash) { throw 'Packing placements differ between runs.' }
            $sample = Import-Csv "$prefix-timings.csv"
            $rows += [pscustomobject]@{
                variant = $variant; round = $round; evaluations = $sample.evaluations
                milliseconds = $sample.milliseconds; loaded_items = $sample.loaded_items
                loaded_volume = $sample.loaded_volume; placements_sha256 = $hash
            }
        }
    }
    $rows | Export-Csv (Join-Path $output 'comparison.csv') -NoTypeInformation
    foreach ($variant in @('before', 'after')) {
        $values = @($rows | Where-Object variant -eq $variant | ForEach-Object { [double]::Parse($_.milliseconds, [System.Globalization.CultureInfo]::InvariantCulture) } | Sort-Object)
        $middle = [int][Math]::Floor($values.Count / 2)
        $median = if ($values.Count % 2) { $values[$middle] } else { ($values[$middle - 1] + $values[$middle]) / 2 }
        Write-Output "$variant median: $median ms per 300 DBLF evaluations"
    }
    Write-Output "PASS: all placements identical (SHA256 $referenceHash)"
    Write-Output "Results: $output/comparison.csv"
}
finally {
    Pop-Location
    $env:PATH = $previousPath
}
