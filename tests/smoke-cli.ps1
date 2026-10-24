param([Parameter(Mandatory=$true)][string]$Binary)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$executable=(Resolve-Path -LiteralPath $Binary).Path
# A fresh, ignored directory keeps the test independent of previous checkpoints.
$output=Join-Path $root ('build/cli-smoke-'+[guid]::NewGuid().ToString('N'))
Push-Location $root
try {
    $help=& $executable --help
    if($LASTEXITCODE -ne 0 -or ($help -join "`n") -notmatch '--algorithm') {throw 'CLI help failed'}
    $arguments=@('--root',$root,'--output',$output,'--algorithm','all','--dataset','P1A2','--instances','1','--population','4','--iterations','2','--rounds','2','--seed','123')
    $first=@(& $executable @arguments)
    if($LASTEXITCODE -ne 0 -or @($first | Where-Object {$_ -match 'fitness='}).Count -ne 6) {throw 'CLI did not run all three algorithms and both rounds'}
    $second=@(& $executable @arguments)
    if($LASTEXITCODE -ne 0 -or @($second | Where-Object {$_ -match 'resumed=complete'}).Count -ne 6) {throw 'CLI resume reran completed experiments'}
    if(@(Get-ChildItem $output -Recurse -Filter '*.checkpoint').Count -ne 6) {throw 'CLI result identities collided'}
    $different=@(& $executable @arguments --candidates best)
    if($LASTEXITCODE -ne 0 -or @($different | Where-Object {$_ -match 'fitness='}).Count -ne 6) {throw 'Candidate change reused old checkpoints'}
    if(@(Get-ChildItem $output -Recurse -Filter '*.checkpoint').Count -ne 12) {throw 'Candidate policy was not included in identity'}
    $brArguments=@('--root',$root,'--output',$output,'--algorithm','GA','--dataset','BR0','--instances','1','--population','2','--iterations','0','--seed','123')
    $br=@(& $executable @brArguments)
    if($LASTEXITCODE -ne 0 -or @($br | Where-Object {$_ -match 'fitness='}).Count -ne 1) {throw 'CLI cannot read a named BR instance'}
    $brArguments[$brArguments.IndexOf('BR0')]=Join-Path $root 'Instance/BR/BR0.txt'
    $brPath=@(& $executable @brArguments)
    if($LASTEXITCODE -ne 0 -or @($brPath | Where-Object {$_ -match 'resumed=complete'}).Count -ne 1) {throw 'BR path/name aliases did not share the correct experiment identity'}
    Write-Output 'PASS: CLI runs GA/ABC/FFA, two rounds, resumes, separates policies, reads BR by name/path and exits without input'
} finally {Pop-Location}
