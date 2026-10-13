# 教学优化试用验收报告

日期：2026-10-13。基线：v5.6.1（`651e741`）。工作区：`D:\gangyiAI-teaching`。当前交付是“教学优化试用1”，正式发布版本尚未确定。

**实现与本地回归已完成；整体教学效果验收仍未完成。** 当前配置模型 `deepseek-flash` 返回 HTTP 402，服务拒绝继续请求。修正后的版本未完成同模型复测，18个多轮案例尚未全部运行，不能声称整体教学效果已改善。

## 已实现的教学机制

- 评价记录具体卡点、学生回答原话依据、补讲方向及是否需要澄清；引用不在实际回答中时拒绝保存，证据不足时不能形成可信评分。
- 普通讲解、本题提示、完整示范分别记录。帮助核对题目身份、课时、板块版本和实际展示内容；旧对话、未完成回复及未绑定题目的旧示范明确标注不确定性。
- 课堂提供“给一点提示、看示范、再试一道、换一种讲法”。操作作为教学请求，保留草稿和选项，不参与作答评分。
- 新题关联原问题，复练使用自己的帮助历史。报告展示实际回答、帮助、诊断依据和复练结果，并提供继续巩固入口。教学请求、追问与作答分开标明，同一回答合并展示评价。
- AI仍决定教法、题量、难度、复练时机和完成判断。程序校验证据结构、权限、版本及保存状态，评分标准继续保存在私有数据与模型上下文中。

## 本地验证

核心 CTest 共24项，覆盖主控、课堂、画像、流式内容、数据库事务、输入恢复、全局暂停恢复、课程调整与原有接口。教学专项还覆盖：普通讲解不算提示、跨题帮助隔离、小提示拒绝完整示范、追问不评分、诊断引用核对、独立新题关联、报告合并与私有答案过滤。

浏览器检查覆盖原聊天课堂、四种教学请求、草稿与选项保持、刷新恢复、桌面与窄屏、六维雷达、我的AI11个页面、进程重启恢复及原生窗口页面就绪脚本。浏览器使用隔离模型夹具，不收费，也不作为真实教学能力证明。

安装验收检查安装包和免安装包的校验值、资源、启动器自检、未配置模型时的服务、静默安装及卸载、学习数据保留。试用构建使用独立安装标识、注册表、凭据目标和数据目录；首次启动用现有 SQLite 备份机制复制正式档案，并沿用模型设置。正式档案作为只读来源，安卓工作区未改动。

最终构建与安装验收日志保存在本工作区 `verification`，该目录不进入源码提交和安装包。Linux CI、Sanitizer及正式发布流水线未在本轮执行。

## 真实模型对照的实际进展

通过 Promptfoo 0.124.1 的 Python provider 调用实际 C++ 主控，使用隔离数据库、预先编写的回答和评分要求。基线入口复用 v5.6.1 对应源码的构建对象；已核对参考工作区与基线标签没有源码差异。两边沿用当前模型和教学偏好。

首批尝试了数理、语言、知识技能各2个案例，共6个。探索版中，基线完整走完3个案例，初版优化候选完整走完2个；只有分数相加、力与运动两组形成完整配对。其他记录包含服务拒绝或协议失败，不能算验收通过。

首轮发现：模型会漏写帮助类型，导致新记录仍落入“帮助程度未确定”；一条分数诊断的原话引用正确，但反馈中出现了与原题不符的运算数字。随后已把帮助类型加入主控输出格式和板块校验，并向模型明确提供原题快照，同时要求核对题干、学生回答和标准。

修正后的复测中，优化版6个案例均在首次请求遇到 HTTP 402，未产生有效教学回复。复测基线复用了3份完全相同材料、相同模型及偏好的完整记录；另外3个基线请求同样被拒绝。复用记录不计为新增模型请求。扩展的12个案例材料已经准备，服务恢复后可继续完成18个案例。

实际调用与用量，包括前置连通试跑：主控调用模型143次，其中126次取得服务商返回用量，累计 **1,274,042 Token**。17次未取得用量的调用保持未知，不估算为零；其中包括服务拒绝或未完整取得结果的调用。调用数按验收入口逐次记录；本轮没有独立统计SDK内部兼容重试的HTTP次数。金额未从服务商取得，本报告不估算费用。

该对照使用固定学生回答，不能代替真人长期学习成效验证。诊断准确性、帮助适合度、独立复练表现和整体改善结论，仍需在模型服务恢复后检查最终版本的完整记录。

## 可查看的交付材料

- [试用安装包](D:/gangyiAI-teaching/dist/installer/gangyiAI-teaching-preview1-v5.6.1-x64.exe)
- [免安装包](D:/gangyiAI-teaching/dist/installer/gangyiAI-teaching-preview1-v5.6.1-x64.zip)
- [SHA256校验值](D:/gangyiAI-teaching/dist/installer/SHA256SUMS.txt)
- [课堂预览](D:/gangyiAI-teaching/verification/preview/classroom-1440.png)、[窄屏课堂](D:/gangyiAI-teaching/verification/preview/classroom-390.png)、[学习报告预览](D:/gangyiAI-teaching/verification/preview/reports-1440.png)
- [首轮探索对话](D:/gangyiAI-teaching/verification/candidate1/teaching-comparison.html)、[复测及服务拒绝记录](D:/gangyiAI-teaching/verification/teaching-comparison.html)
- [机制与兼容说明](D:/gangyiAI-teaching/docs/teaching-mechanism.md)

页面预览使用当前程序生成的**固定演示档案**，明确标注来源，收费模型请求0次；不把演示分数或截图当作真实教学效果证明。

## 继续验收

在原生 AI 设置或服务商处恢复当前 `deepseek-flash` 服务后，先核实最终版6个代表案例，再扩展到18个。继续记录卡点判断、帮助适合度、复练表现、耗时与实际用量，完成事实及教法审阅后再决定正式发布版本。

```powershell
cd D:\gangyiAI-teaching
.\scripts\build_teaching_eval.ps1 -BuildDir .\build-preview
$env:PROMPTFOO_PYTHON = 'D:\python\python.exe'
$env:PROMPTFOO_DISABLE_TELEMETRY = '1'
.\verification\tools\node_modules\.bin\promptfoo.cmd eval -c verification\stage6.json --no-cache --no-share --max-concurrency 2 -o verification\stage6-results.json verification\stage6-results.html
.\verification\tools\node_modules\.bin\promptfoo.cmd eval -c verification\expand12.json --no-cache --no-share --max-concurrency 2 -o verification\expand12-results.json verification\expand12-results.html
```

实际模型不可用期间，不更换模型冒充同模型对照，不把失败或演示结果记为真实验收通过。
