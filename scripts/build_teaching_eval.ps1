# 【逐行批注】 声明脚本参数，控制本次构建或测试行为。
param([Parameter(Mandatory=$true)][string]$BuildDir, [string]$OutputName = 'teaching-eval-driver.exe', [string]$BaselineRoot)
# 【逐行批注】 执行命令并将结果保存到 $ErrorActionPreference。
$ErrorActionPreference = 'Stop'
# 【逐行批注】 执行命令并将结果保存到 $repo。
$repo = Split-Path -Parent $PSScriptRoot
# 【逐行批注】 执行命令并将结果保存到 $destination。
$destination = [IO.Path]::GetFullPath((Join-Path $BuildDir $OutputName))
# 【逐行批注】 执行命令并将结果保存到 $protectedRoot。
$protectedRoot = [IO.Path]::GetFullPath((Join-Path $repo 'verification\v564-baseline'))
# 【逐行批注】 判断条件 ($destination.StartsWith($protectedRoot + [IO.Path]::DirectorySe…，成立时执行对应流程。
if ($destination.StartsWith($protectedRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw '禁止向冻结目录写入新入口。' }
# 【逐行批注】 执行命令并将结果保存到 $headers。
$headers = Join-Path $repo 'include'

# 先冻结输入并刷新链接对象；构建结束再次核对，避免凭据绑定到中途改变的源码。
if (-not $BaselineRoot) {
    & python -B -X utf8 (Join-Path $repo 'scripts/tutor_build_identity.py') capture --snapshot ($destination + '.inputs.json')
    if ($LASTEXITCODE -ne 0) { throw '构建输入冻结失败。' }
    & cmake --build $BuildDir --target learning-agent-tests --parallel 3
    if ($LASTEXITCODE -ne 0) { throw '教学主控对象刷新失败。' }
}

# 【逐行批注】 执行命令并将结果保存到 $sqlite。
$sqlite = Join-Path $BuildDir 'libsqlite3_amalgamation.a'
# 【逐行批注】 判断条件 ($BaselineRoot) {，成立时执行对应流程。
if ($BaselineRoot) {
    # 【逐行批注】 执行命令并将结果保存到 $frozen。
    $frozen = (Resolve-Path -LiteralPath $BaselineRoot).Path
    # 【逐行批注】 执行命令并将结果保存到 $destination。
    $destination = [IO.Path]::GetFullPath((Join-Path $BuildDir $OutputName))
    # 【逐行批注】 判断条件 ($destination.StartsWith($frozen + [IO.Path]::DirectorySeparator…，成立时执行对应流程。
    if ($destination.StartsWith($frozen + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw '禁止向冻结目录写入新入口。' }
    # 【逐行批注】 执行命令并将结果保存到 $manifest。
    $manifest = Get-Content -LiteralPath (Join-Path $frozen 'freeze.json') -Raw | ConvertFrom-Json
    # 【逐行批注】 判断条件 ($manifest.sourceCommit -ne 'effb401c7fe2a73785fcebe12b173204cfe…，成立时执行对应流程。
    if ($manifest.sourceCommit -ne 'effb401c7fe2a73785fcebe12b173204cfee769e') { throw '不是已确认的v5.6.3产品提交。' }
    # 【逐行批注】 逐个处理集合中的元素：foreach ($entry in $manifest.objects) {。
    foreach ($entry in $manifest.objects) {
        # 【逐行批注】 判断条件 ((Get-FileHash -LiteralPath (Join-Path $frozen ('objects\' + $en…，成立时执行对应流程。
        if ((Get-FileHash -LiteralPath (Join-Path $frozen ('objects\' + $entry.name)) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $entry.sha256) { throw '冻结对象校验失败。' }
    # 【逐行批注】 开始或结束当前脚本代码块。
    }
    # 【逐行批注】 执行命令并将结果保存到 $objects。
    $objects = Get-ChildItem -LiteralPath (Join-Path $frozen 'objects') -Filter '*.obj'
    # 【逐行批注】 执行命令并将结果保存到 $headers。
    $headers = Join-Path $frozen 'include'
    # 【逐行批注】 执行命令并将结果保存到 $sqlite。
    $sqlite = Join-Path $frozen 'libsqlite3_amalgamation.a'
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：} else {。
} else {
    # 【逐行批注】 执行命令并将结果保存到 $objects。
    $objects = Get-ChildItem -LiteralPath (Join-Path $BuildDir 'CMakeFiles\learning-agent-tests.dir\src') -Filter '*.obj'
# 【逐行批注】 开始或结束当前脚本代码块。
}
# 【逐行批注】 判断条件 (-not $objects) { throw '请先完成 learning-agent-tests 构建。' }，成立时执行对应流程。
if (-not $objects) { throw '请先完成 learning-agent-tests 构建。' }
# 【逐行批注】 执行命令并将结果保存到 $curl。
$curl = Get-ChildItem -LiteralPath (Join-Path $repo 'third_party\curl') -Directory | Select-Object -First 1
# 【逐行批注】 执行命令并将结果保存到 $argsList。
$argsList = @('-municode','-std=c++17','-O2','-D_GLIBCXX_ATOMIC_WORD_BUILTINS=1','-I', $headers,'-I',(Join-Path $repo 'third_party\json-3.11.3\include'),'-I',(Join-Path $curl.FullName 'include'), (Join-Path $repo 'tests\teaching_eval_driver.cpp'))
# 当前主控依赖构建生成的提示模块；使用同一构建目录，避免编译入口时漏头文件。
if (-not $BaselineRoot) { $argsList += @('-I', (Join-Path $BuildDir 'generated')) }
# 【逐行批注】 执行命令并将结果保存到 $argsList +。
$argsList += $objects.FullName
# 【逐行批注】 执行命令并将结果保存到 $argsList +。
$argsList += @($sqlite,(Join-Path $curl.FullName 'lib\libcurl.dll.a'),'-lws2_32','-Wl,--wrap=curl_easy_perform','-o',(Join-Path $BuildDir $OutputName))
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：& g++ @argsList。
& g++ @argsList
# 【逐行批注】 判断条件 ($LASTEXITCODE -ne 0) { throw '教学验收入口编译失败。' }，成立时执行对应流程。
if ($LASTEXITCODE -ne 0) { throw '教学验收入口编译失败。' }
# 【逐行批注】 执行命令并将结果保存到 $receipt。
$receipt = @{ harnessSha256=(Get-FileHash -LiteralPath (Join-Path $repo 'tests\teaching_eval_driver.cpp') -Algorithm SHA256).Hash.ToLowerInvariant(); headers=$headers; headerHashes=@(Get-ChildItem -LiteralPath $headers -File | ForEach-Object { @{ name=$_.Name; sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() } }); sqliteSha256=(Get-FileHash -LiteralPath $sqlite -Algorithm SHA256).Hash.ToLowerInvariant(); frozenBaseline=[bool]$BaselineRoot; objects=@($objects | ForEach-Object { @{ name=$_.Name; sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() } }) }
# 【逐行批注】 执行当前 PowerShell 构建或测试命令：$receipt | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $BuildDir ($Outp…。
$receipt | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $BuildDir ($OutputName + '.build.json')) -Encoding utf8
# 【逐行批注】 执行命令并将结果保存到 $curlSource。
$curlSource = Join-Path $curl.FullName 'bin\libcurl-x64.dll'
# 【逐行批注】 执行命令并将结果保存到 $curlTarget。
$curlTarget = Join-Path $BuildDir 'libcurl-x64.dll'
# 【逐行批注】 判断条件 (-not (Test-Path -LiteralPath $curlTarget) -or，成立时执行对应流程。
if (-not (Test-Path -LiteralPath $curlTarget) -or
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：(Get-FileHash -LiteralPath $curlSource -Algorithm SHA256).Hash -ne (Get-FileHash -Literal…。
    (Get-FileHash -LiteralPath $curlSource -Algorithm SHA256).Hash -ne (Get-FileHash -LiteralPath $curlTarget -Algorithm SHA256).Hash) {
    # 【逐行批注】 执行当前 PowerShell 构建或测试命令：Copy-Item -LiteralPath $curlSource -Destination $BuildDir -Force。
    Copy-Item -LiteralPath $curlSource -Destination $BuildDir -Force
# 【逐行批注】 开始或结束当前脚本代码块。
}

if (-not $BaselineRoot) {
    & python -B -X utf8 (Join-Path $repo 'scripts/tutor_build_identity.py') stamp --snapshot ($destination + '.inputs.json') --driver $destination --build-dir $BuildDir
    if ($LASTEXITCODE -ne 0) { throw '评测入口构建身份核对失败。' }
}
