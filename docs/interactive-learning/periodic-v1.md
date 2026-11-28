# 元素周期表与中性原子探索 v1

`periodic_lab` 提供 118 种元素的选择、两个元素比较及同周期/同族高亮。界面保留可读的元素符号和原子序数；窄屏在表内横向滚动，两个选择框可以直接定位任意元素。表内左右方向键按原子序数移动，上下键选择同一显示列的相邻元素，Home/End 定位当前显示行端点。

数据来自 [PubChem 官方周期表接口](https://pubchem.ncbi.nlm.nih.gov/rest/pug/periodictable/JSON)，2026-11-28 获取，原始响应与 SHA-256 位于 `verification/periodic-source-20261128/`。发布资料只保留原子序数、符号、英文名称，以及前 20 号中性原子由该响应电子组态提取的电子层计数；没有导入原子量、未经核对的性质或图片。英文元素名称沿用 PubChem 的用法。

布局采用 18 列的周期表形式，参照 [OpenStax Chemistry 2e 2.5](https://openstax.org/books/chemistry-2e/pages/2-5-the-periodic-table)。镧系 57—71、锕系 89—103 单列，这些成员的 `group` 为 `null`；该实现不裁定第三族成员归属，也不把系列整体判为同族。选择系列元素并按族高亮时仅标记当前元素，比较的同族结论保持未知。

- 对中性原子，质子数和电子数均取原子序数。没有指定具体同位素，中子数为 `null`；不能把相对原子质量直接四舍五入当成质量数。
- 前 20 号的电子层图只表示基态中性原子的分层计数；圆圈不是电子的真实轨道，也不表示真实比例。电子组态和电子层的概念参照 [OpenStax Chemistry 2e 6.4](https://openstax.org/books/chemistry-2e/pages/6-4-electronic-structure-of-atoms-electron-configurations)。20 号以后只显示已注册的元素事实及中性粒子数，不外推简化填层规则。
- 元素事实位于 `public/periodic-elements-data.js` 唯一 JSON 数据块，浏览器直接读取。CMake 校验标记、JSON、118 个连续原子序数并将同一块嵌入 C++ 程序，缺失或顺序错误会拒绝构建；没有两份手工维护的元素名单。数据版本绑定状态，旧资料版本不能覆盖当前探索。
- 学生状态只含协议/课件/资料版本、`selectedZ`、`comparisonZ` 和 `highlight`；服务端重新获取事实、计算比较与高亮，不接受自报电子层、质子数、元素名称或成绩。探索接入原课堂摘要、详细轨迹及草稿恢复。
- 不提供离子、同位素、分子、化学键、反应预测或未知元素的性质推断；点击和比较不等于掌握，也不自动评分。真实模型选配与专家/学生教学验收仍独立进行。

配置见 `interactive-labs-v1.schema.json` 的 `periodic_lab` 分支；状态为 `interactive-periodic-state-v1.schema.json`。默认比较 Na 与 Cl，不强制所有化学课使用同一示例，模型可配置任意已注册元素。
