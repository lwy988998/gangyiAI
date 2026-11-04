# 【逐行批注】 声明脚本参数，控制本次构建或测试行为。
param(
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$SkipDependencies,。
    [switch]$SkipDependencies,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$Preview,。
    [switch]$Preview,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$TeachingPreview,。
    [switch]$TeachingPreview,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$Candidate,。
    [switch]$Candidate,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$SkipInstaller,。
    [switch]$SkipInstaller,
    # 【逐行批注】 执行命令并将结果保存到 [string]$BuildDir。
    [string]$BuildDir = '',
    # 【逐行批注】 执行命令并将结果保存到 [string]$Generator。
    [string]$Generator = 'MinGW Makefiles'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：)。
)

# 【逐行批注】 执行命令并将结果保存到 $ErrorActionPreference。
$ErrorActionPreference = 'Stop'
# 【逐行批注】 判断条件 ($Candidate -and -not $Preview) { throw '离线候选必须使用学习增强试用版标识：-Cand…，成立时执行对应流程。
if ($Candidate -and -not $Preview) { throw '离线候选必须使用学习增强试用版标识：-Candidate -Preview' }
# 【逐行批注】 判断条件 ($Preview -and $TeachingPreview) { throw '一次只能选择一种试用版安装标识' }，成立时执行对应流程。
if ($Preview -and $TeachingPreview) { throw '一次只能选择一种试用版安装标识' }
# 【逐行批注】 执行命令并将结果保存到 [Net.ServicePointManager]::SecurityProtocol。
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
# 【逐行批注】 执行命令并将结果保存到 $repoRoot。
$repoRoot = Split-Path -Parent $PSScriptRoot
# 【逐行批注】 执行命令并将结果保存到 $thirdParty。
$thirdParty = Join-Path $repoRoot 'third_party'
# 【逐行批注】 执行命令并将结果保存到 $buildDir。
$buildDir = if ($BuildDir) { $BuildDir } else { Join-Path $repoRoot 'build-windows' }
# 【逐行批注】 执行命令并将结果保存到 $distDir。
$distDir = Join-Path $repoRoot 'dist\windows'

