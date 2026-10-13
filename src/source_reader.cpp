#include <windows.h>
#include <webview/webview.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
namespace {
using Json = nlohmann::json;
std::string read(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("资料文件无法读取");
    std::ostringstream out; out << file.rdbuf(); return out.str();
}
void write(const std::filesystem::path& path, const Json& value) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc); file << value.dump(-1, ' ', false, Json::error_handler_t::replace);
}
std::string utf8(const wchar_t* text, int length) {
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, length, nullptr, 0, nullptr, nullptr);
    std::string out(size, '\0'); WideCharToMultiByte(CP_UTF8, 0, text, length, out.data(), size, nullptr, nullptr); return out;
}
std::string decode(const std::string& bytes, const std::string& mime) {
    auto meta = mime + bytes.substr(0, 1600);
    for (auto& ch : meta) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    const auto encoding = meta.find("gbk") != std::string::npos || meta.find("gb2312") != std::string::npos ? 936U :
        meta.find("big5") != std::string::npos ? 950U : CP_UTF8;
    const int size = MultiByteToWideChar(encoding, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    std::wstring text(size, L'\0'); MultiByteToWideChar(encoding, 0, bytes.data(), static_cast<int>(bytes.size()), text.data(), size);
    return utf8(text.data(), size);
}
template<typename Function> Function symbol(HMODULE module, const char* name) {
    const auto address = GetProcAddress(module, name);
    if (!address) throw std::runtime_error("PDF读取组件不完整");
    return reinterpret_cast<Function>(address);
}
Json pdf(const std::string& bytes, const std::filesystem::path& root) {
    struct DLL { HMODULE value; ~DLL() { if (value) FreeLibrary(value); } } dll{LoadLibraryW((root / L"pdfium.dll").c_str())};
    if (!dll.value) throw std::runtime_error("PDF读取组件无法载入");
    const auto initialize = symbol<void(*)()>(dll.value, "FPDF_InitLibrary");
    const auto destroy = symbol<void(*)()>(dll.value, "FPDF_DestroyLibrary");
    const auto open = symbol<void*(*)(const void*, size_t, const char*)>(dll.value, "FPDF_LoadMemDocument64");
    const auto close = symbol<void(*)(void*)>(dll.value, "FPDF_CloseDocument");
    const auto pageCount = symbol<int(*)(void*)>(dll.value, "FPDF_GetPageCount");
    const auto pageOpen = symbol<void*(*)(void*, int)>(dll.value, "FPDF_LoadPage");
    const auto pageClose = symbol<void(*)(void*)>(dll.value, "FPDF_ClosePage");
    const auto textOpen = symbol<void*(*)(void*)>(dll.value, "FPDFText_LoadPage");
    const auto textClose = symbol<void(*)(void*)>(dll.value, "FPDFText_ClosePage");
    const auto charCount = symbol<int(*)(void*)>(dll.value, "FPDFText_CountChars");
    const auto getText = symbol<int(*)(void*, int, int, unsigned short*)>(dll.value, "FPDFText_GetText");
    initialize();
    struct Library { decltype(destroy) cleanup; ~Library() { cleanup(); } } library{destroy};
    const auto document = open(bytes.data(), bytes.size(), nullptr);
    if (!document) return {{"status", "unreadable"}, {"kind", "pdf"}, {"reason", "PDF损坏、加密或无法读取"}, {"segments", Json::array()}};
    struct Handle { void* value; void(*close)(void*); ~Handle() { if (value) close(value); } } documentHandle{document, close};
    Json result = {{"status", "read"}, {"kind", "pdf"}, {"segments", Json::array()}, {"pages", pageCount(document)},
        {"coverage", "已提取PDF文字；图像、扫描页和无法正确提取的公式不作为已核验内容"}};
    size_t total = 0; int emptyPages = 0;
    const int count = pageCount(document);
    for (int index = 0; index < count; ++index) {
        Handle page{pageOpen(document, index), pageClose};
        Handle text{page.value ? textOpen(page.value) : nullptr, textClose};
        const int length = text.value ? charCount(text.value) : 0;
        if (length <= 0) { ++emptyPages; continue; }
        if (length > 1000000 || total > 6 * 1024 * 1024 || index >= 1500) {
            result["status"] = "partial"; result["reason"] = "资料较大，仅读取已列出的页码范围"; break;
        }
        std::vector<unsigned short> buffer(static_cast<size_t>(length) + 1);
        const auto readLength = getText(text.value, 0, length, buffer.data());
        if (readLength <= 1) { ++emptyPages; continue; }
        const auto body = utf8(reinterpret_cast<const wchar_t*>(buffer.data()), readLength - 1);
        // 按页内段落拆分，保存引用的位置，不把整页文字塞进一次模型请求。
        std::istringstream lines(body); std::string line, paragraph; int paragraphId = 0;
        const auto append = [&] {
            if (paragraph.empty()) return;
            result["segments"].push_back({{"id", "p" + std::to_string(index + 1) + "-" + std::to_string(++paragraphId)},
                {"page", index + 1}, {"text", paragraph}}); total += paragraph.size(); paragraph.clear();
        };
        while (std::getline(lines, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || paragraph.size() > 2600) append();
            if (!line.empty()) paragraph += (paragraph.empty() ? "" : "\n") + line;
        }
        append();
    }
    result["unreadPages"] = emptyPages;
    if (result["segments"].empty()) { result["status"] = "unreadable"; result["reason"] = "未提取到文字，可能为扫描PDF"; }
    else if (emptyPages) { result["status"] = "partial"; result["reason"] = "部分页面没有可提取文字"; }
    return result;
}
struct WebResult { webview_t view; std::filesystem::path output; };
void delivered(const char* id, const char* arguments, void* context) {
    auto& state = *static_cast<WebResult*>(context);
    try {
        const auto values = Json::parse(arguments);
        const auto value = values.at(0).is_string() ? Json::parse(values.at(0).get<std::string>()) : values.at(0);
        write(state.output, value);
        webview_return(state.view, id, 0, "{}");
    } catch (...) { write(state.output, {{"status", "unreadable"}, {"reason", "网页正文提取失败"}, {"segments", Json::array()}}); }
    webview_terminate(state.view);
}
void html(const std::string& bytes, const Json& input, const std::filesystem::path& root, const std::filesystem::path& output) {
    const auto parser = read(root / "public/vendor/readability/JSDOMParser.js");
    const auto readability = read(root / "public/vendor/readability/Readability.js");
    const auto body = decode(bytes, input.value("mime", ""));
    const auto view = webview_create(0, nullptr); if (!view) throw std::runtime_error("网页解析环境不可用");
    struct View { webview_t value; ~View() { webview_destroy(value); } } handle{view};
    WebResult state{view, output};
    webview_set_size(view, 1, 1, WEBVIEW_HINT_NONE); ShowWindow(static_cast<HWND>(webview_get_window(view)), SW_HIDE);
    webview_bind(view, "deliver", delivered, &state);
    webview_set_html(view, "<!doctype html><html><head><meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; script-src 'unsafe-inline'\"></head><body></body></html>");
    // 外部HTML仅传给纯数据DOM解析器，绝不导航到外部网站或执行网页脚本。
    const auto script = parser + "\n" + readability + "\n(() => { try { const raw=" + Json(body).dump() +
        "; const url=" + input.at("url").dump() + R"JS(;
        const document = new JSDOMParser().parse(raw); document.documentURI=url; document.baseURI=url;
        const article = new Readability(document).parse();
        const original = new DOMParser().parseFromString(raw, 'text/html');
        const tracks = [...original.querySelectorAll('track[src]')].filter(t=>['subtitles','captions'].includes(t.getAttribute('kind')||'subtitles')).map(t=>({
          url:new URL(t.getAttribute('src'),url).href,language:t.getAttribute('srclang')||'',title:t.getAttribute('label')||'公开字幕'}));
        if(!article || !article.textContent?.trim()) { deliver(JSON.stringify({status:'unreadable',kind:'html',reason:'未提取到正文，可能需登录、动态加载或为视频页面',segments:[],tracks})); return; }
        const parsed = new DOMParser().parseFromString(article.content,'text/html');
        const segments=[]; let index=0,used=0;
        for(const node of parsed.querySelectorAll('h1,h2,h3,h4,p,li,pre,blockquote,table')) {
          if(node.parentElement?.closest('li,pre,blockquote,table'))continue;
          const text=node.textContent.trim(); if(!text)continue;
          if(used+text.length>2000000)break;
          // 大段落继续拆分，使模型读取和原文定位都能分段进行。
          for(let at=0;at<text.length;at+=3000) { const piece=text.slice(at,at+3000); segments.push({id:'s'+(++index),text:piece}); used+=piece.length; }
        }
        if(!segments.length)segments.push({id:'s1',text:article.textContent.slice(0,6000)});
        const published = article.publishedTime || original.querySelector('meta[property="article:published_time"],meta[name="date"]')?.content || null;
        const updated = original.querySelector('meta[property="article:modified_time"],meta[itemprop="dateModified"]')?.content || null;
        deliver(JSON.stringify({status:used>=2000000?'partial':'read',kind:'html',title:article.title,author:article.byline||null,
          publishedAt:published,sourceUpdatedAt:updated,segments,tracks,coverage:'已提取网页正文；未执行网页脚本，图片和音视频不在本次文字依据内'}));
        } catch(error) { deliver(JSON.stringify({status:'unreadable',reason:'正文提取失败：'+error.message,segments:[]})); } })();
        )JS";
    webview_eval(view, script.c_str()); webview_run(view);
}
}
int wmain(int argc, wchar_t** argv) {
    if (argc != 3) return 2;
    const std::filesystem::path inputPath(argv[1]), output(argv[2]);
    try {
        const auto input = Json::parse(read(inputPath));
        const auto bytes = read(inputPath.parent_path() / "document.bin");
        wchar_t executable[MAX_PATH]{}; GetModuleFileNameW(nullptr, executable, MAX_PATH);
        const auto root = std::filesystem::path(executable).parent_path();
        if (bytes.size() > 24 * 1024 * 1024) throw std::runtime_error("资料超过读取上限");
        if (bytes.rfind("%PDF-", 0) == 0 || input.value("mime", "").find("application/pdf") != std::string::npos) write(output, pdf(bytes, root));
        else html(bytes, input, root, output);
        return 0;
    } catch (const std::exception& error) {
        write(output, {{"status", "unreadable"}, {"reason", error.what()}, {"segments", Json::array()}}); return 1;
    }
}
