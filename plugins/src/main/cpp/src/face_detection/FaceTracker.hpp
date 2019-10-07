//
//  FaceTracker.hpp
//  SDKExamples
//
//  Created by Andreas Schacherbauer on 26.11.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#ifndef FaceTracker_hpp
#define FaceTracker_hpp

#include <string>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#include <opencv.hpp>
#pragma clang diagnostic pop

#ifdef INCLUDE_WIKITUDE_AS_USER_HEADER
    #include "Matrix4.hpp"
    #include "RuntimeParameters.hpp"
    #include "ManagedCameraFrame.hpp"
#else
    #import <WikitudeSDK/Matrix4.hpp>
    #import <WikitudeSDK/RuntimeParameters.hpp>
    #import <WikitudeSDK/ManagedCameraFrame.hpp>
#endif


class FaceTracker {
public:
    FaceTracker(long id_, const std::string& databasePath_, wikitude::sdk::RuntimeParameters* runtimeParameters_, const std::string& temporaryDirectory_);

    bool isLoaded() const;

    void processCameraFrame(wikitude::sdk::ManagedCameraFrame& managedCameraFrame_, std::vector<wikitude::sdk::Matrix4>& augmentationData_);

protected:
    wikitude::sdk::Matrix4 convertFacePositionToViewMatrix(cv::Mat& frame_, cv::Rect& faceRect_, float cameraToSurfaceAngle_, wikitude::sdk::Size<int> cameraFrameSize_);

protected:
    long                                    _id;

    bool                                    _isDatabaseLoaded;
    std::string                             _databasePath;
    cv::Mat                                 _grayFrame;

    wikitude::sdk::RuntimeParameters*       _runtimeParameters;

    cv::CascadeClassifier                   _cascadeDetector;
};
#endif /* FaceTracker_hpp */
