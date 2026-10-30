# Shared, retrying output primitives. No batch is started by dot-sourcing this file.

function Commit-PoolFile([string]$Temporary, [string]$Path) {
    for ($attempt = 0; $attempt -lt 200; $attempt++) {
        try {
            Move-Item -LiteralPath $Temporary -Destination $Path -Force
            return
        }
        catch {
            if ($attempt -eq 199) {
                throw
            }
            Start-Sleep -Milliseconds 30
        }
    }
}

function Write-PoolText([string]$Path, [string[]]$Lines) {
    $temporary = $Path + '.tmp-' + [guid]::NewGuid().ToString('N')
    $Lines | Set-Content -LiteralPath $temporary -Encoding utf8
    Commit-PoolFile $temporary $Path
}

function Read-PoolText([string]$Path) {
    $stream = [IO.FileStream]::new($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read, ([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
    $reader = [IO.StreamReader]::new($stream)

    try {
        return $reader.ReadToEnd()
    } finally {
        $reader.Dispose()
        $stream.Dispose()
    }
}

function Write-PoolProgress([string]$Path, [string[]]$Lines) {
    try {
        Write-PoolText $Path $Lines
    } catch {
        Write-Warning "Progress update skipped: $($_.Exception.Message)"
    }
}
