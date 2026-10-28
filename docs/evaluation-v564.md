# v5.6.4 同协议离线对照入口

> v5.6.4 开发阶段的离线验收入口记录。文中预算与本机升级状态按该阶段解读，不构成新的付费授权；后续验收范围见[正式版本与候选记录](README.md#正式版本与候选记录)。

当前预算决定维持累计100万Token，本轮新增付费调用关闭。`live`模式先读取本项目的预算决定，在读取凭据和启动子进程前拒绝；C++教学驱动还会拒绝非本机模拟地址。今后即使预算决定更新，也需要显式审查驱动门禁，不能仅修改清单就启动付费调用。

## 构建同一教学驱动

在仓库根目录运行。两次编译使用同一`tests/teaching_eval_driver.cpp`，基线只链接已冻结v5.6.3头文件、对象和SQLite，输出均写候选构建目录，不覆盖冻结原件。每个exe旁有`.build.json`记录来源和哈希。

```powershell
pwsh -NoProfile -File scripts/build_teaching_eval.ps1 -BuildDir build-library-release -OutputName teaching-eval-task4-baseline.exe -BaselineRoot verification/v564-baseline
pwsh -NoProfile -File scripts/build_teaching_eval.ps1 -BuildDir build-library-release -OutputName teaching-eval-task4-candidate.exe
python -X utf8 tests/evaluation_entry_tests.py -v
python -X utf8 tests/teaching_eval_driver_smoke.py build-library-release/teaching-eval-task4-candidate.exe
```

## 零调用预检

在仓库根目录执行命令，以下示例将清单保存在私有、未跟踪的 `verification/private-evaluation` 目录；清单内相对路径也从仓库根目录解析。`casesFile`为外部案例数组，`preferencesFile`为两版共用的固定偏好JSON。`profileName`必须明确填写；离线模式不会打开凭据管理器、注册表或真实用户数据库。预检强制`maxCalls=0`，即使父进程存在真实服务环境变量也不会连接真实服务。每次运行创建新目录，预算只写`budget-copy.json`，上限最多取用户确认的100万。

```json
{
  "mode": "preflight",
  "casesFile": "verification/private-evaluation/cases.json",
  "shared": {
    "profileName": "GangyiAI-Library-Preview",
    "preferencesFile": "verification/private-evaluation/preferences.json"
  },
  "entries": [
    {
      "variant": "baseline",
      "sourceCommit": "effb401c7fe2a73785fcebe12b173204cfee769e",
      "driver": "build-library-release/teaching-eval-task4-baseline.exe"
    },
    {
      "variant": "candidate",
      "sourceCommit": "填写候选产品提交",
      "driver": "build-library-release/teaching-eval-task4-candidate.exe"
    }
  ]
}
```

```powershell
python -X utf8 scripts/evaluation_compare.py verification/private-evaluation/manifest.json --output verification/private-evaluation/preflight-001
```

v5.6.3支持原生`teaching_request`，默认不转换。`entryProtocol: legacy-chat`只能显式用于其他旧基线；已知v5.6.3提交会拒绝该选项。

## 完整产品三组模拟流程

同一入口支持`mode: fixture`；各entry设置`productExecutable`，分别指向冻结v5.6.3服务exe和候选服务exe。若只跑三组流程，可省略`casesFile`和driver。入口会在新隔离档案中通过真实产品API执行管家偏好保存、建课及备课、读取同份已保存原文快照；产品自身注册全部工具。所有模型HTTP指向入口创建的本机模拟服务，搜索密钥清空，资料检索与抓取请求断言为0。此模式不支持真实资料联网。

教学`fixture`模式另外需要`fixtureUrl`指向明确的本机模拟服务；HTTP兼容重试、发送前预算停止与失败任务恢复可直接运行上述smoke脚本。

## 复用、统计及恢复

- `reuseDir`仅复用完整成功的baseline记录。指纹涵盖案例、模型与地址、偏好、资料数据库及WAL内容、源提交、入口协议、模式、harness、driver和调用限制。缺少源提交、历史无指纹或失败记录均重新执行，不当作通过。
- 总入口在隔离目录维护`budget-current.json`，把每例副本结算传递到下一例，两版共享累计用量与未知预留；复用记录不会二次扣款。原始账本始终只读。中断后以已有各例的`budgetStatus`核对累计检查点，不从原预算重新开始付费阶段。
- 子进程退出后，provider先独立读取预算并在外层回执返回；结果JSON损坏、汇总失败或报告无法写入也不能丢失预留。总入口先原子保存预算再解析结果；预算读取、校验或累计保存失败立即停止后续案例与版本。
- 资料快照同时冻结主数据库及WAL的存在性与哈希，最初不存在的WAL也记录为null；每例前检测新增、删除或变化，避免只核对主数据库而漏掉WAL中已提交的资料变更。
- `httpRequests[].tokens`优先于逻辑调用`usage`，失败请求有用量照计，兼容重试独立计数；缺失用量保持未知。未知请求预留不释放，HTTP未实际开始时保留整笔保守预留。
- SDK故障尝试次数加一次兼容重发全部发送前预留；输入按UTF-8字节、每消息64、输出上限及1024余量估算。这是保守预算额度，不是实际Token预测。
- `queueMs`、`firstContentMs`、`completionMs`从任务读取，按任务ID去重，旧版缺失为null。HTTP的`durationMs`仍是单笔请求，不能相加重复携带的任务指标。
- 同一C++harness记录`externalProcessMs`及`externalElapsedMs`，单调时钟包含重试和动作保存；复用已有终态轮的process耗时为null。完整产品入口记录提交API至终态轮询返回的`externalApiMs`，包含轮询间隔。它们均不能冒充产品内部三项指标或真实速度收益。
- 每教学轮写原生检查点，每完整产品步骤及失败退出保存`result.json`；总入口逐例更新`summary.json`，中断后可检查已落盘结果。教学超时保留已有检查点，并标为未完整完成。失败记录保留诊断，不能复用为通过。

未来真实入口的非密冻结配置格式为`{ "profileName": "GangyiAI-Library-Preview", "baseUrl": "服务地址", "model": "模型名" }`，两版共用同一文件；凭据目标为该profile的`/APIKey`，不打印密钥。当前授权门禁和驱动门禁均关闭真实请求。私有评分材料、配置和运行结果均不得提交或打包。
## Task5 离线候选接入补充

入口回归已接入 CTest 的 `evaluation-entry`（含原17项与原生预算保存失败停止后续版本的新测试），CI 使用公开合成决定，不需要私有 verification 文件。Windows CI另构建开发驱动并执行本机模拟 smoke，覆盖预留写入失败HTTP0、原子替换失败保留旧文件、结算失败保留预留与专用退出码73；Python遇73立即抛BudgetStateError，不把旧有效文件当作新结算。

候选构建命令为 `pwsh -NoProfile -File scripts/build_windows.ps1 -Preview -Candidate`。自动化安装、5.6.3试用版升级和虚构档案备份恢复仅由临时CI runner运行 `scripts/test_windows_release.ps1 -LibraryPreview` 与 `scripts/test_windows_upgrade.ps1`。2026-10-20 按用户明确要求执行了本机原位升级；升级前已复制程序与档案目录、导出设置并记录凭据目标，不读取密钥正文。升级后服务与启动器版本均为 5.6.4、自检通过、数据库哈希未变且桌面入口保留。脚本的自动化CI结果仍单独记录，不能用编译或本机离线测试替代。
