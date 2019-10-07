//
//  MarkerTracker.cpp
//  WikitudeSDKTestsHost
//
//  Created by Andreas Schacherbauer on 30.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "MarkerTracker.hpp"

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "Geometry.hpp"
    #include "ManagedCameraFrame.hpp"
    #include "RuntimeParameters.hpp"
#else
    #import <WikitudeSDK/Geometry.hpp>
    #import <WikitudeSDK/ManagedCameraFrame.hpp>
    #import <WikitudeSDK/RuntimeParameters.hpp>
#endif


MarkerTracker::MarkerTracker(wikitude::sdk::RuntimeParameters* runtimeParameters_)
:
_runtimeParameters(runtimeParameters_)
{ /* Intentionally Left Blank */ }

void MarkerTracker::processCameraFrame(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_, std::function<void(int markerId_, wikitude::sdk::Matrix4& matrix_)> markerConversionHandler_) {
    if (!managedCameraFrame_.get()[0].getData()) {
        return;
    }

    const wikitude::sdk::Size<int>& cameraFrameSize = managedCameraFrame_.getColorMetadata().getPixelSize();
    const void* data = managedCameraFrame_.get()[0].getData();
    const cv::Mat frameLuminance = [&](){
        switch (managedCameraFrame_.getColorMetadata().getFrameColorSpace()) {
            case wikitude::sdk::ColorSpace::YUV_420_NV21:
            case wikitude::sdk::impl::YUV_420_NV12:
            case wikitude::sdk::impl::YUV_420_YV12:
            case wikitude::sdk::impl::YUV_420_888: {
                size_t rowStride = static_cast<size_t>(cameraFrameSize.width);
                if (managedCameraFrame_.get()[0].getRowStride() > 0) {
                    rowStride = static_cast<size_t>(managedCameraFrame_.get()[0].getRowStride());
                }
                return cv::Mat(cameraFrameSize.height, cameraFrameSize.width, CV_8UC1, const_cast<void*>(data), rowStride);
            }
            case wikitude::sdk::ColorSpace::RGB:
                return cv::Mat(cameraFrameSize.height, cameraFrameSize.width, CV_8UC3, const_cast<void *>(data));

            case wikitude::sdk::impl::UNKNOWN:
            case wikitude::sdk::impl::RGBA:
            default:
                return cv::Mat(0, 0, CV_8UC1, nullptr);
        }
    }();

    cv::Mat cameraMatrix = cv::Mat::zeros(3, 3, CV_32F);

    // calculate the focal length in pixels (fx, fy)
    // assumes an iPhone5
    const float focalLengthInMillimeter = 4.12f;
    const float CCDWidthInMillimeter = 4.536f;
    const float CCDHeightInMillimeter = 3.416f;

    const float focalLengthInPixelsX = cameraFrameSize.width * focalLengthInMillimeter / CCDWidthInMillimeter;
    const float focalLengthInPixelsY = cameraFrameSize.height * focalLengthInMillimeter / CCDHeightInMillimeter;

    cameraMatrix.at<float>(0, 0) = focalLengthInPixelsX;
    cameraMatrix.at<float>(1, 1) = focalLengthInPixelsY;

    // calculate the frame center (cx, cy)
    cameraMatrix.at<float>(0, 2) = 0.5f * cameraFrameSize.width;
    cameraMatrix.at<float>(1, 2) = 0.5f * cameraFrameSize.height;

    cameraMatrix.at<float>(2, 2) = 1.0f;

    const float markerSizeInMeters = 0.1f;

    std::vector<aruco::Marker> markers;
    _detector.detect(frameLuminance, markers, cameraMatrix, cv::Mat(), markerSizeInMeters);

    double viewMatrixData[16];

    for (auto& marker : markers) {
        
        wikitude::sdk::Matrix4 viewMatrix; /* viewMatrix is set to ::identity at this time */
        for (int i = 0; i < 16; ++i) {
            viewMatrixData[i] = viewMatrix[i];
        }
        
        marker.calculateExtrinsics(markerSizeInMeters, cameraMatrix, cv::Mat(), false);
        marker.glGetModelViewMatrix(viewMatrixData);
        
        
        const wikitude::sdk::Scale2D<float>& cameraToSurfaceScaling = _runtimeParameters->getCameraToSurfaceScaling();
        wikitude::sdk::Matrix4 aspectRatioCorrection;
        aspectRatioCorrection.scale(cameraToSurfaceScaling.x, cameraToSurfaceScaling.y, 1.0f);
        
        for (int i = 0; i < 16; ++i) {
            viewMatrix[i] = static_cast<float>(viewMatrixData[i]);
        }
        
        // OpenCV left handed coordinate system to OpenGL right handed coordinate system
        viewMatrix.scale(1.0f, -1.0f, 1.0f);

        
        wikitude::sdk::Matrix4 modelViewMatrix;
        if ( _runtimeParameters->getCameraToSurfaceAngle() == 90.f || _runtimeParameters->getCameraToSurfaceAngle() == 270.f ) { /* we need a better comparison here */
            const float aspectRatio = static_cast<float>(cameraFrameSize.width) / static_cast<float>(cameraFrameSize.height);
            wikitude::sdk::Matrix4 portraitAndUpsideDownCorrection;
            portraitAndUpsideDownCorrection.scale(aspectRatio, 1.0f / aspectRatio, 1.0f);
            
            modelViewMatrix *= portraitAndUpsideDownCorrection;
        }
        
        modelViewMatrix *= aspectRatioCorrection;

        wikitude::sdk::Matrix4 rotation;
        float rotationAngle = 360.f - _runtimeParameters->getCameraToSurfaceAngle();
        modelViewMatrix *= rotation.rotateZ(rotationAngle);

        modelViewMatrix *= viewMatrix;
        markerConversionHandler_(marker.id, modelViewMatrix);
    }
}
