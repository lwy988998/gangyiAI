"""真实C++服务与固定模型验证本课教学意图，不使用付费请求或真实学生档案。"""
from pathlib import Path
import sys
from deep_course_integration_smoke import DeepFixture, main

if __name__ == '__main__':
    DeepFixture.include_intents = True
    DeepFixture.include_sets = True
    DeepFixture.include_mastery = True
    main(sys.argv[1], Path(sys.argv[2]).resolve())
