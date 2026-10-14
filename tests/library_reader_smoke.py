"""核验隔离正文读取、日期、字幕发现和PDF页码，不访问模型服务。"""
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def pdf_bytes(content):
    objects = ['<< /Type /Catalog /Pages 2 0 R >>', '<< /Type /Pages /Kids [3 0 R] /Count 1 >>',
               '<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Resources << /Font << /F1 4 0 R >> >> /Contents 5 0 R >>',
               '<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>',
               f'<< /Length {len(content)} >>\nstream\n{content}\nendstream']
    data = b'%PDF-1.4\n'
    offsets = []
    for index, obj in enumerate(objects, 1):
        offsets.append(len(data))
        data += f'{index} 0 obj\n{obj}\nendobj\n'.encode()
    start = len(data)
    data += b'xref\n0 6\n0000000000 65535 f \n'
    data += b''.join(f'{offset:010} 00000 n \n'.encode() for offset in offsets)
    return data + f'trailer\n<< /Size 6 /Root 1 0 R >>\nstartxref\n{start}\n%%EOF'.encode()


reader = Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='gangyi-reader-check-') as folder:
    root = Path(folder)
    def parse(body, mime):
        (root/'document.bin').write_bytes(body)
        (root/'input.json').write_text(json.dumps({'url': 'https://example.org/book', 'mime': mime}), encoding='utf-8')
        subprocess.run([str(reader), str(root/'input.json'), str(root/'output.json')], timeout=35, check=True,
                       creationflags=subprocess.CREATE_NO_WINDOW, capture_output=True)
        return json.loads((root/'output.json').read_text(encoding='utf-8'))

    formula = '<math><semantics><mfrac><mn>1</mn><mn>2</mn></mfrac><annotation encoding="application/x-tex">\\frac{1}{2}</annotation></semantics></math>'
    article = '<html><head><title>正文提取</title><meta property="article:published_time" content="2020-01-02"><meta property="article:modified_time" content="2024-05-06"></head><body><article><h1>分数相加</h1><p>' + '先通分、相加分子、再约分。'*100 + formula + '<math><mfrac><mn>1</mn><mn>3</mn></mfrac></math></p><dl><dt>list.append(x)</dt><dd><p>加入一个元素。</p></dd></dl><div><span>用更早的过去动作解释过去完成时。</span></div><table><tr><td>had</td><td>worked</td></tr></table><track kind="captions" src="/subtitles.vtt" srclang="zh"><script>throw Error("外部脚本绝不能执行")</script></article></body></html>'
    parsed = parse(article.encode(), 'text/html; charset=utf-8')
    assert parsed['status'] == 'read' and parsed['publishedAt'] == '2020-01-02' and parsed['sourceUpdatedAt'] == '2024-05-06'
    assert parsed['segments'][0]['id'] == 's1' and '外部脚本绝不能执行' not in json.dumps(parsed['segments'], ensure_ascii=False)
    assert parsed['tracks'][0]['url'] == 'https://example.org/subtitles.vtt'
    body = ' '.join(s['text'] for s in parsed['segments'])
    assert '\\frac{1}{2}' in body and '(1)/(3)' in body and 'list.append(x)' in body
    assert '用更早的过去动作解释过去完成时。' in body and 'had | worked' in body
    text_pdf = parse(pdf_bytes('BT /F1 18 Tf 72 700 Td (Evidence page one.) Tj ET'), 'application/pdf')
    assert text_pdf['status'] == 'read' and text_pdf['segments'][0]['page'] == 1 and 'Evidence page one' in text_pdf['segments'][0]['text']
    assert parse(pdf_bytes(''), 'application/pdf')['status'] == 'unreadable'
    assert parse(b'%PDF-invalid', 'application/pdf')['status'] == 'unreadable'
print('隔离网页、日期、字幕链接、PDF页码及不可读提示检查通过')
