# 【逐行批注】 执行当前 Python 语句："""核验隔离正文读取、日期、字幕发现和PDF页码，不访问模型服务。"""。
"""核验隔离正文读取、日期、字幕发现和PDF页码，不访问模型服务。"""
# 【逐行批注】 导入后续逻辑需要的模块或符号：import json。
import json
# 【逐行批注】 导入后续逻辑需要的模块或符号：from pathlib import Path。
from pathlib import Path
# 【逐行批注】 导入后续逻辑需要的模块或符号：import subprocess。
import subprocess
# 【逐行批注】 导入后续逻辑需要的模块或符号：import sys。
import sys
# 【逐行批注】 导入后续逻辑需要的模块或符号：import tempfile。
import tempfile


# 【逐行批注】 定义函数 pdf_bytes，封装可复用的处理步骤。
def pdf_bytes(content):
    # 【逐行批注】 计算右侧结果并赋给 objects，更新脚本状态。
    objects = ['<< /Type /Catalog /Pages 2 0 R >>', '<< /Type /Pages /Kids [3 0 R] /Count 1 >>',
               # 【逐行批注】 执行当前 Python 语句：'<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Resources << /Font << /F1 4 0 R >>…。
               '<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Resources << /Font << /F1 4 0 R >> >> /Contents 5 0 R >>',
               # 【逐行批注】 执行当前 Python 语句：'<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>',。
               '<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>',
               # 【逐行批注】 调用 len 完成当前脚本或测试步骤：f'<< /Length {len(content)} >>\nstream\n{content}\nendstream']。
               f'<< /Length {len(content)} >>\nstream\n{content}\nendstream']
    # 【逐行批注】 计算右侧结果并赋给 data，更新脚本状态。
    data = b'%PDF-1.4\n'
    # 【逐行批注】 计算右侧结果并赋给 offsets，更新脚本状态。
    offsets = []
    # 【逐行批注】 遍历数据并重复执行循环体：for index, obj in enumerate(objects, 1):。
    for index, obj in enumerate(objects, 1):
        # 【逐行批注】 调用 len 完成当前脚本或测试步骤：offsets.append(len(data))。
        offsets.append(len(data))
        # 【逐行批注】 计算右侧结果并赋给 data +，更新脚本状态。
        data += f'{index} 0 obj\n{obj}\nendobj\n'.encode()
    # 【逐行批注】 计算右侧结果并赋给 start，更新脚本状态。
    start = len(data)
    # 【逐行批注】 计算右侧结果并赋给 data +，更新脚本状态。
    data += b'xref\n0 6\n0000000000 65535 f \n'
    # 【逐行批注】 计算右侧结果并赋给 data +，更新脚本状态。
    data += b''.join(f'{offset:010} 00000 n \n'.encode() for offset in offsets)
    # 【逐行批注】 结束当前函数并返回结果：return data + f'trailer\n<< /Size 6 /Root 1 0 R >>\nstartxref\n{start…。
    return data + f'trailer\n<< /Size 6 /Root 1 0 R >>\nstartxref\n{start}\n%%EOF'.encode()


