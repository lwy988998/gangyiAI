#include "db.hpp"
#include <stdexcept>
#include <cstdio>

int main() {
    gangyi::Database db;
    db.open(":memory:");
    db.migrate();
    db.setProfileMeta("atomic-test", "原有版本");
    try {
        db.transaction([&] {
            db.setProfileMeta("atomic-test", "半成品版本");
            throw std::runtime_error("模拟提交失败");
        });
    } catch (const std::runtime_error&) {}
    if (db.profileMeta("atomic-test") != "原有版本") return 1;
    db.transaction([&] { db.setProfileMeta("atomic-test", "完整版本"); });
    if (db.profileMeta("atomic-test") != "完整版本") return 2;
    std::puts("事务提交与异常回滚验收通过");
}
