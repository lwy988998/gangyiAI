"""真实服务配固定模型验证理解状态、原评价保留、文本安全及课时隔离。"""

from pathlib import Path
import sys

from deep_course_integration_smoke import DeepFixture, main


if __name__ == "__main__":
    DeepFixture.include_mastery = True
    DeepFixture.include_sets = True
    main(sys.argv[1], Path(sys.argv[2] if len(sys.argv) > 2 else "verification/learning-state-browser").resolve())
