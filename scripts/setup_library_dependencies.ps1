# 下载固定版本的正文读取组件，并校验上游摘要。
param([string]$GhPath = 'D:\OpenClaw\tools\gh\bin\gh.exe')
$ErrorActionPreference = 'Stop'
$libraryRoot = Split-Path -Parent $PSScriptRoot
$readabilityPath = Join-Path $libraryRoot 'public\vendor\readability'
New-Item -ItemType Directory -Path $readabilityPath -Force | Out-Null
foreach ($name in @('Readability.js', 'JSDOMParser.js', 'LICENSE.md')) {
    $resource = & $GhPath api "repos/mozilla/readability/contents/$($name)?ref=04fd32f72b448c12b02ba6c40928b67e510bac49" | ConvertFrom-Json
    if ($LASTEXITCODE -ne 0) { throw "无法读取固定版本的 $name" }
    $bytes = [Convert]::FromBase64String($resource.content)
    $prefix = [Text.Encoding]::UTF8.GetBytes("blob $($bytes.Length)" + [char]0)
    $hash = [Security.Cryptography.SHA1]::Create()
    $actual = [Convert]::ToHexString($hash.ComputeHash($prefix + $bytes)).ToLowerInvariant()
    if ($actual -ne $resource.sha) { throw "$name 的Git内容摘要不一致" }
    [IO.File]::WriteAllBytes((Join-Path $readabilityPath $name), $bytes)
}
$pdfiumPath = Join-Path $libraryRoot 'third_party\pdfium-8100'
New-Item -ItemType Directory -Path $pdfiumPath -Force | Out-Null
$archive = Join-Path $pdfiumPath 'pdfium-win-x64.tgz'
if (-not (Test-Path -LiteralPath $archive)) {
    & $GhPath release download 'chromium/8100' --repo bblanchon/pdfium-binaries --pattern pdfium-win-x64.tgz --dir $pdfiumPath
    if ($LASTEXITCODE -ne 0) { throw 'PDFium下载失败' }
}
if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne '51621bcc1d9be9ea448f4b84b0516862b928e3968d3fca899af1a6d6e2848d6d') { throw 'PDFium摘要校验失败' }
& tar -xzf $archive -C $pdfiumPath
if ($LASTEXITCODE -ne 0) { throw 'PDFium解包失败' }
Write-Output 'Readability 0.6.0 与 PDFium 8100 已校验'
