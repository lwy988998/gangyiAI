"""在隔离 CMake 项目中核对共享元素数据的嵌入与构建拒绝规则。"""

import json
from pathlib import Path
import re
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'public/periodic-elements-data.js'
MODULE = ROOT / 'cmake/PeriodicElements.cmake'


class PeriodicDataBuildTests(unittest.TestCase):
    def configure(self, script):
        with tempfile.TemporaryDirectory(prefix='gangyi-periodic-build-') as temporary:
            project = Path(temporary)
            (project / 'public').mkdir()
            (project / 'public/periodic-elements-data.js').write_text(script, encoding='utf-8')
            (project / 'CMakeLists.txt').write_text(
                'cmake_minimum_required(VERSION 3.20)\nproject(元素资料检查 NONE)\n'
                f'include("{MODULE.as_posix()}")\n', encoding='utf-8')
            completed = subprocess.run(['cmake', '-S', str(project), '-B', str(project / 'build')],
                                       capture_output=True, text=True, encoding='utf-8', errors='replace')
            header = project / 'build/generated/periodic_elements_data.hpp'
            return completed.returncode, completed.stdout + completed.stderr, header.read_text(encoding='utf-8') if header.exists() else None

    def test_same_json_reaches_cpp_without_rewriting_facts(self):
        script = SOURCE.read_text(encoding='utf-8')
        code, output, header = self.configure(script)
        self.assertEqual(code, 0, output)
        canonical = json.loads(script.split('/* GY_ELEMENTS_JSON_BEGIN */')[1].split('/* GY_ELEMENTS_JSON_END */')[0])
        embedded = json.loads(re.search(r'R"GY_ELEMENTS\((.*)\)GY_ELEMENTS"', header, re.S)[1])
        self.assertEqual(embedded, canonical)

    def test_missing_marker_or_incomplete_table_is_rejected(self):
        script = SOURCE.read_text(encoding='utf-8')
        begin, body = script.split('/* GY_ELEMENTS_JSON_BEGIN */')
        raw, end = body.split('/* GY_ELEMENTS_JSON_END */')
        data = json.loads(raw)
        cases = [script.replace('GY_ELEMENTS_JSON_END', 'MISSING_END')]
        data['elements'].pop()
        cases.append(begin + '/* GY_ELEMENTS_JSON_BEGIN */' + json.dumps(data) + '/* GY_ELEMENTS_JSON_END */' + end)
        data = json.loads(raw); data['elements'][1]['atomicNumber'] = 3
        cases.append(begin + '/* GY_ELEMENTS_JSON_BEGIN */' + json.dumps(data) + '/* GY_ELEMENTS_JSON_END */' + end)
        for index, invalid in enumerate(cases):
            with self.subTest(index=index):
                code, _, _ = self.configure(invalid)
                self.assertNotEqual(code, 0)


if __name__ == '__main__':
    unittest.main()
