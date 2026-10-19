param([string]$RepoRoot = (Split-Path -Parent $PSScriptRoot), [string]$BuildDir = 'build-windows')
$ErrorActionPreference = 'Stop'
if ($env:GITHUB_ACTIONS -ne 'true') { throw '同安装标识升级验收只允许临时 GitHub runner，禁止在现用电脑执行。' }
$root = Join-Path $env:RUNNER_TEMP ('gangyi-upgrade-assets-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $root | Out-Null
try {
    # 下载公开旧包后先核对该版本发布的SHA256；禁止替换成正式版冒充试用版升级。
    gh release download v5.6.3 --repo lwy988998/gangyiAI --pattern 'gangyiAI-learning-preview1-v5.6.3-x64.exe' --pattern 'SHA256SUMS-learning-preview.txt' --dir $root
    if ($LASTEXITCODE -ne 0) { throw '下载旧版试用安装包失败' }
    $name = 'gangyiAI-learning-preview1-v5.6.3-x64.exe'
    $line = @(Get-Content (Join-Path $root 'SHA256SUMS-learning-preview.txt') | Where-Object { ($_ -split '\s+',2)[1] -ceq "*$name" })
    if ($line.Count -ne 1) { throw '旧版校验清单缺少唯一试用安装包记录' }
    if ((Get-FileHash (Join-Path $root $name) -Algorithm SHA256).Hash.ToLowerInvariant() -ne ($line[0] -split '\s+')[0]) { throw '旧版试用安装包校验失败' }
    $text = Get-Content (Join-Path $RepoRoot 'CMakeLists.txt') -Raw
    if ($text -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw '无法读取候选版本' }
    $version = $Matches[1]
    python -X utf8 (Join-Path $RepoRoot 'tests/windows_upgrade_fixture.py') `
        --baseline (Join-Path $root $name) `
        --candidate (Join-Path $RepoRoot "dist/installer/gangyiAI-learning-preview1-v$version-x64.exe") `
        --backup-tool (Join-Path $RepoRoot "$BuildDir/launcher-support-tests.exe") --version $version
    if ($LASTEXITCODE -ne 0) { throw '试用版升级、档案或备份恢复验证失败' }
} finally {
    $resolved = [IO.Path]::GetFullPath($root)
    $allowed = [IO.Path]::GetFullPath($env:RUNNER_TEMP).TrimEnd('\') + '\'
    if (-not $resolved.StartsWith($allowed, [StringComparison]::OrdinalIgnoreCase)) { throw '拒绝清理runner临时目录之外路径' }
    Remove-Item -LiteralPath $resolved -Recurse -Force -ErrorAction SilentlyContinue
}