# 【逐行批注】 定义 PowerShell 函数 Require-Command，封装可复用流程。
function Require-Command([string]$Name) {
    # 【逐行批注】 判断条件 (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {，成立时执行对应流程。
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        # 【逐行批注】 在错误情形终止流程并报告原因：throw "缺少构建工具：$Name"。
        throw "缺少构建工具：$Name"
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 定义 PowerShell 函数 Install-Archive，封装可复用流程。
function Install-Archive(
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[string]$Name,。
    [string]$Name,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[string]$Url,。
    [string]$Url,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[string]$Parent,。
    [string]$Parent,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[string]$ExpectedDirectory,。
    [string]$ExpectedDirectory,
    # 【逐行批注】 执行命令并将结果保存到 [string]$Sha256。
    [string]$Sha256 = ''
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：) {。
) {
    # 【逐行批注】 执行命令并将结果保存到 $destination。
    $destination = Join-Path $Parent $ExpectedDirectory
    # 【逐行批注】 判断条件 (Test-Path -LiteralPath $destination) {，成立时执行对应流程。
    if (Test-Path -LiteralPath $destination) {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Write-Host "[依赖已存在] $Name"。
        Write-Host "[依赖已存在] $Name"
        # 【逐行批注】 返回函数结果并结束当前流程：return。
        return
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Force -Path $Parent | Out-Null。
    New-Item -ItemType Directory -Force -Path $Parent | Out-Null
    # 【逐行批注】 执行命令并将结果保存到 $temporary。
    $temporary = Join-Path ([System.IO.Path]::GetTempPath()) ("gangyiAI-" + [guid]::NewGuid().ToString('N'))
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Path $temporary | Out-Null。
    New-Item -ItemType Directory -Path $temporary | Out-Null
    # 【逐行批注】 开始可能失败的 PowerShell 操作。
    try {
        # 【逐行批注】 执行命令并将结果保存到 $archive。
        $archive = Join-Path $temporary 'archive.zip'
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Write-Host "[下载] $Name"。
        Write-Host "[下载] $Name"
        # 【逐行批注】 执行命令并将结果保存到 $downloaded。
        $downloaded = $false
        # 【逐行批注】 逐个处理集合中的元素：for ($attempt = 1; $attempt -le 3; $attempt++) {。
        for ($attempt = 1; $attempt -le 3; $attempt++) {
            # 【逐行批注】 开始可能失败的 PowerShell 操作。
            try {
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Invoke-WebRequest -Uri $Url -OutFile $archive。
                Invoke-WebRequest -Uri $Url -OutFile $archive
                # 【逐行批注】 执行命令并将结果保存到 $downloaded。
                $downloaded = $true
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：break。
                break
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} catch {。
            } catch {
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Remove-Item -LiteralPath $archive -Force -ErrorAction SilentlyContinue。
                Remove-Item -LiteralPath $archive -Force -ErrorAction SilentlyContinue
                # 【逐行批注】 判断条件 ($attempt -eq 3) { throw }，成立时执行对应流程。
                if ($attempt -eq 3) { throw }
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Start-Sleep -Seconds ([math]::Pow(2, $attempt))。
                Start-Sleep -Seconds ([math]::Pow(2, $attempt))
                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Write-Host "[重试] $Name（第 $($attempt + 1) 次）"。
                Write-Host "[重试] $Name（第 $($attempt + 1) 次）"
            # 【逐行批注】 开始或结束当前脚本代码块。
            }
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 判断条件 (-not $downloaded) { throw "$Name 下载失败" }，成立时执行对应流程。
        if (-not $downloaded) { throw "$Name 下载失败" }
        # 【逐行批注】 判断条件 ($Sha256) {，成立时执行对应流程。
        if ($Sha256) {
            # 【逐行批注】 执行命令并将结果保存到 $actual。
            $actual = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
            # 【逐行批注】 判断条件 ($actual -ne $Sha256.ToLowerInvariant()) {，成立时执行对应流程。
            if ($actual -ne $Sha256.ToLowerInvariant()) {
                # 【逐行批注】 在错误情形终止流程并报告原因：throw "$Name 的 SHA256 校验失败"。
                throw "$Name 的 SHA256 校验失败"
            # 【逐行批注】 开始或结束当前脚本代码块。
            }
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Expand-Archive -LiteralPath $archive -DestinationPath $Parent -Force。
        Expand-Archive -LiteralPath $archive -DestinationPath $Parent -Force
        # 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath $destination)) {，成立时执行对应流程。
        if (-not (Test-Path -LiteralPath $destination)) {
            # 【逐行批注】 在错误情形终止流程并报告原因：throw "$Name 解压后未找到预期目录：$ExpectedDirectory"。
            throw "$Name 解压后未找到预期目录：$ExpectedDirectory"
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
    } finally {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Remove-Item -LiteralPath $temporary -Recurse -Force -ErrorAction SilentlyContinue。
        Remove-Item -LiteralPath $temporary -Recurse -Force -ErrorAction SilentlyContinue
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Require-Command 'cmake'。
Require-Command 'cmake'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Require-Command 'g++'。
Require-Command 'g++'

# 【逐行批注】 判断条件 (-not $SkipDependencies) {，成立时执行对应流程。
if (-not $SkipDependencies) {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Install-Archive 'Crow 1.3.0' `。
    Install-Archive 'Crow 1.3.0' `
        'https://github.com/CrowCpp/Crow/archive/refs/tags/v1.3.0.zip' `
        $thirdParty 'Crow-1.3.0' `
        'da99fcf439a3725c5bd48a4d6c04a7994163e1c711003ec5aa881c2e4156763e'
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Install-Archive 'nlohmann/json 3.11.3' `。
    Install-Archive 'nlohmann/json 3.11.3' `
        'https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.zip' `
        $thirdParty 'json-3.11.3' `
        '04022b05d806eb5ff73023c280b68697d12b93e1b7267a0b22a1a39ec7578069'
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Install-Archive 'Asio 1.28.2' `。
    Install-Archive 'Asio 1.28.2' `
        'https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-28-2.zip' `
        $thirdParty 'asio-asio-1-28-2' `
        '417536ed4d31645b550d6c475a952544d1aab30a04d7228e6ce6ca935bf2dc1b'
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Install-Archive 'SQLite 3.53.4' `。
    Install-Archive 'SQLite 3.53.4' `
        'https://www.sqlite.org/2026/sqlite-amalgamation-3530400.zip' `
        (Join-Path $thirdParty 'sqlite') 'sqlite-amalgamation-3530400' `
        '1e71ddf93849c6a6ecf58b827c0692073d2dd7ee40196158068f7b29f422e87d'
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Install-Archive 'curl 8.22.0_1' `。
    Install-Archive 'curl 8.22.0_1' `
        'https://curl.se/windows/dl-8.22.0_1/curl-8.22.0_1-win64-mingw.zip' `
        (Join-Path $thirdParty 'curl') 'curl-8.22.0_1-win64-mingw' `
        '7f23b039f6ea4197362d4468e1a0e71428201222e1bef3b680d5ef7b2aefb714'
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 执行命令并将结果保存到 $cmakeArgs。
$cmakeArgs = @('-S', $repoRoot, '-B', $buildDir, '-G', $Generator, '-DCMAKE_BUILD_TYPE=Release')
# 【逐行批注】 执行命令并将结果保存到 $curlCa。
$curlCa = Get-ChildItem -LiteralPath (Join-Path $thirdParty 'curl') -Filter 'curl-ca-bundle.crt' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
# 【逐行批注】 判断条件 ($curlCa) { $cmakeArgs += "-DCMAKE_TLS_CAINFO=$($curlCa.FullName…，成立时执行对应流程。
if ($curlCa) { $cmakeArgs += "-DCMAKE_TLS_CAINFO=$($curlCa.FullName.Replace('\', '/'))" }
# 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath (Join-Path $repoRoot 'third_party\…，成立时执行对应流程。
if (-not (Test-Path -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\bin\pdfium.dll'))) { & (Join-Path $PSScriptRoot 'setup_library_dependencies.ps1') }
# 【逐行批注】 执行命令并将结果保存到 $myAIPreviewMode。
$myAIPreviewMode = if ($Preview) { "ON" } else { "OFF" }
# 【逐行批注】 执行命令并将结果保存到 $cmakeArgs +。
$cmakeArgs += "-DGANGYI_PREVIEW_BUILD=$myAIPreviewMode"
# 【逐行批注】 执行命令并将结果保存到 $cmakeArgs +。
$cmakeArgs += "-DGANGYI_OFFLINE_CANDIDATE=$(if ($Candidate) { 'ON' } else { 'OFF' })"
# 【逐行批注】 执行命令并将结果保存到 $cmakeArgs +。
$cmakeArgs += "-DGANGYI_TEACHING_PREVIEW=$(if ($TeachingPreview) { 'ON' } else { 'OFF' })"
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：cmake @cmakeArgs。
cmake @cmakeArgs
# 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw 'CMake 配置失败' }，成立时执行对应流程。
if ($LASTEXITCODE -ne 0) { throw 'CMake 配置失败' }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：cmake --build $buildDir --parallel 4。
cmake --build $buildDir --parallel 4
# 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw 'C++ 编译失败' }，成立时执行对应流程。
if ($LASTEXITCODE -ne 0) { throw 'C++ 编译失败' }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：ctest --test-dir $buildDir --output-on-failure。
ctest --test-dir $buildDir --output-on-failure
# 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw 'C++ 测试失败' }，成立时执行对应流程。
if ($LASTEXITCODE -ne 0) { throw 'C++ 测试失败' }

# 【逐行批注】 执行命令并将结果保存到 $launcher。
$launcher = Join-Path $buildDir 'gangyiAI-launcher.exe'
# 【逐行批注】 执行命令并将结果保存到 $server。
$server = Join-Path $buildDir 'gangyiAI.exe'
# 【逐行批注】 执行命令并将结果保存到 $selfTest。
$selfTest = Start-Process -FilePath $launcher -ArgumentList '--self-test' -Wait -PassThru -WindowStyle Hidden
# 【逐行批注】 判断条件 ($selfTest.ExitCode -ne 0) { throw '启动器自检失败' }，成立时执行对应流程。
if ($selfTest.ExitCode -ne 0) { throw '启动器自检失败' }
# 【逐行批注】 执行命令并将结果保存到 $cmakeText。
$cmakeText = Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'CMakeLists.txt')
# 【逐行批注】 判断条件 ($cmakeText -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { t…，成立时执行对应流程。
if ($cmakeText -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw '无法读取项目版本号' }
# 【逐行批注】 执行命令并将结果保存到 $version。
$version = $Matches[1]

# 【逐行批注】 判断条件 (Test-Path -LiteralPath $distDir) {，成立时执行对应流程。
if (Test-Path -LiteralPath $distDir) {
    # 【逐行批注】 执行命令并将结果保存到 $resolvedRepo。
    $resolvedRepo = (Resolve-Path -LiteralPath $repoRoot).Path.TrimEnd('\') + '\'
    # 【逐行批注】 执行命令并将结果保存到 $resolvedDist。
    $resolvedDist = (Resolve-Path -LiteralPath $distDir).Path
    # 【逐行批注】 判断条件 (-not $resolvedDist.StartsWith($resolvedRepo, [StringComparison]…，成立时执行对应流程。
    if (-not $resolvedDist.StartsWith($resolvedRepo, [StringComparison]::OrdinalIgnoreCase)) {
        # 【逐行批注】 在错误情形终止流程并报告原因：throw "拒绝清理仓库之外的目录：$resolvedDist"。
        throw "拒绝清理仓库之外的目录：$resolvedDist"
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Remove-Item -LiteralPath $distDir -Recurse -Force。
    Remove-Item -LiteralPath $distDir -Recurse -Force
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Force -Path $distDir | Out-Null。
New-Item -ItemType Directory -Force -Path $distDir | Out-Null
# 【逐行批注】 将安装说明复制进发行目录，供安装包和免安装包随附。
Copy-Item -LiteralPath (Join-Path $repoRoot 'installer\安装说明.txt') -Destination (Join-Path $distDir '安装说明.txt') -Force
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath $server, $launcher, (Join-Path $buildDir 'gangyiAI-source-reader.e…。
Copy-Item -LiteralPath $server, $launcher, (Join-Path $buildDir 'gangyiAI-source-reader.exe') -Destination $distDir
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\bin\pdfium.dll') -De…。
Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\bin\pdfium.dll') -Destination $distDir
# 【逐行批注】 执行命令并将结果保存到 $licenseDist。
$licenseDist = Join-Path $distDir 'licenses\pdfium'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Force -Path $licenseDist | Out-Null。
New-Item -ItemType Directory -Force -Path $licenseDist | Out-Null
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\LICENSE') -Destinati…。
Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\LICENSE') -Destination $licenseDist
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\licenses') -Destinat…。
Copy-Item -LiteralPath (Join-Path $repoRoot 'third_party\pdfium-8100\licenses') -Destination $licenseDist -Recurse
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $buildDir 'libcurl-x64.dll') -Destination $distDir。
Copy-Item -LiteralPath (Join-Path $buildDir 'libcurl-x64.dll') -Destination $distDir
# 【逐行批注】 执行命令并将结果保存到 $compiler。
$compiler = (Get-Command 'g++').Source
# 【逐行批注】 逐个处理集合中的元素：foreach ($runtime in @('libwinpthread-1.dll', 'libgcc_s_seh-1.dll', 'libstdc++-…。
foreach ($runtime in @('libwinpthread-1.dll', 'libgcc_s_seh-1.dll', 'libstdc++-6.dll')) {
    # 【逐行批注】 执行命令并将结果保存到 $runtimePath。
    $runtimePath = (& $compiler "-print-file-name=$runtime").Trim()
    # 【逐行批注】 判断条件 (-not [System.IO.Path]::IsPathRooted($runtimePath)) {，成立时执行对应流程。
    if (-not [System.IO.Path]::IsPathRooted($runtimePath)) {
        # 【逐行批注】 执行命令并将结果保存到 $runtimePath。
        $runtimePath = Join-Path (Split-Path -Parent $compiler) $runtime
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 判断条件 (Test-Path -LiteralPath $runtimePath) { Copy-Item -LiteralPath $…，成立时执行对应流程。
    if (Test-Path -LiteralPath $runtimePath) { Copy-Item -LiteralPath $runtimePath -Destination $distDir }
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 执行命令并将结果保存到 $caBundle。
$caBundle = Join-Path $buildDir 'curl-ca-bundle.crt'
# 【逐行批注】 判断条件 (Test-Path -LiteralPath $caBundle) { Copy-Item -LiteralPath $caB…，成立时执行对应流程。
if (Test-Path -LiteralPath $caBundle) { Copy-Item -LiteralPath $caBundle -Destination $distDir }
# 【逐行批注】 执行命令并将结果保存到 $publicDist。
$publicDist = Join-Path $distDir 'public'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Force -Path $publicDist | Out-Null。
New-Item -ItemType Directory -Force -Path $publicDist | Out-Null
# 【逐行批注】 逐个处理集合中的元素：foreach ($asset in @('clarity.css', 'clarity.js', 'deep-course.css', 'deep-cour…。
foreach ($asset in @('clarity.css', 'clarity.js', 'deep-course.css', 'deep-course.js', 'library.css', 'library.js', 'my-ai.css', 'my-ai.js', 'my-ai-study.js', 'styles.css', 'plan.css', 'dark.css', 'dark.js', 'app-layout.css', 'app-layout.js', 'profile-radar.js', 'profile-radar.css', 'startup.css', 'startup.js', 'ask.js', 'classroom.js', 'classroom.css', 'learning-flow.js', 'learning-flow.css', 'learning-agent.js', 'agent-trace.css', 'agent-trace.js', 'agent-shell.js', 'agent-classroom.html', 'agent-classroom.js', 'agent-classroom.css', 'agent-prepare.html', 'agent-prepare.js', 'chat-render.js', 'user-center.js', 'school-logo.png', 'campus-background.jpg')) {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $repoRoot "public\$asset") -Destination $publicDist。
    Copy-Item -LiteralPath (Join-Path $repoRoot "public\$asset") -Destination $publicDist
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 公式渲染引擎随包发布：断网也能显示数学公式。
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $repoRoot 'public\vendor') -Destination $publicDist -Re…。
Copy-Item -LiteralPath (Join-Path $repoRoot 'public\vendor') -Destination $publicDist -Recurse -Force
# 【逐行批注】 执行命令并将结果保存到 $requiredFiles。
$requiredFiles = @(
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'gangyiAI.exe', 'gangyiAI-launcher.exe', 'gangyiAI-source-reader.exe', 'pdfium.dll', 'pub…。
    'gangyiAI.exe', 'gangyiAI-launcher.exe', 'gangyiAI-source-reader.exe', 'pdfium.dll', 'public\library.js', 'public\library.css', 'libcurl-x64.dll',
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll',。
    'libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll',
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'public\clarity.css', 'public\clarity.js', 'public\deep-course.css', 'public\deep-course.…。
    'public\clarity.css', 'public\clarity.js', 'public\deep-course.css', 'public\deep-course.js',
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'public\my-ai.css', 'public\my-ai.js', 'public\my-ai-study.js', 'public\vendor\echarts\ec…。
    'public\my-ai.css', 'public\my-ai.js', 'public\my-ai-study.js', 'public\vendor\echarts\echarts.min.js', 'public\styles.css', 'public\dark.css', 'public\dark.js', 'public\startup.css', 'public\startup.js', 'public\ask.js',
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'public\app-layout.js', 'public\app-layout.css', 'public\profile-radar.js', 'public\profi…。
    'public\app-layout.js', 'public\app-layout.css', 'public\profile-radar.js', 'public\profile-radar.css', 'public\classroom.js', 'public\classroom.css', 'public\learning-flow.js', 'public\learning-flow.css', 'public\learning-agent.js', 'public\agent-shell.js', 'public\agent-classroom.html', 'public\agent-classroom.js', 'public\agent-classroom.css', 'public\agent-prepare.html', 'public\agent-prepare.js', 'public\chat-render.js', 'public\user-center.js',
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'public\vendor\katex\katex.min.js', 'public\vendor\katex\katex.min.css',。
    'public\vendor\katex\katex.min.js', 'public\vendor\katex\katex.min.css',
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'public\vendor\katex\fonts\KaTeX_Main-Regular.woff2',。
    'public\vendor\katex\fonts\KaTeX_Main-Regular.woff2',
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'public\school-logo.png', 'public\campus-background.jpg'。
    'public\school-logo.png', 'public\campus-background.jpg'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：)。
)
# 【逐行批注】 逐个处理集合中的元素：foreach ($required in $requiredFiles) {。
foreach ($required in $requiredFiles) {
    # 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath (Join-Path $distDir $required))) {…，成立时执行对应流程。
    if (-not (Test-Path -LiteralPath (Join-Path $distDir $required))) { throw "发布文件缺失：$required" }
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 逐个处理集合中的元素：foreach ($executable in @(。
foreach ($executable in @(
    # 【逐行批注】 执行命令并将结果保存到 @{ Path。
    @{ Path = $server; OriginalName = 'gangyiAI.exe' },
    # 【逐行批注】 执行命令并将结果保存到 @{ Path。
    @{ Path = $launcher; OriginalName = 'gangyiAI-launcher.exe' }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：)) {。
)) {
    # 【逐行批注】 执行命令并将结果保存到 $versionInfo。
    $versionInfo = (Get-Item -LiteralPath $executable.Path).VersionInfo
    # 【逐行批注】 执行命令并将结果保存到 $productVersion。
    $productVersion = $versionInfo.ProductVersion.Trim()
    # 【逐行批注】 判断条件 ($productVersion -ne $version) { throw "程序版本不一致：$($executable.Pa…，成立时执行对应流程。
    if ($productVersion -ne $version) { throw "程序版本不一致：$($executable.Path) ($productVersion != $version)" }
    # 【逐行批注】 判断条件 ($versionInfo.OriginalFilename -ne $executable.OriginalName) {，成立时执行对应流程。
    if ($versionInfo.OriginalFilename -ne $executable.OriginalName) {
        # 【逐行批注】 在错误情形终止流程并报告原因：throw "程序版本资源错误：$($executable.Path) 的原始文件名不是 $($executable.OriginalName)"。
        throw "程序版本资源错误：$($executable.Path) 的原始文件名不是 $($executable.OriginalName)"
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 执行命令并将结果保存到 $profileDirectoryName。
$profileDirectoryName = if ($TeachingPreview) { "GangyiAI-Teaching-Preview" } elseif ($Preview) { "GangyiAI-Library-Preview" } else { "GangyiAI" }
# 【逐行批注】 执行命令并将结果保存到 $portableGuide。
$portableGuide = @"
钢一定制AI v$version 免安装版

1. 请完整解压 ZIP，不要直接在压缩包内运行。
2. 双击 gangyiAI-launcher.exe，可先点击“暂不配置，直接使用”；需要 AI 功能时再配置服务商、API Key 和模型。
3. 配置、凭据、日志和课程数据库保存在 %LOCALAPPDATA%\$profileDirectoryName。
4. 本版本无需 CMake、编译器、Visual Studio、Python 或 Node.js。
5. 独立窗口界面需要 Microsoft Edge WebView2 Runtime；若提示缺失，请联网安装后重试。
6. 当前程序尚未签名，Windows SmartScreen 可能提示未知发布者。
"@
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Set-Content -LiteralPath (Join-Path $distDir '免安装版使用说明.txt') -Value $portableGuide -Encod…。
Set-Content -LiteralPath (Join-Path $distDir '免安装版使用说明.txt') -Value $portableGuide -Encoding utf8
# 【逐行批注】 判断条件 ($Candidate) {，成立时执行对应流程。
if ($Candidate) {
    # 【逐行批注】 执行命令并将结果保存到 $candidateGuide。
    $candidateGuide = @"
钢一定制AI v$version 离线候选（非正式发布）

本包使用现有学习增强试用版标识 GangyiAI-Library-Preview。
升级前请备份程序目录和 %LOCALAPPDATA%\GangyiAI-Library-Preview 中的学习档案与设置，并确认凭据目标仍存在。
本包不会自动安装；本机是否升级、真实验收用量与结果，以同目录版本验收说明为准。
自动化安装、升级、卸载及备份恢复使用临时 Windows runner 的虚构数据；以该版本实际 CI 结果为准。
必要真实教学配对尚未全部通过，不宣称教学质量已验收或整体节省比例。
开发验收驱动、凭据、预算账本和学习数据不随包分发。使用 AI 功能仍按原设置发起正常请求。
"@
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Set-Content -LiteralPath (Join-Path $distDir '离线候选说明.txt') -Value $candidateGuide -Encodi…。
    Set-Content -LiteralPath (Join-Path $distDir '离线候选说明.txt') -Value $candidateGuide -Encoding utf8
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Set-Content -LiteralPath (Join-Path $distDir '免安装版使用说明.txt') -Value $candidateGuide -Enco…。
    Set-Content -LiteralPath (Join-Path $distDir '免安装版使用说明.txt') -Value $candidateGuide -Encoding utf8
    # 【逐行批注】 执行命令并将结果保存到 @{ version。
    @{ version=$version; channel='offline-candidate'; profileName='GangyiAI-Library-Preview'; realModelEvaluationComplete=$false; acceptanceReport="v$version-candidate.md" } |
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：ConvertTo-Json | Set-Content -LiteralPath (Join-Path $distDir 'candidate-metadata.json') …。
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $distDir 'candidate-metadata.json') -Encoding utf8
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 打包前只检查本次新建的发布目录，拒绝数据库、开发入口、验证证据与私有配置。
# 【逐行批注】 执行命令并将结果保存到 $privateFiles。
$privateFiles = Get-ChildItem -LiteralPath $distDir -Recurse -File | Where-Object {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$_.Name -match '\.(db|sqlite|sqlite3)(-(wal|shm))?$|\.log$|^\.env|^settings\.json$|^home-…。
    $_.Name -match '\.(db|sqlite|sqlite3)(-(wal|shm))?$|\.log$|^\.env|^settings\.json$|^home-recommendations\.json' -or
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$_.FullName -match '[\\/](verification|\.superpowers|preview-data|backups|tests|scripts)[…。
    $_.FullName -match '[\\/](verification|\.superpowers|preview-data|backups|tests|scripts)[\\/]'
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 判断条件 ($privateFiles) { throw '候选目录含禁止打包的数据或开发材料' }，成立时执行对应流程。
if ($privateFiles) { throw '候选目录含禁止打包的数据或开发材料' }
# 【逐行批注】 执行命令并将结果保存到 $portableName。
$portableName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.zip" } elseif ($Preview) { "gangyiAI-learning-preview1-v$version-x64.zip" } else { "gangyiAI-portable-v$version-x64.zip" }
# 【逐行批注】 执行命令并将结果保存到 $portablePath。
$portablePath = Join-Path $repoRoot "dist\installer\$portableName"
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Force -Path (Split-Path -Parent $portablePath) | Out-Null。
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $portablePath) | Out-Null
# 【逐行批注】 判断条件 ($Candidate) {，成立时执行对应流程。
if ($Candidate) {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $repoRoot "docs/releases/v$version-candidate.md") -Dest…。
    Copy-Item -LiteralPath (Join-Path $repoRoot "docs/releases/v$version-candidate.md") -Destination (Join-Path $distDir "v$version-candidate.md") -Force
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath (Join-Path $repoRoot "docs/releases/v$version-candidate.md") -Dest…。
    Copy-Item -LiteralPath (Join-Path $repoRoot "docs/releases/v$version-candidate.md") -Destination (Join-Path $repoRoot 'dist/installer/离线候选说明.md') -Force
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 判断条件 (Test-Path -LiteralPath $portablePath) { Remove-Item -LiteralPat…，成立时执行对应流程。
if (Test-Path -LiteralPath $portablePath) { Remove-Item -LiteralPath $portablePath -Force }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Compress-Archive -Path (Join-Path $distDir '*') -DestinationPath $portablePath -Compressi…。
Compress-Archive -Path (Join-Path $distDir '*') -DestinationPath $portablePath -CompressionLevel Optimal

# 【逐行批注】 判断条件 (-not $SkipInstaller) {，成立时执行对应流程。
if (-not $SkipInstaller) {
    # 随安装包携带 x64 离线运行时，确保无网络的电脑也能安装。
    # 【逐行批注】 执行命令并将结果保存到 $webViewRuntime。
    $webViewRuntime = Join-Path $buildDir 'MicrosoftEdgeWebView2RuntimeInstallerX64.exe'
    # 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath $webViewRuntime)) {，成立时执行对应流程。
    if (-not (Test-Path -LiteralPath $webViewRuntime)) {
        # 【逐行批注】 执行命令并将结果保存到 Invoke-WebRequest -Uri 'https://go.microsoft…。
        Invoke-WebRequest -Uri 'https://go.microsoft.com/fwlink/?linkid=2124701' -OutFile $webViewRuntime -UseBasicParsing
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 执行命令并将结果保存到 $signature。
    $signature = Get-AuthenticodeSignature -LiteralPath $webViewRuntime
    # 【逐行批注】 判断条件 ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.…，成立时执行对应流程。
    if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'O=Microsoft Corporation') {
        # 【逐行批注】 在错误情形终止流程并报告原因：throw 'WebView2 离线运行时未通过微软签名校验'。
        throw 'WebView2 离线运行时未通过微软签名校验'
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath $webViewRuntime -Destination (Join-Path $repoRoot 'dist\installer\…。
    Copy-Item -LiteralPath $webViewRuntime -Destination (Join-Path $repoRoot 'dist\installer\MicrosoftEdgeWebView2RuntimeInstallerX64.exe') -Force
    # 【逐行批注】 执行命令并将结果保存到 $iscc。
    $iscc = Get-Command 'ISCC.exe' -ErrorAction SilentlyContinue
    # 【逐行批注】 判断条件 (-not $iscc) {，成立时执行对应流程。
    if (-not $iscc) {
        # 【逐行批注】 执行命令并将结果保存到 $isccCandidates。
        $isccCandidates = @(
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：(Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),。
            (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：(Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe')。
            (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe')
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：)。
        )
        # 【逐行批注】 执行命令并将结果保存到 $defaultIscc。
        $defaultIscc = $isccCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
        # 【逐行批注】 判断条件 ($defaultIscc) { $iscc = Get-Item -LiteralPath $defaultIscc }，成立时执行对应流程。
        if ($defaultIscc) { $iscc = Get-Item -LiteralPath $defaultIscc }
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 判断条件 (-not $iscc) { throw '缺少 Inno Setup 6（ISCC.exe）' }，成立时执行对应流程。
    if (-not $iscc) { throw '缺少 Inno Setup 6（ISCC.exe）' }
    # 【逐行批注】 执行命令并将结果保存到 $isccPath。
    $isccPath = if ($iscc -is [System.Management.Automation.CommandInfo]) { $iscc.Source } else { $iscc.FullName }
    # 【逐行批注】 判断条件 ($TeachingPreview) { & $isccPath "/DTeachingPreview=1" "/DMyAppV…，成立时执行对应流程。
    if ($TeachingPreview) { & $isccPath "/DTeachingPreview=1" "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss') }
    # 【逐行批注】 当前一条件不成立时继续判断：($Preview) { & $isccPath "/DMyAIPreview=1" "/DMyAppVersion=$vers…。
    elseif ($Preview) { & $isccPath "/DMyAIPreview=1" "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss') }
    # 【逐行批注】 执行前一条件不成立时的流程。
    else { & $isccPath "/DMyAppVersion=$version" (Join-Path $repoRoot 'installer\gangyiAI.iss') }
    # 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw 'Inno Setup 打包失败' }，成立时执行对应流程。
    if ($LASTEXITCODE -ne 0) { throw 'Inno Setup 打包失败' }

    # 【逐行批注】 执行命令并将结果保存到 $installerName。
    $installerName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.exe" } elseif ($Preview) { "gangyiAI-learning-preview1-v$version-x64.exe" } else { "gangyiAI-setup-v$version-x64.exe" }
    # 【逐行批注】 执行命令并将结果保存到 $installerPath。
    $installerPath = Join-Path $repoRoot "dist\installer\$installerName"
    # 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath $installerPath)) { throw "安装包未生成：$…，成立时执行对应流程。
    if (-not (Test-Path -LiteralPath $installerPath)) { throw "安装包未生成：$installerPath" }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 执行命令并将结果保存到 $checksums。
$checksums = @()
# 【逐行批注】 判断条件 (-not $SkipInstaller) {，成立时执行对应流程。
if (-not $SkipInstaller) {
    # 【逐行批注】 执行命令并将结果保存到 $installerName。
    $installerName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.exe" } elseif ($Preview) { "gangyiAI-learning-preview1-v$version-x64.exe" } else { "gangyiAI-setup-v$version-x64.exe" }
    # 【逐行批注】 执行命令并将结果保存到 $installerPath。
    $installerPath = Join-Path $repoRoot "dist\installer\$installerName"
    # 【逐行批注】 执行命令并将结果保存到 $installerHash。
    $installerHash = (Get-FileHash -LiteralPath $installerPath -Algorithm SHA256).Hash.ToLowerInvariant()
    # 【逐行批注】 执行命令并将结果保存到 $checksums +。
    $checksums += "$installerHash *$installerName"
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 执行命令并将结果保存到 $portableHash。
$portableHash = (Get-FileHash -LiteralPath $portablePath -Algorithm SHA256).Hash.ToLowerInvariant()
# 【逐行批注】 执行命令并将结果保存到 $checksums +。
$checksums += "$portableHash *$portableName"
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：$checksums | Set-Content -LiteralPath (Join-Path $repoRoot 'dist\installer\SHA256SUMS.txt…。
$checksums | Set-Content -LiteralPath (Join-Path $repoRoot 'dist\installer\SHA256SUMS.txt') -Encoding ascii

# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Write-Host '[完成] Windows 发布产物已生成。'。
Write-Host '[完成] Windows 发布产物已生成。'
