param([ValidateSet('fixed','search','all')][string]$Mode='all',[string]$OutputDirectory,[string]$Compiler='C:/mingw64/bin/g++.exe')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
if(!$OutputDirectory){$OutputDirectory=Join-Path $root ('Results/DecoderComparison/'+(Get-Date -Format yyyyMMdd-HHmmss))}
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$outputs=if($Mode -eq 'all'){@('evaluations.csv','search.csv')}elseif($Mode -eq 'search'){@('search.csv')}else{@('evaluations.csv')}
foreach($file in $outputs){if(Test-Path (Join-Path $OutputDirectory $file)){throw 'Use a fresh output directory; results are preserved'}}
$binary=Join-Path $root 'build/decoder_comparison.exe'
$previousPath=$env:PATH
try {
    $env:PATH=(Split-Path $Compiler -Parent)+';'+$env:PATH
    $sources=@(Join-Path $PSScriptRoot 'DecoderComparison.cpp')+@(Get-ChildItem "$root/src" -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 -O3 -DNDEBUG $sources -o $binary
    if($LASTEXITCODE -ne 0){throw 'Comparison build failed'}
    Copy-Item $binary (Join-Path $OutputDirectory 'decoder_comparison.exe') -Force
    @{compiler=(& $Compiler --version | Select-Object -First 1);flags='-std=c++14 -O3 -DNDEBUG';binary_sha256=(Get-FileHash $binary).Hash;BR0_sha256=(Get-FileHash "$root/Instance/BR/BR0.txt").Hash;BR15_sha256=(Get-FileHash "$root/Instance/BR/BR15.txt").Hash;seeds=@(123,456);instance_position=1;search_limit_seconds=30;clock='steady_clock wall';threads=1;cache=0;points='legacy three-points';orientations='six permitted by BR flags';mutation='C2';population=20;replacement=$true;paired_genomes_per_instance=32} | ConvertTo-Json | Set-Content (Join-Path $OutputDirectory 'manifest.json')
    & $binary $OutputDirectory $root $Mode
    if($LASTEXITCODE -ne 0){throw 'Comparison failed; completed rows remain saved'}
    Write-Output "Results: $OutputDirectory"
} finally {$env:PATH=$previousPath}
