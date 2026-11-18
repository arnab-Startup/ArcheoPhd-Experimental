# ArchaeoPhD - Master Local System Installer (Experimental Workstation)
$ErrorActionPreference = 'Stop'

Write-Host "============================================================"
Write-Host "  Installing ArchaeoPhD Experimental to Local System"
Write-Host "============================================================"

# 1. Paths
$localAppData = [Environment]::GetFolderPath('LocalApplicationData')
$destDir = Join-Path $localAppData 'ArchaeoPhD_Experimental'
$desktopDir = Resolve-Path (Join-Path $PSScriptRoot '..')
$srcExe = Join-Path $desktopDir 'release\Experimental.exe'
$srcLoader = Join-Path $desktopDir 'lib\WebView2Loader.dll'
$srcDist = Join-Path $desktopDir 'dist'

Write-Host "Source Directory: $desktopDir"
Write-Host "Target Directory: $destDir"

# 2. Stop running process
Write-Host "[1/6] Terminating active Experimental processes..."
Get-Process -Name Experimental -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 800

# 3. Ensure target directory structure
Write-Host "[2/6] Preparing directories..."
if (-not (Test-Path $destDir)) {
    New-Item -ItemType Directory -Path $destDir -Force | Out-Null
}
$distDest = Join-Path $destDir 'dist'
if (-not (Test-Path $distDest)) {
    New-Item -ItemType Directory -Path $distDest -Force | Out-Null
}

# 4. Copy binary and loader
Write-Host "[3/6] Installing Experimental.exe and native dependencies..."
Copy-Item -Path $srcExe -Destination (Join-Path $destDir 'Experimental.exe') -Force
Copy-Item -Path $srcLoader -Destination (Join-Path $destDir 'WebView2Loader.dll') -Force

# 5. Mirror dist assets (mirrors so deleted/old chunks are removed)
Write-Host "[4/6] Mirroring runtime dist assets..."
robocopy $srcDist $distDest /MIR /NP /NDL /NFL /NJH /NJS
Write-Host "Dist synchronization complete."

# 6. Purge WebView2 cache
Write-Host "[5/6] Purging WebView2 browser cache..."
$wvData = Join-Path $destDir 'WebView2Data'
if (Test-Path $wvData) {
    Get-ChildItem -Path $wvData -Recurse -Include 'Cache', 'Code Cache' -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        Remove-Item -Path $_.FullName -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# 7. Create/Refresh Shortcuts
Write-Host "[6/6] Updating Desktop and Start Menu shortcuts..."
$wsh = New-Object -ComObject WScript.Shell
$targetExe = Join-Path $destDir 'ArchaeoPhD.exe'

$desktopFolder = [Environment]::GetFolderPath('Desktop')
$programsFolder = [Environment]::GetFolderPath('Programs')

$shortcutLocations = @(
    'C:\Users\User\OneDrive\Desktop\ArchaeoPhD.lnk'
)
if (![string]::IsNullOrEmpty($desktopFolder)) {
    $shortcutLocations += (Join-Path $desktopFolder 'ArchaeoPhD.lnk')
}
if (![string]::IsNullOrEmpty($programsFolder)) {
    $shortcutLocations += (Join-Path $programsFolder 'ArchaeoPhD.lnk')
}

foreach ($scPath in $shortcutLocations) {
    try {
        $parentDir = Split-Path -Parent $scPath
        if (Test-Path $parentDir -ErrorAction SilentlyContinue) {
            $sc = $wsh.CreateShortcut($scPath)
            $sc.TargetPath = $targetExe
            $sc.WorkingDirectory = $destDir
            $sc.Description = "ArchaeoPhD - Offline Research Workstation"
            $sc.IconLocation = "$targetExe,0"
            $sc.Save()
            Write-Host "  Shortcut updated: $scPath"
        }
    } catch {
        # Skip locations outside current permission sandbox
    }
}

# 8. Reset debug log to clean state
$logPath = Join-Path $destDir 'live_debug.log'
if (Test-Path $logPath) {
    Remove-Item -Path $logPath -Force -ErrorAction SilentlyContinue
}

Write-Host "============================================================"
Write-Host "  Verifying Installed Artifacts"
Write-Host "============================================================"
Get-Item (Join-Path $destDir 'Experimental.exe'), (Join-Path $distDest 'index.html'), (Join-Path $distDest 'assets\Dashboard-W5Q9sdSC.js') | Select-Object FullName, Length, LastWriteTime | Format-Table -AutoSize

Write-Host "[INSTALLATION COMPLETE] ArchaeoPhD Experimental successfully installed to local system!"
