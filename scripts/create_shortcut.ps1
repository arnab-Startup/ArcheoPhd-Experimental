$programsFolder = [Environment]::GetFolderPath('Programs')
$shortcutPath = Join-Path $programsFolder "ArchaeoPhD.lnk"
$exePath = "D:\Prorgram\Project\ArcheoPhd\desktop\release\ArchaeoPhD.exe"
$workingDir = "D:\Prorgram\Project\ArcheoPhd\desktop\release"

$wshShell = New-Object -ComObject WScript.Shell
$shortcut = $wshShell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $exePath
$shortcut.WorkingDirectory = $workingDir
$shortcut.Description = "ArchaeoPhD - Offline Research Workstation"
$shortcut.IconLocation = "$exePath,0"
$shortcut.Save()

$desktopFolder = [Environment]::GetFolderPath('Desktop')
$desktopShortcutPath = Join-Path $desktopFolder "ArchaeoPhD.lnk"
$desktopShortcut = $wshShell.CreateShortcut($desktopShortcutPath)
$desktopShortcut.TargetPath = $exePath
$desktopShortcut.WorkingDirectory = $workingDir
$desktopShortcut.Description = "ArchaeoPhD - Offline Research Workstation"
$desktopShortcut.IconLocation = "$exePath,0"
$desktopShortcut.Save()

Write-Output "StartMenuShortcut: $(Test-Path $shortcutPath)"
Write-Output "DesktopShortcut: $(Test-Path $desktopShortcutPath)"
