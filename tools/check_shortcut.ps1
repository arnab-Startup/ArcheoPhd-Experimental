$desktop = [Environment]::GetFolderPath("Desktop")
$lnkPath = Join-Path $desktop "ArchaeoPhD.lnk"

Write-Output "Checking Desktop path: $lnkPath"
if (Test-Path $lnkPath) {
    $wsh = New-Object -ComObject WScript.Shell
    $shortcut = $wsh.CreateShortcut($lnkPath)
    Write-Output "TargetPath:   $($shortcut.TargetPath)"
    Write-Output "IconLocation: $($shortcut.IconLocation)"
    Write-Output "WorkingDirectory: $($shortcut.WorkingDirectory)"
} else {
    Write-Output "Shortcut not found on Desktop!"
}

