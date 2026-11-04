# 【逐行批注】 执行当前 Python 语句："""只读统计已有验收记录中的提示体积，不调用模型，不输出提示正文。"""。
"""只读统计已有验收记录中的提示体积，不调用模型，不输出提示正文。"""

# 【逐行批注】 导入后续逻辑需要的模块或符号：import argparse。
import argparse
# 【逐行批注】 导入后续逻辑需要的模块或符号：import hashlib。
import hashlib
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path


# 【逐行批注】 定义函数 json_bytes，封装可复用的处理步骤。
def json_bytes(value):
    # 【逐行批注】 执行当前 Python 语句："""使用紧凑JSON计算UTF-8字节数，不能用作实际Token统计。"""。
    """使用紧凑JSON计算UTF-8字节数，不能用作实际Token统计。"""
    # 【逐行批注】 结束当前函数并返回结果：return len(json.dumps(value, ensure_ascii=False, separators=(",", ":"…。
    return len(json.dumps(value, ensure_ascii=False, separators=(",", ":")).encode("utf-8"))


# 【逐行批注】 定义函数 audit_record，封装可复用的处理步骤。
def audit_record(record_path):
    # 【逐行批注】 计算右侧结果并赋给 raw，更新脚本状态。
    raw = record_path.read_bytes()
    # 【逐行批注】 计算右侧结果并赋给 record，更新脚本状态。
    record = json.loads(raw.decode("utf-8-sig"))
    # 【逐行批注】 计算右侧结果并赋给 audits，更新脚本状态。
    audits = []
    # 【逐行批注】 遍历数据并重复执行循环体：for prompt in record.get("promptAudit", []):。
    for prompt in record.get("promptAudit", []):
        # 【逐行批注】 计算右侧结果并赋给 rows，更新脚本状态。
        rows = []
        # 【逐行批注】 计算右侧结果并赋给 system_bytes，更新脚本状态。
        system_bytes = 0
        # 【逐行批注】 遍历数据并重复执行循环体：for index, message in enumerate(prompt["messages"]):。
        for index, message in enumerate(prompt["messages"]):
            # 【逐行批注】 计算右侧结果并赋给 content，更新脚本状态。
            content = message["content"]
            # 【逐行批注】 计算右侧结果并赋给 size，更新脚本状态。
            size = len(content.encode("utf-8"))
            # 【逐行批注】 计算右侧结果并赋给 row，更新脚本状态。
            row = {"index": index, "role": message["role"], "utf8Bytes": size}
            # 【逐行批注】 判断条件 message["role"] == "system":，成立时执行对应分支。
            if message["role"] == "system":
                # 【逐行批注】 计算右侧结果并赋给 system_bytes +，更新脚本状态。
                system_bytes += size
            # 【逐行批注】 开始可能抛出异常的保护代码块。
            try:
                # 【逐行批注】 计算右侧结果并赋给 value，更新脚本状态。
                value = json.loads(content)
                # 【逐行批注】 判断条件 isinstance(value, dict):，成立时执行对应分支。
                if isinstance(value, dict):
                    # 【逐行批注】 计算右侧结果并赋给 row["fieldUtf8Bytes"]，更新脚本状态。
                    row["fieldUtf8Bytes"] = {key: json_bytes(item) for key, item in value.items()}
            # 【逐行批注】 捕获并处理异常：except json.JSONDecodeError:。
            except json.JSONDecodeError:
                # 【逐行批注】 执行当前 Python 语句：pass。
                pass
            # 【逐行批注】 计算右侧结果并赋给 marker，更新脚本状态。
            marker = "可用工具："
            # 【逐行批注】 判断条件 message["role"] == "system" and marker in content:，成立时执行对应分支。
            if message["role"] == "system" and marker in content:
                # 【逐行批注】 开始可能抛出异常的保护代码块。
                try:
                    # 【逐行批注】 计算右侧结果并赋给 catalog, _，更新脚本状态。
                    catalog, _ = json.JSONDecoder().raw_decode(content.split(marker, 1)[1])
                    # 【逐行批注】 判断条件 isinstance(catalog, list):，成立时执行对应分支。
                    if isinstance(catalog, list):
                        # 【逐行批注】 计算右侧结果并赋给 row["catalogCount"]，更新脚本状态。
                        row["catalogCount"] = len(catalog)
                        # 【逐行批注】 计算右侧结果并赋给 row["catalogUtf8Bytes"]，更新脚本状态。
                        row["catalogUtf8Bytes"] = json_bytes(catalog)
                # 【逐行批注】 捕获并处理异常：except json.JSONDecodeError:。
                except json.JSONDecodeError:
                    # 【逐行批注】 执行当前 Python 语句：pass。
                    pass
            # 【逐行批注】 执行当前 Python 语句：rows.append(row)。
            rows.append(row)
        # 【逐行批注】 计算右侧结果并赋给 total，更新脚本状态。
        total = sum(row["utf8Bytes"] for row in rows)
        # 【逐行批注】 执行当前 Python 语句：audits.append({。
        audits.append({
            # 【逐行批注】 调用 len 完成当前脚本或测试步骤："messageCount": len(rows), "inputContentUtf8Bytes": total,。
            "messageCount": len(rows), "inputContentUtf8Bytes": total,
            # 【逐行批注】 执行当前 Python 语句："reportedInputBytes": prompt.get("inputBytes"),。
            "reportedInputBytes": prompt.get("inputBytes"),
            # 【逐行批注】 执行当前 Python 语句："systemUtf8Bytes": system_bytes,。
            "systemUtf8Bytes": system_bytes,
            # 【逐行批注】 调用 round 完成当前脚本或测试步骤："systemSharePercent": round(100 * system_bytes / total, 2) if total e…。
            "systemSharePercent": round(100 * system_bytes / total, 2) if total else None,
            # 【逐行批注】 执行当前 Python 语句："messages": rows,。
            "messages": rows,
        # 【逐行批注】 执行当前 Python 语句：})。
        })
    # 【逐行批注】 结束当前函数并返回结果：return {。
    return {
        # 【逐行批注】 执行当前 Python 语句："recordSha256": hashlib.sha256(raw).hexdigest(), "prompts": audits,。
        "recordSha256": hashlib.sha256(raw).hexdigest(), "prompts": audits,
        # 【逐行批注】 执行当前 Python 语句："说明": "仅统计已有记录，不发送模型请求。字节数不是Token，不代表平均成本或优化收益。",。
        "说明": "仅统计已有记录，不发送模型请求。字节数不是Token，不代表平均成本或优化收益。",
    # 【逐行批注】 执行当前 Python 语句：}。
    }


