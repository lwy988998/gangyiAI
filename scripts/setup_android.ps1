param([string]$ToolchainRoot = 'G:\GangyiAI-Android-Toolchain', [switch]$WithEmulator)
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
$downloadDir = Join-Path $ToolchainRoot 'downloads'
$sdkDir = Join-Path $ToolchainRoot 'sdk'
New-Item -ItemType Directory -Force -Path $downloadDir, $sdkDir | Out-Null

function Get-VerifiedArchive([string]$Url, [string]$Path, [string]$Checksum, [string]$Algorithm = 'SHA256') {
    if ((Test-Path -LiteralPath $Path) -and (Get-FileHash -LiteralPath $Path -Algorithm $Algorithm).Hash -eq $Checksum) { return }
    if ($Checksum -notmatch '^[0-9a-fA-F]{40,64}$') { throw "下载缺少有效校验值：$Url" }
    # 大型 SDK 下载支持断点续传，网络中断不丢弃已下载的数据。
    & 'C:\Windows\System32\curl.exe' --fail --location --retry 4 --retry-delay 2 --continue-at - --output $Path $Url
    if ($LASTEXITCODE -ne 0) { throw "下载未完成，可重新运行继续：$Path" }
    if ((Get-FileHash -LiteralPath $Path -Algorithm $Algorithm).Hash -ne $Checksum) { throw "下载校验失败：$Path" }
}

$jdkParent = Join-Path $ToolchainRoot 'jdk-17'
$jdkHome = Get-ChildItem -LiteralPath $jdkParent -Directory -ErrorAction SilentlyContinue |
    Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'bin\java.exe') } | Select-Object -First 1 -ExpandProperty FullName
if (-not $jdkHome) {
    $jdk = @(Invoke-RestMethod -Uri 'https://api.adoptium.net/v3/assets/latest/17/hotspot?architecture=x64&image_type=jdk&os=windows&vendor=eclipse')[0]
    $jdkZip = Join-Path $downloadDir 'temurin-17-windows-x64.zip'
    Get-VerifiedArchive $jdk.binary.package.link $jdkZip $jdk.binary.package.checksum
    Expand-Archive -LiteralPath $jdkZip -DestinationPath $jdkParent
    $jdkHome = Get-ChildItem -LiteralPath $jdkParent -Directory | Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'bin\java.exe') } |
        Select-Object -First 1 -ExpandProperty FullName
}
if (-not $jdkHome) { throw '未找到 JDK 17' }
$env:JAVA_HOME = $jdkHome
$env:ANDROID_HOME = $sdkDir
$env:ANDROID_SDK_ROOT = $sdkDir

$sdkManager = Join-Path $sdkDir 'cmdline-tools\latest\bin\sdkmanager.bat'
if (-not (Test-Path -LiteralPath $sdkManager)) {
    [xml]$manifest = (Invoke-WebRequest -Uri 'https://dl.google.com/android/repository/repository2-1.xml').Content
    $packages = @($manifest.SelectNodes("//*[local-name()='remotePackage']") | Where-Object { $_.GetAttribute('path') -like 'cmdline-tools;*' })
    $package = $packages | Where-Object { $_.GetAttribute('path') -eq 'cmdline-tools;latest' } | Select-Object -First 1
    if (-not $package) { $package = $packages | Sort-Object { [int]$_.revision.major } -Descending | Select-Object -First 1 }
    $archive = @($package.archives.archive | Where-Object { $_.'host-os' -eq 'windows' })[0].complete
    if (-not $archive.url) { throw '微软 Windows 版 Android 命令行工具未找到' }
    $toolsZip = Join-Path $downloadDir 'android-commandline-tools.zip'
    Get-VerifiedArchive ('https://dl.google.com/android/repository/' + $archive.url) $toolsZip ([string]$archive.checksum) 'SHA1'
    $toolsExtract = Join-Path $ToolchainRoot 'commandline-extracted'
    Expand-Archive -LiteralPath $toolsZip -DestinationPath $toolsExtract
    $toolsSource = Join-Path $toolsExtract 'cmdline-tools'
    $toolsDestination = Join-Path $sdkDir 'cmdline-tools\latest'
    New-Item -ItemType Directory -Force -Path $toolsDestination | Out-Null
    Copy-Item -Path (Join-Path $toolsSource '*') -Destination $toolsDestination -Recurse
}

# 构建环境只写入工具链目录，不改变系统 Java 或现有 Android 软件。
1..100 | ForEach-Object { 'y' } | & $sdkManager "--sdk_root=$sdkDir" --licenses
if ($LASTEXITCODE -ne 0) { throw 'Android SDK 许可准备失败' }
$packagesToInstall = @('platform-tools', 'platforms/android-36', 'build-tools/36.0.0', 'ndk/28.2.13676358', 'cmake/3.22.1')
if ($WithEmulator) { $packagesToInstall += @('emulator', 'system-images/android-36/google_apis/x86_64', 'system-images/android-29/google_apis/x86_64') }
$androidCli = Join-Path $sdkDir 'cmdline-tools\latest\bin\android.exe'
foreach ($packageName in $packagesToInstall) {
    if (Test-Path -LiteralPath $androidCli) { & $androidCli "--sdk=$sdkDir" sdk install $packageName }
    else { & $sdkManager "--sdk_root=$sdkDir" $packageName.Replace('/', ';') }
    if ($LASTEXITCODE -ne 0) { throw "Android SDK 组件安装失败：$packageName" }
}
foreach ($requiredFile in @('platforms/android-36/android.jar', 'ndk/28.2.13676358/build/cmake/android.toolchain.cmake', 'cmake/3.22.1/bin/cmake.exe', 'build-tools/36.0.0/apksigner.bat')) {
    if (-not (Test-Path -LiteralPath (Join-Path $sdkDir $requiredFile))) { throw "SDK 缺少必要组件：$requiredFile" }
}
[pscustomobject]@{ JAVA_HOME = $jdkHome; ANDROID_HOME = $sdkDir; NDK = '28.2.13676358' } | ConvertTo-Json
