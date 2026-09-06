param(
    [string]$RepoRoot = (Split-Path -Parent $PSScriptRoot),
    [switch]$AllowLocalUserData
)

$ErrorActionPreference = 'Stop'
$cmakeText = Get-Content -Raw -LiteralPath (Join-Path $RepoRoot 'CMakeLists.txt')
if ($cmakeText -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw '无法读取版本号' }
$version = $Matches[1]
if ($env:GITHUB_ACTIONS -ne 'true' -and -not $AllowLocalUserData) {
    throw '发布验收会安装和卸载程序，并清理当前用户配置。仅可在 GitHub Actions 中运行；确需在隔离账户运行时请显式传入 -AllowLocalUserData。'
}
$installer = Join-Path $RepoRoot "dist\installer\gangyiAI-setup-v$version-x64.exe"
$portable = Join-Path $RepoRoot "dist\installer\gangyiAI-portable-v$version-x64.zip"
$checksumFile = Join-Path $RepoRoot 'dist\installer\SHA256SUMS.txt'
foreach ($path in @($installer, $portable, $checksumFile)) {
    if (-not (Test-Path -LiteralPath $path)) { throw "验收文件缺失：$path" }
}
$checksums = Get-Content -LiteralPath $checksumFile
foreach ($artifact in @($installer, $portable)) {
    $name = Split-Path -Leaf $artifact
    $expectedLine = $checksums | Where-Object { ($_ -split ' ', 2)[1] -eq "*$name" }
    if (-not $expectedLine) { throw "SHA256SUMS.txt 缺少：$name" }
    $expected = ($expectedLine -split ' ')[0]
    $actual = (Get-FileHash -LiteralPath $artifact -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $expected) { throw "$name 的 SHA256 不匹配" }
}

$temporary = Join-Path ([System.IO.Path]::GetTempPath()) ("GangyiAI-CI-" + [guid]::NewGuid().ToString('N'))
$portableDir = Join-Path $temporary 'portable'
$installDir = Join-Path $temporary 'installed'
$dataDir = Join-Path $env:LOCALAPPDATA 'GangyiAI\data'
$marker = Join-Path $dataDir 'ci-preserve.marker'
New-Item -ItemType Directory -Force -Path $temporary, $portableDir, $dataDir | Out-Null
try {
    Expand-Archive -LiteralPath $portable -DestinationPath $portableDir
    $portableSelfTest = Start-Process (Join-Path $portableDir 'gangyiAI-launcher.exe') -ArgumentList '--self-test' -Wait -PassThru
    if ($portableSelfTest.ExitCode -ne 0) { throw '免安装版启动器自检失败' }

    $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, 0)
    $listener.Start()
    $port = ([System.Net.IPEndPoint]$listener.LocalEndpoint).Port
    $listener.Stop()
    $env:HOST = '127.0.0.1'
    $env:PORT = "$port"
    $env:LOCAL_CONTROL_TOKEN = 'ci-control-token'
    $env:DATABASE_PATH = Join-Path $temporary 'smoke.db'
    $env:AI_BASE_URL = 'http://127.0.0.1:9/v1'
    $env:AI_API_KEY = 'ci-placeholder-key'
    $env:AI_MODEL = 'ci-model'
    $service = Start-Process (Join-Path $portableDir 'gangyiAI.exe') -WorkingDirectory $portableDir -PassThru -WindowStyle Hidden
    try {
        $healthy = $false
        for ($attempt = 0; $attempt -lt 100; $attempt++) {
            try {
                $health = Invoke-WebRequest "http://127.0.0.1:$port/health" -UseBasicParsing -TimeoutSec 1
                if ($health.StatusCode -eq 200) { $healthy = $true; break }
            } catch {}
            Start-Sleep -Milliseconds 150
        }
        if (-not $healthy) { throw '免安装版服务健康检查失败' }
        $logo = Invoke-WebRequest "http://127.0.0.1:$port/school-logo.png" -UseBasicParsing
        if ($logo.Headers.'Content-Type' -ne 'image/png') { throw '校徽 MIME 类型错误' }
        Invoke-WebRequest "http://127.0.0.1:$port/internal/shutdown" -Method Post `
            -Headers @{'X-Gangyi-Control-Token'='ci-control-token'} -UseBasicParsing | Out-Null
        if (-not $service.WaitForExit(5000)) { throw '服务未能优雅退出' }
    } finally {
        if (-not $service.HasExited) { $service.Kill() }
    }

    Set-Content -LiteralPath $marker -Value 'preserve' -Encoding ascii
    $setupArgs = "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP- /NOICONS /DIR=`"$installDir`""
    $setup = Start-Process $installer -ArgumentList $setupArgs -Wait -PassThru
    if ($setup.ExitCode -ne 0) { throw "静默安装失败：$($setup.ExitCode)" }
    $installedSelfTest = Start-Process (Join-Path $installDir 'gangyiAI-launcher.exe') -ArgumentList '--self-test' -Wait -PassThru
    if ($installedSelfTest.ExitCode -ne 0) { throw '已安装启动器自检失败' }
    foreach ($name in @('gangyiAI.exe', 'gangyiAI-launcher.exe')) {
        $actualVersion = (Get-Item (Join-Path $installDir $name)).VersionInfo.ProductVersion.Trim()
        if ($actualVersion -ne $version) { throw "$name 版本不正确：$actualVersion" }
    }
    $uninstall = Start-Process (Join-Path $installDir 'unins000.exe') `
        -ArgumentList '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART' -Wait -PassThru
    for ($attempt = 0; $attempt -lt 20 -and (Test-Path $installDir); $attempt++) {
        Start-Sleep -Milliseconds 250
    }
    if ($uninstall.ExitCode -ne 0 -or (Test-Path $installDir)) { throw '静默卸载失败' }
    if (-not (Test-Path $marker)) { throw '卸载错误删除了用户课程数据' }
} finally {
    Remove-Item -LiteralPath $marker -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $temporary -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host '[通过] Windows 发布产物验收完成。'
