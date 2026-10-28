---
name: "钢一定制 AI · 澄明"
description: "以蓝白、留白和清晰层级承载学习内容的中文学习工作台。"
colors:
  primary: "#3d61ce"
  primary-hover: "#2f50b8"
  neutral-bg: "#f6f8fc"
  surface: "#fff"
  surface-alt: "#eef3fc"
  text: "#21324b"
  muted: "#5e6d82"
  border: "#dfe6ef"
  danger: "#a4373e"
  danger-bg: "#fff3f2"
  warning: "#78541e"
  warning-bg: "#fff7ea"
typography:
  display:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "30px"
    fontWeight: 650
    lineHeight: 1.4
    letterSpacing: "-0.025em"
  headline:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "24px"
    fontWeight: 650
    lineHeight: 1.45
  title:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "20px"
    fontWeight: 600
    lineHeight: 1.5
  body:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "15px"
    fontWeight: 400
    lineHeight: 1.7
  reading:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "15px"
    fontWeight: 400
    lineHeight: 1.95
  navigation:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "14px"
    fontWeight: 400
    lineHeight: 1.7
  action:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "13px"
    fontWeight: 600
    lineHeight: 1.7
  label:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "12px"
    fontWeight: 400
    lineHeight: 1.7
  slide-title:
    fontFamily: '"Segoe UI", "Microsoft YaHei UI", "Microsoft YaHei", sans-serif'
    fontSize: "27px"
    fontWeight: 600
    lineHeight: 1.5
    letterSpacing: "-0.02em"
rounded:
  control: "7px"
  input: "9px"
  compact-panel: "10px"
  panel: "12px"
  home-panel: "14px"
spacing:
  xs: "4px"
  sm: "8px"
  control: "12px"
  md: "16px"
  panel: "22px"
  lg: "24px"
  xl: "28px"
  section: "32px"
  wide: "36px"
components:
  button-primary:
    backgroundColor: "{colors.primary}"
    textColor: "{colors.surface}"
    typography: "{typography.action}"
    rounded: "{rounded.control}"
    padding: "9px 15px"
  button-primary-hover:
    backgroundColor: "{colors.primary-hover}"
    textColor: "{colors.surface}"
  button-secondary:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.text}"
    rounded: "{rounded.control}"
    padding: "8px 13px"
  button-text:
    backgroundColor: "transparent"
    textColor: "{colors.muted}"
    padding: "5px 2px"
  goal-input:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.text}"
    typography: "{typography.body}"
    rounded: "{rounded.input}"
    padding: "15px 16px 10px"
  top-navigation:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.muted}"
    typography: "{typography.navigation}"
    height: "72px"
    padding: "0 36px"
  mode-chip:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.muted}"
    typography: "{typography.label}"
    rounded: "{rounded.control}"
    padding: "9px 11px"
  mode-chip-selected:
    backgroundColor: "{colors.surface-alt}"
    textColor: "{colors.primary}"
  card-standard:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.text}"
    typography: "{typography.body}"
    rounded: "{rounded.panel}"
    padding: "22px"
  more-menu:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.text}"
    typography: "{typography.action}"
    rounded: "{rounded.input}"
    padding: "7px"
    width: "180px"
  teaching-timeline:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.text}"
    rounded: "{rounded.panel}"
    padding: "18px"
  lesson-slide:
    backgroundColor: "{colors.surface-alt}"
    textColor: "{colors.text}"
    rounded: "{rounded.panel}"
    padding: "29px 32px"
---

# Design System: 钢一定制 AI · 澄明

## Overview

**Creative North Star: "澄明阅览室"**

蓝白、留白、顶部导航和清楚的字体层级来自用户已确认的 A「澄明」方向。浅灰蓝背景托住白色内容面，蓝色标记当前位置与主要行动；学校校徽保持原始比例，作为稳定的身份入口。界面面向中文学习，视觉密度让位于阅读、作答和继续学习。

首页、课程管理、快速规划、深度课程、资料与知识库，以及“我的 AI”十二类页面共享这套视觉体系。列表以细分隔线组织，确有边界的内容、表单与媒体才使用容器。课堂的次要工具和 AI 调用详情按需展开，保留学习内容与输入区的可见空间。

