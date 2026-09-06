param(
    [Parameter(Mandatory = $true)]
    [string]$SourceImage
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$repoRoot = Split-Path -Parent $PSScriptRoot
$assetDir = Join-Path $repoRoot 'assets\windows'
$publicDir = Join-Path $repoRoot 'public'
New-Item -ItemType Directory -Force -Path $assetDir, $publicDir | Out-Null

function New-TransparentLogo([string]$Source, [string]$Destination) {
    $sourceBitmap = [System.Drawing.Bitmap]::FromFile((Resolve-Path -LiteralPath $Source))
    try {
        if ($sourceBitmap.Width -ne $sourceBitmap.Height) {
            throw 'The source logo must be square.'
        }

        $padding = 20
        $size = $sourceBitmap.Width + $padding * 2
        $output = New-Object System.Drawing.Bitmap $size, $size, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            $graphics = [System.Drawing.Graphics]::FromImage($output)
            try {
                $graphics.Clear([System.Drawing.Color]::Transparent)
                $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
                $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic

                $clip = New-Object System.Drawing.Drawing2D.GraphicsPath
                try {
                    $clip.AddEllipse($padding + 1, $padding + 1, $sourceBitmap.Width - 2, $sourceBitmap.Height - 2)
                    $graphics.SetClip($clip)
                    $graphics.DrawImage($sourceBitmap, $padding, $padding, $sourceBitmap.Width, $sourceBitmap.Height)
                } finally {
                    $clip.Dispose()
                }
            } finally {
                $graphics.Dispose()
            }
            $output.Save($Destination, [System.Drawing.Imaging.ImageFormat]::Png)
        } finally {
            $output.Dispose()
        }
    } finally {
        $sourceBitmap.Dispose()
    }
}

function New-ResizedPngBytes([System.Drawing.Image]$Source, [int]$Size) {
    $bitmap = New-Object System.Drawing.Bitmap $Size, $Size, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try {
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        try {
            $graphics.Clear([System.Drawing.Color]::Transparent)
            $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            $graphics.DrawImage($Source, 0, 0, $Size, $Size)
        } finally {
            $graphics.Dispose()
        }
        $stream = New-Object System.IO.MemoryStream
        $bitmap.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)
        return ,$stream.ToArray()
    } finally {
        $bitmap.Dispose()
    }
}

function New-MultiSizeIcon([string]$Source, [string]$Destination) {
    $image = [System.Drawing.Image]::FromFile($Source)
    try {
        $sizes = @(16, 24, 32, 48, 64, 128, 256)
        $images = foreach ($size in $sizes) { New-ResizedPngBytes $image $size }
        $stream = [System.IO.File]::Create($Destination)
        $writer = New-Object System.IO.BinaryWriter $stream
        try {
            $writer.Write([uint16]0)
            $writer.Write([uint16]1)
            $writer.Write([uint16]$images.Count)
            $offset = 6 + 16 * $images.Count
            for ($index = 0; $index -lt $images.Count; $index++) {
                $size = $sizes[$index]
                $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
                $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
                $writer.Write([byte]0)
                $writer.Write([byte]0)
                $writer.Write([uint16]1)
                $writer.Write([uint16]32)
                $writer.Write([uint32]$images[$index].Length)
                $writer.Write([uint32]$offset)
                $offset += $images[$index].Length
            }
            foreach ($bytes in $images) { $writer.Write($bytes) }
        } finally {
            $writer.Dispose()
            $stream.Dispose()
        }
    } finally {
        $image.Dispose()
    }
}

function New-WizardImage([string]$LogoPath, [string]$Destination, [int]$Width, [int]$Height, [bool]$Compact) {
    $bitmap = New-Object System.Drawing.Bitmap $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $logo = [System.Drawing.Image]::FromFile($LogoPath)
    try {
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        try {
            $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $rect = New-Object System.Drawing.Rectangle 0, 0, $Width, $Height
            $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush $rect, ([System.Drawing.Color]::FromArgb(240, 249, 255)), ([System.Drawing.Color]::FromArgb(14, 165, 233)), 90
            try { $graphics.FillRectangle($brush, $rect) } finally { $brush.Dispose() }

            $logoSize = if ($Compact) { [Math]::Min($Width - 8, $Height - 8) } else { [Math]::Min($Width - 28, 128) }
            $logoX = [int](($Width - $logoSize) / 2)
            $logoY = if ($Compact) { [int](($Height - $logoSize) / 2) } else { 32 }
            $graphics.DrawImage($logo, $logoX, $logoY, $logoSize, $logoSize)

            if (-not $Compact) {
                $font = New-Object System.Drawing.Font -ArgumentList 'Microsoft YaHei UI', 14, ([System.Drawing.FontStyle]::Bold), ([System.Drawing.GraphicsUnit]::Pixel)
                $textBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(7, 89, 133))
                $format = New-Object System.Drawing.StringFormat
                try {
                    $format.Alignment = [System.Drawing.StringAlignment]::Center
                    $brandName = -join @([char]0x94A2, [char]0x4E00, [char]0x5B9A, [char]0x5236, 'A', 'I')
                    $textRect = New-Object System.Drawing.RectangleF -ArgumentList 8, ($logoY + $logoSize + 18), ($Width - 16), 52
                    $graphics.DrawString($brandName, $font, $textBrush, $textRect, $format)
                } finally {
                    $format.Dispose()
                    $textBrush.Dispose()
                    $font.Dispose()
                }
            }
        } finally {
            $graphics.Dispose()
        }
        $bitmap.Save($Destination, [System.Drawing.Imaging.ImageFormat]::Bmp)
    } finally {
        $logo.Dispose()
        $bitmap.Dispose()
    }
}

$transparentLogo = Join-Path $publicDir 'school-logo.png'
New-TransparentLogo $SourceImage $transparentLogo
Copy-Item -LiteralPath $transparentLogo -Destination (Join-Path $assetDir 'school-logo.png') -Force
New-MultiSizeIcon $transparentLogo (Join-Path $assetDir 'gangyiAI.ico')
New-WizardImage $transparentLogo (Join-Path $assetDir 'wizard-large.bmp') 164 314 $false
New-WizardImage $transparentLogo (Join-Path $assetDir 'wizard-small.bmp') 55 55 $true

Write-Host '[done] Brand assets generated.'
