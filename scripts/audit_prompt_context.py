"""只读统计已有验收记录中的提示体积，不调用模型，不输出提示正文。"""

import argparse
import hashlib
import json
from pathlib import Path


def json_bytes(value):
    """使用紧凑JSON计算UTF-8字节数，不能用作实际Token统计。"""
    return len(json.dumps(value, ensure_ascii=False, separators=(",", ":")).encode("utf-8"))


def audit_record(record_path):
    raw = record_path.read_bytes()
    record = json.loads(raw.decode("utf-8-sig"))
    audits = []
    for prompt in record.get("promptAudit", []):
        rows = []
        system_bytes = 0
        for index, message in enumerate(prompt["messages"]):
            content = message["content"]
            size = len(content.encode("utf-8"))
            row = {"index": index, "role": message["role"], "utf8Bytes": size}
            if message["role"] == "system":
                system_bytes += size
            try:
                value = json.loads(content)
                if isinstance(value, dict):
                    row["fieldUtf8Bytes"] = {key: json_bytes(item) for key, item in value.items()}
            except json.JSONDecodeError:
                pass
            marker = "可用工具："
            if message["role"] == "system" and marker in content:
                try:
                    catalog, _ = json.JSONDecoder().raw_decode(content.split(marker, 1)[1])
                    if isinstance(catalog, list):
                        row["catalogCount"] = len(catalog)
                        row["catalogUtf8Bytes"] = json_bytes(catalog)
                except json.JSONDecodeError:
                    pass
            rows.append(row)
        total = sum(row["utf8Bytes"] for row in rows)
        audits.append({
            "messageCount": len(rows), "inputContentUtf8Bytes": total,
            "reportedInputBytes": prompt.get("inputBytes"),
            "systemUtf8Bytes": system_bytes,
            "systemSharePercent": round(100 * system_bytes / total, 2) if total else None,
            "messages": rows,
        })
    return {
        "recordSha256": hashlib.sha256(raw).hexdigest(), "prompts": audits,
        "说明": "仅统计已有记录，不发送模型请求。字节数不是Token，不代表平均成本或优化收益。",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("record", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.record.resolve() == args.output.resolve():
        parser.error("输出路径不能覆盖原始记录")
    report = audit_record(args.record)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"promptCount": len(report["prompts"]), "modelRequestsAdded": 0}))


if __name__ == "__main__":
    main()
