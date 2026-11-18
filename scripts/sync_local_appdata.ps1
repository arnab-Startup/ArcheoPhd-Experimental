# ArchaeoPhD - Local AppData Deployment Synchronizer
$ErrorActionPreference = 'Stop'

$localAppData = [Environment]::GetFolderPath('LocalApplicationData')
$dest = Join-Path $localAppData 'ArchaeoPhD'
$desktopDir = Resolve-Path (Join-Path $PSScriptRoot '..')

Write-Host "Target local directory: $dest"
Write-Host "Source desktop directory: $desktopDir"

# 1. Stop running ArchaeoPhD processes
Get-Process -Name ArchaeoPhD -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

# 2. Ensure directories exist
if (-not (Test-Path $dest)) {
    New-Item -ItemType Directory -Path $dest -Force | Out-Null
}
$distDest = Join-Path $dest 'dist'
if (-not (Test-Path $distDest)) {
    New-Item -ItemType Directory -Path $distDest -Force | Out-Null
}

# 3. Copy binary and loader
$srcExe = Join-Path $desktopDir 'release\ArchaeoPhD.exe'
$srcLoader = Join-Path $desktopDir 'lib\WebView2Loader.dll'

Copy-Item -Path $srcExe -Destination (Join-Path $dest 'ArchaeoPhD.exe') -Force
Write-Host "Updated: ArchaeoPhD.exe"

Copy-Item -Path $srcLoader -Destination (Join-Path $dest 'WebView2Loader.dll') -Force
Write-Host "Updated: WebView2Loader.dll"

# 4. Copy dist assets recursively
$srcDist = Join-Path $desktopDir 'dist'
robocopy $srcDist $distDest /E /IS /IT /NP /NDL /NFL /NJH /NJS
Write-Host "Synchronized: dist assets"

# 5. Purge WebView2 cache so old bundles are never served
$wvData = Join-Path $dest 'WebView2Data'
if (Test-Path $wvData) {
    Get-ChildItem -Path $wvData -Recurse -Include 'Cache', 'Code Cache' -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        Remove-Item -Path $_.FullName -Recurse -Force -ErrorAction SilentlyContinue
    }
    Write-Host "Purged: WebView2 disk & code cache"
}

Write-Host "Verification:"
Get-Item (Join-Path $dest 'ArchaeoPhD.exe'), (Join-Path $distDest 'index.html'), (Join-Path $distDest 'assets\Dashboard-W5Q9sdSC.js') | Select-Object FullName, Length, LastWriteTime | Format-Table -AutoSize
Write-Host "[SUCCESS] Local workstation updated successfully!"
