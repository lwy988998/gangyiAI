// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <cstdlib>，为后续实现提供类型或接口声明。
#include <cstdlib>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <memory>，为后续实现提供类型或接口声明。
#include <memory>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明数据结构或类型 Tracked 及其成员接口。
struct Tracked : std::enable_shared_from_this<Tracked> {
    // 【逐行批注】 执行当前声明或表达式：static std::atomic<int> freed;。
    static std::atomic<int> freed;
    // 【逐行批注】 调用 Tracked 完成本行所需操作：~Tracked() { ++freed; }。
    ~Tracked() { ++freed; }
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 执行当前声明或表达式：std::atomic<int> Tracked::freed{0};。
std::atomic<int> Tracked::freed{0};

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 根引用始终存活；复制、弱引用锁定与 shared_from_this 不能提前析构对象。
    // 【逐行批注】 按循环规则重复处理：for (int round = 0; round < 40; ++round) {。
    for (int round = 0; round < 40; ++round) {
        // 【逐行批注】 计算右侧结果并写入 root，更新当前运行状态。
        auto root = std::make_shared<Tracked>();
        // 【逐行批注】 执行当前声明或表达式：std::weak_ptr<Tracked> weak = root;。
        std::weak_ptr<Tracked> weak = root;
        // 【逐行批注】 执行当前声明或表达式：std::vector<std::thread> workers;。
        std::vector<std::thread> workers;
        // 【逐行批注】 按循环规则重复处理：for (int index = 0; index < 8; ++index) workers.emplace_back([root, weak] {。
        for (int index = 0; index < 8; ++index) workers.emplace_back([root, weak] {
            // 【逐行批注】 按循环规则重复处理：for (int count = 0; count < 6000; ++count) {。
            for (int count = 0; count < 6000; ++count) {
                // 【逐行批注】 计算右侧结果并写入 copy，更新当前运行状态。
                auto copy = root;
                // 【逐行批注】 计算右侧结果并写入 locked，更新当前运行状态。
                auto locked = weak.lock();
                // 【逐行批注】 计算右侧结果并写入 self，更新当前运行状态。
                auto self = copy->shared_from_this();
                // 【逐行批注】 判断条件 (!locked || self.get() != root.get()) std::abort();，成立时执行对应分支。
                if (!locked || self.get() != root.get()) std::abort();
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 按循环规则重复处理：for (auto& worker : workers) worker.join();。
        for (auto& worker : workers) worker.join();
        // 【逐行批注】 判断条件 (root.use_count() != 1 || Tracked::freed != round) {，成立时执行对应分支。
        if (root.use_count() != 1 || Tracked::freed != round) {
            // 【逐行批注】 执行当前声明或表达式：std::cerr << "多线程共享指针计数损坏。\n";。
            std::cerr << "多线程共享指针计数损坏。\n";
            // 【逐行批注】 结束当前函数并返回结果：return 1;。
            return 1;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 执行当前声明或表达式：root.reset();。
        root.reset();
        // 【逐行批注】 判断条件 (!weak.expired() || Tracked::freed != round + 1) {，成立时执行对应分支。
        if (!weak.expired() || Tracked::freed != round + 1) {
            // 【逐行批注】 执行当前声明或表达式：std::cerr << "共享指针未按最后一个引用释放对象。\n";。
            std::cerr << "共享指针未按最后一个引用释放对象。\n";
            // 【逐行批注】 结束当前函数并返回结果：return 1;。
            return 1;
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 执行当前声明或表达式：std::cout << "多线程共享指针计数、弱引用锁定与最终析构通过。\n";。
    std::cout << "多线程共享指针计数、弱引用锁定与最终析构通过。\n";
    // 【逐行批注】 结束当前函数并返回结果：return 0;。
    return 0;
// 【逐行批注】 结束当前代码块。
}
