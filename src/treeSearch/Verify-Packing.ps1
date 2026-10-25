param([Parameter(Mandatory)][string]$InputFile,[Parameter(Mandatory)][string]$PackingFile,[string]$ReportFile)
$ErrorActionPreference='Stop'
$inputData=Get-Content -LiteralPath $InputFile -Raw | ConvertFrom-Json
$packing=Get-Content -LiteralPath $PackingFile -Raw | ConvertFrom-Json
$limit=$inputData.large_object.measurement
foreach($axis in @('x','y','z')){if($limit.$axis -le 0 -or $packing.large_object.measurement.$axis -ne $limit.$axis){throw 'Container mismatch'}}
$counts=@{};$volume=[long]0;$boxes=@($packing.small_items)
if($boxes.Count -eq 0){throw 'Empty packing'}
for($i=0;$i -lt $boxes.Count;$i++) {
    $box=$boxes[$i]
    if($null -eq $box.type_index -or $box.type_index -ne [math]::Floor($box.type_index) -or $box.type_index -lt 0 -or $box.type_index -ge $inputData.small_items.Count){throw 'Invalid item type index'}
    $type=$inputData.small_items[$box.type_index];$orientation=[string]$box.orientation
    if($orientation -notin @('xyz','yxz','zyx','xzy','zxy','yzx') -or !$type.constraint.orientation.$orientation){throw 'Forbidden orientation'}
    if(!$counts.ContainsKey($box.type_index)){$counts[$box.type_index]=0}
    if(++$counts[$box.type_index] -gt $type.quantity){throw 'Quantity exceeded'}
    for($a=0;$a -lt 3;$a++) {
        $axis=@('x','y','z')[$a];$sourceAxis=[string]$orientation[$a]
        $size=$box.measurement.$axis;$position=$box.position.$axis
        if($size -ne $type.measurement.$sourceAxis -or $size -le 0 -or $size -ne [math]::Floor($size)){throw 'Dimensions/orientation mismatch'}
        if($null -eq $position -or $position -ne [math]::Floor($position) -or $position -lt 0 -or $position+$size -gt $limit.$axis){throw 'Box outside container'}
    }
    $volume += [long]$box.measurement.x*$box.measurement.y*$box.measurement.z
    for($j=0;$j -lt $i;$j++) {
        $other=$boxes[$j];$overlap=$true
        foreach($axis in @('x','y','z')) {
            if($box.position.$axis -ge $other.position.$axis+$other.measurement.$axis -or $other.position.$axis -ge $box.position.$axis+$box.measurement.$axis){$overlap=$false;break}
        }
        if($overlap){throw "Intersection between boxes $i and $j"}
    }
}
$capacity=[long]$limit.x*$limit.y*$limit.z
if($volume -ne $packing.appendix.packing_statistics.total_volume_of_allocated_small_items){throw 'Reported volume mismatch'}
$result=[ordered]@{valid=$true;boxes=$boxes.Count;volume=$volume;container_volume=$capacity;occupation_percent=100.0*$volume/$capacity;pairwise_checks=$boxes.Count*($boxes.Count-1)/2;orientation_quantities_bounds_intersections='passed'}
if($ReportFile){$result | ConvertTo-Json | Set-Content -LiteralPath $ReportFile -Encoding utf8}
[pscustomobject]$result