**Key Characteristics:**

- 蓝白与中性浅色构成主体，蓝色服务于导航、选择和行动。
- 中文系统字体、可读正文和有节制的标题层级。
- 平面内容、轻边框、适量圆角与充足留白。
- 桌面与窄屏共享校徽、学习入口、课程目录和清楚的焦点。
- 真实进度与状态进入原有组件，部分失败提供就地恢复入口。

本文件提取自 `public/clarity.css` 和 `public/deep-course.css`，行为参照对应 renderer 与脚本。前置令牌为机器可读记录；圆角和间距是已使用的 CSS 常量归纳，不表示源码已经定义同名变量。颜色在源码使用 `--gy-*`；深度课堂的 `--dc-*` 继承相应 `--gy-*`，二者保持同一视觉来源。产品范围见 [PRODUCT.md](PRODUCT.md)，页面构图见 [界面整改简报](docs/ui-clarity-brief.md)。

## Colors

清楚的钴蓝与冷白构成主要色彩，深蓝灰提供正文对比，低饱和浅蓝区分相邻区域。颜色值以文件前置令牌为准。sidecar 的八阶色带为规范面板的派生预览，不扩展源码调色板。

### Primary

- **澄明蓝**（`primary`，源码 `--gy-accent`）：主按钮、当前导航、选中标签、进度、时间轴节点和教学图线。
- **沉静蓝**（`primary-hover`，源码 `--gy-accent-hover`）：首页主按钮的悬停色。深度课堂按钮沿用该蓝色家族中的实际局部悬停值，不新增第二套品牌色。

### Neutral

- **冷白底**（`neutral-bg`，源码 `--gy-bg`）：全局画布。
- **内容白**（`surface`，源码 `--gy-surface`）：导航、表单、阅读区与主要内容容器。
- **浅蓝层**（`surface-alt`，源码 `--gy-surface-alt`）：选中状态、用户消息、代码块及课件背景。
- **深蓝灰**（`text`，源码 `--gy-text`）：正文、标题与主要数值。
- **辅助蓝灰**（`muted`，源码 `--gy-muted`）：上下文、日期、来源、占位说明和次要操作。
- **分隔灰蓝**（`border`，源码 `--gy-border`）：分隔线与一像素控件边界。

错误与危险操作使用 `danger`／`danger-bg`，确认或注意事项使用 `warning`／`warning-bg`。这些状态色已有对应中文说明；状态不只靠颜色传达。教学图中的多条受力线可以采用其实际语义色，不能据此扩大品牌主色。

**The 蓝色有义 Rule.** 蓝色标记可行动、已选中或当前学习位置，同一状态同时保留文字、边线或形状线索。

## Typography

**Display Font / Body Font:** Windows 中文系统字体栈，以 Segoe UI 承担可用的拉丁字形，依次回退到 Microsoft YaHei UI、Microsoft YaHei 和系统无衬线。源码使用等宽数字，便于比较进度和统计。

**Label/Mono Font:** 辅助文字沿用正文系统字体；深度讲义的代码采用 Consolas、Cascadia Code、monospace。

### Hierarchy

| 用途 | 已实现层级与使用方式 |
| --- | --- |
| 首页主标题 | `display`；桌面（30px），窄屏（25px）。 |
| 快速课堂标题 | `headline`；桌面（24px），窄屏及矮窗口（21px）。 |
| 通用区域标题 | `title`；主要为（20px），小区域标题使用（17–19px）。 |
| 正文与消息 | `body`；常规（15px），快速课堂与聊天长文行高（1.9）。 |
| 深度讲义 | `reading`；桌面（15px）、行高（1.95），窄屏（14px）；阅读列最大宽度（76ch）。 |
| 翻页课件 | `slide-title`；桌面标题（27px）、正文（17px）；窄屏标题（23px）、正文（15px）。 |
| 导航与操作 | 导航通常（14px），紧凑操作（12–13px）；选中或主要操作用中等字重。 |
| 辅助信息 | `label`；说明、来源、日期、保存状态以（12–13px）呈现。 |

“我的 AI”页面主标题为（28px），窄屏为（25px）；深度课堂标题为（23px），随窗口逐步减小。现有雷达图数值刻度使用（11px），仅承担图形坐标；维度、操作和说明均保留（12px）以上。新增正文与辅助信息不沿用该图形刻度尺寸。