# 【逐行批注】 计算右侧结果并赋给 reader，更新脚本状态。
reader = Path(sys.argv[1]).resolve()
# 【逐行批注】 在受控上下文中执行资源操作：with tempfile.TemporaryDirectory(prefix='gangyi-reader-check-') as folder:。
with tempfile.TemporaryDirectory(prefix='gangyi-reader-check-') as folder:
    # 【逐行批注】 计算右侧结果并赋给 root，更新脚本状态。
    root = Path(folder)
    # 【逐行批注】 定义函数 parse，封装可复用的处理步骤。
    def parse(body, mime):
        # 【逐行批注】 执行当前 Python 语句：(root/'document.bin').write_bytes(body)。
        (root/'document.bin').write_bytes(body)
        # 【逐行批注】 计算右侧结果并赋给 (root/'input.json').write_text(json.dumps({'…，更新脚本状态。
        (root/'input.json').write_text(json.dumps({'url': 'https://example.org/book', 'mime': mime}), encoding='utf-8')
        # 【逐行批注】 计算右侧结果并赋给 subprocess.run([str(reader), str(root/'input…，更新脚本状态。
        subprocess.run([str(reader), str(root/'input.json'), str(root/'output.json')], timeout=35, check=True,
                       # 【逐行批注】 计算右侧结果并赋给 creationflags，更新脚本状态。
                       creationflags=subprocess.CREATE_NO_WINDOW, capture_output=True)
        # 【逐行批注】 结束当前函数并返回结果：return json.loads((root/'output.json').read_text(encoding='utf-8'))。
        return json.loads((root/'output.json').read_text(encoding='utf-8'))

    # 【逐行批注】 计算右侧结果并赋给 formula，更新脚本状态。
    formula = '<math><semantics><mfrac><mn>1</mn><mn>2</mn></mfrac><annotation encoding="application/x-tex">\\frac{1}{2}</annotation></semantics></math>'
    # 【逐行批注】 计算右侧结果并赋给 article，更新脚本状态。
    article = '<html><head><title>正文提取</title><meta property="article:published_time" content="2020-01-02"><meta property="article:modified_time" content="2024-05-06"></head><body><article><h1>分数相加</h1><p>' + '先通分、相加分子、再约分。'*100 + formula + '<math><mfrac><mn>1</mn><mn>3</mn></mfrac></math></p><dl><dt>list.append(x)</dt><dd><p>加入一个元素。</p></dd></dl><div><span>用更早的过去动作解释过去完成时。</span></div><table><tr><td>had</td><td>worked</td></tr></table><track kind="captions" src="/subtitles.vtt" srclang="zh"><script>throw Error("外部脚本绝不能执行")</script></article></body></html>'
    # 【逐行批注】 计算右侧结果并赋给 parsed，更新脚本状态。
    parsed = parse(article.encode(), 'text/html; charset=utf-8')
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parsed['status'] == 'read' and parsed['publishedAt'] == '2020-01-02' and…。
    assert parsed['status'] == 'read' and parsed['publishedAt'] == '2020-01-02' and parsed['sourceUpdatedAt'] == '2024-05-06'
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parsed['segments'][0]['id'] == 's1' and '外部脚本绝不能执行' not in json.dumps(pa…。
    assert parsed['segments'][0]['id'] == 's1' and '外部脚本绝不能执行' not in json.dumps(parsed['segments'], ensure_ascii=False)
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parsed['tracks'][0]['url'] == 'https://example.org/subtitles.vtt'。
    assert parsed['tracks'][0]['url'] == 'https://example.org/subtitles.vtt'
    # 【逐行批注】 计算右侧结果并赋给 body，更新脚本状态。
    body = ' '.join(s['text'] for s in parsed['segments'])
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '\\frac{1}{2}' in body and '(1)/(3)' in body and 'list.append(x)' in body。
    assert '\\frac{1}{2}' in body and '(1)/(3)' in body and 'list.append(x)' in body
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert '用更早的过去动作解释过去完成时。' in body and 'had | worked' in body。
    assert '用更早的过去动作解释过去完成时。' in body and 'had | worked' in body
    # 【逐行批注】 计算右侧结果并赋给 text_pdf，更新脚本状态。
    text_pdf = parse(pdf_bytes('BT /F1 18 Tf 72 700 Td (Evidence page one.) Tj ET'), 'application/pdf')
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert text_pdf['status'] == 'read' and text_pdf['segments'][0]['page'] == 1 an…。
    assert text_pdf['status'] == 'read' and text_pdf['segments'][0]['page'] == 1 and 'Evidence page one' in text_pdf['segments'][0]['text']
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parse(pdf_bytes(''), 'application/pdf')['status'] == 'unreadable'。
    assert parse(pdf_bytes(''), 'application/pdf')['status'] == 'unreadable'
    # 【逐行批注】 校验关键条件，测试失败时立即报错：assert parse(b'%PDF-invalid', 'application/pdf')['status'] == 'unreadable'。
    assert parse(b'%PDF-invalid', 'application/pdf')['status'] == 'unreadable'
# 【逐行批注】 输出当前运行结果：print('隔离网页、日期、字幕链接、PDF页码及不可读提示检查通过')。
print('隔离网页、日期、字幕链接、PDF页码及不可读提示检查通过')
