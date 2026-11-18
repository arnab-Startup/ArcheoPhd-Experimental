Get-Process -Name "ArchaeoPhD" -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 600

$localApp = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::LocalApplicationData)
$appDataDir = Join-Path $localApp "ArchaeoPhD"

if (Test-Path $appDataDir) {
    Copy-Item -Path "desktop\release\ArchaeoPhD.exe" -Destination (Join-Path $appDataDir "ArchaeoPhD.exe") -Force
    
    $distDir = Join-Path $appDataDir "dist"
    if (Test-Path $distDir) {
        Copy-Item -Path "desktop\dist\*" -Destination $distDir -Recurse -Force
    }
    
    $appDistDir = Join-Path $appDataDir "app\dist"
    if (Test-Path $appDistDir) {
        Copy-Item -Path "desktop\dist\*" -Destination $appDistDir -Recurse -Force
    }
    
    $sig = Join-Path $appDataDir "app\build_signature.txt"
    if (Test-Path $sig) {
        Remove-Item -Path $sig -Force
    }
    Write-Output "AppData deployed successfully."
} else {
    Write-Output "AppData directory does not exist."
}

Start-Process -FilePath "desktop\release\ArchaeoPhD.exe"
Start-Sleep -Seconds 1
Get-Process -Name "ArchaeoPhD" | Select-Object Id, ProcessName, Path
