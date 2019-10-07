//
//  FaceTracker.cpp
//  SDKExamples
//
//  Created by Andreas Schacherbauer on 26.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "FaceTracker.hpp"

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <jni.h>

#include <fstream>

static AAsset* s_asset;

extern "C" JNIEXPORT void JNICALL Java_com_wikitude_samples_advanced_plugins_FaceDetectionPluginExtension_passContext(JNIEnv* env_, jobject obj_, jobject context_) {
    jclass contextClass = env_->GetObjectClass(context_);
    jmethodID getAssetManager = env_->GetMethodID(contextClass, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject jAssetManager = env_->CallObjectMethod(context_, getAssetManager);

    AAssetManager* assetManager = AAssetManager_fromJava(env_, jAssetManager);
    AAsset* asset = AAssetManager_open(assetManager, "samples/13_PluginsAPI_2_FaceDetection/assets/high_database.xml", AASSET_MODE_RANDOM);

    s_asset = asset;
}

FaceTracker::FaceTracker(long id_, const std::string& databasePath_, wikitude::sdk::RuntimeParameters* runtimeParameters_, const std::string& temporaryDirectory_)
: _id(id_)
, _isDatabaseLoaded(false)
, _databasePath(databasePath_)
, _runtimeParameters(runtimeParameters_)
{
    if ( _databasePath.compare(0, 21, "file:///android_asset") == 0 ) {
        if (s_asset != nullptr) {
            const void* data = AAsset_getBuffer(s_asset);
            off_t length = AAsset_getLength(s_asset);

            std::string outputFilePath_ = temporaryDirectory_ + "/" + "face_detection_database.xml";

            std::fstream out(outputFilePath_, std::ios::out | std::ios::binary);
            out.write(static_cast<const char*>(data), length);
            out.close();

            _databasePath = outputFilePath_;
        }
    }
    else if ( _databasePath.compare(0, 7, "file://") == 0 ) {
        _databasePath = _databasePath.substr(7, _databasePath.length());
    }
    _isDatabaseLoaded = _cascadeDetector.load(_databasePath);
}

bool FaceTracker::isLoaded() const {
    return _isDatabaseLoaded;
}

void FaceTracker::processCameraFrame(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_, std::vector<wikitude::sdk::Matrix4>& augmentationData_) {

    if ( !_isDatabaseLoaded ) {
        return;
    }
    
    if ( _grayFrame.empty() ) {
        wikitude::sdk::Size<int> cameraFrameSize = managedCameraFrame_.getColorMetadata().getPixelSize();
        _grayFrame = cv::Mat(cameraFrameSize.height, cameraFrameSize.width, CV_8UC1);
    }

    wikitude::sdk::Size<int> cameraFrameSize = managedCameraFrame_.getColorMetadata().getPixelSize();

    // only one plane that contains luminance and chrominance
    const unsigned luminanceDataSize = managedCameraFrame_.get()[0].getDataSize() * 2 / 3;

    std::memcpy(_grayFrame.data, managedCameraFrame_.get()[0].getData(), luminanceDataSize);

    cv::Mat smallImg = cv::Mat(static_cast<int>(cameraFrameSize.height * 0.5f), static_cast<int>(cameraFrameSize.width * 0.5f), CV_8UC1);
    cv::resize(_grayFrame, smallImg, smallImg.size(), CV_INTER_AREA);

    /* Depending on the device orientation, the camera frame needs to be rotated in order to detect faces in it */
    float currentCameraToSurfaceAngle = _runtimeParameters->getCameraToSurfaceAngle();
    if (currentCameraToSurfaceAngle == 90) {
        cv::transpose(smallImg, smallImg);
        cv::flip(smallImg, smallImg, 1);
    } else if (currentCameraToSurfaceAngle == 180) {
        cv::flip(smallImg, smallImg, 0);
    } else if (currentCameraToSurfaceAngle == 270) {
        cv::transpose(smallImg, smallImg);
        cv::flip(smallImg, smallImg, -1);
    } else if (currentCameraToSurfaceAngle == 0) {
        // nop for landscape right
    }

    cv::Rect crop = cv::Rect(smallImg.cols / 4, smallImg.rows / 4, smallImg.cols / 2, smallImg.rows / 2);
    cv::Mat croppedImg = smallImg(crop);

    std::vector<cv::Rect> results;
    _cascadeDetector.detectMultiScale(croppedImg, results, 1.1, 2, 0, cv::Size(20, 20));
    
    for ( cv::Rect& facePosition : results ) {
        augmentationData_.emplace_back(convertFacePositionToViewMatrix(croppedImg, facePosition, currentCameraToSurfaceAngle, cameraFrameSize));
    }
}

wikitude::sdk::Matrix4 FaceTracker::convertFacePositionToViewMatrix(cv::Mat& frame_, cv::Rect& faceRect_, float cameraToSurfaceAngle_, wikitude::sdk::Size<int> cameraFrameSize_) {
    float centeredX = (float) faceRect_.x + (faceRect_.width * .5f);
    float centeredY = (float) faceRect_.y + (faceRect_.height * .5f);

    float xtmp = centeredX * 4.0f;
    float ytmp = centeredY * 4.0f;

    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    float scaleX = 0.0f;
    float scaleY = 0.0f;

    wikitude::sdk::Scale2D<float> cameraScaling = _runtimeParameters->getCameraToSurfaceScaling();
    float cameraFrameAspectRatio = static_cast<float>(cameraFrameSize_.height) / static_cast<float>(cameraFrameSize_.width);

    if (cameraToSurfaceAngle_ == 90 || cameraToSurfaceAngle_ == 270) {
        y = (ytmp / cameraFrameSize_.width) - 0.5f;

        scaleX = ((float) faceRect_.width / (float) frame_.rows * cameraScaling.x);
        scaleY = ((float) faceRect_.height / (float) frame_.cols);
    } else {
        y = (ytmp / cameraFrameSize_.width) - 0.5f * cameraFrameAspectRatio;

        scaleX = ((float) faceRect_.width / (float) frame_.cols);
        scaleY = ((float) faceRect_.height / (float) frame_.rows * cameraScaling.y);
    }

    if (cameraToSurfaceAngle_ == 90) {
        x = (xtmp / cameraFrameSize_.width) - 0.5f * cameraFrameAspectRatio;
    } else if (cameraToSurfaceAngle_ == 180) {
        x = 0.5f - (xtmp / cameraFrameSize_.width);
    } else if (cameraToSurfaceAngle_ == 270) {
        x = 0.5f * cameraFrameAspectRatio - (xtmp / cameraFrameSize_.width);
    } else {
        x = (xtmp / cameraFrameSize_.width) - 0.5f;
    }

    return {
        scaleX, 0.f,    0.f,  0.f,
        0.f,    scaleY, 0.f,  0.f,
        0.f,    0.f,    1.f,  0.f,
        x,       -y,      z,  1.f
    };
}
