Add-Type -AssemblyName System.Drawing

$srcPath = "D:\Prorgram\Project\ArcheoPhd\desktop\src\app_logo.jpg"
if (-not (Test-Path $srcPath)) {
    Write-Error "Source image not found: $srcPath"
    exit 1
}

$srcBmp = [System.Drawing.Bitmap]::FromFile($srcPath)
Write-Output ("Source loaded: {0}x{1}" -f $srcBmp.Width, $srcBmp.Height)

# Create a clean high-res 512x512 and 256x256 PNG
$sizes = @(256, 128, 64, 48, 32, 16)
$pngStreams = @()

foreach ($sz in $sizes) {
    $targetBmp = New-Object System.Drawing.Bitmap $sz, $sz, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($targetBmp)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $g.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
    $g.Clear([System.Drawing.Color]::Transparent)

    # Draw rounded rectangle clipping or full logo
    # For a clean desktop icon: draw srcBmp scaled into target
    $rect = New-Object System.Drawing.Rectangle 0, 0, $sz, $sz
    $g.DrawImage($srcBmp, $rect)
    $g.Dispose()

    $ms = New-Object System.IO.MemoryStream
    $targetBmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $bytes = $ms.ToArray()
    $ms.Dispose()
    $targetBmp.Dispose()

    $pngStreams += ,@($sz, $bytes)
}

# Also save full 512x512 app-logo.png for UI usage
$uiBmp = New-Object System.Drawing.Bitmap 512, 512, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$gu = [System.Drawing.Graphics]::FromImage($uiBmp)
$gu.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$gu.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
$gu.DrawImage($srcBmp, (New-Object System.Drawing.Rectangle 0, 0, 512, 512))
$gu.Dispose()

$uiBmp.Save("D:\Prorgram\Project\ArcheoPhd\desktop\dist\assets\app-logo.png", [System.Drawing.Imaging.ImageFormat]::Png)
if (Test-Path "D:\Prorgram\Project\ArcheoPhd\reference\public") {
    $uiBmp.Save("D:\Prorgram\Project\ArcheoPhd\reference\public\app-logo.png", [System.Drawing.Imaging.ImageFormat]::Png)
}
$uiBmp.Dispose()
$srcBmp.Dispose()

# Now construct the .ico binary format
# Header: 6 bytes
# 0-1: 0 (reserved)
# 2-3: 1 (icon type)
# 4-5: count of images
$icoStream = New-Object System.IO.MemoryStream
$bw = New-Object System.IO.BinaryWriter $icoStream

$bw.Write([uint16]0)
$bw.Write([uint16]1)
$bw.Write([uint16]$pngStreams.Count)

$offset = 6 + ($pngStreams.Count * 16)

# Write Directory entries
foreach ($entry in $pngStreams) {
    $sz = $entry[0]
    $data = $entry[1]

    $wByte = if ($sz -ge 256) { [byte]0 } else { [byte]$sz }
    $hByte = if ($sz -ge 256) { [byte]0 } else { [byte]$sz }

    $bw.Write($wByte)                 # width
    $bw.Write($hByte)                 # height
    $bw.Write([byte]0)                # color count
    $bw.Write([byte]0)                # reserved
    $bw.Write([uint16]1)              # planes
    $bw.Write([uint16]32)             # bit count
    $bw.Write([uint32]$data.Length)   # bytes in res
    $bw.Write([uint32]$offset)        # image offset

    $offset += $data.Length
}

# Write image data
foreach ($entry in $pngStreams) {
    $data = $entry[1]
    $bw.Write($data, 0, $data.Length)
}

$bw.Flush()
$icoBytes = $icoStream.ToArray()
$bw.Dispose()
$icoStream.Dispose()

# Write app.ico to desktop/src and web/installer
[System.IO.File]::WriteAllBytes("D:\Prorgram\Project\ArcheoPhd\desktop\src\app.ico", $icoBytes)
Write-Output ("Wrote desktop\src\app.ico ({0} bytes)" -f $icoBytes.Length)

if (Test-Path "D:\Prorgram\Project\ArcheoPhd\web\installer") {
    [System.IO.File]::WriteAllBytes("D:\Prorgram\Project\ArcheoPhd\web\installer\app.ico", $icoBytes)
    Write-Output ("Wrote web\installer\app.ico ({0} bytes)" -f $icoBytes.Length)
}

Write-Output "Icon generation complete!"

