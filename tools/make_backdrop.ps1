Add-Type -AssemblyName System.Drawing

$srcPath = "D:\Prorgram\Project\ArcheoPhd\desktop\src\app_logo.jpg" # logo
$bgPath = "D:\Prorgram\Project\ArcheoPhd\desktop\dist\assets\welcome-bg.jpg" # 1024x576

$bmp = [System.Drawing.Bitmap]::FromFile($bgPath)
$w = $bmp.Width
$h = $bmp.Height

# We will create a backdrop bitmap where the center is shaded so HTML text and cards are 100% crisp and readable
$outBmp = New-Object System.Drawing.Bitmap $w, $h, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$g = [System.Drawing.Graphics]::FromImage($outBmp)
$g.DrawImage($bmp, 0, 0, $w, $h)

# Create a smooth elliptical gradient brush to soften the center area (where interactive HTML elements live)
$path = New-Object System.Drawing.Drawing2D.GraphicsPath
# Center ellipse covering x: 180..844, y: 50..520
$rect = New-Object System.Drawing.Rectangle 180, 50, 664, 470
$path.AddEllipse($rect)

$pgb = New-Object System.Drawing.Drawing2D.PathGradientBrush $path
# Center color: deep rich dark brown #100b09 with 94% opacity
$pgb.CenterColor = [System.Drawing.Color]::FromArgb(242, 16, 11, 9)
# Surround colors: fade to 0 opacity at the edges so temple on left and Buddha on right remain vibrant
$surroundColors = @([System.Drawing.Color]::FromArgb(0, 16, 11, 9))
$pgb.SurroundColors = $surroundColors

$g.FillPath($pgb, $path)
$pgb.Dispose()
$path.Dispose()

# Save as welcome-backdrop.jpg
$outPath = "D:\Prorgram\Project\ArcheoPhd\desktop\dist\assets\welcome-backdrop.jpg"
$outBmp.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Jpeg)

$outBmp.Dispose()
$bmp.Dispose()
$g.Dispose()

Write-Output "Successfully generated welcome-backdrop.jpg!"

