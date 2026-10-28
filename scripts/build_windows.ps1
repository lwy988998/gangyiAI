param(
    [switch]$SkipDependencies,
    [switch]$Preview,
    [switch]$TeachingPreview,
    [switch]$Candidate,
    [switch]$SkipInstaller,
    [string]$BuildDir = '',
    [string]$Generator = 'MinGW Makefiles'
)

$ErrorActionPreference = 'Stop'
if ($Candidate -and -not $Preview) { throw '离线候选必须使用学习增强试用版标识：-Candidate -Preview' }
if ($Preview -and $TeachingPreview) { throw '一次只能选择一种试用版安装标识' }
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$repoRoot = Split-Path -Parent $PSScriptRoot
$thirdParty = Join-Path $repoRoot 'third_party'
$buildDir = if ($BuildDir) { $BuildDir } else { Join-Path $repoRoot 'build-windows' }
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
    Install-Archive 'Crow 1.3.0' `
        'https://github.com/CrowCpp/Crow/archive/refs/tags/v1.3.0.zip' `
        $thirdParty 'Crow-1.3.0' `
        'da99fcf439a3725c5bd48a4d6c04a7994163e1c711003ec5aa881c2e4156763e'
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

$cmakeArgs = @('-S', $repoRoot, '-B', $buildDir, '-G', $Generator, '-DCMAKE_BUILD_TYPE=Release')
$curlCa = Get-ChildItem -LiteralPath (Join-Path $thirdParty 'curl') -Filter 'curl-ca-bundle.crt' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
if ($curlCa) { $cmakeArgs += "-DCMAKE_TLS_CAINFO=$($curlCa.FullName.Replace('\', '/'))" }
if (-not (Test-Path -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\bin\pdfium.dll'))) { & (Join-Path $PSScriptRoot 'setup_library_dependencies.ps1') }
$myAIPreviewMode = if ($Preview) { "ON" } else { "OFF" }
$cmakeArgs += "-DGANGYI_PREVIEW_BUILD=$myAIPreviewMode"
$cmakeArgs += "-DGANGYI_OFFLINE_CANDIDATE=$(if ($Candidate) { 'ON' } else { 'OFF' })"
$cmakeArgs += "-DGANGYI_TEACHING_PREVIEW=$(if ($TeachingPreview) { 'ON' } else { 'OFF' })"
cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw 'CMake 配置失败' }
cmake --build $buildDir --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'C++ 编译失败' }
ctest --test-dir $buildDir --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'C++ 测试失败' }

$launcher = Join-Path $buildDir 'gangyiAI-launcher.exe'
$server = Join-Path $buildDir 'gangyiAI.exe'
$selfTest = Start-Process -FilePath $launcher -ArgumentList '--self-test' -Wait -PassThru -WindowStyle Hidden
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
Copy-Item -LiteralPath $server, $launcher, (Join-Path $buildDir 'gangyiAI-source-reader.exe') -Destination $distDir
Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\bin\pdfium.dll') -Destination $distDir
$licenseDist = Join-Path $distDir 'licenses\pdfium'
New-Item -ItemType Directory -Force -Path $licenseDist | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\LICENSE') -Destination $licenseDist
Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\licenses') -Destination $licenseDist -Recurse
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
$publicDist = Join-Path $distDir 'public'
New-Item -ItemType Directory -Force -Path $publicDist | Out-Null
foreach ($asset in @('clarity.css', 'clarity.js', 'deep-course.css', 'deep-course.js', 'library.css', 'library.js', 'my-ai.css', 'my-ai.js', 'my-ai-study.js', 'styles.css', 'plan.css', 'dark.css', 'dark.js', 'app-layout.css', 'app-layout.js', 'profile-radar.js', 'profile-radar.css', 'startup.css', 'startup.js', 'ask.js', 'classroom.js', 'classroom.css', 'learning-flow.js', 'learning-flow.css', 'learning-agent.js', 'agent-shell.js', 'agent-classroom.html', 'agent-classroom.js', 'agent-classroom.css', 'agent-prepare.html', 'agent-prepare.js', 'chat-render.js', 'user-center.js', 'school-logo.png', 'campus-background.jpg')) {
    Copy-Item -LiteralPath (Join-Path $repoRoot "public\$asset") -Destination $publicDist
}
# 公式渲染引擎随包发布：断网也能显示数学公式。
Copy-Item -LiteralPath (Join-Path $repoRoot 'public\vendor') -Destination $publicDist -Recurse -Force
$requiredFiles = @(
    'gangyiAI.exe', 'gangyiAI-launcher.exe', 'gangyiAI-source-reader.exe', 'pdfium.dll', 'public\library.js', 'public\library.css', 'libcurl-x64.dll',
    'libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll',
    'public\clarity.css', 'public\clarity.js', 'public\deep-course.css', 'public\deep-course.js',
    'public\my-ai.css', 'public\my-ai.js', 'public\my-ai-study.js', 'public\vendor\echarts\echarts.min.js', 'public\styles.css', 'public\dark.css', 'public\dark.js', 'public\startup.css', 'public\startup.js', 'public\ask.js',
    'public\app-layout.js', 'public\app-layout.css', 'public\profile-radar.js', 'public\profile-radar.css', 'public\classroom.js', 'public\classroom.css', 'public\learning-flow.js', 'public\learning-flow.css', 'public\learning-agent.js', 'public\agent-shell.js', 'public\agent-classroom.html', 'public\agent-classroom.js', 'public\agent-classroom.css', 'public\agent-prepare.html', 'public\agent-prepare.js', 'public\chat-render.js', 'public\user-center.js',
    'public\vendor\katex\katex.min.js', 'public\vendor\katex\katex.min.css',
    'public\vendor\katex\fonts\KaTeX_Main-Regular.woff2',
    'public\school-logo.png', 'public\campus-background.jpg'
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

$profileDirectoryName = if ($TeachingPreview) { "GangyiAI-Teaching-Preview" } elseif ($Preview) { "GangyiAI-Library-Preview" } else { "GangyiAI" }
$portableGuide = @"
钢一定制AI v$version 免安装版

1. 请完整解压 ZIP，不要直接在压缩包内运行。
2. 双击 gangyiAI-launcher.exe，可先点击“暂不配置，直接使用”；需要 AI 功能时再配置服务商、API Key 和模型。
3. 配置、凭据、日志和课程数据库保存在 %LOCALAPPDATA%\$profileDirectoryName。
4. 本版本无需 CMake、编译器、Visual Studio、Python 或 Node.js。
5. 独立窗口界面需要 Microsoft Edge WebView2 Runtime；若提示缺失，请联网安装后重试。
6. 当前程序尚未签名，Windows SmartScreen 可能提示未知发布者。
"@
Set-Content -LiteralPath (Join-Path $distDir '免安装版使用说明.txt') -Value $portableGuide -Encoding utf8
if ($Candidate) {
    $candidateGuide = @"
钢一定制AI v$version 离线候选（非正式发布）

本包使用现有学习增强试用版标识 GangyiAI-Library-Preview。
升级前请备份程序目录和 %LOCALAPPDATA%\GangyiAI-Library-Preview 中的学习档案与设置，并确认凭据目标仍存在。
本包不会自动安装；本机是否升级、真实验收用量与结果，以同目录版本验收说明为准。
自动化安装、升级、卸载及备份恢复使用临时 Windows runner 的虚构数据；以该版本实际 CI 结果为准。
必要真实教学配对尚未全部通过，不宣称教学质量已验收或整体节省比例。
开发验收驱动、凭据、预算账本和学习数据不随包分发。使用 AI 功能仍按原设置发起正常请求。
"@
    Set-Content -LiteralPath (Join-Path $distDir '离线候选说明.txt') -Value $candidateGuide -Encoding utf8
    Set-Content -LiteralPath (Join-Path $distDir '免安装版使用说明.txt') -Value $candidateGuide -Encoding utf8
    @{ version=$version; channel='offline-candidate'; profileName='GangyiAI-Library-Preview'; realModelEvaluationComplete=$false; acceptanceReport="v$version-candidate.md" } |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $distDir 'candidate-metadata.json') -Encoding utf8
}
# 打包前只检查本次新建的发布目录，拒绝数据库、开发入口、验证证据与私有配置。
$privateFiles = Get-ChildItem -LiteralPath $distDir -Recurse -File | Where-Object {
    $_.Name -match '\.(db|sqlite|sqlite3)(-(wal|shm))?$|\.log$|^\.env|^settings\.json$|^home-recommendations\.json' -or
    $_.FullName -match '[\\/](verification|\.superpowers|preview-data|backups|tests|scripts)[\\/]'
}
if ($privateFiles) { throw '候选目录含禁止打包的数据或开发材料' }
$portableName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.zip" } elseif ($Preview) { "gangyiAI-learning-preview1-v$version-x64.zip" } else { "gangyiAI-portable-v$version-x64.zip" }
$portablePath = Join-Path $repoRoot "dist\installer\$portableName"
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $portablePath) | Out-Null
if ($Candidate) {
    Copy-Item -LiteralPath (Join-Path $repoRoot "docs/releases/v$version-candidate.md") -Destination (Join-Path $distDir "v$version-candidate.md") -Force
    Copy-Item -LiteralPath (Join-Path $repoRoot "docs/releases/v$version-candidate.md") -Destination (Join-Path $repoRoot 'dist/installer/离线候选说明.md') -Force
}
if (Test-Path -LiteralPath $portablePath) { Remove-Item -LiteralPath $portablePath -Force }
Compress-Archive -Path (Join-Path $distDir '*') -DestinationPath $portablePath -CompressionLevel Optimal

if (-not $SkipInstaller) {
    $bootstrapper = Join-Path $buildDir 'MicrosoftEdgeWebView2Setup.exe'
    if (-not (Test-Path -LiteralPath $bootstrapper)) {
        Invoke-WebRequest -Uri 'https://go.microsoft.com/fwlink/p/?LinkId=2124703' -OutFile $bootstrapper -UseBasicParsing
    }
    $signature = Get-AuthenticodeSignature -LiteralPath $bootstrapper
    if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'O=Microsoft Corporation') {
        throw 'WebView2 安装引导程序未通过微软签名校验'
    }
    Copy-Item -LiteralPath $bootstrapper -Destination (Join-Path $repoRoot 'dist\installer\MicrosoftEdgeWebView2Setup.exe') -Force
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
    if ($TeachingPreview) { & $isccPath "/DTeachingPreview=1" "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss') }
    elseif ($Preview) { & $isccPath "/DMyAIPreview=1" "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss') }
    else { & $isccPath "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss') }
    if ($LASTEXITCODE -ne 0) { throw 'Inno Setup 打包失败' }

    $installerName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.exe" } elseif ($Preview) { "gangyiAI-learning-preview1-v$version-x64.exe" } else { "gangyiAI-setup-v$version-x64.exe" }
    $installerPath = Join-Path $repoRoot "dist\installer\$installerName"
    if (-not (Test-Path -LiteralPath $installerPath)) { throw "安装包未生成：$installerPath" }
}

$checksums = @()
if (-not $SkipInstaller) {
    $installerName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.exe" } elseif ($Preview) { "gangyiAI-learning-preview1-v$version-x64.exe" } else { "gangyiAI-setup-v$version-x64.exe" }
    $installerPath = Join-Path $repoRoot "dist\installer\$installerName"
    $installerHash = (Get-FileHash -LiteralPath $installerPath -Algorithm SHA256).Hash.ToLowerInvariant()
    $checksums += "$installerHash *$installerName"
}
$portableHash = (Get-FileHash -LiteralPath $portablePath -Algorithm SHA256).Hash.ToLowerInvariant()
$checksums += "$portableHash *$portableName"
$checksums | Set-Content -LiteralPath (Join-Path $repoRoot 'dist\installer\SHA256SUMS.txt') -Encoding ascii

Write-Host '[完成] Windows 发布产物已生成。'
