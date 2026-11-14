Add-Type -AssemblyName System.Drawing

$lnk = "C:\Users\User\OneDrive\Desktop\ArchaeoPhD.lnk"
$wsh = New-Object -ComObject WScript.Shell
$sc = $wsh.CreateShortcut($lnk)
Write-Output "Current Shortcut IconLocation: $($sc.IconLocation)"

# Extract icon from the shortcut's target or IconLocation
$icoPath = $sc.IconLocation.Split(',')[0]
Write-Output "Icon file: $icoPath"
if (Test-Path $icoPath) {
    $icon = [System.Drawing.Icon]::ExtractAssociatedIcon($icoPath)
    $bmp = $icon.ToBitmap()
    $bmp.Save("D:\Prorgram\Project\ArcheoPhd\desktop\shortcut_icon_verified.png", [System.Drawing.Imaging.ImageFormat]::Png)
    Write-Output "Verified: Shortcut icon successfully extracted ($($bmp.Width)x$($bmp.Height))"
    $icon.Dispose()
    $bmp.Dispose()
}