**The 阅读优先 Rule.** 主体以（15px）为基准；窄屏通过重排和隐藏冗余说明保留空间，辅助正文与操作文字保持至少（12px）。

## Layout

顶部导航是稳定的全局入口，桌面标称高度（72px），主站窄屏标称（64px）；脚本测量实际头部高度，课堂使用剩余动态视口。桌面导航内宽上限（1440px），左右内距（36px）。校徽桌面（36px）、窄屏（32px），保持原图比例。“我的 AI”在窄屏保留品牌行和第二行横向导航，实际头部高度由测量值决定。

首页容器上限（1240px），桌面左右内距（40px）；学习入口和继续课程以（1.42:1）的双栏关系并列，下方最近课程与画像沿用该对齐关系。一般课程与规划容器上限（1160px）。列表优先用行距、分隔线和标题区别信息，避免每条信息重复包裹容器。

快速课堂中间内容独立滚动，底部输入区参与纵向布局并保持可见；目录默认宽（230px），收起后内容扩展。深度课堂采用左侧目录、主阅读区与可收起右侧答疑：通常目录（218px），展开答疑时为（200px）目录和（304px）答疑。主阅读区使用 `minmax(0,1fr)`，正文、表格、公式与图片在自身区域内换行或滚动。

“我的 AI”桌面侧栏（205px），内容区上限（1200px）。概览、AI 导师、资料与知识库、学习报告、学习计划、偏好、记忆、任务、用量、画像、设置和数据管理沿用共同的导航、表单和列表样式。

### 响应行为

- **窗口收窄**：全局在（1180px／1000px）逐步缩小边距和导航间隔；深度课堂在（1150px）压缩目录与答疑栏。
- **课程目录**：全局课程在（900px）以内使用可展开目录；深度课堂在同等宽度优先为阅读与答疑留空间。
- **个人中心**：在（780px）以内，十二类侧栏改为横向可滚动入口，内容转为单栏。
- **窄屏**：在（700px）以内，首页转为纵向布局、主站导航收为菜单；深度课程目录与答疑使用独立对话框入口，阅读区占满可用宽度。
- **矮窗口**：快速课堂在高度（690px）以内减少目的说明与标题占位，保留内容滚动区和输入区。
- **宽窗口**：深度课堂在（1500px）以上适度增加目录、答疑栏和阅读内距，不无限拉长讲义行宽。

主要验收尺寸为（1280×720），对应 Windows 主场景的（150%）缩放；同时核对（1440×900）与（390×844）。页面允许纵向滚动，课堂输入区与当前主要操作留在可用视口，窄屏保留校徽和课程目录入口。

## Elevation & Depth

静态内容以平面、浅色差和细边框构成层次，主要卡片、首页表单与课堂输入框不加投影。阴影用于悬浮菜单、AI 状态详情和弹出入口，帮助说明它们覆盖在当前内容之上。对话框使用半透明遮罩区分前后层。

### Shadow Vocabulary

- **导航菜单**（`0 8px 24px #21324b18`）：窄屏导航的局部浮层。
- **课堂更多菜单**（`0 8px 28px #1c34511a`）：快速课堂的原生弹出工具菜单。
- **深度课堂菜单**（`0 8px 24px #24395914`）：深度课堂次要工具浮层。
- **AI 状态详情**（`0 10px 35px #1f36531c`）：头部展开的状态详情。

**The 平面承载 Rule.** 学习内容与常驻表单保持平面；覆盖内容的浮层才使用投影或遮罩建立前后关系。

交互色彩过渡通常为（0.18s），不通过抬升、放大或持续装饰动画争夺注意。系统请求减少动态效果时，停用界面过渡、动画和流畅滚动。

## Shapes

常规操作控件采用轻圆角（7px），复合输入框采用（9px），紧凑面板采用（10px），内容和教学媒体容器采用（12px），首页主要容器采用（14px）。边界通常为一像素细线。菜单项使用更小圆角以嵌入浮层；时间轴节点与图形数据点使用圆形。

