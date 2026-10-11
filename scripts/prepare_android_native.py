"""准备固定版本的安卓原生依赖；下载、缓存和构建均在指定目录内。"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import urllib.request
import zipfile

ARCHIVES = {
    "boringssl": ("https://codeload.github.com/google/boringssl/zip/3fed4f207543470b3d639db6ed40737e30308b51", "8f846e1471a4eee97a9edcf390ea1983598a6c8939bec74e981791a502725a93"),
    "curl": ("https://curl.se/download/curl-8.22.0.tar.xz", "f7ef3ae8a22e521f289803fe93543eb64c329b58aa73a9e224dfd915a2a5f4f7"),
    "crow": ("https://codeload.github.com/CrowCpp/Crow/zip/537b44d3d3faa88095885c876aad2059d3315e93", "33c5153529429a383b0d829e1a47d5c3c9bc050f4d1a94bb6c18da24d8ca46bc"),
    "json": ("https://codeload.github.com/nlohmann/json/zip/refs/tags/v3.11.3", "04022b05d806eb5ff73023c280b68697d12b93e1b7267a0b22a1a39ec7578069"),
    "asio": ("https://codeload.github.com/chriskohlhoff/asio/zip/refs/tags/asio-1-28-2", "417536ed4d31645b550d6c475a952544d1aab30a04d7228e6ce6ca935bf2dc1b"),
    "sqlite": ("https://www.sqlite.org/2026/sqlite-amalgamation-3530400.zip", "1e71ddf93849c6a6ecf58b827c0692073d2dd7ee40196158068f7b29f422e87d"),
}

def sources(root):
    downloads = root / "downloads"
    downloads.mkdir(parents=True, exist_ok=True)
    for name, (url, expected) in ARCHIVES.items():
        archive = downloads / (name + ".archive")
        if not archive.exists():
            with urllib.request.urlopen(url, timeout=120) as response, archive.open("wb") as stream:
                shutil.copyfileobj(response, stream)
        actual = hashlib.sha256(archive.read_bytes()).hexdigest()
        if actual != expected:
            raise RuntimeError("依赖校验失败：" + name)
        destination = root / name
        if destination.exists():
            continue
        temporary = root / (name + "-extract")
        temporary.mkdir(exist_ok=True)
        if zipfile.is_zipfile(archive):
            with zipfile.ZipFile(archive) as package:
                for item in package.infolist():
                    if not (temporary / item.filename).resolve().is_relative_to(temporary.resolve()):
                        raise RuntimeError("依赖包含越界路径")
                package.extractall(temporary)
        else:
            with tarfile.open(archive) as package:
                package.extractall(temporary, filter="data")
        folders = list(temporary.iterdir())
        if len(folders) != 1 or not folders[0].is_dir():
            raise RuntimeError("依赖压缩包结构异常：" + name)
        shutil.move(str(folders[0]), destination)
        temporary.rmdir()
    assets = root / "assets"
    assets.mkdir(exist_ok=True)
    repo = Path(__file__).resolve().parent.parent
    shutil.copyfile(repo / "android/assets/curl-ca-bundle.crt", assets / "curl-ca-bundle.crt")
    # 将实际使用的许可一并装入 APK，避免发布源码测试夹具。
    notices = assets / "licenses"
    notices.mkdir(exist_ok=True)
    for name in ARCHIVES:
        for pattern in ("LICENSE", "LICENSE.txt", "COPYING"):
            license_file = root / name / pattern
            if license_file.exists():
                shutil.copyfile(license_file, notices / (name + ".txt"))
                break

def build(root, sdk, abi, jobs):
    executable = ".exe" if os.name == "nt" else ""
    cmake = sdk / "cmake/3.22.1/bin" / ("cmake" + executable)
    ninja = sdk / "cmake/3.22.1/bin" / ("ninja" + executable)
    ndk = sdk / "ndk/28.2.13676358"
    prefix = root / "install" / abi
    prefix.joinpath("lib").mkdir(parents=True, exist_ok=True)
    common = ["-G", "Ninja", "-DCMAKE_MAKE_PROGRAM=" + str(ninja),
              "-DCMAKE_TOOLCHAIN_FILE=" + str(ndk / "build/cmake/android.toolchain.cmake"),
              "-DANDROID_ABI=" + abi, "-DANDROID_PLATFORM=android-29", "-DANDROID_STL=c++_shared",
              "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
              "-DCMAKE_INSTALL_PREFIX=" + str(prefix)]
    log_path = root / ("build-" + abi + ".log")
    def run(arguments):
        with log_path.open("a", encoding="utf-8") as log:
            outcome = subprocess.run([str(cmake), *arguments], stdout=log, stderr=subprocess.STDOUT)
        if outcome.returncode:
            raise RuntimeError("原生构建失败，见 " + str(log_path))
    ssl_build = root / "build" / abi / "boringssl"
    if not (prefix / "lib/libssl.a").exists():
        run(["-S", str(root / "boringssl"), "-B", str(ssl_build), *common,
             "-DBUILD_TESTING=OFF", "-DOPENSSL_NO_ASM=ON"])
        run(["--build", str(ssl_build), "--parallel", str(jobs), "--target", "ssl", "crypto"])
        for name in ("ssl", "crypto"):
            candidates = list(ssl_build.rglob("lib" + name + ".a"))
            if len(candidates) != 1:
                raise RuntimeError("无法确定 TLS 库位置")
            shutil.copyfile(candidates[0], prefix / ("lib/lib" + name + ".a"))
        shutil.copytree(root / "boringssl/include", prefix / "include", dirs_exist_ok=True)
    curl_build = root / "build" / abi / "curl"
    if not (prefix / "lib/libcurl.a").exists():
        run(["-S", str(root / "curl"), "-B", str(curl_build), *common,
             "-DBUILD_SHARED_LIBS=OFF", "-DBUILD_CURL_EXE=OFF", "-DBUILD_TESTING=OFF",
             "-DCURL_USE_OPENSSL=ON", "-DCURL_USE_LIBPSL=OFF", "-DCURL_USE_LIBSSH2=OFF",
             "-DCURL_USE_PKGCONFIG=OFF", "-DCURL_BROTLI=OFF", "-DCURL_ZSTD=OFF", "-DUSE_NGHTTP2=OFF",
             "-DHTTP_ONLY=ON", "-DCURL_DISABLE_LDAP=ON", "-DCURL_DISABLE_LDAPS=ON",
             "-DOPENSSL_ROOT_DIR=" + str(prefix), "-DOPENSSL_INCLUDE_DIR=" + str(prefix / "include"),
             "-DOPENSSL_SSL_LIBRARY=" + str(prefix / "lib/libssl.a"),
             "-DOPENSSL_CRYPTO_LIBRARY=" + str(prefix / "lib/libcrypto.a")])
        run(["--build", str(curl_build), "--parallel", str(jobs), "--target", "libcurl_static"])
        candidates = list(curl_build.rglob("libcurl.a"))
        if len(candidates) != 1:
            raise RuntimeError("无法确定 libcurl 位置")
        shutil.copyfile(candidates[0], prefix / "lib/libcurl.a")
    print("已准备原生依赖：" + abi, flush=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--abi", choices=("arm64-v8a", "x86_64"), default="arm64-v8a")
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    sources(args.root.resolve())
    build(args.root.resolve(), args.sdk.resolve(), args.abi, args.jobs)
