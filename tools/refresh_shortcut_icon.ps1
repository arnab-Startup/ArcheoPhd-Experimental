$installDir = [System.IO.Path]::Combine($env:LOCALAPPDATA, 'ArchaeoPhD')
$desktop = [Environment]::GetFolderPath("Desktop")
$oneDriveDesktop = [System.IO.Path]::Combine($env:USERPROFILE, 'OneDrive', 'Desktop')

# 1. Copy the new app.ico directly into the install directory
Copy-Item "desktop\src\app.ico" (Join-Path $installDir "app.ico") -Force
Write-Output "Copied app.ico to $installDir\app.ico"

# 2. Update shortcut on Desktop and OneDrive Desktop
$desktopPaths = @($desktop, $oneDriveDesktop)

$wsh = New-Object -ComObject WScript.Shell

foreach ($dPath in $desktopPaths) {
    if (Test-Path $dPath) {
        $lnkPath = Join-Path $dPath "ArchaeoPhD.lnk"
        if (Test-Path $lnkPath) {
            Write-Output "Updating shortcut at $lnkPath..."
            $shortcut = $wsh.CreateShortcut($lnkPath)
            $shortcut.TargetPath = Join-Path $installDir "ArchaeoPhD.exe"
            $shortcut.WorkingDirectory = $installDir
            # Point directly to the new .ico file so Windows Explorer bypasses the stale EXE icon cache
            $shortcut.IconLocation = (Join-Path $installDir "app.ico") + ",0"
            $shortcut.Save()
            
            # Touch timestamp
            (Get-Item $lnkPath).LastWriteTime = Get-Date
            Write-Output "Saved shortcut with IconLocation: $($shortcut.IconLocation)"
        }
    }
}

# 3. Notify Windows Shell to refresh all icon caches
Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public class ShellRefresh {
    [DllImport("shell32.dll")]
    public static extern void SHChangeNotify(int wEventId, int uFlags, IntPtr dwItem1, IntPtr dwItem2);
}
"@
# SHCNE_ASSOCCHANGED = 0x08000000, SHCNF_IDLIST = 0x0000
[ShellRefresh]::SHChangeNotify(0x08000000, 0, [IntPtr]::Zero, [IntPtr]::Zero)

# Also run ie4uinit to refresh icon cache on modern Windows 10/11
Start-Process -FilePath "ie4uinit.exe" -ArgumentList "-show" -Wait -ErrorAction SilentlyContinue

Write-Output "Icon cache notification broadcasted successfully!"

