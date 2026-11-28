# 【逐行批注】 声明脚本参数，控制本次构建或测试行为。
param([string]$RepoRoot = (Split-Path -Parent $PSScriptRoot), [string]$BuildDir = 'build-windows')
# 【逐行批注】 执行命令并将结果保存到 $ErrorActionPreference。
$ErrorActionPreference = 'Stop'
# 【逐行批注】 判断条件 ($env:GITHUB_ACTIONS -ne 'true') { throw '同安装标识升级验收只允许临时 GitHub …，成立时执行对应流程。
if ($env:GITHUB_ACTIONS -ne 'true') { throw '同安装标识升级验收只允许临时 GitHub runner，禁止在现用电脑执行。' }
# 【逐行批注】 执行命令并将结果保存到 $root。
$root = Join-Path $env:RUNNER_TEMP ('gangyi-upgrade-assets-' + [guid]::NewGuid().ToString('N'))
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Path $root | Out-Null。
New-Item -ItemType Directory -Path $root | Out-Null
# 【逐行批注】 开始可能失败的 PowerShell 操作。
try {
    # 下载公开旧包后先核对该版本发布的SHA256；禁止替换成正式版冒充试用版升级。
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：gh release download v5.6.3 --repo lwy988998/gangyiAI --pattern 'gangyiAI-learning-preview…。
    gh release download v5.6.3 --repo lwy988998/gangyiAI --pattern 'gangyiAI-learning-preview1-v5.6.3-x64.exe' --pattern 'SHA256SUMS-learning-preview.txt' --dir $root
    # 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw '下载旧版试用安装包失败' }，成立时执行对应流程。
    if ($LASTEXITCODE -ne 0) { throw '下载旧版试用安装包失败' }
    # 【逐行批注】 执行命令并将结果保存到 $name。
    $name = 'gangyiAI-learning-preview1-v5.6.3-x64.exe'
    # 【逐行批注】 执行命令并将结果保存到 $line。
    $line = @(Get-Content (Join-Path $root 'SHA256SUMS-learning-preview.txt') | Where-Object { ($_ -split '\s+',2)[1] -ceq "*$name" })
    # 【逐行批注】 判断条件 ($line.Count -ne 1) { throw '旧版校验清单缺少唯一试用安装包记录' }，成立时执行对应流程。
    if ($line.Count -ne 1) { throw '旧版校验清单缺少唯一试用安装包记录' }
    # 【逐行批注】 判断条件 ((Get-FileHash (Join-Path $root $name) -Algorithm SHA256).Hash.T…，成立时执行对应流程。
    if ((Get-FileHash (Join-Path $root $name) -Algorithm SHA256).Hash.ToLowerInvariant() -ne ($line[0] -split '\s+')[0]) { throw '旧版试用安装包校验失败' }
    # 【逐行批注】 执行命令并将结果保存到 $text。
    $text = Get-Content (Join-Path $RepoRoot 'CMakeLists.txt') -Raw
    # 【逐行批注】 判断条件 ($text -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw …，成立时执行对应流程。
    if ($text -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw '无法读取候选版本' }
    # 【逐行批注】 执行命令并将结果保存到 $version。
    $version = $Matches[1]
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：python -X utf8 (Join-Path $RepoRoot 'tests/windows_upgrade_fixture.py') `。
    python -X utf8 (Join-Path $RepoRoot 'tests/windows_upgrade_fixture.py') `
        --baseline (Join-Path $root $name) `
        --candidate (Join-Path $RepoRoot "dist/installer/gangyiAI-learning-preview1-v$version-x64.exe") `
        --backup-tool (Join-Path $RepoRoot "$BuildDir/launcher-support-tests.exe") --version $version
    # 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw '试用版升级、档案或备份恢复验证失败' }，成立时执行对应流程。
    if ($LASTEXITCODE -ne 0) { throw '试用版升级、档案或备份恢复验证失败' }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
} finally {
    # 【逐行批注】 执行命令并将结果保存到 $resolved。
    $resolved = [IO.Path]::GetFullPath($root)
    # 【逐行批注】 执行命令并将结果保存到 $allowed。
    $allowed = [IO.Path]::GetFullPath($env:RUNNER_TEMP).TrimEnd('\') + '\'
    # 【逐行批注】 判断条件 (-not $resolved.StartsWith($allowed, [StringComparison]::Ordinal…，成立时执行对应流程。
    if (-not $resolved.StartsWith($allowed, [StringComparison]::OrdinalIgnoreCase)) { throw '拒绝清理runner临时目录之外路径' }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Remove-Item -LiteralPath $resolved -Recurse -Force -ErrorAction SilentlyContinue。
    Remove-Item -LiteralPath $resolved -Recurse -Force -ErrorAction SilentlyContinue
# 【逐行批注】 开始或结束当前脚本代码块。
}
