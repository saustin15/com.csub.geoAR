//
//  YUVFrameInputPlugin.h
//
//  Created by Daniel Guttenberg on 28.05.2018.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef YUVFrameInputPlugin_h
#define YUVFrameInputPlugin_h

#include <jni.h>

#include <Plugin.hpp>


class YUVFrameInputPlugin : public wikitude::sdk::Plugin {
public:
    YUVFrameInputPlugin();
    virtual ~YUVFrameInputPlugin();

    // from Plugin
    void initialize(const std::string& temporaryDirectory_, wikitude::sdk::PluginParameterCollection& pluginParameterCollection_) override;
    void pause() override;
    void resume(unsigned int pausedTime_) override;
    void destroy() override;

    void cameraFrameAvailable(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_) override;
    void update(const wikitude::sdk::RecognizedTargetsBucket& recognizedTargetsBucket_) override;

    void initializeJni(JNIEnv* env_);

protected:
    void callInitializedJNIMethod(jmethodID methodId_);

public:
    static YUVFrameInputPlugin* instance;

private:
    jmethodID _sdkCameraReleasedMethodId;
    jmethodID _pluginPausedMethodId;
    jmethodID _pluginResumedMethodId;
    jmethodID _pluginDestroyedMethodId;
};

#endif /* YUVFrameInputPlugin_h */
