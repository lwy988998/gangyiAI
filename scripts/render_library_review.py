"""将隔离验收的实际记录整理为可阅读报告，不生成模拟课堂截图。"""

import argparse
import html
import json
from pathlib import Path


def escaped(value):
    return html.escape(str(value))


def record(path):
    return json.loads(path.read_text(encoding="utf-8"))


def render(folder, output):
    rows = []
    details = []
    labels = {"math": "数学", "physics": "理科", "language": "语言", "programming": "编程", "timeliness": "版本时效"}
    for case_id, label in labels.items():
        cells = []
        for variant, name in [("baseline", "v5.6.1基线"), ("candidate", "资料增强版")]:
            data = record(folder / f"{case_id}-{variant}" / "result.json")
            task = data["turns"][-1]["task"]
            requests = data["httpRequests"]
            tokens = sum((item.get("tokens") or {}).get("total_tokens", 0) for item in requests)
            unknown = sum(item.get("tokens") is None for item in requests)
            status = "等待作答" if task["status"] == "waiting_student" else "未完成"
            cells.append(f"{status}<br>{len(requests)}次请求 · {tokens:,}已知Token" + (f"<br>{unknown}次未提供用量" if unknown else ""))
            answer = task.get("message", "") or "本轮没有有效公开回复。"
            questions = [section for section in data["lesson"]["sections"] if section.get("kind") == "question" and section.get("createdAt")]
            question_html = "".join(
                "<div class='question'><b>实际保存的新题</b><pre>" + escaped(item["question"]["question"]) +
                "</pre><details><summary>开发验收私有标准（学生页面不展示）</summary><pre>" +
                escaped(item["question"].get("expectedAnswer", "")) + "\n" + escaped(item["question"].get("rubric", "")) + "</pre></details></div>"
                for item in questions
            )
            relative = f"{case_id}-{variant}/result.json"
            details.append(f"<article><h3>{label} · {name}</h3><p class='meta'>{cells[-1]}</p><pre>{escaped(answer)}</pre>{question_html}<p><a href='{relative}'>查看完整原始记录</a></p></article>")
        rows.append(f"<tr><th>{label}</th><td>{cells[0]}</td><td>{cells[1]}</td></tr>")
    extra = []
    for name, label in [("language-candidate-reviewed", "英语更换全文来源后的补充复核"), ("language-final-retry", "英语最终协议重试"), ("language-current-final", "最新合并版英语讲解与新题"), ("programming-baseline-retry", "编程基线原任务重试（未完成）"), ("timeliness-baseline-retry", "时效基线原任务重试（未完成）"), ("../personalized-teaching-advanced", "进阶编程讲解（有限适配证据）")]:
        path = folder / name / "result.json"
        if not path.exists():
            continue
        value = record(path)
        task = value.get("task") or value["turns"][-1]["task"]
        requests = value.get("newHttpRequests", value.get("httpRequests", []))
        tokens = sum((item.get("tokens") or {}).get("total_tokens", 0) for item in requests)
        unknown = sum(item.get("tokens") is None for item in requests)
        usage = f"{len(requests)}次请求 · {tokens:,}已知Token · {unknown}次未提供用量"
        extra.append(f"<article><h3>{label}</h3><p>任务状态：{escaped(task['status'])} · {usage}</p><pre>{escaped(task.get('message', ''))}</pre><a href='{name}/result.json'>原始记录</a></article>")
    page = """<!doctype html><html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>资料与教学整合 · 真实验收记录</title><style>
body{margin:0;background:#131719;color:#e8ecea;font:16px/1.65 system-ui,"Microsoft YaHei",sans-serif}main{max-width:1080px;margin:auto;padding:40px 24px}h1{font-size:30px;line-height:1.3}h2{margin-top:40px}h3{font-size:19px}a{color:#9dceb7}table{width:100%;border-collapse:collapse}th,td{text-align:left;padding:16px;border-bottom:1px solid #35413b;vertical-align:top}thead{background:#252d28}article{padding:22px;background:#1c2420;border:1px solid #35413b;border-radius:12px;margin:18px 0}.notice{padding:18px;background:#352c1e;border:1px solid #735e35;border-radius:10px}pre{font:inherit;white-space:pre-wrap;overflow-wrap:anywhere;margin:16px 0}.meta{color:#afbfb6}.question{border-top:1px solid #3a4a40;padding-top:14px}summary{cursor:pointer;color:#c1d5c8}@media(max-width:640px){main{padding:24px 14px}th,td{padding:9px;font-size:14px}h1{font-size:25px}}
</style><main><p class="meta">2026-10-14 · Windows v5.6.1基线 · 当前模型 deepseek-flash</p>
<h1>资料与教学整合验收记录</h1><p>这是实际验收记录的阅读页面，包含原始回复与私有标准；不是项目界面的截图或学生档案。</p>
<div class="notice">五领域均已尝试真实模型对照。两个基线案例未完成，英语来源与协议另行复核；没有据此宣布整体教学效果改善。每例是一次教学请求，不能替代教学专项6/18案例的完整多轮验收。</div>
<h2>实际请求与用量</h2><table><thead><tr><th>领域</th><th>v5.6.1基线</th><th>资料增强版</th></tr></thead><tbody>""" + "".join(rows) + """</tbody></table><p class="meta">Token只使用服务商回执，未提供的部分不估算。总体20次HTTP请求、133,338已知Token；请求较少不代表总Token降低。</p><h2>实际讲解与题目</h2>""" + "".join(details) + "<h2>补充复核</h2>" + "".join(extra) + "</main></html>"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(page, encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="生成资料教学的真实验收记录页面")
    parser.add_argument("folder", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    render(args.folder, args.output)
