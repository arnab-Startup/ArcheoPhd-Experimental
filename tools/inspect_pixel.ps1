Add-Type -AssemblyName System.Drawing

$b = [System.Drawing.Bitmap]::FromFile("D:\Prorgram\Project\ArcheoPhd\desktop\release_icon.png")
Write-Output "Size: $($b.Width)x$($b.Height)"
# Check center pixel
$c = $b.GetPixel($b.Width / 2, $b.Height / 2)
Write-Output "Center pixel: R=$($c.R), G=$($c.G), B=$($c.B)"
$b.Dispose()

