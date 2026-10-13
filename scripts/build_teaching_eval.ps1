param([Parameter(Mandatory=$true)][string]$BuildDir, [string]$OutputName = 'teaching-eval-driver.exe')
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$objects = Get-ChildItem -LiteralPath (Join-Path $BuildDir 'CMakeFiles\learning-agent-tests.dir\src') -Filter '*.obj'
if (-not $objects) { throw '请先完成 learning-agent-tests 构建。' }
$curl = Get-ChildItem -LiteralPath (Join-Path $repo 'third_party\curl') -Directory | Select-Object -First 1
$argsList = @('-std=c++17','-O2','-D_GLIBCXX_ATOMIC_WORD_BUILTINS=1','-I', (Join-Path $repo 'include'),'-I',(Join-Path $repo 'third_party\json-3.11.3\include'),'-I',(Join-Path $curl.FullName 'include'), (Join-Path $repo 'tests\teaching_eval_driver.cpp'))
$argsList += $objects.FullName
$argsList += @((Join-Path $BuildDir 'libsqlite3_amalgamation.a'),(Join-Path $curl.FullName 'lib\libcurl.dll.a'),'-lws2_32','-o',(Join-Path $BuildDir $OutputName))
& g++ @argsList
if ($LASTEXITCODE -ne 0) { throw '教学验收入口编译失败。' }
$curlSource = Join-Path $curl.FullName 'bin\libcurl-x64.dll'
$curlTarget = Join-Path $BuildDir 'libcurl-x64.dll'
if (-not (Test-Path -LiteralPath $curlTarget) -or
    (Get-FileHash -LiteralPath $curlSource -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath $curlTarget -Algorithm SHA256).Hash) {
    Copy-Item -LiteralPath $curlSource -Destination $BuildDir -Force
}
