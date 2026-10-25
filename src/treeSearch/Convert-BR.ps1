param([Parameter(Mandatory)][string]$InputFile,[int]$Instance=1,[Parameter(Mandatory)][string]$OutputFile)
$ErrorActionPreference='Stop'
$lines=@(Get-Content -LiteralPath $InputFile | Where-Object {$_.Trim()})
$cursor=0
function Read-Row([int]$Count) {
    if($script:cursor -ge $lines.Count){throw 'Truncated BR input'}
    $row=@($lines[$script:cursor++].Trim() -split '\s+' | ForEach-Object {[long]::Parse($_)})
    if($row.Count -ne $Count){throw 'Invalid BR row width'}
    return ,$row
}
$total=(Read-Row 1)[0]
if($Instance -lt 1 -or $Instance -gt $total){throw 'BR instance is out of range'}
$selected=$null
for($number=1;$number -le $total;$number++) {
    $identity=Read-Row 2;$container=Read-Row 3;$count=(Read-Row 1)[0];$items=@();$ids=@{}
    if($count -le 0 -or @($container | Where-Object {$_ -le 0}).Count){throw 'Invalid BR dimensions/type count'}
    for($type=0;$type -lt $count;$type++) {
        $r=Read-Row 8
        if($r[0] -le 0 -or $ids.ContainsKey($r[0]) -or $r[1] -le 0 -or $r[3] -le 0 -or $r[5] -le 0 -or $r[7] -le 0){throw 'Invalid BR type/dimensions/quantity'}
        $ids[$r[0]]=$true
        if($r[7] -gt 32767){throw 'This Windows build retains the original 16-bit quantity type; quantity exceeds 32767'}
        foreach($flag in @($r[2],$r[4],$r[6])){if($flag -notin @(0,1)){throw 'Invalid BR orientation flag'}}
        # BR flags indicate which ORIGINAL dimension may be vertical (JSON Z).
        $items+=@{quantity=$r[7];measurement=@{x=$r[1];y=$r[3];z=$r[5]};profit=$r[1]*$r[3]*$r[5];br_type_id=$r[0];constraint=@{orientation=@{
            xyz=[bool]$r[6];yxz=[bool]$r[6];zyx=[bool]$r[2];yzx=[bool]$r[2];xzy=[bool]$r[4];zxy=[bool]$r[4]}}}
    }
    if($number -eq $Instance){$selected=@{type='input';version='0.8.0';large_object=@{measurement=@{x=$container[0];y=$container[1];z=$container[2]}};small_items=$items;bb3d_source=@{file=[IO.Path]::GetFileName($InputFile);position=$number;id=$identity[0];seed=$identity[1]}}}
}
if($cursor -ne $lines.Count){throw 'Extra BR records'}
$selected | ConvertTo-Json -Depth 15 | Set-Content -LiteralPath $OutputFile -Encoding utf8
