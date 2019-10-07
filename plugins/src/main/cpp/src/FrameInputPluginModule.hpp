
//
// FrameInputPluginModule
//
// Created by Alexander Bendl on 22.05.18.
// Copyright (c) 2018 Wikitude. All rights reserved.
//

#ifndef FrameInputPluginModule_hpp
#define FrameInputPluginModule_hpp

#include <CameraFrameInputPluginModule.hpp>


class FrameInputPluginModule : public wikitude::sdk::CameraFrameInputPluginModule {
public:
    FrameInputPluginModule(bool requestRendering_, std::function<void()> cameraReleaseHandler_);

    void onCameraReleased() override;
    void onCameraReleaseFailed(const wikitude::sdk::Error& error_) override;

    void newCameraFrameAvailable(const wikitude::sdk::CameraFrame& cameraFrame_);
    void cameraReleased();
    void cameraToSurfaceAngleChanged(float angle_);
    void notifyNewCameraFrameYUV_420_888(const std::vector<wikitude::sdk::CameraFramePlane>& planes_);
    void notifyNewCameraFrameNV21(const std::vector<wikitude::sdk::CameraFramePlane>& planes_);

    void fieldOfViewChanged(float fov_);
private:
    wikitude::sdk::ColorCameraFrameMetadata _metadata;
    std::function<void()> _cameraReleaseHandler;
    long _frameId;
};


#endif //FrameInputPluginModule_hpp
