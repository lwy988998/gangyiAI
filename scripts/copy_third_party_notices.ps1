# 把已归档的第三方通知随程序交付，并记录本次实际编译器和运行库摘要。
param(
    [Parameter(Mandatory = $true)][string]$DistDir,
    [string]$CompilerPath = 'g++'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$distributionRoot = (Resolve-Path -LiteralPath $DistDir).Path

function Resolve-NoticePath([string]$Base, [string]$Relative) {
    if ([IO.Path]::IsPathRooted($Relative)) { throw "通知清单包含绝对路径：$Relative" }
    $prefix = [IO.Path]::GetFullPath($Base).TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar
    $candidate = [IO.Path]::GetFullPath((Join-Path $Base $Relative))
    if (-not $candidate.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "通知清单路径超出目录：$Relative"
    }
    return $candidate
}

function Get-NoticeTextHash([string]$Path) {
    # 只在校验时统一 UTF-8 文本换行，复制时保留文件原字节。
    $text = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8).Replace("`r`n", "`n")
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try {
        return ([BitConverter]::ToString($algorithm.ComputeHash([Text.Encoding]::UTF8.GetBytes($text)))).Replace('-', '').ToLowerInvariant()
    } finally {
        $algorithm.Dispose()
    }
}

$manifestPath = Join-Path $repoRoot 'licenses/notices-manifest.json'
$manifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $manifestPath | ConvertFrom-Json
if ($manifest.schemaVersion -ne 1 -or -not $manifest.files.Count) { throw '第三方通知清单为空或版本不受支持' }
$noticeRecords = [Collections.Generic.List[object]]::new()
foreach ($entry in $manifest.files) {
    $relative = [string]$entry.path
    $source = Resolve-NoticePath $repoRoot $relative
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "第三方通知缺失：$relative" }
    if ((Get-NoticeTextHash $source) -ne $entry.lfSha256) { throw "第三方通知与归档摘要不符：$relative" }
    $destination = Resolve-NoticePath $distributionRoot $relative
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
    Copy-Item -LiteralPath $source -Destination $destination -Force
    $writtenHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $destination).Hash.ToLowerInvariant()
    $sourceHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $source).Hash.ToLowerInvariant()
    if ($writtenHash -ne $sourceHash) { throw "第三方通知复制后摘要不符：$relative" }
    $noticeRecords.Add([ordered]@{ path = $relative; sha256 = $writtenHash; component = $entry.component; sourceVersion = $entry.sourceVersion })
}

# PDFium 的通知由固定上游归档解包，随现有打包流程复制，逐项纳入本次回执。
$pdfiumLicense = Join-Path $distributionRoot 'licenses/pdfium/LICENSE'
$pdfiumNotices = Join-Path $distributionRoot 'licenses/pdfium/licenses'
if (-not (Test-Path -LiteralPath $pdfiumLicense -PathType Leaf) -or -not (Test-Path -LiteralPath $pdfiumNotices -PathType Container)) {
    throw 'PDFium 原始许可目录未随程序复制'
}
foreach ($file in (Get-ChildItem -LiteralPath (Join-Path $distributionRoot 'licenses/pdfium') -Recurse -File | Sort-Object FullName)) {
    $relative = $file.FullName.Substring($distributionRoot.TrimEnd('\', '/').Length + 1).Replace('\', '/')
    $noticeRecords.Add([ordered]@{ path = $relative; sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $file.FullName).Hash.ToLowerInvariant(); component = 'PDFium'; sourceVersion = 'chromium/8100' })
}

$compilerVersion = (& $CompilerPath -dumpfullversion | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or -not $compilerVersion) { throw '无法读取实际 GCC 版本' }
$compilerTarget = (& $CompilerPath -dumpmachine | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or -not $compilerTarget) { throw '无法读取实际 GCC 目标平台' }
$runtimeRecords = foreach ($name in @('gangyiAI.exe', 'gangyiAI-launcher.exe', 'gangyiAI-source-reader.exe', 'libcurl-x64.dll', 'pdfium.dll', 'libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll')) {
    $path = Join-Path $distributionRoot $name
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "无法归档缺失的运行文件：$name" }
    [ordered]@{ path = $name; bytes = (Get-Item -LiteralPath $path).Length; sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash.ToLowerInvariant() }
}

$licenseDirectory = Join-Path $distributionRoot 'licenses'
Copy-Item -LiteralPath $manifestPath -Destination (Join-Path $licenseDirectory 'notices-manifest.json') -Force
$inventory = [ordered]@{
    schemaVersion = 1
    compiler = [ordered]@{ version = $compilerVersion; target = $compilerTarget }
    noticeArchiveBaseline = $manifest.toolchainArchiveBaseline
    noticeManifestSha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $manifestPath).Hash.ToLowerInvariant()
    runtimeFiles = @($runtimeRecords)
    noticeFiles = $noticeRecords.ToArray()
    finalDependencyReviewComplete = $false
}
$inventory | ConvertTo-Json -Depth 8 | Set-Content -Encoding UTF8 -LiteralPath (Join-Path $licenseDirectory 'dependency-inventory.json')
Write-Host "第三方通知已复制并核对：$($noticeRecords.Count) 份；实际 GCC $compilerVersion，目标 $compilerTarget。"