校徽本身不裁切、不额外包裹装饰圆形。列表行依靠对齐和分隔线保持整洁，圆角并非所有信息的默认外框。

## Components

### Buttons

主按钮使用澄明蓝、白字与轻圆角；首页主操作最小高度（40px），常规课程与个人中心按钮通常（38px），深度课堂紧凑按钮通常（36px）。次要按钮使用白底、正文色和边框；提示、重讲等低频操作收进“更多”。

悬停改变背景或文字色；键盘焦点采用蓝色外描边（2px），全局偏移（4px），深度课堂偏移（3px）。禁用按钮降低不透明度至（0.55），并使用等待光标。主要按钮、停止和重试仅在相应业务状态显示，文案描述当前行动。

### Chips

模式选择使用低饱和浅蓝选中背景、蓝字与边线。首页快速规划／深度课程按钮保留 `aria-pressed`；页签保留 `aria-selected`；当前目录与导航保留 `aria-current`。课程状态标签使用同一套浅色与辅助字号，不以标签数量取代内容层级。

### Cards / Containers

通用内容容器使用内容白、细边框与（12px）圆角，个人中心典型内距（22px）。首页继续学习区以浅蓝底区分，但保持与相邻表单对齐。最近课程、快捷入口等重复内容主要采用信息行；教学媒体容器独立包裹图片、图解及说明。

### Inputs / Fields

表单使用白底、深蓝灰正文和浅色边界。目标输入框让整块输入面承担边界；取得焦点时边线转蓝。普通控件保留可见键盘外描边；组合输入的 textarea 沿用容器焦点边线与光标提示。多行输入高度有限制，课堂中的长内容优先在消息区滚动。

错误、空状态与保存状态就地使用简短中文说明；危险操作与确认信息使用对应语义色和边框。部分媒体失败保留可用讲义，并在失败项旁提供重试入口。

### Navigation

全局导航以校徽与产品名开头，桌面链接水平排列；当前页面以蓝字、底部细线和字重区分。“我的 AI”二级页面由同套浅色侧栏承载，窄屏改为横向入口。移动主菜单使用原生 `details`，快速课堂更多工具使用原生 popover，深度课堂窄屏目录和答疑使用对话框。关闭后返回触发入口，避免遮挡常态输入区。

AI 调用详情默认折叠在头部。展开后在头部下方显示，保留暂停、恢复与重试等既有操作；调用细节不常驻覆盖课堂底部。

### 讲义、课件与教学媒体

深度课堂在同一主区切换讲义与翻页课件。讲义保留适合中文阅读的行高，课件以浅蓝整面和较大标题形成页感；左右翻页按钮与页码固定在视图工具栏下方，长正文独立滚动。答疑栏共享消息、来源和输入的视觉语言，可按需收起。

时间轴采用日期列、细竖线和蓝色节点；流程图使用浅蓝节点；函数和受力图采用清晰坐标、曲线、读数及滑块。媒体说明和联网图片来源以辅助字号显示。已生成、加载中、空内容、局部失败和重试均在所属内容区呈现，不要求每节课同时出现所有媒体形式。

## Do's and Don'ts

### Do:

- **Do** 沿用澄明蓝、白色内容面和共享中性色，让首页、两类课堂及个人中心保持一致。
- **Do** 保留原始校徽比例、中文说明、真实进度与已有功能入口。
- **Do** 以（15px）正文和至少（12px）的辅助信息建立层级，并为长讲义保留舒展行高。
- **Do** 在（1280×720）与窄屏核对输入区、目录入口、导航和键盘焦点。
- **Do** 用细边框、间距和按需展开组织信息，把停止、失败说明和重试放在相关状态中。

### Don't:

- **Don't** 在共享页面重新引入黑绿、玻璃网格或大面积装饰背景，冲淡已选定的蓝白方向。
- **Don't** 将每条列表信息变成独立高阴影卡片，或用密集按钮挤压阅读和作答空间。
- **Don't** 缩小辅助正文来容纳更多信息，或仅以颜色表示选中、错误和学习状态。
- **Don't** 让弹层、状态详情或目录遮挡常态课堂输入，或让宽表格与公式撑破整页。
- **Don't** 把资料库、旧课程或深度课程扩展成另一套不兼容的颜色、导航与控件样式。
