//
//  FaceTrackerOpenGLESRenderPluginModule.cpp
//  SDKExamples
//
//  Created by Andreas Schacherbauer on 26.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "FaceTrackerOpenGLESRenderPluginModule.hpp"


FaceTrackerOpenGLESRenderPluginModule::FaceTrackerOpenGLESRenderPluginModule(wikitude::sdk::RuntimeParameters* runtimeParameters_, wikitude::sdk::CameraParameters* cameraParameters_)
: _runtimeParameters(runtimeParameters_)
, _cameraParameters(cameraParameters_)
{
    _runtimeParameters->addCameraToSurfaceAngleChangedHandler(reinterpret_cast<std::uintptr_t>(this), [&](float cameraToSurfaceAngle_) {
        float cameraFrameAspectRatio = static_cast<float>(_cameraParameters->getCameraFrameSize().height) / static_cast<float>(_cameraParameters->getCameraFrameSize().width);
        calculateProjection(cameraToSurfaceAngle_, cameraFrameAspectRatio, -1.f, 1.f, -1.f, 1.f, 0.f, 500.f);
    });
    _cameraParameters->addCameraFrameSizeChangedHandler(reinterpret_cast<std::uintptr_t>(this), [&](wikitude::sdk::Size<int> cameraFrameSize_) {
        float cameraToSurfaceAngle = _runtimeParameters->getCameraToSurfaceAngle();
        float cameraFrameAspectRatio = static_cast<float>(cameraFrameSize_.height) / static_cast<float>(cameraFrameSize_.width);
        calculateProjection(cameraToSurfaceAngle, cameraFrameAspectRatio, -1.f, 1.f, -1.f, 1.f, 0.f, 500.f);
    });
}

void FaceTrackerOpenGLESRenderPluginModule::startRender(wikitude::sdk::RenderableCameraFrameBucket& /* frameBucket_ */) {
    updateRenderableAugmentations();

    for ( size_t i = 0; i < _augmentationsData.size(); ++i ) {
        _augmentations[i].render(_augmentationsData[i], _projectionMatrix);
    }
}

void FaceTrackerOpenGLESRenderPluginModule::endRender(wikitude::sdk::RenderableCameraFrameBucket& /* frameBucket_ */) {
    /* Intentionally Left Blank */
}

void FaceTrackerOpenGLESRenderPluginModule::setAugmentationData(std::vector<wikitude::sdk::Matrix4>& augmentationsData_) {
    std::lock_guard<std::mutex> updateLock(_updateMutex);
    _augmentationsData = augmentationsData_;
}

void FaceTrackerOpenGLESRenderPluginModule::releaseAugmentations() {
    for ( wikitude::sdk::opengl::StrokedRectangle& strokedRectangle : _augmentations ) {
        strokedRectangle.release();
    }
}

void FaceTrackerOpenGLESRenderPluginModule::updateRenderableAugmentations() {
    std::lock_guard<std::mutex> updateLock(_updateMutex);
    _augmentations.resize(_augmentationsData.size());
}

void FaceTrackerOpenGLESRenderPluginModule::calculateProjection(float cameraToSurfaceAngle_, float cameraFrameAspectRatio_, float left, float right, float bottom, float top, float near, float far) {
    if (cameraToSurfaceAngle_ == 90 || cameraToSurfaceAngle_ == 270) {
        left *= cameraFrameAspectRatio_;
        right *= cameraFrameAspectRatio_;
    } else {
        top *= cameraFrameAspectRatio_;
        bottom *= cameraFrameAspectRatio_;
    }
    _projectionMatrix[0] = 2 / (right - left); _projectionMatrix[4] = 0;                  _projectionMatrix[8]  = 0;                _projectionMatrix[12] = -(right + left) / (right - left);
    _projectionMatrix[1] = 0;                  _projectionMatrix[5] = 2 / (top - bottom); _projectionMatrix[9]  = 0;                _projectionMatrix[13] = -(top + bottom) / (top - bottom);
    _projectionMatrix[2] = 0;                  _projectionMatrix[6] = 0;                  _projectionMatrix[10] = 2 / (far - near); _projectionMatrix[14] = -(far + near) / (far - near);
    _projectionMatrix[3] = 0;                  _projectionMatrix[7] = 0;                  _projectionMatrix[11] = 0;                _projectionMatrix[15] = 1;
}
