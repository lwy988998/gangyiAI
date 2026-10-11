#include "server_runtime.hpp"
#include <cstdlib>

int main() {
    gangyi::ServerOptions options;
    if (const char* value = std::getenv("GANGYI_MOBILE")) options.mobile = std::string(value) == "1";
    if (const char* value = std::getenv("GANGYI_MOBILE_SESSION")) options.sessionToken = value;
    if (const char* value = std::getenv("GANGYI_PUBLIC_ROOT")) options.publicRoot = value;
    return gangyi::runServer(gangyi::Config::from_environment(), options);
}