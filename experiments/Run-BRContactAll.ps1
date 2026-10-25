param([Parameter(Mandatory)][string]$OutputDirectory,[string]$Compiler='C:/mingw64/bin/g++.exe')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$OutputDirectory=(Resolve-Path $OutputDirectory).Path
$statusFile=Join-Path $OutputDirectory 'estado.json'
if(Test-Path $statusFile){$old=Get-Content $statusFile -Raw | ConvertFrom-Json;if($old.manager_pid -and (Get-Process -Id $old.manager_pid -ErrorAction SilentlyContinue)){throw 'A batch manager is already running'}}
$processes=@{};$previousPath=$env:PATH
function Publish-Progress([string]$State) {
    $rows=@(foreach($a in @('GA','ABC','FFA')){if(Test-Path "$OutputDirectory/$a.csv"){Import-Csv "$OutputDirectory/$a.csv" | Where-Object {$_.valid -eq '1' -and $_.packing}}})
    $rows | Export-Csv "$OutputDirectory/resultados-todas.csv" -NoTypeInformation -Encoding utf8
    $lines=@('# BR1-BR15: todas las instancias',"Estado: $State; completadas $($rows.Count)/4500.",'',
      'Contacto; C2; adicion (replacement=false); poblacion 20; semilla 123; seis orientaciones con permisos; 30 segundos por corrida.',
      'Las medias son parciales hasta completar 100 instancias por celda. Acomodos y auditoria geometrica por corrida.',
      '','| BR | GA (%) | n/100 | ABC (%) | n/100 | Firefly (%) | n/100 |','| --- | ---: | ---: | ---: | ---: | ---: | ---: |')
    for($i=1;$i -le 15;$i++) {
        $cells=@()
        foreach($a in @('GA','ABC','FFA')) {
            $subset=@($rows | Where-Object {$_.dataset -eq "BR$i" -and $_.algorithm -eq $a})
            $cells+=if($subset.Count){'{0:F3}' -f (($subset | ForEach-Object {[double]$_.occupation_percent} | Measure-Object -Average).Average)}else{'pendiente'}
            $cells+=[string]$subset.Count
        }
        $lines+='| BR{0} | {1} | {2} | {3} | {4} | {5} | {6} |' -f $i,$cells[0],$cells[1],$cells[2],$cells[3],$cells[4],$cells[5]
    }
    $lines | Set-Content "$OutputDirectory/tabla-todas.md" -Encoding utf8
    @{state=$State;completed=$rows.Count;total=4500;manager_pid=$PID;updated_utc=[DateTime]::UtcNow.ToString('o');workers=@(foreach($a in $processes.Keys){@{algorithm=$a;pid=$processes[$a].Id}})} | ConvertTo-Json -Depth 5 | Set-Content $statusFile
}
try {
    $env:PATH=(Split-Path $Compiler -Parent)+';'+$env:PATH
    Publish-Progress 'compilando'
    $binary=Join-Path $root 'build/br_contact_adding_all.exe'
    $sources=@(Join-Path $PSScriptRoot 'BRContactAdding.cpp')+@(Get-ChildItem "$root/src" -Recurse -Filter *.cpp | Where-Object Name -ne 'main.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++14 -O3 -DNDEBUG $sources -o $binary
    if($LASTEXITCODE -ne 0){throw 'Compilation failed'}
    Copy-Item $binary "$OutputDirectory/br_contact_adding_all.exe" -Force
    $hashes=@{};1..15 | ForEach-Object {$hashes["BR$_"]=(Get-FileHash "$root/Instance/BR/BR$_.txt").Hash}
    if(Test-Path "$OutputDirectory/manifest.json") {
        $previous=Get-Content "$OutputDirectory/manifest.json" -Raw | ConvertFrom-Json
        foreach($name in $hashes.Keys){if($previous.datasets_sha256.$name -ne $hashes[$name]){throw "Dataset changed: $name"}}
    }
    @{instances='all 100 instances of BR1-BR15';total_runs=4500;population=20;seed=123;decoder='contact';mutation='C2';replacement=$false;rotation=6;seconds_per_run=30;clock='wall';compiler=(& $Compiler --version | Select-Object -First 1);flags='-std=c++14 -O3 -DNDEBUG';binary_sha256=(Get-FileHash $binary).Hash;datasets_sha256=$hashes;parallel_solver_processes=3;logical_cpu_masks=@{GA=1;ABC=4;FFA=16}} | ConvertTo-Json -Depth 6 | Set-Content "$OutputDirectory/manifest-todas.json"
    # A previous first-instance sweep may still be finishing. Reuse it only after
    # all three original workers have exited, so CSV writers never overlap.
    while(Get-Process -Name br_contact_adding -ErrorAction SilentlyContinue){Start-Sleep -Seconds 5}
    foreach($a in @('GA','ABC','FFA')) {
        $processes[$a]=Start-Process -FilePath $binary -ArgumentList @(('"'+$OutputDirectory+'"'),('"'+$root+'"'),$a,'--all') -WindowStyle Hidden -PassThru -RedirectStandardOutput "$OutputDirectory/$a-all.log" -RedirectStandardError "$OutputDirectory/$a-all-errors.log"
        $null=$processes[$a].Handle
    }
    do {Publish-Progress 'ejecutando';Start-Sleep -Seconds 30}while(@($processes.Values | Where-Object {!$_.HasExited}).Count)
    foreach($a in $processes.Keys){$processes[$a].WaitForExit();if($processes[$a].ExitCode -ne 0){throw "$a failed; completed results preserved"}}
    Publish-Progress 'completado'
    $finished=Get-Content $statusFile -Raw | ConvertFrom-Json
    if($finished.completed -ne 4500){throw 'Incomplete batch'}
} catch {
    # Stop the other workers before permitting a clean resume.
    foreach($p in $processes.Values){if(!$p.HasExited){$p.Kill();$p.WaitForExit()}}
    Publish-Progress ('interrumpido: '+$_.Exception.Message)
    throw
} finally {$env:PATH=$previousPath}
