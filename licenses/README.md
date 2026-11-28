# Windows 随包依赖通知

此目录保留当前学习增强试用版所使用的依赖通知原文。`notices-manifest.json` 记录来源、归档版本、导入字节摘要及统一 UTF-8 换行后的文本摘要。打包时核对文本摘要，复制文件原字节，并把实际写入的摘要记录到 `dependency-inventory.json`。

| 依赖 | 当前归档来源 | 随包位置 |
| --- | --- | --- |
| Crow | 1.3.0，构建脚本固定版本 | `licenses/crow/LICENSE` |
| nlohmann/json | 3.11.3 | `licenses/nlohmann-json/LICENSE.MIT` |
| Asio | 1.28.2 | `licenses/asio/LICENSE_1_0.txt` |
| webview | 0.12.0 构建依赖目录的原始通知 | `licenses/webview/LICENSE` |
| WebView2 SDK 通知 | 当前本地头文件依赖目录；SDK 精确版本和来源仍需冻结 | `licenses/webview2/LICENSE.txt` |
| SQLite | 3.53.4 原始头文件开头的版权放弃说明 | `licenses/sqlite/NOTICE.txt` |
| curl 及包内依赖通知 | curl 官方 Windows 8.22.0_1 发行包 | `licenses/curl/` |
| GCC 运行库许可与运行库例外 | GCC 16.1.0 上游源码的 COPYING3、COPYING.RUNTIME | `licenses/gcc-16.1.0/` |
| MinGW-w64 与 winpthreads 通知 | MinGW-w64 14.0.0 上游源码 | `licenses/mingw-w64-14.0.0/` |
| PDFium 及其依赖 | 原有固定 chromium/8100 归档的 LICENSE 和 licenses 全目录 | `licenses/pdfium/` |
| KaTeX | npm 官方 0.16.11 包，已有 JS、CSS 和 20 份字体逐项核对；CSS 只有末尾换行差异 | `public/vendor/katex/LICENSE`、`SOURCE.json` |
| JSXGraph、ECharts、Readability、PDF.js 及字体 | 原有随包通知，保持归档内容 | 各自的 `public/vendor/` 子目录 |

`dependency-inventory.json` 同时记录本次实际 GCC 版本、目标平台以及三个程序和五份 DLL 的字节数与 SHA-256，不包含本机用户名、绝对构建路径或密钥。源码包保留静态通知和打包脚本，实际构建回执由对应构建生成。

本通知归档以本机 GCC 16.1.0 / MinGW-w64 14.0.0 试用构建为基线。现有 CI 使用 GCC 13.2.0，构建回执会明确记录差异。正式候选仍需冻结各构建的完整依赖版本、WebView2 SDK 来源以及对应源码取得材料；当前不把通知归档等同于最终依赖与许可审查完成。元素事实和教材等资料另有来源边界，不包含在本文件的库通知范围内。
