$installDir = [System.IO.Path]::Combine($env:LOCALAPPDATA, 'ArchaeoPhD')
Write-Output "Updating installed ArchaeoPhD at: $installDir"

# 1. Terminate running process
Get-Process -Name ArchaeoPhD -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 1200

# 2. Clear stale WebView2 cache so brand-new JS/HTML is loaded fresh
$cachePaths = @(
    (Join-Path $installDir 'WebView2Data\EBWebView\Default\Cache'),
    (Join-Path $installDir 'WebView2Data\EBWebView\Default\Code Cache'),
    (Join-Path $installDir 'WebView2Data\EBWebView\Default\GPUCache'),
    (Join-Path $installDir 'WebView2Data\EBWebView\Default\DawnGraphiteCache'),
    (Join-Path $installDir 'WebView2Data\EBWebView\Default\DawnWebGPUCache')
)
foreach ($cp in $cachePaths) {
    if (Test-Path $cp) {
        Remove-Item $cp -Recurse -Force -ErrorAction SilentlyContinue
    }
}
Write-Output "Cleared WebView2 cache to force fresh assets reload"

# 3. Copy dist
$rootDist = Join-Path $installDir 'dist'
if (-not (Test-Path $rootDist)) {
    New-Item -ItemType Directory -Path $rootDist -Force
}
Copy-Item 'desktop\dist\*' $rootDist -Recurse -Force
Write-Output "Copied to $rootDist"

# Also copy to app\dist if exists
$appDist = Join-Path $installDir 'app\dist'
if (Test-Path (Join-Path $installDir 'app')) {
    Copy-Item 'desktop\dist\*' $appDist -Recurse -Force
}

# 4. Update EXE
Copy-Item 'desktop\release\ArchaeoPhD.exe' (Join-Path $installDir 'ArchaeoPhD.exe') -Force
Write-Output "Copied ArchaeoPhD.exe"

# 5. Ensure Desktop shortcut is verified
$dtFolder = [Environment]::GetFolderPath("Desktop")
$desktopPaths = @(
    (Join-Path $dtFolder "ArchaeoPhD.lnk"),
    "C:\Users\User\OneDrive\Desktop\ArchaeoPhD.lnk"
)
$wsh = New-Object -ComObject WScript.Shell
$targetExe = Join-Path $installDir 'ArchaeoPhD.exe'
foreach ($lnkPath in $desktopPaths) {
    if (Test-Path (Split-Path $lnkPath)) {
        $sc = $wsh.CreateShortcut($lnkPath)
        $sc.TargetPath = $targetExe
        $sc.WorkingDirectory = $installDir
        $ico = Join-Path $installDir 'app.ico'
        if (Test-Path $ico) {
            $sc.IconLocation = "$ico,0"
        }
        $sc.Save()
        Write-Output "Updated shortcut: $lnkPath"
    }
}

# 6. Launch application
Start-Process -FilePath $targetExe -WorkingDirectory $installDir
Start-Sleep -Seconds 1

$p = Get-Process -Name ArchaeoPhD -ErrorAction SilentlyContinue
if ($p) {
    Write-Output ("Running ArchaeoPhD PID: {0}" -f $p.Id)
} else {
    Write-Output "Process not detected immediately"
}
