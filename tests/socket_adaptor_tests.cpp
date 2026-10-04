#include "crow/socket_adaptors.h"
#include <iostream>

int main() {
    try {
        asio::io_context context;
        crow::SocketAdaptor adaptor(context, nullptr);
        // 未连接、已关闭及客户端提前断开的连接都不能抛异常。
        for (int index = 0; index < 100; ++index) {
            if (!adaptor.address().empty()) return 1;
            adaptor.socket().open(crow::tcp::v4());
            if (!adaptor.address().empty()) return 1;
            adaptor.socket().close();
            adaptor.remote_endpoint();
        }
        crow::tcp::acceptor acceptor(context, crow::tcp::endpoint(crow::tcp::v4(), 0));
        adaptor.socket().connect(crow::tcp::endpoint(asio::ip::address_v4::loopback(), acceptor.local_endpoint().port()));
        crow::tcp::socket peer(context); acceptor.accept(peer);
        if (adaptor.address() != "127.0.0.1") return 1;
        peer.close(); adaptor.socket().close();
        if (!adaptor.address().empty()) return 1;
        std::cout << "断开连接的地址读取验收通过\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "连接状态验收失败：" << error.what() << '\n';
        return 1;
    }
}
