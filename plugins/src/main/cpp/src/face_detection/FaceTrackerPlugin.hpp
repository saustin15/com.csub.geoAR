//
//  FaceTrackerPlugin.hpp
//  SDKExamples
//
//  Created by Andreas Schacherbauer on 26.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef FaceTrackerPlugin_hpp
#define FaceTrackerPlugin_hpp

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "ArchitectPlugin.hpp"
#else
    #import <WikitudeSDK/ArchitectPlugin.hpp>
#endif

#include "FaceTracker.hpp"


class FaceTrackerJavaScriptPluginModule;
class FaceTrackerOpenGLESRenderPluginModule;
class FaceTrackerPlugin : public wikitude::sdk::ArchitectPlugin {
public:
    FaceTrackerPlugin();

    void initialize(const std::string& /* temporaryDirectory_ */, wikitude::sdk::PluginParameterCollection& pluginParameterCollection_) override;
    void pause() override;

    void cameraFrameAvailable(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_) override;

    FaceTrackerJavaScriptPluginModule* getJavaScriptPluginModule();
    FaceTrackerOpenGLESRenderPluginModule* getOpenGLESRenderingPluginModule();

protected:
    std::unordered_map<long, std::unique_ptr<FaceTracker>>      _registeredFaceTracker;
};

#endif /* FaceTrackerPlugin_hpp */
