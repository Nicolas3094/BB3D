param([string]$Compiler='C:/mingw64/bin/g++.exe',[int]$Jobs=2)
$ErrorActionPreference='Stop'
$root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$downloads=Join-Path $root 'build/sclp-original';New-Item -ItemType Directory -Force $downloads | Out-Null
$boost=Join-Path $downloads 'boost_1_83_0';$archive=Join-Path $downloads 'boost_1_83_0.tar.gz'
if(!(Test-Path "$boost/boost/version.hpp")) {
    if(!(Test-Path $archive)){Invoke-WebRequest 'https://archives.boost.io/release/1.83.0/source/boost_1_83_0.tar.gz' -OutFile $archive}
    if((Get-FileHash $archive -Algorithm SHA256).Hash -ne 'C0685B68DD44CC46574CCE86C4E17C0F611B15E195BE9848DFD0769A0A207628'){throw 'Boost archive hash mismatch'}
    & tar -xzf $archive -C $downloads boost_1_83_0/boost boost_1_83_0/libs/timer/src boost_1_83_0/libs/chrono/src boost_1_83_0/LICENSE_1_0.txt
    if($LASTEXITCODE -ne 0){throw 'Boost extraction failed'}
}
$previousPath=$env:PATH
try {
    $env:PATH=(Split-Path $Compiler -Parent)+';'+$env:PATH
    & cmake -S $PSScriptRoot -B "$root/build/sclp-release" -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release "-DCMAKE_CXX_COMPILER=$Compiler" "-DBOOST_ROOT=$boost"
    if($LASTEXITCODE -ne 0){throw 'SCLP configuration failed'}
    # CMake may clear cache options once when switching the compiler executable.
    & cmake -S $PSScriptRoot -B "$root/build/sclp-release" -DCMAKE_BUILD_TYPE=Release "-DBOOST_ROOT=$boost"
    if($LASTEXITCODE -ne 0){throw 'Release configuration failed'}
    & cmake --build "$root/build/sclp-release" --parallel $Jobs
    if($LASTEXITCODE -ne 0){throw 'SCLP compilation failed'}
} finally {$env:PATH=$previousPath}
