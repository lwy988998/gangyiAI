# 【逐行批注】 声明脚本参数，控制本次构建或测试行为。
param(
    # 【逐行批注】 执行命令并将结果保存到 [Parameter(Mandatory。
    [Parameter(Mandatory = $true)]
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[string]$SourceImage。
    [string]$SourceImage
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：)。
)

# 【逐行批注】 执行命令并将结果保存到 $ErrorActionPreference。
$ErrorActionPreference = 'Stop'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Add-Type -AssemblyName System.Drawing。
Add-Type -AssemblyName System.Drawing

# 【逐行批注】 执行命令并将结果保存到 $repoRoot。
$repoRoot = Split-Path -Parent $PSScriptRoot
# 【逐行批注】 执行命令并将结果保存到 $assetDir。
$assetDir = Join-Path $repoRoot 'assets\windows'
# 【逐行批注】 执行命令并将结果保存到 $publicDir。
$publicDir = Join-Path $repoRoot 'public'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Force -Path $assetDir, $publicDir | Out-Null。
New-Item -ItemType Directory -Force -Path $assetDir, $publicDir | Out-Null

# 【逐行批注】 定义 PowerShell 函数 New-TransparentLogo，封装可复用流程。
function New-TransparentLogo([string]$Source, [string]$Destination) {
    # 【逐行批注】 执行命令并将结果保存到 $sourceBitmap。
    $sourceBitmap = [System.Drawing.Bitmap]::FromFile((Resolve-Path -LiteralPath $Source))
    # 【逐行批注】 开始可能失败的 PowerShell 操作。
    try {
        # 【逐行批注】 判断条件 ($sourceBitmap.Width -ne $sourceBitmap.Height) {，成立时执行对应流程。
        if ($sourceBitmap.Width -ne $sourceBitmap.Height) {
            # 【逐行批注】 在错误情形终止流程并报告原因：throw 'The source logo must be square.'。
            throw 'The source logo must be square.'
        # 【逐行批注】 开始或结束当前脚本代码块。
        }

        # 【逐行批注】 执行命令并将结果保存到 $padding。
        $padding = 20
        # 【逐行批注】 执行命令并将结果保存到 $size。
        $size = $sourceBitmap.Width + $padding * 2
        # 【逐行批注】 执行命令并将结果保存到 $output。
        $output = New-Object System.Drawing.Bitmap $size, $size, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        # 【逐行批注】 开始可能失败的 PowerShell 操作。
        try {
            # 【逐行批注】 执行命令并将结果保存到 $graphics。
            $graphics = [System.Drawing.Graphics]::FromImage($output)
            # 【逐行批注】 开始可能失败的 PowerShell 操作。
            try {
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.Clear([System.Drawing.Color]::Transparent)。
                $graphics.Clear([System.Drawing.Color]::Transparent)
                # 【逐行批注】 执行命令并将结果保存到 $graphics.SmoothingMode。
                $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
                # 【逐行批注】 执行命令并将结果保存到 $graphics.InterpolationMode。
                $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic

                # 【逐行批注】 执行命令并将结果保存到 $clip。
                $clip = New-Object System.Drawing.Drawing2D.GraphicsPath
                # 【逐行批注】 开始可能失败的 PowerShell 操作。
                try {
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$clip.AddEllipse($padding + 1, $padding + 1, $sourceBitmap.Width - 2, $sourceBitmap.Heigh…。
                    $clip.AddEllipse($padding + 1, $padding + 1, $sourceBitmap.Width - 2, $sourceBitmap.Height - 2)
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.SetClip($clip)。
                    $graphics.SetClip($clip)
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.DrawImage($sourceBitmap, $padding, $padding, $sourceBitmap.Width, $sourceBitmap…。
                    $graphics.DrawImage($sourceBitmap, $padding, $padding, $sourceBitmap.Width, $sourceBitmap.Height)
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
                } finally {
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$clip.Dispose()。
                    $clip.Dispose()
                # 【逐行批注】 开始或结束当前脚本代码块。
                }
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
            } finally {
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.Dispose()。
                $graphics.Dispose()
            # 【逐行批注】 开始或结束当前脚本代码块。
            }
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$output.Save($Destination, [System.Drawing.Imaging.ImageFormat]::Png)。
            $output.Save($Destination, [System.Drawing.Imaging.ImageFormat]::Png)
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
        } finally {
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$output.Dispose()。
            $output.Dispose()
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
    } finally {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$sourceBitmap.Dispose()。
        $sourceBitmap.Dispose()
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 定义 PowerShell 函数 New-ResizedPngBytes，封装可复用流程。
function New-ResizedPngBytes([System.Drawing.Image]$Source, [int]$Size) {
    # 【逐行批注】 执行命令并将结果保存到 $bitmap。
    $bitmap = New-Object System.Drawing.Bitmap $Size, $Size, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    # 【逐行批注】 开始可能失败的 PowerShell 操作。
    try {
        # 【逐行批注】 执行命令并将结果保存到 $graphics。
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        # 【逐行批注】 开始可能失败的 PowerShell 操作。
        try {
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.Clear([System.Drawing.Color]::Transparent)。
            $graphics.Clear([System.Drawing.Color]::Transparent)
            # 【逐行批注】 执行命令并将结果保存到 $graphics.SmoothingMode。
            $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
            # 【逐行批注】 执行命令并将结果保存到 $graphics.InterpolationMode。
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            # 【逐行批注】 执行命令并将结果保存到 $graphics.PixelOffsetMode。
            $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.DrawImage($Source, 0, 0, $Size, $Size)。
            $graphics.DrawImage($Source, 0, 0, $Size, $Size)
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
        } finally {
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.Dispose()。
            $graphics.Dispose()
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 执行命令并将结果保存到 $stream。
        $stream = New-Object System.IO.MemoryStream
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$bitmap.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)。
        $bitmap.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)
        # 【逐行批注】 返回函数结果并结束当前流程：return ,$stream.ToArray()。
        return ,$stream.ToArray()
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
    } finally {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$bitmap.Dispose()。
        $bitmap.Dispose()
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 定义 PowerShell 函数 New-MultiSizeIcon，封装可复用流程。
function New-MultiSizeIcon([string]$Source, [string]$Destination) {
    # 【逐行批注】 执行命令并将结果保存到 $image。
    $image = [System.Drawing.Image]::FromFile($Source)
    # 【逐行批注】 开始可能失败的 PowerShell 操作。
    try {
        # 【逐行批注】 执行命令并将结果保存到 $sizes。
        $sizes = @(16, 24, 32, 48, 64, 128, 256)
        # 【逐行批注】 执行命令并将结果保存到 $images。
        $images = foreach ($size in $sizes) { New-ResizedPngBytes $image $size }
        # 【逐行批注】 执行命令并将结果保存到 $stream。
        $stream = [System.IO.File]::Create($Destination)
        # 【逐行批注】 执行命令并将结果保存到 $writer。
        $writer = New-Object System.IO.BinaryWriter $stream
        # 【逐行批注】 开始可能失败的 PowerShell 操作。
        try {
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([uint16]0)。
            $writer.Write([uint16]0)
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([uint16]1)。
            $writer.Write([uint16]1)
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([uint16]$images.Count)。
            $writer.Write([uint16]$images.Count)
            # 【逐行批注】 执行命令并将结果保存到 $offset。
            $offset = 6 + 16 * $images.Count
            # 【逐行批注】 逐个处理集合中的元素：for ($index = 0; $index -lt $images.Count; $index++) {。
            for ($index = 0; $index -lt $images.Count; $index++) {
                # 【逐行批注】 执行命令并将结果保存到 $size。
                $size = $sizes[$index]
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))。
                $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))。
                $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([byte]0)。
                $writer.Write([byte]0)
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([byte]0)。
                $writer.Write([byte]0)
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([uint16]1)。
                $writer.Write([uint16]1)
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([uint16]32)。
                $writer.Write([uint16]32)
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([uint32]$images[$index].Length)。
                $writer.Write([uint32]$images[$index].Length)
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Write([uint32]$offset)。
                $writer.Write([uint32]$offset)
                # 【逐行批注】 执行命令并将结果保存到 $offset +。
                $offset += $images[$index].Length
            # 【逐行批注】 开始或结束当前脚本代码块。
            }
            # 【逐行批注】 逐个处理集合中的元素：foreach ($bytes in $images) { $writer.Write($bytes) }。
            foreach ($bytes in $images) { $writer.Write($bytes) }
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
        } finally {
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$writer.Dispose()。
            $writer.Dispose()
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$stream.Dispose()。
            $stream.Dispose()
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
    } finally {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$image.Dispose()。
        $image.Dispose()
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 定义 PowerShell 函数 New-WizardImage，封装可复用流程。
function New-WizardImage([string]$LogoPath, [string]$Destination, [int]$Width, [int]$Height, [bool]$Compact) {
    # 【逐行批注】 执行命令并将结果保存到 $bitmap。
    $bitmap = New-Object System.Drawing.Bitmap $Width, $Height, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    # 【逐行批注】 执行命令并将结果保存到 $logo。
    $logo = [System.Drawing.Image]::FromFile($LogoPath)
    # 【逐行批注】 开始可能失败的 PowerShell 操作。
    try {
        # 【逐行批注】 执行命令并将结果保存到 $graphics。
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        # 【逐行批注】 开始可能失败的 PowerShell 操作。
        try {
            # 【逐行批注】 执行命令并将结果保存到 $graphics.SmoothingMode。
            $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
            # 【逐行批注】 执行命令并将结果保存到 $graphics.InterpolationMode。
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            # 【逐行批注】 执行命令并将结果保存到 $rect。
            $rect = New-Object System.Drawing.Rectangle 0, 0, $Width, $Height
            # 【逐行批注】 执行命令并将结果保存到 $brush。
            $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush $rect, ([System.Drawing.Color]::FromArgb(240, 249, 255)), ([System.Drawing.Color]::FromArgb(14, 165, 233)), 90
            # 【逐行批注】 开始可能失败的 PowerShell 操作。
            try { $graphics.FillRectangle($brush, $rect) } finally { $brush.Dispose() }

            # 【逐行批注】 执行命令并将结果保存到 $logoSize。
            $logoSize = if ($Compact) { [Math]::Min($Width - 8, $Height - 8) } else { [Math]::Min($Width - 28, 128) }
            # 【逐行批注】 执行命令并将结果保存到 $logoX。
            $logoX = [int](($Width - $logoSize) / 2)
            # 【逐行批注】 执行命令并将结果保存到 $logoY。
            $logoY = if ($Compact) { [int](($Height - $logoSize) / 2) } else { 32 }
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.DrawImage($logo, $logoX, $logoY, $logoSize, $logoSize)。
            $graphics.DrawImage($logo, $logoX, $logoY, $logoSize, $logoSize)

            # 【逐行批注】 判断条件 (-not $Compact) {，成立时执行对应流程。
            if (-not $Compact) {
                # 【逐行批注】 执行命令并将结果保存到 $font。
                $font = New-Object System.Drawing.Font -ArgumentList 'Microsoft YaHei UI', 14, ([System.Drawing.FontStyle]::Bold), ([System.Drawing.GraphicsUnit]::Pixel)
                # 【逐行批注】 执行命令并将结果保存到 $textBrush。
                $textBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(7, 89, 133))
                # 【逐行批注】 执行命令并将结果保存到 $format。
                $format = New-Object System.Drawing.StringFormat
                # 【逐行批注】 开始可能失败的 PowerShell 操作。
                try {
                    # 【逐行批注】 执行命令并将结果保存到 $format.Alignment。
                    $format.Alignment = [System.Drawing.StringAlignment]::Center
                    # 【逐行批注】 执行命令并将结果保存到 $brandName。
                    $brandName = -join @([char]0x94A2, [char]0x4E00, [char]0x5B9A, [char]0x5236, 'A', 'I')
                    # 【逐行批注】 执行命令并将结果保存到 $textRect。
                    $textRect = New-Object System.Drawing.RectangleF -ArgumentList 8, ($logoY + $logoSize + 18), ($Width - 16), 52
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.DrawString($brandName, $font, $textBrush, $textRect, $format)。
                    $graphics.DrawString($brandName, $font, $textBrush, $textRect, $format)
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
                } finally {
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$format.Dispose()。
                    $format.Dispose()
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$textBrush.Dispose()。
                    $textBrush.Dispose()
                    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$font.Dispose()。
                    $font.Dispose()
                # 【逐行批注】 开始或结束当前脚本代码块。
                }
            # 【逐行批注】 开始或结束当前脚本代码块。
            }
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
        } finally {
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$graphics.Dispose()。
            $graphics.Dispose()
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$bitmap.Save($Destination, [System.Drawing.Imaging.ImageFormat]::Bmp)。
        $bitmap.Save($Destination, [System.Drawing.Imaging.ImageFormat]::Bmp)
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
    } finally {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$logo.Dispose()。
        $logo.Dispose()
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$bitmap.Dispose()。
        $bitmap.Dispose()
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 执行命令并将结果保存到 $transparentLogo。
$transparentLogo = Join-Path $publicDir 'school-logo.png'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-TransparentLogo $SourceImage $transparentLogo。
New-TransparentLogo $SourceImage $transparentLogo
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath $transparentLogo -Destination (Join-Path $assetDir 'school-logo.pn…。
Copy-Item -LiteralPath $transparentLogo -Destination (Join-Path $assetDir 'school-logo.png') -Force
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-MultiSizeIcon $transparentLogo (Join-Path $assetDir 'gangyiAI.ico')。
New-MultiSizeIcon $transparentLogo (Join-Path $assetDir 'gangyiAI.ico')
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-WizardImage $transparentLogo (Join-Path $assetDir 'wizard-large.bmp') 164 314 $false。
New-WizardImage $transparentLogo (Join-Path $assetDir 'wizard-large.bmp') 164 314 $false
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-WizardImage $transparentLogo (Join-Path $assetDir 'wizard-small.bmp') 55 55 $true。
New-WizardImage $transparentLogo (Join-Path $assetDir 'wizard-small.bmp') 55 55 $true

# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Write-Host '[done] Brand assets generated.'。
Write-Host '[done] Brand assets generated.'
