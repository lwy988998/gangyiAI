# 【逐行批注】 声明脚本参数，控制本次构建或测试行为。
param(
    # 【逐行批注】 执行命令并将结果保存到 [string]$RepoRoot。
    [string]$RepoRoot = (Split-Path -Parent $PSScriptRoot),
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$TeachingPreview,。
    [switch]$TeachingPreview,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$LibraryPreview,。
    [switch]$LibraryPreview,
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：[switch]$AllowLocalUserData。
    [switch]$AllowLocalUserData
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：)。
)

# 【逐行批注】 执行命令并将结果保存到 $ErrorActionPreference。
$ErrorActionPreference = 'Stop'
# 【逐行批注】 执行命令并将结果保存到 $cmakeText。
$cmakeText = Get-Content -Raw -LiteralPath (Join-Path $RepoRoot 'CMakeLists.txt')
# 【逐行批注】 判断条件 ($cmakeText -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { t…，成立时执行对应流程。
if ($cmakeText -notmatch 'project\(gangyiAI VERSION ([0-9.]+)') { throw '无法读取版本号' }
# 【逐行批注】 执行命令并将结果保存到 $version。
$version = $Matches[1]
# 【逐行批注】 判断条件 ($TeachingPreview -and $LibraryPreview) { throw '一次只验收一种试用安装标识' }，成立时执行对应流程。
if ($TeachingPreview -and $LibraryPreview) { throw '一次只验收一种试用安装标识' }
# 【逐行批注】 判断条件 ($env:GITHUB_ACTIONS -ne 'true' -and -not $AllowLocalUserData) {，成立时执行对应流程。
if ($env:GITHUB_ACTIONS -ne 'true' -and -not $AllowLocalUserData) {
    # 【逐行批注】 在错误情形终止流程并报告原因：throw '发布验收会安装和卸载程序，并清理当前用户配置。仅可在 GitHub Actions 中运行；确需在隔离账户运行时请显式传入 -Allo…。
    throw '发布验收会安装和卸载程序，并清理当前用户配置。仅可在 GitHub Actions 中运行；确需在隔离账户运行时请显式传入 -AllowLocalUserData。'
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 执行命令并将结果保存到 $installerName。
$installerName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.exe" } elseif ($LibraryPreview) { "gangyiAI-learning-preview1-v$version-x64.exe" } else { "gangyiAI-setup-v$version-x64.exe" }
# 【逐行批注】 执行命令并将结果保存到 $portableName。
$portableName = if ($TeachingPreview) { "gangyiAI-teaching-preview1-v$version-x64.zip" } elseif ($LibraryPreview) { "gangyiAI-learning-preview1-v$version-x64.zip" } else { "gangyiAI-portable-v$version-x64.zip" }
# 【逐行批注】 执行命令并将结果保存到 $installer。
$installer = Join-Path $RepoRoot "dist\installer\$installerName"
# 【逐行批注】 执行命令并将结果保存到 $portable。
$portable = Join-Path $RepoRoot "dist\installer\$portableName"
# 【逐行批注】 执行命令并将结果保存到 $checksumFile。
$checksumFile = Join-Path $RepoRoot 'dist\installer\SHA256SUMS.txt'
# 【逐行批注】 逐个处理集合中的元素：foreach ($path in @($installer, $portable, $checksumFile)) {。
foreach ($path in @($installer, $portable, $checksumFile)) {
    # 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath $path)) { throw "验收文件缺失：$path" }，成立时执行对应流程。
    if (-not (Test-Path -LiteralPath $path)) { throw "验收文件缺失：$path" }
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 执行命令并将结果保存到 $checksums。
$checksums = Get-Content -LiteralPath $checksumFile
# 【逐行批注】 逐个处理集合中的元素：foreach ($artifact in @($installer, $portable)) {。
foreach ($artifact in @($installer, $portable)) {
    # 【逐行批注】 执行命令并将结果保存到 $name。
    $name = Split-Path -Leaf $artifact
    # 【逐行批注】 执行命令并将结果保存到 $expectedLine。
    $expectedLine = $checksums | Where-Object { ($_ -split ' ', 2)[1] -ceq "*$name" }
    # 【逐行批注】 判断条件 (-not $expectedLine) { throw "SHA256SUMS.txt 缺少：$name" }，成立时执行对应流程。
    if (-not $expectedLine) { throw "SHA256SUMS.txt 缺少：$name" }
    # 【逐行批注】 执行命令并将结果保存到 $expected。
    $expected = ($expectedLine -split ' ')[0]
    # 【逐行批注】 执行命令并将结果保存到 $actual。
    $actual = (Get-FileHash -LiteralPath $artifact -Algorithm SHA256).Hash.ToLowerInvariant()
    # 【逐行批注】 判断条件 ($actual -ne $expected) { throw "$name 的 SHA256 不匹配" }，成立时执行对应流程。
    if ($actual -ne $expected) { throw "$name 的 SHA256 不匹配" }
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 执行命令并将结果保存到 $temporary。
$temporary = Join-Path ([System.IO.Path]::GetTempPath()) ("GangyiAI-CI-" + [guid]::NewGuid().ToString('N'))
# 【逐行批注】 执行命令并将结果保存到 $portableDir。
$portableDir = Join-Path $temporary 'portable'
# 【逐行批注】 执行命令并将结果保存到 $installDir。
$installDir = Join-Path $temporary 'installed'
# 【逐行批注】 执行命令并将结果保存到 $profileName。
$profileName = if ($TeachingPreview) { 'GangyiAI-Teaching-Preview' } elseif ($LibraryPreview) { 'GangyiAI-Library-Preview' } else { 'GangyiAI' }
# 【逐行批注】 执行命令并将结果保存到 $dataDir。
$dataDir = Join-Path $env:LOCALAPPDATA "$profileName\data"
# 【逐行批注】 执行命令并将结果保存到 $marker。
$marker = Join-Path $dataDir 'ci-preserve.marker'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：New-Item -ItemType Directory -Force -Path $temporary, $portableDir, $dataDir | Out-Null。
New-Item -ItemType Directory -Force -Path $temporary, $portableDir, $dataDir | Out-Null
# 【逐行批注】 开始可能失败的 PowerShell 操作。
try {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Expand-Archive -LiteralPath $portable -DestinationPath $portableDir。
    Expand-Archive -LiteralPath $portable -DestinationPath $portableDir
    # 【逐行批注】 执行命令并将结果保存到 $privateFiles。
    $privateFiles = Get-ChildItem -LiteralPath $portableDir -Recurse -File | Where-Object {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$_.Extension -in @('.db', '.log') -or $_.Name -like '.env*' -or。
        $_.Extension -in @('.db', '.log') -or $_.Name -like '.env*' -or
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$_.Name -like 'home-recommendations.json*' -or $_.Name -eq 'settings.json' -or。
        $_.Name -like 'home-recommendations.json*' -or $_.Name -eq 'settings.json' -or
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$_.FullName -match '[\\/](verification|preview-data|backups)[\\/]'。
        $_.FullName -match '[\\/](verification|preview-data|backups)[\\/]'
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 判断条件 ($privateFiles) { throw '发布包包含本机数据或验证材料' }，成立时执行对应流程。
    if ($privateFiles) { throw '发布包包含本机数据或验证材料' }
    # 【逐行批注】 执行命令并将结果保存到 $portableSelfTest。
    $portableSelfTest = Start-Process (Join-Path $portableDir 'gangyiAI-launcher.exe') -ArgumentList '--self-test' -Wait -PassThru -WindowStyle Hidden
    # 【逐行批注】 判断条件 ($portableSelfTest.ExitCode -ne 0) { throw '免安装版启动器自检失败' }，成立时执行对应流程。
    if ($portableSelfTest.ExitCode -ne 0) { throw '免安装版启动器自检失败' }

    # 【逐行批注】 执行命令并将结果保存到 $listener。
    $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, 0)
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$listener.Start()。
    $listener.Start()
    # 【逐行批注】 执行命令并将结果保存到 $port。
    $port = ([System.Net.IPEndPoint]$listener.LocalEndpoint).Port
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：$listener.Stop()。
    $listener.Stop()
    # 【逐行批注】 执行命令并将结果保存到 $env:HOST。
    $env:HOST = '127.0.0.1'
    # 【逐行批注】 执行命令并将结果保存到 $env:PORT。
    $env:PORT = "$port"
    # 【逐行批注】 执行命令并将结果保存到 $env:LOCAL_CONTROL_TOKEN。
    $env:LOCAL_CONTROL_TOKEN = 'ci-control-token'
    # 【逐行批注】 执行命令并将结果保存到 $env:DATABASE_PATH。
    $env:DATABASE_PATH = Join-Path $temporary 'smoke.db'
    # 【逐行批注】 执行命令并将结果保存到 $env:AI_BASE_URL。
    $env:AI_BASE_URL = 'http://127.0.0.1:9/v1'
    # 【逐行批注】 执行命令并将结果保存到 $env:AI_API_KEY。
    $env:AI_API_KEY = ''
    # 【逐行批注】 执行命令并将结果保存到 $env:AI_MODEL。
    $env:AI_MODEL = 'ci-model'
    # 【逐行批注】 执行命令并将结果保存到 $service。
    $service = Start-Process (Join-Path $portableDir 'gangyiAI.exe') -WorkingDirectory $portableDir -PassThru -WindowStyle Hidden
    # 【逐行批注】 开始可能失败的 PowerShell 操作。
    try {
        # 【逐行批注】 执行命令并将结果保存到 $healthy。
        $healthy = $false
        # 【逐行批注】 逐个处理集合中的元素：for ($attempt = 0; $attempt -lt 100; $attempt++) {。
        for ($attempt = 0; $attempt -lt 100; $attempt++) {
            # 【逐行批注】 开始可能失败的 PowerShell 操作。
            try {
                # 【逐行批注】 执行命令并将结果保存到 $health。
                $health = Invoke-WebRequest "http://127.0.0.1:$port/health" -UseBasicParsing -TimeoutSec 1
                # 【逐行批注】 判断条件 ($health.StatusCode -eq 200) { $healthy = $true; break }，成立时执行对应流程。
                if ($health.StatusCode -eq 200) { $healthy = $true; break }
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} catch {}。
            } catch {}
            # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Start-Sleep -Milliseconds 150。
            Start-Sleep -Milliseconds 150
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 判断条件 (-not $healthy) { throw '免安装版服务健康检查失败' }，成立时执行对应流程。
        if (-not $healthy) { throw '免安装版服务健康检查失败' }
        # 【逐行批注】 执行命令并将结果保存到 $homeResponse。
        $homeResponse = Invoke-WebRequest "http://127.0.0.1:$port/" -UseBasicParsing
        # 【逐行批注】 判断条件 ($homeResponse.StatusCode -ne 200 -or $homeResponse.Content -not…，成立时执行对应流程。
        if ($homeResponse.StatusCode -ne 200 -or $homeResponse.Content -notmatch '</html>') {
            # 【逐行批注】 在错误情形终止流程并报告原因：throw '未配置 API Key 时首页不可用'。
            throw '未配置 API Key 时首页不可用'
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 执行命令并将结果保存到 $recommendations。
        $recommendations = Invoke-RestMethod "http://127.0.0.1:$port/api/home/recommendations"
        # 【逐行批注】 判断条件 ($recommendations.items.lite.Count -ne 0 -or $recommendations.it…，成立时执行对应流程。
        if ($recommendations.items.lite.Count -ne 0 -or $recommendations.items.deep.Count -ne 0) {
            # 【逐行批注】 在错误情形终止流程并报告原因：throw '未配置真实 AI 时不应编造首页推荐'。
            throw '未配置真实 AI 时不应编造首页推荐'
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 执行命令并将结果保存到 $logo。
        $logo = Invoke-WebRequest "http://127.0.0.1:$port/school-logo.png" -UseBasicParsing
        # 【逐行批注】 判断条件 ($logo.Headers.'Content-Type' -ne 'image/png') { throw '校徽 MIME …，成立时执行对应流程。
        if ($logo.Headers.'Content-Type' -ne 'image/png') { throw '校徽 MIME 类型错误' }
        # 【逐行批注】 执行命令并将结果保存到 $background。
        $background = Invoke-WebRequest "http://127.0.0.1:$port/campus-background.jpg" -UseBasicParsing
        # 【逐行批注】 判断条件 ($background.Headers.'Content-Type' -ne 'image/jpeg') { throw '校…，成立时执行对应流程。
        if ($background.Headers.'Content-Type' -ne 'image/jpeg') { throw '校园背景图 MIME 类型错误' }
        # 【逐行批注】 逐个处理集合中的元素：foreach ($resource in @('clarity.css', 'clarity.js', 'deep-course.css', 'deep-c…。
        foreach ($resource in @('clarity.css', 'clarity.js', 'deep-course.css', 'deep-course.js', 'library.js', 'library.css', 'vendor/readability/Readability.js', 'app-layout.css', 'app-layout.js', 'profile-radar.js', 'profile-radar.css', 'classroom.js', 'classroom.css', 'ask.js', 'chat-render.js', 'user-center.js', 'learning-agent.js', 'agent-shell.js', 'agent-classroom.html', 'agent-classroom.js', 'agent-classroom.css', 'agent-prepare.html', 'agent-prepare.js',
                                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'vendor/katex/katex.min.js', 'vendor/katex/katex.min.css',。
                                'vendor/katex/katex.min.js', 'vendor/katex/katex.min.css',
                                # 【逐行批注】 执行当前 PowerShell 构建或测试命令：'vendor/katex/fonts/KaTeX_Main-Regular.woff2')) {。
                                'vendor/katex/fonts/KaTeX_Main-Regular.woff2')) {
            # 【逐行批注】 执行命令并将结果保存到 $response。
            $response = Invoke-WebRequest "http://127.0.0.1:$port/$resource" -UseBasicParsing
            # 【逐行批注】 判断条件 ($response.StatusCode -ne 200 -or $response.RawContentLength -lt…，成立时执行对应流程。
            if ($response.StatusCode -ne 200 -or $response.RawContentLength -lt 100) { throw "发布资源缺失：$resource" }
        # 【逐行批注】 开始或结束当前脚本代码块。
        }
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Invoke-WebRequest "http://127.0.0.1:$port/internal/shutdown" -Method Post `。
        Invoke-WebRequest "http://127.0.0.1:$port/internal/shutdown" -Method Post `
            -Headers @{'X-Gangyi-Control-Token'='ci-control-token'} -UseBasicParsing | Out-Null
        # 【逐行批注】 判断条件 (-not $service.WaitForExit(5000)) { throw '服务未能优雅退出' }，成立时执行对应流程。
        if (-not $service.WaitForExit(5000)) { throw '服务未能优雅退出' }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
    } finally {
        # 【逐行批注】 判断条件 (-not $service.HasExited) { $service.Kill() }，成立时执行对应流程。
        if (-not $service.HasExited) { $service.Kill() }
    # 【逐行批注】 开始或结束当前脚本代码块。
    }

    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Set-Content -LiteralPath $marker -Value 'preserve' -Encoding ascii。
    Set-Content -LiteralPath $marker -Value 'preserve' -Encoding ascii
    # 【逐行批注】 执行命令并将结果保存到 $setupArgs。
    $setupArgs = "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP- /NOICONS /DIR=`"$installDir`""
    # 【逐行批注】 执行命令并将结果保存到 $setup。
    $setup = Start-Process $installer -ArgumentList $setupArgs -Wait -PassThru -WindowStyle Hidden
    # 【逐行批注】 判断条件 ($setup.ExitCode -ne 0) { throw "静默安装失败：$($setup.ExitCode)" }，成立时执行对应流程。
    if ($setup.ExitCode -ne 0) { throw "静默安装失败：$($setup.ExitCode)" }
    # 【逐行批注】 执行命令并将结果保存到 $installedSelfTest。
    $installedSelfTest = Start-Process (Join-Path $installDir 'gangyiAI-launcher.exe') -ArgumentList '--self-test' -Wait -PassThru -WindowStyle Hidden
    # 【逐行批注】 判断条件 ($installedSelfTest.ExitCode -ne 0) { throw '已安装启动器自检失败' }，成立时执行对应流程。
    if ($installedSelfTest.ExitCode -ne 0) { throw '已安装启动器自检失败' }
    # 【逐行批注】 逐个处理集合中的元素：foreach ($name in @('gangyiAI.exe', 'gangyiAI-launcher.exe')) {。
    foreach ($name in @('gangyiAI.exe', 'gangyiAI-launcher.exe')) {
        # 【逐行批注】 执行命令并将结果保存到 $actualVersion。
        $actualVersion = (Get-Item (Join-Path $installDir $name)).VersionInfo.ProductVersion.Trim()
        # 【逐行批注】 判断条件 ($actualVersion -ne $version) { throw "$name 版本不正确：$actualVersio…，成立时执行对应流程。
        if ($actualVersion -ne $version) { throw "$name 版本不正确：$actualVersion" }
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 执行命令并将结果保存到 $uninstall。
    $uninstall = Start-Process (Join-Path $installDir 'unins000.exe') `
        -ArgumentList '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART' -Wait -PassThru -WindowStyle Hidden
    # 【逐行批注】 逐个处理集合中的元素：for ($attempt = 0; $attempt -lt 20 -and (Test-Path $installDir); $attempt++) {。
    for ($attempt = 0; $attempt -lt 20 -and (Test-Path $installDir); $attempt++) {
        # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Start-Sleep -Milliseconds 250。
        Start-Sleep -Milliseconds 250
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 判断条件 ($uninstall.ExitCode -ne 0 -or (Test-Path $installDir)) { throw …，成立时执行对应流程。
    if ($uninstall.ExitCode -ne 0 -or (Test-Path $installDir)) { throw '静默卸载失败' }
    # 【逐行批注】 判断条件 (-not (Test-Path $marker)) { throw '卸载错误删除了用户课程数据' }，成立时执行对应流程。
    if (-not (Test-Path $marker)) { throw '卸载错误删除了用户课程数据' }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：} finally {。
} finally {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Remove-Item -LiteralPath $marker -Force -ErrorAction SilentlyContinue。
    Remove-Item -LiteralPath $marker -Force -ErrorAction SilentlyContinue
    # 【逐行批注】 执行命令并将结果保存到 $resolvedTemporary。
    $resolvedTemporary = [System.IO.Path]::GetFullPath($temporary)
    # 【逐行批注】 执行命令并将结果保存到 $resolvedTempRoot。
    $resolvedTempRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath()).TrimEnd('\') + '\'
    # 【逐行批注】 判断条件 (-not $resolvedTemporary.StartsWith($resolvedTempRoot, [StringCo…，成立时执行对应流程。
    if (-not $resolvedTemporary.StartsWith($resolvedTempRoot, [StringComparison]::OrdinalIgnoreCase)) { throw '拒绝清理临时目录之外的路径' }
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Remove-Item -LiteralPath $temporary -Recurse -Force -ErrorAction SilentlyContinue。
    Remove-Item -LiteralPath $temporary -Recurse -Force -ErrorAction SilentlyContinue
# 【逐行批注】 开始或结束当前脚本代码块。
}

# 【逐行批注】 执行当前 PowerShell 构建或测试命令：Write-Host '[通过] Windows 发布产物验收完成。'。
Write-Host '[通过] Windows 发布产物验收完成。'
