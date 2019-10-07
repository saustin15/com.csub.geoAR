//
//  FaceTrackerJavaScriptPluginModule.cpp
//  SDKExamples
//
//  Created by Andreas Schacherbauer on 26.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "FaceTrackerJavaScriptPluginModule.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#include <nlohmann/json.hpp>
#pragma clang diagnostic pop


FaceTrackerJavaScriptPluginModule::FaceTrackerJavaScriptPluginModule(wikitude::sdk::RuntimeParameters* runtimeParameters_, std::unordered_map<long, std::unique_ptr<FaceTracker>>& registeredFaceTracker_, const std::string& temporaryDirectory_)
:
_runtimeParameters(runtimeParameters_),
_registeredFaceTracker(registeredFaceTracker_),
_temporaryDirectory(temporaryDirectory_)
{}

FaceTrackerJavaScriptPluginModule::JavaScriptAPI FaceTrackerJavaScriptPluginModule::getJavaScriptAPI() {
    return {
        R"(
            class FaceTracker {
                constructor(databasePath_, options_) {
                    options_ = options_ != undefined ? options_ : {};
                    this.id = AR.plugins.createInstance("com.wikitude.plugins.face_tracker_demo", this, {
                        databasePath: AR.__resourceUrl(databasePath_)
                    });

                    this.onError = options_.onError ? options_.onError : undefined;
                }

                _error(error) {
                    if ( this.onError != undefined ) {
                        this.onError(error);
                    }
                }
            };
        )",
        { /* Intentionally Left Blank */ }
    };
}

void FaceTrackerJavaScriptPluginModule::createInstance(const std::string& /* className_ */, long id_, const std::string& parameter_) {
    nlohmann::json parameterObject = nlohmann::json::parse(parameter_);
    if ( !parameterObject.is_null() ) {
        std::string databasePath(parameterObject["databasePath"].get<std::string>());
        auto emplaceResult = _registeredFaceTracker.emplace(id_, std::make_unique<FaceTracker>(id_, databasePath, _runtimeParameters, _temporaryDirectory));
        if ( !emplaceResult.first->second->isLoaded() ) {
            callInstance(id_, "_error({code: 1001, message: 'Unable to load given database file ' + '" + databasePath + "'});");
        }
    }
}
