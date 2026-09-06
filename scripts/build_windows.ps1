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
        Invoke-WebRequest -Uri $Url -OutFile $archive
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
        $thirdParty 'Crow-1.2.0'
    Install-Archive 'nlohmann/json 3.11.3' `
        'https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.zip' `
        $thirdParty 'json-3.11.3'
    Install-Archive 'Asio 1.28.2' `
        'https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-28-2.zip' `
        $thirdParty 'asio-asio-1-28-2'
    Install-Archive 'SQLite 3.53.4' `
        'https://www.sqlite.org/2026/sqlite-amalgamation-3530400.zip' `
        (Join-Path $thirdParty 'sqlite') 'sqlite-amalgamation-3530400'
    Install-Archive 'curl 8.22.0_1' `
        'https://curl.se/windows/dl-8.22.0_1/curl-8.22.0_1-win64-mingw.zip' `
        (Join-Path $thirdParty 'curl') 'curl-8.22.0_1-win64-mingw' `
        '7f23b039f6ea4197362d4468e1a0e71428201222e1bef3b680d5ef7b2aefb714'
}

cmake -S $repoRoot -B $buildDir -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw 'CMake 配置失败' }
cmake --build $buildDir --parallel
if ($LASTEXITCODE -ne 0) { throw 'C++ 编译失败' }

$launcher = Join-Path $buildDir 'gangyiAI-launcher.exe'
$server = Join-Path $buildDir 'gangyiAI.exe'
$selfTest = Start-Process -FilePath $launcher -ArgumentList '--self-test' -Wait -PassThru
if ($selfTest.ExitCode -ne 0) { throw '启动器自检失败' }

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
    $cmakeText = Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'CMakeLists.txt')
    if ($cmakeText -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw '无法读取项目版本号' }
    $version = $Matches[1]
    & $isccPath "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss')
    if ($LASTEXITCODE -ne 0) { throw 'Inno Setup 打包失败' }
}

Write-Host '[完成] Windows 发布产物已生成。'
