#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

struct Tracked : std::enable_shared_from_this<Tracked> {
    static std::atomic<int> freed;
    ~Tracked() { ++freed; }
};
std::atomic<int> Tracked::freed{0};

int main() {
    // 根引用始终存活；复制、弱引用锁定与 shared_from_this 不能提前析构对象。
    for (int round = 0; round < 40; ++round) {
        auto root = std::make_shared<Tracked>();
        std::weak_ptr<Tracked> weak = root;
        std::vector<std::thread> workers;
        for (int index = 0; index < 8; ++index) workers.emplace_back([root, weak] {
            for (int count = 0; count < 6000; ++count) {
                auto copy = root;
                auto locked = weak.lock();
                auto self = copy->shared_from_this();
                if (!locked || self.get() != root.get()) std::abort();
            }
        });
        for (auto& worker : workers) worker.join();
        if (root.use_count() != 1 || Tracked::freed != round) {
            std::cerr << "多线程共享指针计数损坏。\n";
            return 1;
        }
        root.reset();
        if (!weak.expired() || Tracked::freed != round + 1) {
            std::cerr << "共享指针未按最后一个引用释放对象。\n";
            return 1;
        }
    }
    std::cout << "多线程共享指针计数、弱引用锁定与最终析构通过。\n";
    return 0;
}
