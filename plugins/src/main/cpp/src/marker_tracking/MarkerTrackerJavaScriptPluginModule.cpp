//
//  MarkerTrackerJavaScriptPluginModule.cpp
//  WikitudeSDKTestsHost
//
//  Created by Andreas Schacherbauer on 30.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "MarkerTrackerJavaScriptPluginModule.hpp"

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "Positionable.hpp"
#else
    #import <WikitudeSDK/Positionable.hpp>
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#include <nlohmann/json.hpp>
#pragma clang diagnostic pop


MarkerTrackerJavaScriptPluginModule::MarkerTrackerJavaScriptPluginModule(CreateMarkerTrackerHandle createMarkerTrackerHandle_, CreateMarkerTrackableHandle createMarkerTrackableHandle_)
:
_createMarkerTrackerHandle(createMarkerTrackerHandle_),
_createMarkerTrackableHandle(createMarkerTrackableHandle_)
{ /* Intentionally Left Blank */ }

MarkerTrackerJavaScriptPluginModule::JavaScriptAPI MarkerTrackerJavaScriptPluginModule::getJavaScriptAPI() {
    return {
        R"(
            class MarkerTracker {
                constructor() {
                    this.id = AR.plugins.createInstance("com.wikitude.plugins.marker_tracker_demo", this, {});
                }
            };
        
            class MarkerTrackable {
                constructor(markerTracker, markerId, options) {
                    options = options != undefined ? options : {};

                    this.id = AR.plugins.createInstance("com.wikitude.plugins.marker_tracker_demo", this, {
                        markerTracker: markerTracker.id,
                        markerId: markerId
                    });

                    options.drawables = options.drawables != undefined ? options.drawables : {};
                    options.drawables.cam = options.drawables.cam != undefined ? options.drawables.cam : [];
                    /*
                     The positionable onEnterFieldOfVision and onExitFieldOfVision callbacks are not used here because they can't provide additional parameter.
                     That's why the additional onMarkerRecognized and onMarkerLost are introduced.
                     */
                    this.positionable = new AR.Positionable("com.wikitude.plugins.marker_tracker_demo.positionable_" + markerId, {
                        drawables: {
                            cam: options.drawables.cam
                        }
                    });

                    this.onMarkerRecognized = options.onMarkerRecognized ? options.onMarkerRecognized : undefined;
                    this.onMarkerLost = options.onMarkerLost ? options.onMarkerLost : undefined;
                }

                destroy() {
                    this.positionable.destroy();
                }

                _markerRecognized(markerId) {
                    if ( this.onMarkerRecognized != undefined ) {
                        this.onMarkerRecognized(markerId);
                    }
                }

                _markerLost(markerId) {
                    if ( this.onMarkerLost != undefined ) {
                        this.onMarkerLost(markerId);
                    }
                }
            };
        )",
        { /* Intentionally Left Blank */ }
    };
}

void MarkerTrackerJavaScriptPluginModule::createInstance(const std::string& className_, long id_, const std::string& parameter_) {
    if ( className_ == "MarkerTracker" ) {
        _createMarkerTrackerHandle(id_);
    } else if ( className_ == "MarkerTrackable" ) {
        nlohmann::json parameterObject = nlohmann::json::parse(parameter_);
        long markerTrackerId = parameterObject["markerTracker"].get<long>();
        int markerId = parameterObject["markerId"].get<int>();
        _createMarkerTrackableHandle(id_, markerTrackerId, markerId);
    }
}

void MarkerTrackerJavaScriptPluginModule::updatePositionables(const std::unordered_map<std::string, wikitude::sdk::Positionable*>& positionables_) {

    std::lock_guard<std::mutex> positionableDataReadLock(_currentPositionablesUpdateMutex);
    for ( auto& sdkPositionablePair : positionables_ ) {

        for ( auto& positionableData : _currentPositionableData ) {

            auto matchSearch = std::find_if(positionableData.begin(), positionableData.end(), [&](auto currentPair_) {
                return sdkPositionablePair.first == currentPair_.first;
            });
            if ( matchSearch != positionableData.end() ) {
                if ( matchSearch->second._state == demo::PositionableState::Recognized ) {
                    callInstance(matchSearch->second._trackableId, "_markerRecognized(" + std::to_string(matchSearch->second._marker._id) + ")");
                    sdkPositionablePair.second->enteredFieldOfVision();
                } else if ( matchSearch->second._state == demo::PositionableState::Lost ) {
                    callInstance(matchSearch->second._trackableId, "_markerLost(" + std::to_string(matchSearch->second._marker._id) + ")");
                    sdkPositionablePair.second->exitedFieldOfVision();
                }
                sdkPositionablePair.second->setViewMatrix(matchSearch->second._marker._matrix.get());
                sdkPositionablePair.second->setProjectionMatrix(_projection.get());
                sdkPositionablePair.second->setWorldMatrix(_identity.get());
            }
        }
    }
    _currentPositionableData.clear();
}

void MarkerTrackerJavaScriptPluginModule::setPositionableData(std::unordered_map<std::string, demo::PositionableData>& positionableData_, wikitude::sdk::Matrix4 projection_) {
    {
        std::lock_guard<std::mutex> positionableDataUpdateLock(_currentPositionablesUpdateMutex);
        _currentPositionableData.push_back(positionableData_);
    }
    _projection = projection_;
}
