param([Parameter(Mandatory=$true)][string]$BuildDir, [string]$OutputName = 'teaching-eval-driver.exe', [string]$BaselineRoot)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$destination = [IO.Path]::GetFullPath((Join-Path $BuildDir $OutputName))
$protectedRoot = [IO.Path]::GetFullPath((Join-Path $repo 'verification\v564-baseline'))
if ($destination.StartsWith($protectedRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw '禁止向冻结目录写入新入口。' }
$headers = Join-Path $repo 'include'
$sqlite = Join-Path $BuildDir 'libsqlite3_amalgamation.a'
if ($BaselineRoot) {
    $frozen = (Resolve-Path -LiteralPath $BaselineRoot).Path
    $destination = [IO.Path]::GetFullPath((Join-Path $BuildDir $OutputName))
    if ($destination.StartsWith($frozen + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw '禁止向冻结目录写入新入口。' }
    $manifest = Get-Content -LiteralPath (Join-Path $frozen 'freeze.json') -Raw | ConvertFrom-Json
    if ($manifest.sourceCommit -ne 'effb401c7fe2a73785fcebe12b173204cfee769e') { throw '不是已确认的v5.6.3产品提交。' }
    foreach ($entry in $manifest.objects) {
        if ((Get-FileHash -LiteralPath (Join-Path $frozen ('objects\' + $entry.name)) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.sha256) { throw '冻结对象校验失败。' }
    }
    $objects = Get-ChildItem -LiteralPath (Join-Path $frozen 'objects') -Filter '*.obj'
    $headers = Join-Path $frozen 'include'
    $sqlite = Join-Path $frozen 'libsqlite3_amalgamation.a'
} else {
    $objects = Get-ChildItem -LiteralPath (Join-Path $BuildDir 'CMakeFiles\learning-agent-tests.dir\src') -Filter '*.obj'
}
if (-not $objects) { throw '请先完成 learning-agent-tests 构建。' }
$curl = Get-ChildItem -LiteralPath (Join-Path $repo 'third_party\curl') -Directory | Select-Object -First 1
$argsList = @('-std=c++17','-O2','-D_GLIBCXX_ATOMIC_WORD_BUILTINS=1','-I', $headers,'-I',(Join-Path $repo 'third_party\json-3.11.3\include'),'-I',(Join-Path $curl.FullName 'include'), (Join-Path $repo 'tests\teaching_eval_driver.cpp'))
$argsList += $objects.FullName
$argsList += @($sqlite,(Join-Path $curl.FullName 'lib\libcurl.dll.a'),'-lws2_32','-o',(Join-Path $BuildDir $OutputName))
& g++ @argsList
if ($LASTEXITCODE -ne 0) { throw '教学验收入口编译失败。' }
$receipt = @{ harnessSha256=(Get-FileHash -LiteralPath (Join-Path $repo 'tests\teaching_eval_driver.cpp') -Algorithm SHA256).Hash.ToLowerInvariant(); headers=$headers; headerHashes=@(Get-ChildItem -LiteralPath $headers -File | ForEach-Object { @{ name=$_.Name; sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() } }); sqliteSha256=(Get-FileHash -LiteralPath $sqlite -Algorithm SHA256).Hash.ToLowerInvariant(); frozenBaseline=[bool]$BaselineRoot; objects=@($objects | ForEach-Object { @{ name=$_.Name; sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() } }) }
$receipt | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $BuildDir ($OutputName + '.build.json')) -Encoding utf8
$curlSource = Join-Path $curl.FullName 'bin\libcurl-x64.dll'
$curlTarget = Join-Path $BuildDir 'libcurl-x64.dll'
if (-not (Test-Path -LiteralPath $curlTarget) -or
    (Get-FileHash -LiteralPath $curlSource -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath $curlTarget -Algorithm SHA256).Hash) {
    Copy-Item -LiteralPath $curlSource -Destination $BuildDir -Force
}
