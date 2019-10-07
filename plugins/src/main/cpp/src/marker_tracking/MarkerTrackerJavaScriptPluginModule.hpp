//
//  MarkerTrackerJavaScriptPluginModule.hpp
//  WikitudeSDKTestsHost
//
//  Created by Andreas Schacherbauer on 30.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef MarkerTrackerJavaScriptPluginModule_hpp
#define MarkerTrackerJavaScriptPluginModule_hpp

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "JavaScriptPluginModule.hpp"
#else
    #import <WikitudeSDK/JavaScriptPluginModule.hpp>
#endif

#include "Marker.hpp"


namespace wikitude {
    namespace sdk {
        namespace impl {
            class Positionable;
        }
        using impl::Positionable;
    }
}

class MarkerTrackerJavaScriptPluginModule : public wikitude::sdk::JavaScriptPluginModule {
public:
    using CreateMarkerTrackerHandle = std::function<void(long id_)>;
    using CreateMarkerTrackableHandle = std::function<void(long id_, long markerTrackerId_, int markerId_)>;

public:
    MarkerTrackerJavaScriptPluginModule(CreateMarkerTrackerHandle createMarkerTrackerHandle_, CreateMarkerTrackableHandle createMarkerTrackableHandle_);

    /* From JavaScriptPluginModule */
    JavaScriptPluginModule::JavaScriptAPI getJavaScriptAPI() override;
    void createInstance(const std::string& className_, long id_, const std::string& parameter_) override;
    void updatePositionables(const std::unordered_map<std::string, wikitude::sdk::Positionable*>& positionables_) override;
    
    /* Internal Plugin API */
    void setPositionableData(std::unordered_map<std::string, demo::PositionableData>& positionableData_, wikitude::sdk::Matrix4 projection_);

protected:
    CreateMarkerTrackerHandle       _createMarkerTrackerHandle;
    CreateMarkerTrackableHandle     _createMarkerTrackableHandle;

    std::mutex                                                              _currentPositionablesUpdateMutex;
    std::vector<std::unordered_map<std::string, demo::PositionableData>>    _currentPositionableData;

    wikitude::sdk::Matrix4                              _projection;
    wikitude::sdk::Matrix4                              _identity;
};

#endif /* MarkerTrackerJavaScriptPluginModule_hpp */
