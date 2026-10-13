#ifdef NDEBUG
#undef NDEBUG
#endif
#include "learning_library.hpp"
#include <cassert>
#include <iostream>
using Json = nlohmann::json;
void runLibraryChecks() {
    gangyi::Database db; db.open(":memory:"); db.migrate();
    assert(gangyi::libraryDigest("abc")=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    assert(!gangyi::libraryPublicURL("file:///C:/Users/private.txt"));
    assert(!gangyi::libraryPublicURL("http://127.0.0.1/api/my-ai/profile"));
    assert(!gangyi::libraryPublicURL("https://user:password@example.org/"));
    assert(!gangyi::libraryPublicURL("http://172.16.2.3/private"));
    assert(!gangyi::libraryPublicURL("http://[fc00::1]/private"));
    assert(!gangyi::libraryPublicURL("http://localhost./private"));
    assert(gangyi::libraryPublicURL("https://example.org/Study?Version=A"));
    assert(gangyi::libraryCanonicalURL("https://Example.org/Study?Version=A#part") == "https://example.org/Study?Version=A");
    auto source = gangyi::libraryWrite(db, {{"url","https://example.org/Study"},{"title","固定原文"}});
    const std::string id = source["id"];
    const Json first = {{"sourceId",id},{"checkedAt","2026-10-13T00:00:00Z"},{"publishedAt",nullptr},
        {"segments",Json::array({{{"id","s1"},{"text","分母相同才可以直接相加分子。"}}})}};
    assert(db.saveMyAIRecord({"snap-first","library-snapshot",first.dump(),"2026-10-13",0},0));
    auto value = Json::parse(db.myAIRecord(id)->payload); value["snapshotId"]="snap-first"; value["readStatus"]="read";
    assert(db.saveMyAIRecord({id,"library-source",value.dump(),"2026-10-13",0},source["version"]));
    const Json citation = {{"sourceId",id},{"snapshotId","snap-first"},{"segmentId","s1"},{"quote","分母相同"}};
    const auto verified=gangyi::libraryValidateCitations(db,Json::array({citation}));
    assert(verified[0]["publishedAt"].is_null()&&verified[0]["locator"]["text"]==first["segments"][0]["text"]);
    bool badQuote=false;try{auto invalid=citation;invalid["quote"]="原文并不存在这句话";gangyi::libraryValidateCitations(db,Json::array({invalid}));}catch(...){badQuote=true;}assert(badQuote);
    bool badSegment=false;try{auto invalid=citation;invalid["segmentId"]="p99";gangyi::libraryValidateCitations(db,Json::array({invalid}));}catch(...){badSegment=true;}assert(badSegment);
    auto detail=gangyi::libraryRead(db,{{"id",id}});
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"favorite",true}});
    bool conflict=false;try{gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"favorite",false}});}catch(...){conflict=true;}assert(conflict);
    detail=gangyi::libraryRead(db,{{"id",id}});
    gangyi::libraryWrite(db,{{"id",id},{"version",detail["version"]},{"excluded",true}});
    assert(gangyi::libraryContext(db)["sources"].empty());
    assert(!gangyi::libraryValidateCitations(db,Json::array({citation}),false).empty());
    bool excluded=false;try{gangyi::libraryValidateCitations(db,Json::array({citation}));}catch(...){excluded=true;}assert(excluded);
    const auto captions=gangyi::libraryParseCaptions("WEBVTT\n\n00:01.000 --> 00:03.000\n先通分。\n\n00:03.000 --> 00:05.000\n再相加。\n","text/vtt");
    assert(captions["segments"].size()==2&&captions["segments"][0]["start"]==1.0&&captions["segments"][1]["end"]==5.0);
    assert(captions["automatic"].is_null());
    const auto jsonCaptions=gangyi::libraryParseCaptions(R"({"body":[{"from":2,"to":4,"content":"第二段字幕"}]})","application/json");
    assert(jsonCaptions["status"]=="read"&&jsonCaptions["segments"][0]["id"]=="t1");
    assert(gangyi::libraryParseCaptions("WEBVTT\n\n00:07.000 --> 00:03.000\n错误时间范围\n","text/vtt")["segments"].empty());
    assert(gangyi::libraryParseCaptions(R"({"body":[{"from":-1,"to":"invalid","content":"不能用于引用"}]})","application/json")["segments"].empty());
    assert(gangyi::libraryUsage(db)["searchRequests"]==0&&gangyi::libraryUsage(db)["tokens"].is_null());
    assert(db.listInteractions().empty()&&db.listMastery().empty());
    std::cout<<"资料引用、快照、排除、冲突及字幕专项检查通过\n";
}
