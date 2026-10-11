#pragma once
#include <cstdlib>
#include <curl/curl.h>

namespace gangyi {
inline void configureBundledTls(CURL* curl) {
    if (const char* path = std::getenv("GANGYI_CA_BUNDLE")) {
        if (*path) curl_easy_setopt(curl, CURLOPT_CAINFO, path);
    }
}
}
