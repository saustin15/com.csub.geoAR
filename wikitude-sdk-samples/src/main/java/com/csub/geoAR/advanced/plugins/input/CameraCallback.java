package com.csub.geoAR.advanced.plugins.input;

import java.nio.ByteBuffer;

interface CameraCallback {
    void notifyNewCameraFrameYUV420888(ByteBuffer luminanceData, ByteBuffer chromaBlueData, ByteBuffer chromaRedData, int rowStrideLuminance, int pixelStrideChroma, int rowStrideChroma);

    void notifyNewCameraFrameNV21(byte[] data);

    void fieldOfViewChanged(float fov);

    void cameraReleased();
}
