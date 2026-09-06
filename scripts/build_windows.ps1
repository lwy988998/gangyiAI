param(
    [switch]$SkipDependencies,
    [switch]$SkipInstaller
)

$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$repoRoot = Split-Path -Parent $PSScriptRoot
$thirdParty = Join-Path $repoRoot 'third_party'
$buildDir = Join-Path $repoRoot 'build-windows'
$distDir = Join-Path $repoRoot 'dist\windows'

function Require-Command([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "缺少构建工具：$Name"
    }
}

function Install-Archive(
    [string]$Name,
    [string]$Url,
    [string]$Parent,
    [string]$ExpectedDirectory,
    [string]$Sha256 = ''
) {
    $destination = Join-Path $Parent $ExpectedDirectory
    if (Test-Path -LiteralPath $destination) {
        Write-Host "[依赖已存在] $Name"
        return
    }
    New-Item -ItemType Directory -Force -Path $Parent | Out-Null
    $temporary = Join-Path ([System.IO.Path]::GetTempPath()) ("gangyiAI-" + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $temporary | Out-Null
    try {
        $archive = Join-Path $temporary 'archive.zip'
        Write-Host "[下载] $Name"
        $downloaded = $false
        for ($attempt = 1; $attempt -le 3; $attempt++) {
            try {
                Invoke-WebRequest -Uri $Url -OutFile $archive
                $downloaded = $true
                break
            } catch {
                Remove-Item -LiteralPath $archive -Force -ErrorAction SilentlyContinue
                if ($attempt -eq 3) { throw }
                Start-Sleep -Seconds ([math]::Pow(2, $attempt))
                Write-Host "[重试] $Name（第 $($attempt + 1) 次）"
            }
        }
        if (-not $downloaded) { throw "$Name 下载失败" }
        if ($Sha256) {
            $actual = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
            if ($actual -ne $Sha256.ToLowerInvariant()) {
                throw "$Name 的 SHA256 校验失败"
            }
        }
        Expand-Archive -LiteralPath $archive -DestinationPath $Parent -Force
        if (-not (Test-Path -LiteralPath $destination)) {
            throw "$Name 解压后未找到预期目录：$ExpectedDirectory"
        }
    } finally {
        Remove-Item -LiteralPath $temporary -Recurse -Force -ErrorAction SilentlyContinue
    }
}

Require-Command 'cmake'
Require-Command 'g++'

if (-not $SkipDependencies) {
    Install-Archive 'Crow 1.2.0' `
        'https://github.com/CrowCpp/Crow/archive/refs/tags/v1.2.0.zip' `
        $thirdParty 'Crow-1.2.0' `
        'f76bacc050cdc8f253f4a0241650cdf1f58c208a89f47f7127de06dcbabe9211'
    Install-Archive 'nlohmann/json 3.11.3' `
        'https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.zip' `
        $thirdParty 'json-3.11.3' `
        '04022b05d806eb5ff73023c280b68697d12b93e1b7267a0b22a1a39ec7578069'
    Install-Archive 'Asio 1.28.2' `
        'https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-28-2.zip' `
        $thirdParty 'asio-asio-1-28-2' `
        '417536ed4d31645b550d6c475a952544d1aab30a04d7228e6ce6ca935bf2dc1b'
    Install-Archive 'SQLite 3.53.4' `
        'https://www.sqlite.org/2026/sqlite-amalgamation-3530400.zip' `
        (Join-Path $thirdParty 'sqlite') 'sqlite-amalgamation-3530400' `
        '1e71ddf93849c6a6ecf58b827c0692073d2dd7ee40196158068f7b29f422e87d'
    Install-Archive 'curl 8.22.0_1' `
        'https://curl.se/windows/dl-8.22.0_1/curl-8.22.0_1-win64-mingw.zip' `
        (Join-Path $thirdParty 'curl') 'curl-8.22.0_1-win64-mingw' `
        '7f23b039f6ea4197362d4468e1a0e71428201222e1bef3b680d5ef7b2aefb714'
}

cmake -S $repoRoot -B $buildDir -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw 'CMake 配置失败' }
cmake --build $buildDir --parallel
if ($LASTEXITCODE -ne 0) { throw 'C++ 编译失败' }
ctest --test-dir $buildDir --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'C++ 测试失败' }

$launcher = Join-Path $buildDir 'gangyiAI-launcher.exe'
$server = Join-Path $buildDir 'gangyiAI.exe'
$selfTest = Start-Process -FilePath $launcher -ArgumentList '--self-test' -Wait -PassThru
if ($selfTest.ExitCode -ne 0) { throw '启动器自检失败' }
$cmakeText = Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'CMakeLists.txt')
if ($cmakeText -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw '无法读取项目版本号' }
$version = $Matches[1]

if (Test-Path -LiteralPath $distDir) {
    $resolvedRepo = (Resolve-Path -LiteralPath $repoRoot).Path.TrimEnd('\') + '\'
    $resolvedDist = (Resolve-Path -LiteralPath $distDir).Path
    if (-not $resolvedDist.StartsWith($resolvedRepo, [StringComparison]::OrdinalIgnoreCase)) {
        throw "拒绝清理仓库之外的目录：$resolvedDist"
    }
    Remove-Item -LiteralPath $distDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $distDir | Out-Null
Copy-Item -LiteralPath $server, $launcher -Destination $distDir
Copy-Item -LiteralPath (Join-Path $buildDir 'libcurl-x64.dll') -Destination $distDir
$compiler = (Get-Command 'g++').Source
foreach ($runtime in @('libwinpthread-1.dll', 'libgcc_s_seh-1.dll', 'libstdc++-6.dll')) {
    $runtimePath = (& $compiler "-print-file-name=$runtime").Trim()
    if (-not [System.IO.Path]::IsPathRooted($runtimePath)) {
        $runtimePath = Join-Path (Split-Path -Parent $compiler) $runtime
    }
    if (Test-Path -LiteralPath $runtimePath) { Copy-Item -LiteralPath $runtimePath -Destination $distDir }
}
$caBundle = Join-Path $buildDir 'curl-ca-bundle.crt'
if (Test-Path -LiteralPath $caBundle) { Copy-Item -LiteralPath $caBundle -Destination $distDir }
Copy-Item -LiteralPath (Join-Path $repoRoot 'public') -Destination $distDir -Recurse
$requiredFiles = @(
    'gangyiAI.exe', 'gangyiAI-launcher.exe', 'libcurl-x64.dll',
    'libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll',
    'public\styles.css', 'public\school-logo.png'
)
foreach ($required in $requiredFiles) {
    if (-not (Test-Path -LiteralPath (Join-Path $distDir $required))) { throw "发布文件缺失：$required" }
}
foreach ($executable in @(
    @{ Path = $server; OriginalName = 'gangyiAI.exe' },
    @{ Path = $launcher; OriginalName = 'gangyiAI-launcher.exe' }
)) {
    $versionInfo = (Get-Item -LiteralPath $executable.Path).VersionInfo
    $productVersion = $versionInfo.ProductVersion.Trim()
    if ($productVersion -ne $version) { throw "程序版本不一致：$($executable.Path) ($productVersion != $version)" }
    if ($versionInfo.OriginalFilename -ne $executable.OriginalName) {
        throw "程序版本资源错误：$($executable.Path) 的原始文件名不是 $($executable.OriginalName)"
    }
}

$portableGuide = @"
钢一定制AI v$version 免安装版

1. 请完整解压 ZIP，不要直接在压缩包内运行。
2. 双击 gangyiAI-launcher.exe，填写 AI 服务商、API Key 和模型。
3. 配置、凭据、日志和课程数据库保存在 %LOCALAPPDATA%\GangyiAI。
4. 本版本无需 CMake、编译器、Visual Studio、Python 或 Node.js。
5. 当前程序尚未签名，Windows SmartScreen 可能提示未知发布者。
"@
Set-Content -LiteralPath (Join-Path $distDir '免安装版使用说明.txt') -Value $portableGuide -Encoding utf8
$portableName = "gangyiAI-portable-v$version-x64.zip"
$portablePath = Join-Path $repoRoot "dist\installer\$portableName"
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $portablePath) | Out-Null
if (Test-Path -LiteralPath $portablePath) { Remove-Item -LiteralPath $portablePath -Force }
Compress-Archive -Path (Join-Path $distDir '*') -DestinationPath $portablePath -CompressionLevel Optimal

if (-not $SkipInstaller) {
    $iscc = Get-Command 'ISCC.exe' -ErrorAction SilentlyContinue
    if (-not $iscc) {
        $isccCandidates = @(
            (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
            (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe')
        )
        $defaultIscc = $isccCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
        if ($defaultIscc) { $iscc = Get-Item -LiteralPath $defaultIscc }
    }
    if (-not $iscc) { throw '缺少 Inno Setup 6（ISCC.exe）' }
    $isccPath = if ($iscc -is [System.Management.Automation.CommandInfo]) { $iscc.Source } else { $iscc.FullName }
    & $isccPath "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss')
    if ($LASTEXITCODE -ne 0) { throw 'Inno Setup 打包失败' }

    $installerName = "gangyiAI-setup-v$version-x64.exe"
    $installerPath = Join-Path $repoRoot "dist\installer\$installerName"
    if (-not (Test-Path -LiteralPath $installerPath)) { throw "安装包未生成：$installerPath" }
}

$checksums = @()
if (-not $SkipInstaller) {
    $installerName = "gangyiAI-setup-v$version-x64.exe"
    $installerPath = Join-Path $repoRoot "dist\installer\$installerName"
    $checksums += "$(Get-FileHash -LiteralPath $installerPath -Algorithm SHA256 | Select-Object -ExpandProperty Hash) *$installerName"
}
$checksums += "$(Get-FileHash -LiteralPath $portablePath -Algorithm SHA256 | Select-Object -ExpandProperty Hash) *$portableName"
$checksums | ForEach-Object { $_.ToLowerInvariant() } |
    Set-Content -LiteralPath (Join-Path $repoRoot 'dist\installer\SHA256SUMS.txt') -Encoding ascii

Write-Host '[完成] Windows 发布产物已生成。'
