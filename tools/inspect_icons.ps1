Add-Type -AssemblyName System.Drawing

$p1 = "D:\Prorgram\Project\ArcheoPhd\desktop\release\ArchaeoPhD.exe"
$p2 = "C:\Users\User\AppData\Local\ArchaeoPhD\ArchaeoPhD.exe"

$ico1 = [System.Drawing.Icon]::ExtractAssociatedIcon($p1)
$bmp1 = $ico1.ToBitmap()
$bmp1.Save("D:\Prorgram\Project\ArcheoPhd\desktop\release_icon.png", [System.Drawing.Imaging.ImageFormat]::Png)

$ico2 = [System.Drawing.Icon]::ExtractAssociatedIcon($p2)
$bmp2 = $ico2.ToBitmap()
$bmp2.Save("D:\Prorgram\Project\ArcheoPhd\desktop\installed_icon.png", [System.Drawing.Imaging.ImageFormat]::Png)

Write-Output "release_icon.png and installed_icon.png saved!"

