param([string]$Dataset='BR15',[int]$Instance=1,[ValidateRange(1,3600)][double]$Seconds=30,[string]$OutputDirectory,[string]$DataRoot)
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$binary=Join-Path $root 'build/sclp-release/sclp_reference.exe'
if(!(Test-Path $binary)){throw 'Compile src/treeSearch first; see README.md'}
if(!$OutputDirectory){$OutputDirectory=Join-Path $root ('Results/TreeSearch/'+$Dataset+'-'+$Instance+'-'+(Get-Date -Format yyyyMMdd-HHmmss)+'-'+[guid]::NewGuid().ToString('N').Substring(0,6))}
if(Test-Path $OutputDirectory){throw 'Use a new output directory to preserve previous experiments'}
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$OutputDirectory=(Resolve-Path $OutputDirectory).Path
if(!$DataRoot){$DataRoot=$root}
$source=Join-Path $DataRoot ('Instance/BR/'+$Dataset+'.txt')
$inputJson=Join-Path $OutputDirectory 'input.json';$packing=Join-Path $OutputDirectory 'packing.json'
& "$PSScriptRoot/Convert-BR.ps1" -InputFile $source -Instance $Instance -OutputFile $inputJson
# Table 3, two-stage-config; memoization strict as in the authors' solve script.
$arguments=@($inputJson,"--output-file=$packing","--maximum_running_time_seconds=$($Seconds.ToString([Globalization.CultureInfo]::InvariantCulture))",
 '--alpha=3.7015','--beta=2.6631','--gamma=0.3968','--delta=0.0139',
 '--alpha_2=2.0934','--beta_2=1.4609','--gamma_2=0.4732','--delta_2=0.0366',
 '--configuration_switch_threshold=0.6740','--skip_type=strict','--initial_branching_factor=4',
 '--maximum_branching_factor=10000','--maximum_number_of_blocks=10000','--block_minimum_volume_ratio=-1')
$previousPath=$env:PATH
try {
    $env:PATH='C:\mingw64\bin;'+$env:PATH
    $watch=[Diagnostics.Stopwatch]::StartNew()
    & $binary @arguments 2>&1 | Tee-Object -FilePath (Join-Path $OutputDirectory 'terminal.txt')
    if($LASTEXITCODE -ne 0){throw 'SCLP execution failed'}
    $watch.Stop()
} finally {$env:PATH=$previousPath}
$verified=& "$PSScriptRoot/Verify-Packing.ps1" -InputFile $inputJson -PackingFile $packing -ReportFile (Join-Path $OutputDirectory 'verification.json')
$result=Get-Content $packing -Raw | ConvertFrom-Json
$manifest=[ordered]@{dataset=$Dataset;instance=$Instance;parameters=$arguments;cpu_limit_seconds=$Seconds;process_wall_seconds=$watch.Elapsed.TotalSeconds;solver_threads=1;build='Release -O3 -DNDEBUG; C++20; no fast-math';binary_sha256=(Get-FileHash $binary).Hash;dataset_sha256=(Get-FileHash $source).Hash;input_sha256=(Get-FileHash $inputJson).Hash;execution=$result.appendix.bb3d_execution;verification=$verified;paper='https://doi.org/10.1016/j.ejor.2026.01.039';source='https://zenodo.org/records/19247237'}
$manifest | ConvertTo-Json -Depth 15 | Set-Content (Join-Path $OutputDirectory 'run.json') -Encoding utf8
Write-Output ('OCUPACION FINAL VERIFICADA: {0:F6} %' -f $verified.occupation_percent)
Write-Output ('Tiempo CPU: {0:F6} s; tiempo total transcurrido: {1:F6} s' -f $result.appendix.bb3d_execution.cpu_seconds_including_blocks,$watch.Elapsed.TotalSeconds)
Write-Output ('Resultados: '+$OutputDirectory)
