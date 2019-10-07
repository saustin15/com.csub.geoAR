
//
// FrameInputPluginModule
//
// Created by Alexander Bendl on 22.05.18.
// Copyright (c) 2018 Wikitude. All rights reserved.
//

#include <jni.h>
#include <android/log.h>
#include "FrameInputPluginModule.hpp"


FrameInputPluginModule::FrameInputPluginModule(bool requestRendering_, std::function<void()> cameraReleaseHandler_)
: _metadata(wikitude::sdk::ColorCameraFrameMetadata(-1, {640,480}, wikitude::sdk::CameraPosition::Back, wikitude::sdk::ColorSpace::YUV_420_888, 0))
, _cameraReleaseHandler(cameraReleaseHandler_)
, _frameId(-1)
{
    _requestsCameraFrameRendering = requestRendering_;
}

void FrameInputPluginModule::onCameraReleased() {
    _cameraReleaseHandler();
}

void FrameInputPluginModule::onCameraReleaseFailed(const wikitude::sdk::Error& error_) {

}

void FrameInputPluginModule::newCameraFrameAvailable(const wikitude::sdk::CameraFrame& cameraFrame_) {

}

void FrameInputPluginModule::cameraReleased() {
    CameraFrameInputPluginModule::notifyPluginCameraReleased();
}

void FrameInputPluginModule::cameraToSurfaceAngleChanged(float angle_){
    CameraFrameInputPluginModule::setCameraToSurfaceAngle(angle_);
}

void FrameInputPluginModule::notifyNewCameraFrameYUV_420_888(const std::vector<wikitude::sdk::CameraFramePlane>& planes_) {
    notifyNewUnmanagedCameraFrameToSDK(wikitude::sdk::CameraFrame(++_frameId, 0, _metadata, planes_));
}

void FrameInputPluginModule::notifyNewCameraFrameNV21(const std::vector<wikitude::sdk::CameraFramePlane>& planes_) {
    if (_metadata.getFrameColorSpace() != wikitude::sdk::ColorSpace::YUV_420_NV21) {
        _metadata = wikitude::sdk::ColorCameraFrameMetadata(_metadata.getHorizontalFieldOfView(), _metadata.getPixelSize(), _metadata.getCameraPosition(), wikitude::sdk::ColorSpace::YUV_420_NV21, _metadata.getTimestampTimescale());
    }
    notifyNewUnmanagedCameraFrameToSDK(wikitude::sdk::CameraFrame(++_frameId, 0, _metadata, planes_));
}

void FrameInputPluginModule::fieldOfViewChanged(float fov_) {
    _metadata = wikitude::sdk::ColorCameraFrameMetadata(fov_, _metadata.getPixelSize(), _metadata.getCameraPosition(), _metadata.getFrameColorSpace(), _metadata.getTimestampTimescale());
}

extern "C" JNIEXPORT void JNICALL Java_com_wikitude_samples_advanced_plugins_input_FrameInputPluginModule_nativeNotifyNewCameraFrameYUV420888(JNIEnv* env_, jobject, jlong nativeHandle_, jobject luminanceData_, jobject chromaBlueData_, jobject chromaRedData_, jint rowStrideLuminance_, jint pixelStrideChroma_, jint rowStrideChroma_) {
    auto module = reinterpret_cast<FrameInputPluginModule*>(nativeHandle_);
    void* luminance = env_->GetDirectBufferAddress(luminanceData_);
    void* chromaBlue = env_->GetDirectBufferAddress(chromaBlueData_);
    void* chromaRed = env_->GetDirectBufferAddress(chromaRedData_);

    unsigned int luminanceDataSize = static_cast<unsigned int>(env_->GetDirectBufferCapacity(luminanceData_));
    unsigned int chromaBlueDataSize = static_cast<unsigned int>(env_->GetDirectBufferCapacity(chromaBlueData_));
    unsigned int chromaRedDataSize = static_cast<unsigned int>(env_->GetDirectBufferCapacity(chromaRedData_));

    std::vector<wikitude::sdk::CameraFramePlane> planes = {
            wikitude::sdk::CameraFramePlane(luminance, luminanceDataSize, 1, rowStrideLuminance_),
            wikitude::sdk::CameraFramePlane(chromaBlue, chromaBlueDataSize, pixelStrideChroma_, rowStrideChroma_),
            wikitude::sdk::CameraFramePlane(chromaRed, chromaRedDataSize, pixelStrideChroma_, rowStrideChroma_)
    };

    module->notifyNewCameraFrameYUV_420_888(planes);
}

extern "C" JNIEXPORT void JNICALL Java_com_wikitude_samples_advanced_plugins_input_FrameInputPluginModule_nativeNotifyNewCameraFrameNV21(JNIEnv* env_, jobject, jlong nativeHandle_, jbyteArray data_) {
    auto module = reinterpret_cast<FrameInputPluginModule*>(nativeHandle_);
    jbyte* data = env_->GetByteArrayElements(data_, JNI_FALSE);
    jsize dataLength = env_->GetArrayLength(data_);

    std::vector<wikitude::sdk::CameraFramePlane> planes = {
        wikitude::sdk::CameraFramePlane(data, static_cast<unsigned int>(dataLength))
    };

    module->notifyNewCameraFrameNV21(planes);
    env_->ReleaseByteArrayElements(data_, data, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL Java_com_wikitude_samples_advanced_plugins_input_FrameInputPluginModule_nativeFieldOfViewChanged(JNIEnv*, jobject, jlong nativeHandle_, jfloat fov_) {
    auto module = reinterpret_cast<FrameInputPluginModule*>(nativeHandle_);
    module->fieldOfViewChanged(fov_);
}

extern "C" JNIEXPORT void JNICALL Java_com_wikitude_samples_advanced_plugins_input_FrameInputPluginModule_nativeCameraToSurfaceAngleChanged(JNIEnv*, jobject, jlong nativeHandle_, jfloat angle_) {
    auto module = reinterpret_cast<FrameInputPluginModule*>(nativeHandle_);
    module->cameraToSurfaceAngleChanged(angle_);
}

extern "C" JNIEXPORT void JNICALL Java_com_wikitude_samples_advanced_plugins_input_FrameInputPluginModule_nativeCameraReleased(JNIEnv*, jobject, jlong nativeHandle_) {
    auto module = reinterpret_cast<FrameInputPluginModule*>(nativeHandle_);
    module->cameraReleased();
}
