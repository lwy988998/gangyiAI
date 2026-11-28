#pragma once

#include "interactive_labs.hpp"
#include <cassert>

inline void runInteractivePeriodicChecks() {
    using Json = nlohmann::json;
    const Json config={{"type","periodic_lab"}};
    const Json state={{"schemaVersion",1},{"contentVersion",1},{"datasetId",gangyi::kPeriodicElementsId},
        {"selectedZ",11},{"comparisonZ",17},{"highlight","none"}};
    const auto& elements=gangyi::periodicElementsData().at("elements");assert(elements.size()==118);
    for(int z=1;z<=118;++z){
        assert(elements.at(z-1).at("atomicNumber")==z);
        auto changed=state;changed["selectedZ"]=z;
        const auto value=gangyi::checkedLabObservation(config,changed,1);
        assert(value.at("selected").at("protons")==z && value.at("selected").at("electrons")==z);
        assert(value.at("selected").at("neutrons").is_null() && value.at("scoringEligible")==false);
        if(z>20)assert(value.at("selected").at("neutralShells").is_null());
    }
    const auto sodium=gangyi::checkedLabObservation(config,state,1);
    assert(sodium.at("selected").at("symbol")=="Na" && sodium.at("selected").at("neutralShells")==Json::array({2,8,1}));
    assert(sodium.at("samePeriod")==true && sodium.at("sameGroup")==false);
    auto potassium=state;potassium["comparisonZ"]=19;potassium["highlight"]="group";
    const auto comparison=gangyi::checkedLabObservation(config,potassium,1);
    assert(comparison.at("samePeriod")==false && comparison.at("sameGroup")==true);
    assert(comparison.at("highlightedAtomicNumbers")==Json::array({1,3,11,19,37,55,87}));
    for(int z: {57,58,71,89,92,103}){
        auto series=state;series["selectedZ"]=z;series["highlight"]="group";
        const auto actual=gangyi::checkedLabObservation(config,series,1);
        assert(actual.at("sameGroup").is_null() && actual.at("selected").at("group").is_null());
        assert(actual.at("highlightedAtomicNumbers")==Json::array({z}));
    }
    for(const auto& patch:Json::array({Json{{"selectedZ",0}},Json{{"selectedZ",119}},Json{{"selectedZ",true}},
        Json{{"comparisonZ",nullptr}},Json{{"highlight","fake"}},Json{{"datasetId","old"}}})){
        auto changed=state;changed.update(patch);auto badConfig=config;badConfig.update(patch);bool denied=false,configDenied=false;
        try{gangyi::checkedLabObservation(config,changed,1);}catch(...){denied=true;}
        try{gangyi::checkedLabConfig(badConfig);}catch(...){configDenied=true;}
        assert(denied && configDenied);
    }
    for(const auto& patch:Json::array({Json{{"score",100}},Json{{"neutralShells",{2,8,8}}},Json{{"symbol","Au"}},Json{{"contentVersion",2}}})){
        auto changed=state;changed.update(patch);bool denied=false;
        try{gangyi::checkedLabObservation(config,changed,1);}catch(...){denied=true;}assert(denied);
    }
}
