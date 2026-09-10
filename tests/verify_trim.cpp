#include <iostream>
#include <string>

int main() {
    std::string large(30000, 'x');
    std::string req = R"({"previousBlocks":{"steps":{"lessonSteps":[{"explanation":")" + large + R"("}]}}})";
    std::cout << "Original size: " << req.size() << std::endl;
    if (req.size() < 20000) {
        std::cout << "PASS: previousBlocks trimmed\n";
        return 0;
    }
    std::cerr << "FAIL: previousBlocks not trimmed\n";
    return 1;
}
