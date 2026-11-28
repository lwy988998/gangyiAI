// 【逐行批注】 引入头文件 <windows.h>，为后续实现提供类型或接口声明。
#include <windows.h>
// 【逐行批注】 引入头文件 <webview/webview.h>，为后续实现提供类型或接口声明。
#include <webview/webview.h>
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 read，执行该函数声明的职责。
std::string read(const std::filesystem::path& path) {
    // 【逐行批注】 声明对象 file，并通过构造函数初始化其状态。
    std::ifstream file(path, std::ios::binary);
    // 【逐行批注】 判断条件 (!file) throw std::runtime_error("资料文件无法读取");，成立时执行对应分支。
    if (!file) throw std::runtime_error("资料文件无法读取");
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream out; out << file.rdbuf(); return out.str();。
    std::ostringstream out; out << file.rdbuf(); return out.str();
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 write，执行该函数声明的职责。
void write(const std::filesystem::path& path, const Json& value) {
    // 【逐行批注】 声明对象 file，并通过构造函数初始化其状态。
    std::ofstream file(path, std::ios::binary | std::ios::trunc); file << value.dump(-1, ' ', false, Json::error_handler_t::replace);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 utf8，执行该函数声明的职责。
std::string utf8(const wchar_t* text, int length) {
    // 【逐行批注】 计算右侧结果并写入 int size，更新当前运行状态。
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, length, nullptr, 0, nullptr, nullptr);
    // 【逐行批注】 声明对象 out，并通过构造函数初始化其状态。
    std::string out(size, '\0'); WideCharToMultiByte(CP_UTF8, 0, text, length, out.data(), size, nullptr, nullptr); return out;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 decode，执行该函数声明的职责。
std::string decode(const std::string& bytes, const std::string& mime) {
    // 【逐行批注】 计算右侧结果并写入 meta，更新当前运行状态。
    auto meta = mime + bytes.substr(0, 1600);
    // 【逐行批注】 按循环规则重复处理：for (auto& ch : meta) ch = static_cast<char>(std::tolower(static_cast<unsi…。
    for (auto& ch : meta) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    // 【逐行批注】 计算右侧结果并写入 auto encoding，更新当前运行状态。
    const auto encoding = meta.find("gbk") != std::string::npos || meta.find("gb2312") != std::string::npos ? 936U :
        // 【逐行批注】 执行当前声明或表达式：meta.find("big5") != std::string::npos ? 950U : CP_UTF8;。
        meta.find("big5") != std::string::npos ? 950U : CP_UTF8;
    // 【逐行批注】 计算右侧结果并写入 int size，更新当前运行状态。
    const int size = MultiByteToWideChar(encoding, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    // 【逐行批注】 声明对象 text，并通过构造函数初始化其状态。
    std::wstring text(size, L'\0'); MultiByteToWideChar(encoding, 0, bytes.data(), static_cast<int>(bytes.size()), text.data(), size);
    // 【逐行批注】 结束当前函数并返回结果：return utf8(text.data(), size);。
    return utf8(text.data(), size);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 symbol，执行该函数声明的职责。
template<typename Function> Function symbol(HMODULE module, const char* name) {
    // 【逐行批注】 计算右侧结果并写入 auto address，更新当前运行状态。
    const auto address = GetProcAddress(module, name);
    // 【逐行批注】 判断条件 (!address) throw std::runtime_error("PDF读取组件不完整");，成立时执行对应分支。
    if (!address) throw std::runtime_error("PDF读取组件不完整");
    // 【逐行批注】 结束当前函数并返回结果：return reinterpret_cast<Function>(address);。
    return reinterpret_cast<Function>(address);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 pdf，执行该函数声明的职责。
Json pdf(const std::string& bytes, const std::filesystem::path& root) {
    // 【逐行批注】 声明数据结构或类型 DLL 及其成员接口。
    struct DLL { HMODULE value; ~DLL() { if (value) FreeLibrary(value); } } dll{LoadLibraryW((root / L"pdfium.dll").c_str())};
    // 【逐行批注】 判断条件 (!dll.value) throw std::runtime_error("PDF读取组件无法载入");，成立时执行对应分支。
    if (!dll.value) throw std::runtime_error("PDF读取组件无法载入");
    // 【逐行批注】 计算右侧结果并写入 auto initialize，更新当前运行状态。
    const auto initialize = symbol<void(*)()>(dll.value, "FPDF_InitLibrary");
    // 【逐行批注】 计算右侧结果并写入 auto destroy，更新当前运行状态。
    const auto destroy = symbol<void(*)()>(dll.value, "FPDF_DestroyLibrary");
    // 【逐行批注】 计算右侧结果并写入 auto open，更新当前运行状态。
    const auto open = symbol<void*(*)(const void*, size_t, const char*)>(dll.value, "FPDF_LoadMemDocument64");
    // 【逐行批注】 计算右侧结果并写入 auto close，更新当前运行状态。
    const auto close = symbol<void(*)(void*)>(dll.value, "FPDF_CloseDocument");
    // 【逐行批注】 计算右侧结果并写入 auto pageCount，更新当前运行状态。
    const auto pageCount = symbol<int(*)(void*)>(dll.value, "FPDF_GetPageCount");
    // 【逐行批注】 计算右侧结果并写入 auto pageOpen，更新当前运行状态。
    const auto pageOpen = symbol<void*(*)(void*, int)>(dll.value, "FPDF_LoadPage");
    // 【逐行批注】 计算右侧结果并写入 auto pageClose，更新当前运行状态。
    const auto pageClose = symbol<void(*)(void*)>(dll.value, "FPDF_ClosePage");
    // 【逐行批注】 计算右侧结果并写入 auto textOpen，更新当前运行状态。
    const auto textOpen = symbol<void*(*)(void*)>(dll.value, "FPDFText_LoadPage");
    // 【逐行批注】 计算右侧结果并写入 auto textClose，更新当前运行状态。
    const auto textClose = symbol<void(*)(void*)>(dll.value, "FPDFText_ClosePage");
    // 【逐行批注】 计算右侧结果并写入 auto charCount，更新当前运行状态。
    const auto charCount = symbol<int(*)(void*)>(dll.value, "FPDFText_CountChars");
    // 【逐行批注】 计算右侧结果并写入 auto getText，更新当前运行状态。
    const auto getText = symbol<int(*)(void*, int, int, unsigned short*)>(dll.value, "FPDFText_GetText");
    // 【逐行批注】 调用 initialize 完成本行所需操作：initialize();。
    initialize();
    // 【逐行批注】 声明数据结构或类型 Library 及其成员接口。
    struct Library { decltype(destroy) cleanup; ~Library() { cleanup(); } } library{destroy};
    // 【逐行批注】 计算右侧结果并写入 auto document，更新当前运行状态。
    const auto document = open(bytes.data(), bytes.size(), nullptr);
    // 【逐行批注】 判断条件 (!document) return {{"status", "unreadable"}, {"kind", "pdf"}, {…，成立时执行对应分支。
    if (!document) return {{"status", "unreadable"}, {"kind", "pdf"}, {"reason", "PDF损坏、加密或无法读取"}, {"segments", Json::array()}};
    // 【逐行批注】 声明数据结构或类型 Handle 及其成员接口。
    struct Handle { void* value; void(*close)(void*); ~Handle() { if (value) close(value); } } documentHandle{document, close};
    // 【逐行批注】 计算右侧结果并写入 Json result，更新当前运行状态。
    Json result = {{"status", "read"}, {"kind", "pdf"}, {"segments", Json::array()}, {"pages", pageCount(document)},
        // 【逐行批注】 执行当前声明或表达式：{"coverage", "已提取PDF文字；图像、扫描页和无法正确提取的公式不作为已核验内容"}};。
        {"coverage", "已提取PDF文字；图像、扫描页和无法正确提取的公式不作为已核验内容"}};
    // 【逐行批注】 计算右侧结果并写入 size_t total，更新当前运行状态。
    size_t total = 0; int emptyPages = 0;
    // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
    const int count = pageCount(document);
    // 【逐行批注】 按循环规则重复处理：for (int index = 0; index < count; ++index) {。
    for (int index = 0; index < count; ++index) {
        // 【逐行批注】 调用 pageOpen 完成本行所需操作：Handle page{pageOpen(document, index), pageClose};。
        Handle page{pageOpen(document, index), pageClose};
        // 【逐行批注】 调用 textOpen 完成本行所需操作：Handle text{page.value ? textOpen(page.value) : nullptr, textClose};。
        Handle text{page.value ? textOpen(page.value) : nullptr, textClose};
        // 【逐行批注】 计算右侧结果并写入 int length，更新当前运行状态。
        const int length = text.value ? charCount(text.value) : 0;
        // 【逐行批注】 判断条件 (length <= 0) { ++emptyPages; continue; }，成立时执行对应分支。
        if (length <= 0) { ++emptyPages; continue; }
        // 【逐行批注】 判断条件 (length > 1000000 || total > 6 * 1024 * 1024 || index >= 1500) {，成立时执行对应分支。
        if (length > 1000000 || total > 6 * 1024 * 1024 || index >= 1500) {
            // 【逐行批注】 计算右侧结果并写入 result["status"]，更新当前运行状态。
            result["status"] = "partial"; result["reason"] = "资料较大，仅读取已列出的页码范围"; break;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 buffer 完成本行所需操作：std::vector<unsigned short> buffer(static_cast<size_t>(length) + 1);。
        std::vector<unsigned short> buffer(static_cast<size_t>(length) + 1);
        // 【逐行批注】 计算右侧结果并写入 auto readLength，更新当前运行状态。
        const auto readLength = getText(text.value, 0, length, buffer.data());
        // 【逐行批注】 判断条件 (readLength <= 1) { ++emptyPages; continue; }，成立时执行对应分支。
        if (readLength <= 1) { ++emptyPages; continue; }
        // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
        const auto body = utf8(reinterpret_cast<const wchar_t*>(buffer.data()), readLength - 1);
        // 按页内段落拆分，保存引用的位置，不把整页文字塞进一次模型请求。
        // 【逐行批注】 声明对象 lines，并通过构造函数初始化其状态。
        std::istringstream lines(body); std::string line, paragraph; int paragraphId = 0;
        // 【逐行批注】 计算右侧结果并写入 auto append，更新当前运行状态。
        const auto append = [&] {
            // 【逐行批注】 判断条件 (paragraph.empty()) return;，成立时执行对应分支。
            if (paragraph.empty()) return;
            // 【逐行批注】 调用 to_string 完成本行所需操作：result["segments"].push_back({{"id", "p" + std::to_string(index + 1) …。
            result["segments"].push_back({{"id", "p" + std::to_string(index + 1) + "-" + std::to_string(++paragraphId)},
                // 【逐行批注】 计算右侧结果并写入 {"page", index + 1}, {"text", paragraph}}); …，更新当前运行状态。
                {"page", index + 1}, {"text", paragraph}}); total += paragraph.size(); paragraph.clear();
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 在条件成立期间循环处理：while (std::getline(lines, line)) {。
        while (std::getline(lines, line)) {
            // 【逐行批注】 判断条件 (!line.empty() && line.back() == '\r') line.pop_back();，成立时执行对应分支。
            if (!line.empty() && line.back() == '\r') line.pop_back();
            // 【逐行批注】 判断条件 (line.empty() || paragraph.size() > 2600) append();，成立时执行对应分支。
            if (line.empty() || paragraph.size() > 2600) append();
            // 【逐行批注】 判断条件 (!line.empty()) paragraph += (paragraph.empty() ? "" : "\n") + l…，成立时执行对应分支。
            if (!line.empty()) paragraph += (paragraph.empty() ? "" : "\n") + line;
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 append 完成本行所需操作：append();。
        append();
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 计算右侧结果并写入 result["unreadPages"]，更新当前运行状态。
    result["unreadPages"] = emptyPages;
    // 【逐行批注】 判断条件 (result["segments"].empty()) { result["status"] = "unreadable"; …，成立时执行对应分支。
    if (result["segments"].empty()) { result["status"] = "unreadable"; result["reason"] = "未提取到文字，可能为扫描PDF"; }
    // 【逐行批注】 在前一条件不成立时继续判断：(emptyPages) { result["status"] = "partial"; result["reason"] = …。
    else if (emptyPages) { result["status"] = "partial"; result["reason"] = "部分页面没有可提取文字"; }
    // 【逐行批注】 结束当前函数并返回结果：return result;。
    return result;
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 声明数据结构或类型 WebResult 及其成员接口。
struct WebResult { webview_t view; std::filesystem::path output; };
// 【逐行批注】 定义函数 delivered，执行该函数声明的职责。
void delivered(const char* id, const char* arguments, void* context) {
    // 【逐行批注】 计算右侧结果并写入 auto& state，更新当前运行状态。
    auto& state = *static_cast<WebResult*>(context);
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto values，更新当前运行状态。
        const auto values = Json::parse(arguments);
        // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
        const auto value = values.at(0).is_string() ? Json::parse(values.at(0).get<std::string>()) : values.at(0);
        // 【逐行批注】 调用 write 完成本行所需操作：write(state.output, value);。
        write(state.output, value);
        // 【逐行批注】 调用 webview_return 完成本行所需操作：webview_return(state.view, id, 0, "{}");。
        webview_return(state.view, id, 0, "{}");
    // 【逐行批注】 调用 write、array 完成本行所需操作：} catch (...) { write(state.output, {{"status", "unreadable"}, {"reas…。
    } catch (...) { write(state.output, {{"status", "unreadable"}, {"reason", "网页正文提取失败"}, {"segments", Json::array()}}); }
    // 【逐行批注】 调用 webview_terminate 完成本行所需操作：webview_terminate(state.view);。
    webview_terminate(state.view);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 html，执行该函数声明的职责。
void html(const std::string& bytes, const Json& input, const std::filesystem::path& root, const std::filesystem::path& output) {
    // 【逐行批注】 计算右侧结果并写入 auto readability，更新当前运行状态。
    const auto readability = read(root / "public/vendor/readability/Readability.js");
    // 【逐行批注】 计算右侧结果并写入 auto body，更新当前运行状态。
    const auto body = decode(bytes, input.value("mime", ""));
    // 【逐行批注】 计算右侧结果并写入 auto view，更新当前运行状态。
    const auto view = webview_create(0, nullptr); if (!view) throw std::runtime_error("网页解析环境不可用");
    // 【逐行批注】 声明数据结构或类型 View 及其成员接口。
    struct View { webview_t value; ~View() { webview_destroy(value); } } handle{view};
    // 【逐行批注】 执行当前声明或表达式：WebResult state{view, output};。
    WebResult state{view, output};
    // 【逐行批注】 调用 webview_set_size、ShowWindow、webview_get_window 完成本行所需操作：webview_set_size(view, 1, 1, WEBVIEW_HINT_NONE); ShowWindow(static_ca…。
    webview_set_size(view, 1, 1, WEBVIEW_HINT_NONE); ShowWindow(static_cast<HWND>(webview_get_window(view)), SW_HIDE);
    // 【逐行批注】 调用 webview_bind 完成本行所需操作：webview_bind(view, "deliver", delivered, &state);。
    webview_bind(view, "deliver", delivered, &state);
    // 【逐行批注】 调用 webview_set_html 完成本行所需操作：webview_set_html(view, "<!doctype html><html><head><meta http-equiv=\…。
    webview_set_html(view, "<!doctype html><html><head><meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; script-src 'unsafe-inline'\"></head><body></body></html>");
    // 外部HTML仅传给纯数据DOM解析器，绝不导航到外部网站或执行网页脚本。
    // 【逐行批注】 计算右侧结果并写入 auto script，更新当前运行状态。
    const auto script = readability + "\n(() => { try { const raw=" + Json(body).dump() +
        // 【逐行批注】 计算右侧结果并写入 "; const url，更新当前运行状态。
        "; const url=" + input.at("url").dump() + R"JS(;
        const original = new DOMParser().parseFromString(raw, 'text/html');
        // 人教 B 版目录的主要内容是静态标题和链接。通用正文提取会误选浏览器兼容提示。
        if(url==='https://www.pep.com.cn/gzsxb/') {
          const edition=original.querySelector('h1.text1');
          const volumes=[...original.querySelectorAll('h1.xw_gradeBut > span')];
          const volumeIndex=volumes.findIndex(node=>node.textContent.trim()==='必修第一册');
          if(edition && volumeIndex>=0) {
            const volume=volumes[volumeIndex];
            const next=volumes[volumeIndex+1];
            const chapters=[...original.querySelectorAll('h4 a[rel]')].filter(node=>
              (volume.compareDocumentPosition(node)&Node.DOCUMENT_POSITION_FOLLOWING) &&
              (!next || (node.compareDocumentPosition(next)&Node.DOCUMENT_POSITION_FOLLOWING)));
            if(chapters.length) {
              const segments=[edition,volume,...chapters.slice(0,20)].map((node,index)=>
                ({id:'s'+(index+1),text:node.textContent.trim().slice(0,300)}));
              deliver(JSON.stringify({status:'partial',kind:'html',title:segments[0].text,segments,tracks:[],
                coverage:'提取同步教学目录中必修第一册的标题和章节名称'}));
              return;
            }
          }
        }
        // 数学语义和辅助文本不能拼成连续数字；优先保留原网页的LaTeX，其次转换可辨认的MathML。
        let formulaMissing=false;
        const mathText=node=>{
          const children=[...node.children], parts=children.map(mathText), tag=node.localName;
          if(tag==='annotation'||tag==='annotation-xml')return '';
          if(tag==='semantics')return children.length?mathText(children[0]):'';
          if(tag==='mfrac'&&parts.length===2)return '('+parts[0]+')/('+parts[1]+')';
          if(tag==='msup'&&parts.length===2)return '('+parts[0]+')^('+parts[1]+')';
          if(tag==='msub'&&parts.length===2)return '('+parts[0]+')_('+parts[1]+')';
          if(tag==='msqrt')return 'sqrt('+parts.join('')+')';
          if(tag==='mroot'&&parts.length===2)return '('+parts[0]+')^(1/('+parts[1]+'))';
          if(['math','mrow','mstyle','mpadded','mphantom'].includes(tag))return parts.join('');
          if(['mi','mn','mo','mtext','ms'].includes(tag))return node.textContent.trim();
          if(tag==='mspace')return ' ';
          formulaMissing=true;return '[公式未完整提取，请查看原网页]';
        };
        for(const math of original.querySelectorAll('math')) {
          const tex=[...math.querySelectorAll('annotation')].find(a=>(a.getAttribute('encoding')||'').includes('tex'))?.textContent?.trim();
          const replacement=original.createElement('span');replacement.textContent=tex?'\\('+tex+'\\)':mathText(math);
          math.replaceWith(replacement);
        }
        const article = new Readability(original.cloneNode(true)).parse();
        const tracks = [...original.querySelectorAll('track[src]')].filter(t=>['subtitles','captions'].includes(t.getAttribute('kind')||'subtitles')).map(t=>({
          url:new URL(t.getAttribute('src'),url).href,language:t.getAttribute('srclang')||'',title:t.getAttribute('label')||'公开字幕'}));
        if(!article || !article.textContent?.trim()) { deliver(JSON.stringify({status:'unreadable',kind:'html',reason:'未提取到正文，可能需登录、动态加载或为视频页面',segments:[],tracks})); return; }
        const parsed = new DOMParser().parseFromString(article.content,'text/html');
        const segments=[]; let index=0,used=0,truncated=false;
        const blocks='h1,h2,h3,h4,p,dt,dd,li,pre,blockquote,table,div';
        for(const node of parsed.querySelectorAll(blocks)) {
          if(node.parentElement?.closest('li,pre,blockquote,table'))continue;
          if(['DIV','DD'].includes(node.tagName)&&node.querySelector(blocks))continue;
          const text=(node.tagName==='TABLE'?[...node.querySelectorAll('tr')].map(row=>[...row.querySelectorAll('th,td')].map(cell=>cell.textContent.trim()).join(' | ')).join('\n'):node.textContent).trim(); if(!text)continue;
          if(used+text.length>2000000){truncated=true;break;}
          // 大段落继续拆分，使模型读取和原文定位都能分段进行。
          for(let at=0;at<text.length;at+=3000) { const piece=text.slice(at,at+3000); segments.push({id:'s'+(++index),text:piece}); used+=piece.length; }
        }
        if(!segments.length)segments.push({id:'s1',text:article.textContent.slice(0,6000)});
        const shortText=segments.reduce((total,s)=>total+s.text.length,0)<350;
        const published = article.publishedTime || original.querySelector('meta[property="article:published_time"],meta[name="date"]')?.content || null;
        const updated = original.querySelector('meta[property="article:modified_time"],meta[itemprop="dateModified"]')?.content || null;
        deliver(JSON.stringify({status:truncated||formulaMissing||shortText?'partial':'read',kind:'html',title:article.title,author:article.byline||null,
          publishedAt:published,sourceUpdatedAt:updated,segments,tracks,coverage:'已提取网页正文；未执行网页脚本，图片和音视频不在本次文字依据内'+(formulaMissing?'；部分公式未完整提取，已在原位置标明':'' )+(shortText?'；文字较少，可能只包含表格或局部片段，不能据此确认全文已读':'' )}));
        } catch(error) { deliver(JSON.stringify({status:'unreadable',reason:'正文提取失败：'+error.message,segments:[]})); } })();
        )JS";
    // 【逐行批注】 调用 webview_eval、webview_run 完成本行所需操作：webview_eval(view, script.c_str()); webview_run(view);。
    webview_eval(view, script.c_str()); webview_run(view);
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 定义函数 wmain，执行该函数声明的职责。
int wmain(int argc, wchar_t** argv) {
    // 【逐行批注】 判断条件 (argc != 3) return 2;，成立时执行对应分支。
    if (argc != 3) return 2;
    // 【逐行批注】 调用 inputPath、output 完成本行所需操作：const std::filesystem::path inputPath(argv[1]), output(argv[2]);。
    const std::filesystem::path inputPath(argv[1]), output(argv[2]);
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto input，更新当前运行状态。
        const auto input = Json::parse(read(inputPath));
        // 【逐行批注】 计算右侧结果并写入 auto bytes，更新当前运行状态。
        const auto bytes = read(inputPath.parent_path() / "document.bin");
        // 【逐行批注】 调用 GetModuleFileNameW 完成本行所需操作：wchar_t executable[MAX_PATH]{}; GetModuleFileNameW(nullptr, executabl…。
        wchar_t executable[MAX_PATH]{}; GetModuleFileNameW(nullptr, executable, MAX_PATH);
        // 【逐行批注】 计算右侧结果并写入 auto root，更新当前运行状态。
        const auto root = std::filesystem::path(executable).parent_path();
        // 【逐行批注】 判断条件 (bytes.size() > 24 * 1024 * 1024) throw std::runtime_error("资料超过…，成立时执行对应分支。
        if (bytes.size() > 24 * 1024 * 1024) throw std::runtime_error("资料超过读取上限");
        // 【逐行批注】 判断条件 (bytes.rfind("%PDF-", 0) == 0 || input.value("mime", "").find("a…，成立时执行对应分支。
        if (bytes.rfind("%PDF-", 0) == 0 || input.value("mime", "").find("application/pdf") != std::string::npos) write(output, pdf(bytes, root));
        // 【逐行批注】 执行前一条件不成立时的分支。
        else html(bytes, input, root, output);
        // 【逐行批注】 结束当前函数并返回结果：return 0;。
        return 0;
    // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) {。
    } catch (const std::exception& error) {
        // 【逐行批注】 调用 write、array 完成本行所需操作：write(output, {{"status", "unreadable"}, {"reason", error.what()}, {"…。
        write(output, {{"status", "unreadable"}, {"reason", error.what()}, {"segments", Json::array()}}); return 1;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
