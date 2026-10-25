# Separate integration suite; does not add Boost/C++20 to existing C++14 tests.
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$tools=Join-Path $root 'src/treeSearch';$directory=Join-Path $root ('build/sclp-tests-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory $directory | Out-Null
$inputFile=Join-Path $directory 'input.json'
& "$tools/Convert-BR.ps1" -InputFile "$root/Instance/BR/BR15.txt" -Instance 1 -OutputFile $inputFile
$converted=Get-Content $inputFile -Raw | ConvertFrom-Json
$official=Get-Content "$root/third_party/sclp/doc/set_15_instance_1.json" -Raw | ConvertFrom-Json
if($converted.small_items.Count -ne $official.small_items.Count){throw 'Official type count mismatch'}
for($i=0;$i -lt $official.small_items.Count;$i++) {
    $a=$converted.small_items[$i];$b=$official.small_items[$i]
    if($a.quantity -ne $b.quantity){throw 'Official quantity mismatch'}
    foreach($axis in @('x','y','z')){if($a.measurement.$axis -ne $b.measurement.$axis){throw 'Official dimension mismatch'}}
    foreach($orientation in @('xyz','yxz','zyx','xzy','zxy','yzx')){if($a.constraint.orientation.$orientation -ne $b.constraint.orientation.$orientation){throw 'Official permission mismatch'}}
}
$original=Join-Path $directory 'original.json';$annotated=Join-Path $directory 'annotated.json'
$previousPath=$env:PATH
try {
    $env:PATH='C:\mingw64\bin;'+$env:PATH
    # Stop after one beam width, before time affects parity. This is a test only.
    foreach($exe in @('sclp_original','sclp_reference')) {
        $target=if($exe -eq 'sclp_original'){$original}else{$annotated}
        & "$root/build/sclp-release/$exe.exe" $inputFile "--output-file=$target" --maximum_running_time_seconds=30 --initial_branching_factor=4 --maximum_branching_factor=4 --skip_type=strict --alpha=3.7015 --beta=2.6631 --gamma=0.3968 --delta=0.0139 --alpha_2=2.0934 --beta_2=1.4609 --gamma_2=0.4732 --delta_2=0.0366 --configuration_switch_threshold=0.6740
        if($LASTEXITCODE -ne 0){throw 'Reference parity execution failed'}
    }
} finally {$env:PATH=$previousPath}
$a=Get-Content $original -Raw | ConvertFrom-Json;$b=Get-Content $annotated -Raw | ConvertFrom-Json
$plain=@($b.small_items | ForEach-Object {[ordered]@{measurement=$_.measurement;position=$_.position}})
if(($a.small_items | ConvertTo-Json -Depth 8 -Compress) -ne ($plain | ConvertTo-Json -Depth 8 -Compress)){throw 'Driver changed original packing'}
& "$tools/Verify-Packing.ps1" -InputFile $inputFile -PackingFile $annotated | Out-Null
function Reject-Modified([scriptblock]$Change) {
    $bad=Get-Content $annotated -Raw | ConvertFrom-Json
    & $Change $bad
    $file=Join-Path $directory 'invalid.json';$bad | ConvertTo-Json -Depth 30 | Set-Content $file
    $caught=$false
    try {& "$tools/Verify-Packing.ps1" -InputFile $inputFile -PackingFile $file | Out-Null}catch{$caught=$true}
    if(!$caught){throw 'Verifier accepted corrupt packing'}
}
Reject-Modified {param($p) $p.small_items[0].position.x=-1}
Reject-Modified {param($p) $p.small_items[0].position.x=587}
Reject-Modified {param($p) $p.small_items[0].orientation='invalid'}
Reject-Modified {param($p) $p.small_items[0].measurement.x++}
Reject-Modified {param($p) $p.small_items[0].type_index=10000}
Reject-Modified {param($p) $p.small_items+= $p.small_items[0]}
Reject-Modified {param($p) $p.appendix.packing_statistics.total_volume_of_allocated_small_items++}
Reject-Modified {param($p) $p.small_items[1].position=$p.small_items[0].position}
Write-Output 'PASS: all 100 official BR15 types, unchanged original packing, independent validation and eight corrupt-packing rejections'
