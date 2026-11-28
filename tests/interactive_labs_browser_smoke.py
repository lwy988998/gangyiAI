"""真实课堂验证JSXGraph、电路及配平的操作、回传、恢复和卸载。"""
from pathlib import Path
import sys
from deep_course_integration_smoke import DeepFixture, main

if __name__ == '__main__':
    DeepFixture.include_labs = True
    DeepFixture.include_sets = True
    main(sys.argv[1],Path(sys.argv[2]).resolve())
