# 文档导航

当前产品功能以 [项目首页](../README.md) 和实际发布源码为准。历史文档保留当时的预算、失败与未完成结论，不代表最新状态；本机私有验收记录未上传，也不作为可公开下载的附件。

## 使用与界面

- [项目介绍、Windows 安装与两种课堂](../README.md)
- [我的 AI、十二类页面与档案渠道](MY_AI.md)
- [AI 状态、暂停恢复与请求统计](AI调用监控.md)
- [产品范围](../PRODUCT.md)、[澄明视觉系统](../DESIGN.md)、[界面构图规范原件](../.impeccable/surfaces/public-clarity-css.md)
- [界面与教学参考项目](design-references.md)

## 架构与协议

- [真实 AI 教学主控](ai-controller.md)
- [深度课程协议（deep-v1）](deep-course-protocol.md)
- [原课堂与页面协议](v5.6.0-ai-classroom.md)：快速规划继续使用的课堂基础。
- [调用复用、缓存与统计](ai-call-reuse.md)
- [诊断、帮助与独立复练机制](teaching-mechanism.md)、[实际展示证据边界](teaching-display-evidence.md)：保留协议演进背景，现行行为同时参照主控协议与源码。

## 构建与发布验收

- [Windows 构建入口](../scripts/build_windows.ps1)
- [Windows 发布前人工检查](windows-release-checklist.md)
- [v5.6.4 同协议离线对照入口](evaluation-v564.md)：特定历史基线与隔离模拟流程，预算数字按该阶段解读。
- [GitHub Actions](https://github.com/lwy988998/gangyiAI/actions/workflows/ci.yml)：CI 临时包保留 14 天，正式 Release 附件保留供下载与回退。

## 正式版本与候选记录

正式说明用于追溯对应版本，不会因文档整理删除。候选记录保留原路径，供构建脚本读取；候选中的未完成结论按当时阶段解读。

| 版本 | 正式说明 | 候选阶段记录 |
| --- | --- | --- |
| v5.7.0 | [澄明界面与深度课程](releases/v5.7.0.md) | — |
| v5.6.7 | [准确性、等价解与传输整改](releases/v5.6.7.md) | [候选记录](releases/v5.6.7-candidate.md) |
| v5.6.6 | [调用复用与后续验收](releases/v5.6.6.md) | [追加预算前候选](releases/v5.6.6-candidate.md) |
| v5.6.5 | [版本说明](releases/v5.6.5.md) | — |
| v5.6.4 | [版本说明](releases/v5.6.4.md) | [离线候选](releases/v5.6.4-candidate.md) |
| v5.6.3 | [版本说明](releases/v5.6.3.md) | — |
| v5.6.1 | [我的 AI 与历史专项验收](releases/v5.6.1.md) | — |
| v5.6.0 | [原界面与统一课堂](releases/v5.6.0.md) | — |

公开下载入口为 [GitHub Releases](https://github.com/lwy988998/gangyiAI/releases)。最新主分支包含 v5.7.0 发布后的导航修复，原 v5.7.0 标签和安装附件不包含该修复；具体差异见项目首页。

## 历史调研与验收

以下记录在原路径归档保留，方便追溯链接。历史累计预算、剩余额度、安装状态和模型配置只描述记录所列日期，不作为当前授权或当前验收结论。

| 阶段 | 文档 | 阅读范围 |
| --- | --- | --- |
| 2026-10-14 教学优化试用 | [教学验收与目标核对矩阵](teaching-acceptance.md) | 含真实失败、未知用量和未完成的 6／18 案例目标。 |
| 2026-10-14 调用减量 | [测量与本机更新记录](token-optimization.md) | 区分输入字节、实际 Token、费用与教学效果。 |
| 2026-10-17 资料与教学整合 | [整合验收](library-acceptance.md)、[资料实施记录](library-implementation.md) | 保留资料来源、安装记录、预算停止和未完成边界。 |
| 2026-10-17 调用效率调研 | [调研与实施记录](ai-call-efficiency-research.md) | 依赖与接口判断按调研日期解读，不作为新的默认配置。 |
| v5.6.7 准确性整改 | [整改过程与最终补充](teaching-rectification.md) | 早期传输约束与失败记录保留，现行结构恢复规则参照主控协议。 |
