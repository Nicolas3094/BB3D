param([string]$OutputDirectory,[string]$Compiler='C:/mingw64/bin/g++.exe')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
if(!$OutputDirectory){$OutputDirectory=Join-Path $root ('Results/BRContactAdding/'+(Get-Date -Format yyyyMMdd-HHmmss))}
if(Test-Path $OutputDirectory){throw 'Use a new output folder to preserve results'}
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$OutputDirectory=(Resolve-Path $OutputDirectory).Path
$binary=Join-Path $root 'build/br_contact_adding.exe'
$previousPath=$env:PATH
try {
    $env:PATH=(Split-Path $Compiler -Parent)+';'+$env:PATH
    $sources=@(Join-Path $PSScriptRoot 'BRContactAdding.cpp')+@(Get-ChildItem "$root/src" -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 -O3 -DNDEBUG $sources -o $binary
    if($LASTEXITCODE -ne 0){throw 'Experiment compilation failed'}
    Copy-Item $binary (Join-Path $OutputDirectory 'br_contact_adding.exe')
    $hashes=@{};1..15 | ForEach-Object {$hashes["BR$_"]=(Get-FileHash "$root/Instance/BR/BR$_.txt").Hash}
    @{instances='first instance of BR1-BR15';population=20;seed=123;decoder='contact';candidates='three points; no projections';mutation='C2';replacement=$false;rotation=6;seconds_per_run=30;clock='wall';compiler=(& $Compiler --version | Select-Object -First 1);flags='-std=c++14 -O3 -DNDEBUG';binary_sha256=(Get-FileHash $binary).Hash;datasets_sha256=$hashes;parallel_solver_processes=3;threads_per_solver=1;logical_cpu_masks=@{GA=1;ABC=4;FFA=16}} | ConvertTo-Json -Depth 6 | Set-Content (Join-Path $OutputDirectory 'manifest.json')
    $processes=@{}
    foreach($algorithm in @('GA','ABC','FFA')) {
        $processes[$algorithm]=Start-Process -FilePath $binary -ArgumentList @(('"'+$OutputDirectory+'"'),('"'+$root+'"'),$algorithm) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $OutputDirectory "$algorithm.log") -RedirectStandardError (Join-Path $OutputDirectory "$algorithm-errors.log")
    }
    while(@($processes.Values | Where-Object {!$_.HasExited}).Count) {Start-Sleep -Seconds 5}
    foreach($algorithm in @('GA','ABC','FFA')) {$processes[$algorithm].WaitForExit();if($processes[$algorithm].ExitCode -ne 0){throw "$algorithm failed; inspect its error log. Completed results are preserved."}}
    $rows=@(foreach($algorithm in @('GA','ABC','FFA')){Import-Csv (Join-Path $OutputDirectory "$algorithm.csv")})
    if($rows.Count -ne 45 -or @($rows | Where-Object valid -ne '1').Count){throw 'Incomplete or invalid results'}
    $rows | Export-Csv (Join-Path $OutputDirectory 'resultados.csv') -NoTypeInformation -Encoding utf8
    $table=@('# Contacto + C2 + adicion de soluciones','','Primera instancia BR1-BR15; 30 s por algoritmo; poblacion 20; semilla 123; seis orientaciones respetando permisos.','',
      'replacement=false: anadir candidatos y conservar los mejores; no crecimiento ilimitado.',
      'Tres procesos simultaneos, cada uno fijado a un nucleo logico diferente; un hilo de busqueda por proceso.',
      'Todos los acomodos auditados independientemente. Una corrida por caso: no son promedios de varias semillas.','',
      '| Instancia | GA (%) | ABC (%) | Firefly (%) |','| --- | ---: | ---: | ---: |')
    for($i=1;$i -le 15;$i++) {
        $values=@(foreach($algorithm in @('GA','ABC','FFA')){[double]($rows | Where-Object {$_.dataset -eq "BR$i" -and $_.algorithm -eq $algorithm}).occupation_percent})
        $table+='| BR{0}/1 | {1:F3} | {2:F3} | {3:F3} |' -f $i,$values[0],$values[1],$values[2]
    }
    $table | Set-Content (Join-Path $OutputDirectory 'resumen.md') -Encoding utf8
    $table
    Write-Output "Results: $OutputDirectory"
} finally {$env:PATH=$previousPath}
