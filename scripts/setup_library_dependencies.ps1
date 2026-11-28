# 下载固定版本的正文读取组件，并校验上游摘要。
# 【逐行批注】 声明脚本参数，控制本次构建或测试行为。
param([string]$GhPath = 'gh')
# 【逐行批注】 执行命令并将结果保存到 $ErrorActionPreference。
$ErrorActionPreference = 'Stop'
# 【逐行批注】 判断条件 (-not (Get-Command $GhPath -ErrorAction SilentlyContinue)) { thr…，成立时执行对应流程。
if (-not (Get-Command $GhPath -ErrorAction SilentlyContinue)) { throw '缺少 GitHub CLI（gh），请先安装或传入 -GhPath' }
# 【逐行批注】 执行命令并将结果保存到 $libraryRoot。
$libraryRoot = Split-Path -Parent $PSScriptRoot
# 【逐行批注】 执行命令并将结果保存到 $readabilityPath。
$readabilityPath = Join-Path $libraryRoot 'public\vendor\readability'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Path $readabilityPath -Force | Out-Null。
New-Item -ItemType Directory -Path $readabilityPath -Force | Out-Null
# 【逐行批注】 逐个处理集合中的元素：foreach ($name in @('Readability.js', 'JSDOMParser.js', 'LICENSE.md')) {。
foreach ($name in @('Readability.js', 'JSDOMParser.js', 'LICENSE.md')) {
    # 【逐行批注】 执行命令并将结果保存到 $resource。
    $resource = & $GhPath api "repos/mozilla/readability/contents/$($name)?ref=04fd32f72b448c12b02ba6c40928b67e510bac49" | ConvertFrom-Json
    # 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw "无法读取固定版本的 $name" }，成立时执行对应流程。
    if ($LASTEXITCODE -ne 0) { throw "无法读取固定版本的 $name" }
    # 【逐行批注】 执行命令并将结果保存到 $bytes。
    $bytes = [Convert]::FromBase64String($resource.content)
    # 【逐行批注】 执行命令并将结果保存到 $prefix。
    $prefix = [Text.Encoding]::UTF8.GetBytes("blob $($bytes.Length)" + [char]0)
    # 【逐行批注】 执行命令并将结果保存到 $hash。
    $hash = [Security.Cryptography.SHA1]::Create()
    # 【逐行批注】 执行命令并将结果保存到 $actual。
    $actual = [Convert]::ToHexString($hash.ComputeHash($prefix + $bytes)).ToLowerInvariant()
    # 【逐行批注】 判断条件 ($actual -ne $resource.sha) { throw "$name 的Git内容摘要不一致" }，成立时执行对应流程。
    if ($actual -ne $resource.sha) { throw "$name 的Git内容摘要不一致" }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[IO.File]::WriteAllBytes((Join-Path $readabilityPath $name), $bytes)。
    [IO.File]::WriteAllBytes((Join-Path $readabilityPath $name), $bytes)
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 执行命令并将结果保存到 $pdfiumPath。
$pdfiumPath = Join-Path $libraryRoot 'third_party\pdfium-8100'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Path $pdfiumPath -Force | Out-Null。
New-Item -ItemType Directory -Path $pdfiumPath -Force | Out-Null
# 【逐行批注】 执行命令并将结果保存到 $archive。
$archive = Join-Path $pdfiumPath 'pdfium-win-x64.tgz'
# 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath $archive)) {，成立时执行对应流程。
if (-not (Test-Path -LiteralPath $archive)) {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：& $GhPath release download 'chromium/8100' --repo bblanchon/pdfium-binaries --pattern pdf…。
    & $GhPath release download 'chromium/8100' --repo bblanchon/pdfium-binaries --pattern pdfium-win-x64.tgz --dir $pdfiumPath
    # 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw 'PDFium下载失败' }，成立时执行对应流程。
    if ($LASTEXITCODE -ne 0) { throw 'PDFium下载失败' }
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 判断条件 ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToL…，成立时执行对应流程。
if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne '51621bcc1d9be9ea448f4b84b0516862b928e3968d3fca899af1a6d6e2848d6d') { throw 'PDFium摘要校验失败' }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：& tar -xzf $archive -C $pdfiumPath。
& tar -xzf $archive -C $pdfiumPath
# 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw 'PDFium解包失败' }，成立时执行对应流程。
if ($LASTEXITCODE -ne 0) { throw 'PDFium解包失败' }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Write-Output 'Readability 0.6.0 与 PDFium 8100 已校验'。
Write-Output 'Readability 0.6.0 与 PDFium 8100 已校验'
