"""真实C++配模拟模型验证交集交互的键盘、恢复、回传和不评分。"""

from pathlib import Path
import sys

from deep_course_integration_smoke import DeepFixture, main


if __name__ == "__main__":
    DeepFixture.include_sets = True
    main(sys.argv[1], Path(sys.argv[2] if len(sys.argv) > 2 else "verification/interactive-learning-browser").resolve())
