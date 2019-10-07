//
//  MarkerPlugin.hpp
//  WikitudeSDKTestsHost
//
//  Created by Andreas Schacherbauer on 30.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef MarkerPlugin_hpp
#define MarkerPlugin_hpp

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "ArchitectPlugin.hpp"
#else
    #import <WikitudeSDK/ArchitectPlugin.hpp>
#endif

#include "MarkerTracker.hpp"
#include "MarkerTrackable.hpp"


namespace wikitude {
    namespace sdk {
        namespace impl {
            class CameraParameters;
        }
        using impl::CameraParameters;
    }
}

class MarkerTrackerJavaScriptPluginModule;
class MarkerTrackerPlugin : public wikitude::sdk::ArchitectPlugin {
public:
    MarkerTrackerPlugin();
    
    /* From ArchitectPlugin */
    void initialize(const std::string& temporaryDirectory_, wikitude::sdk::PluginParameterCollection& pluginParameterCollection_) override;
    void cameraFrameAvailable(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_) override;

    MarkerTrackerJavaScriptPluginModule* getJavaScriptPluginModule();

protected:
    void createMarkerTracker(long id_);
    void createMarkerTrackable(long id_, long markerTrackerId_, int markerId_);
    void calculateProjection(wikitude::sdk::Size<int> cameraFrameSize_);
    
protected:
    std::unordered_map<long, std::unique_ptr<MarkerTracker>>    _registeredMarkerTracker;
    std::unordered_map<long, std::unique_ptr<MarkerTrackable>>  _registeredMarkerTrackables;

    std::vector<int>                    _recentMarkerIDs;

    wikitude::sdk::RuntimeParameters*   _runtimeParameters;
    wikitude::sdk::CameraParameters*    _cameraParameters;
    
    wikitude::sdk::Matrix4              _projection;
};

#endif /* MarkerPlugin_hpp */