# 【逐行批注】 定义函数 main，封装可复用的处理步骤。
def main():
    # 【逐行批注】 计算右侧结果并赋给 parser，更新脚本状态。
    parser = argparse.ArgumentParser(description=__doc__)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument("record", type，更新脚本状态。
    parser.add_argument("record", type=Path)
    # 【逐行批注】 计算右侧结果并赋给 parser.add_argument("--output", type，更新脚本状态。
    parser.add_argument("--output", type=Path, required=True)
    # 【逐行批注】 计算右侧结果并赋给 args，更新脚本状态。
    args = parser.parse_args()
    # 【逐行批注】 判断条件 args.record.resolve() == args.output.resolve():，成立时执行对应分支。
    if args.record.resolve() == args.output.resolve():
        # 【逐行批注】 执行当前 Python 语句：parser.error("输出路径不能覆盖原始记录")。
        parser.error("输出路径不能覆盖原始记录")
    # 【逐行批注】 计算右侧结果并赋给 report，更新脚本状态。
    report = audit_record(args.record)
    # 【逐行批注】 计算右侧结果并赋给 args.output.parent.mkdir(parents，更新脚本状态。
    args.output.parent.mkdir(parents=True, exist_ok=True)
    # 【逐行批注】 计算右侧结果并赋给 args.output.write_text(json.dumps(report, en…，更新脚本状态。
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    # 【逐行批注】 输出当前运行结果：print(json.dumps({"promptCount": len(report["prompts"]), "modelRequestsAdd…。
    print(json.dumps({"promptCount": len(report["prompts"]), "modelRequestsAdded": 0}))


# 【逐行批注】 判断条件 __name__ == "__main__":，成立时执行对应分支。
if __name__ == "__main__":
    # 【逐行批注】 调用 main 完成当前脚本或测试步骤：main()。
    main()
