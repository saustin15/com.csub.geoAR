//
//  MarkerTrackerPlugin.cpp
//  WikitudeSDKTestsHost
//
//  Created by Andreas Schacherbauer on 30.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "MarkerTrackerPlugin.hpp"

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "PluginParameterCollection.hpp"
    #include "RuntimeParameters.hpp"
    #include "CameraParameters.hpp"
#else
    #import <WikitudeSDK/PluginParameterCollection.hpp>
    #import <WikitudeSDK/RuntimeParameters.hpp>
    #import <WikitudeSDK/CameraParameters.hpp>
#endif

#include "MarkerTrackerJavaScriptPluginModule.hpp"


MarkerTrackerPlugin::MarkerTrackerPlugin()
:
wikitude::sdk::ArchitectPlugin("com.wikitude.plugins.marker_tracker_demo"),
_runtimeParameters(nullptr)
{ /* Intentionally Left Blank */ }

void MarkerTrackerPlugin::initialize(const std::string& /* temporaryDirectory_ */, wikitude::sdk::PluginParameterCollection& pluginParameterCollection_) {
    _runtimeParameters = &pluginParameterCollection_.getRuntimeParameters();
    _cameraParameters = &pluginParameterCollection_.getCameraParameters();

    _cameraParameters->addCameraFrameSizeChangedHandler(reinterpret_cast<std::uintptr_t>(this), [&](wikitude::sdk::Size<int> cameraFrameSize_) {
        calculateProjection(cameraFrameSize_);
    });

    setJavaScriptPluginModule(std::make_unique<MarkerTrackerJavaScriptPluginModule>(
                                                                                    std::bind(&MarkerTrackerPlugin::createMarkerTracker, this, std::placeholders::_1),
                                                                                    std::bind(&MarkerTrackerPlugin::createMarkerTrackable, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
                              );
}

void MarkerTrackerPlugin::cameraFrameAvailable(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_) {
    std::vector<int> currentMarkerIDs;
    std::unordered_map<std::string, demo::PositionableData> positionableData;
    for ( auto& trackerPair : _registeredMarkerTracker ) {
        trackerPair.second->processCameraFrame(managedCameraFrame_, [&](int markerId_, wikitude::sdk::Matrix4& matrix_) {
            currentMarkerIDs.emplace_back(markerId_);
            for ( auto& trackablePair : _registeredMarkerTrackables ) {
                trackablePair.second->processTrackedMarker(markerId_, matrix_, positionableData);
            }
        });
    }

    std::vector<int> lostMarkers;
    std::set_difference(_recentMarkerIDs.begin(), _recentMarkerIDs.end(), currentMarkerIDs.begin(), currentMarkerIDs.end(), std::inserter(lostMarkers, lostMarkers.begin()));

    for ( int lostMarker : lostMarkers ) {
        for ( auto& trackablePair : _registeredMarkerTrackables ) {
            trackablePair.second->cleanupRecentMarkers(lostMarker, positionableData);
        }
    }
    
    getJavaScriptPluginModule()->setPositionableData(positionableData, _projection);

    _recentMarkerIDs = currentMarkerIDs;
}

MarkerTrackerJavaScriptPluginModule* MarkerTrackerPlugin::getJavaScriptPluginModule() {
    return dynamic_cast<MarkerTrackerJavaScriptPluginModule*>(ArchitectPlugin::getJavaScriptPluginModule());
}

void MarkerTrackerPlugin::createMarkerTracker(long id_) {
    _registeredMarkerTracker.emplace(id_, std::make_unique<MarkerTracker>(_runtimeParameters));
}

void MarkerTrackerPlugin::createMarkerTrackable(long id_, long markerTrackerId_, int markerId_) {
    _registeredMarkerTrackables.emplace(id_, std::make_unique<MarkerTrackable>(id_, markerTrackerId_, markerId_));
}

void MarkerTrackerPlugin::calculateProjection(wikitude::sdk::Size<int> cameraFrameSize_) {

    /* With defining the camera intrinsics to match the iPhone 5, the fov needs to match this as well */
    _projection.perspective(50.f, static_cast<float>(cameraFrameSize_.width) / static_cast<float>(cameraFrameSize_.height), 0.1f, 100.f);
}
