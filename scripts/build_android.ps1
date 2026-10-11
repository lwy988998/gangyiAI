param(
    [string]$ToolchainRoot = 'G:\GangyiAI-Android-Toolchain',
    [ValidateSet('arm64-v8a', 'x86_64')][string]$Abi = 'arm64-v8a',
    [switch]$Debug, [switch]$Prepare, [switch]$Sign,
    [string]$OutputDirectory = 'G:\GangyiAI-Android',
    [string]$SigningDirectory = 'D:\codex\home\secrets\gangyiAI-android'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$env:JAVA_HOME = Get-ChildItem -LiteralPath (Join-Path $ToolchainRoot 'jdk-17') -Directory |
    Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'bin/java.exe') } | Select-Object -First 1 -ExpandProperty FullName
$env:ANDROID_HOME = Join-Path $ToolchainRoot 'sdk'
$env:ANDROID_SDK_ROOT = $env:ANDROID_HOME
$env:GRADLE_USER_HOME = Join-Path $ToolchainRoot 'gradle-cache'
$env:GANGYI_ANDROID_DEPS = Join-Path $ToolchainRoot 'native'
$env:GANGYI_ANDROID_BUILD = Join-Path $ToolchainRoot "app-native/$Abi"
$env:GANGYI_ANDROID_OUTPUT = Join-Path $ToolchainRoot "app-build/$Abi"
if (-not $env:JAVA_HOME) { throw '请先运行 setup_android.ps1 准备 JDK 17 和 Android SDK' }
if ($Prepare) {
    python (Join-Path $PSScriptRoot 'prepare_android_native.py') --root $env:GANGYI_ANDROID_DEPS --sdk $env:ANDROID_HOME --abi $Abi
    if ($LASTEXITCODE) { throw '原生依赖准备失败' }
}
if ($Sign) {
    New-Item -ItemType Directory -Force -Path $SigningDirectory | Out-Null
    $store = Join-Path $SigningDirectory 'android-release.jks'
    $protectedPassword = Join-Path $SigningDirectory 'password.dpapi'
    Add-Type -AssemblyName System.Security
    if (-not (Test-Path -LiteralPath $protectedPassword)) {
        if (Test-Path -LiteralPath $store) { throw '已有签名库但没有配套密码文件；必须恢复原密码，不能生成替代签名' }
        $bytes = [byte[]]::new(32)
        [System.Security.Cryptography.RandomNumberGenerator]::Fill($bytes)
        $passwordBytes = [System.Text.Encoding]::UTF8.GetBytes([Convert]::ToBase64String($bytes))
        [IO.File]::WriteAllBytes($protectedPassword, [System.Security.Cryptography.ProtectedData]::Protect($passwordBytes, $null, 'CurrentUser'))
    }
    $secret = [System.Security.Cryptography.ProtectedData]::Unprotect([IO.File]::ReadAllBytes($protectedPassword), $null, 'CurrentUser')
    $env:GANGYI_ANDROID_STORE_PASSWORD = [System.Text.Encoding]::UTF8.GetString($secret)
    $env:GANGYI_ANDROID_KEY_PASSWORD = $env:GANGYI_ANDROID_STORE_PASSWORD
    $env:GANGYI_ANDROID_KEYSTORE = $store
    if (-not (Test-Path -LiteralPath $store)) {
        & (Join-Path $env:JAVA_HOME 'bin/keytool.exe') -genkeypair -keystore $store -alias gangyi-android -keyalg RSA -keysize 3072 -validity 10000 -dname 'CN=GangyiAI Android,O=GangyiAI,C=CN' -storepass:env GANGYI_ANDROID_STORE_PASSWORD -keypass:env GANGYI_ANDROID_KEY_PASSWORD
        if ($LASTEXITCODE) { throw '稳定签名生成失败' }
    }
}
try {
    $task = if ($Debug) { 'assembleDebug' } else { 'assembleRelease' }
    $gradle = Join-Path $repo 'android/gradlew.bat'
    & $gradle -p (Join-Path $repo 'android') ":app:$task" "-PgangyiAbi=$Abi" --console plain --max-workers 4
    if ($LASTEXITCODE) { throw '安卓 APK 构建失败' }
    $variant = if ($Debug) { 'debug' } else { 'release' }
    $suffix = if ($Debug) { '' } elseif ($Sign) { '' } else { '-unsigned' }
    $apk = Join-Path $env:GANGYI_ANDROID_OUTPUT "outputs/apk/$variant/app-$variant$suffix.apk"
    if (-not (Test-Path -LiteralPath $apk)) { throw "没有生成 APK：$apk" }
    New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
    $name = "GangyiAI-Android-v1.0.0-preview.1-$Abi-$variant$suffix.apk"
    Copy-Item -LiteralPath $apk -Destination (Join-Path $OutputDirectory $name) -Force
    $hash = (Get-FileHash -LiteralPath $apk -Algorithm SHA256).Hash.ToLowerInvariant()
    Set-Content -LiteralPath (Join-Path $OutputDirectory "$name.sha256") -Value "$hash  $name" -Encoding utf8NoBOM
    if ($Sign -or $Debug) {
        & (Join-Path $env:ANDROID_HOME 'build-tools/36.0.0/apksigner.bat') verify --verbose --print-certs $apk
        if ($LASTEXITCODE) { throw 'APK 签名校验失败' }
    }
    & (Join-Path $env:ANDROID_HOME 'build-tools/36.0.0/zipalign.exe') -c -P 16 4 $apk
    if ($LASTEXITCODE) { throw 'APK 的 16 KB 对齐检查失败' }
    Write-Output "已交付：$(Join-Path $OutputDirectory $name)"
} finally {
    Remove-Item Env:\GANGYI_ANDROID_STORE_PASSWORD -ErrorAction SilentlyContinue
    Remove-Item Env:\GANGYI_ANDROID_KEY_PASSWORD -ErrorAction SilentlyContinue
}
