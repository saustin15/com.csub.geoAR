package com.csub.geoAR.advanced.plugins.input;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.util.Log;

import java.io.IOException;

public class WikitudeCamera implements com.csub.geoAR.advanced.plugins.input.Camera, Camera.ErrorCallback {

    private static final String TAG = "WikitudeCamera";

    private final CameraCallback callback;
    private int frameWidth;
    private int frameHeight;
    private Camera camera;
    private Camera.Parameters cameraParameters;
    private Object texture;

    public WikitudeCamera(CameraCallback callback, int frameWidth, int frameHeight) {
        this.callback = callback;
        this.frameWidth = frameWidth;
        this.frameHeight = frameHeight;
    }

    @Override
    public void start() {
        try {
            camera = Camera.open(getCamera());
            camera.setErrorCallback(this);
            camera.setPreviewCallback(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(final byte[] data, final Camera camera) {
                    callback.notifyNewCameraFrameNV21(data);
                }
            });
            cameraParameters = camera.getParameters();
            cameraParameters.setPreviewFormat(ImageFormat.NV21);
            Camera.Size cameraSize = getCameraSize(frameWidth, frameHeight);
            cameraParameters.setPreviewSize(cameraSize.width, cameraSize.height);
            final double fieldOfView = cameraParameters.getHorizontalViewAngle();
            callback.fieldOfViewChanged((float) fieldOfView);
            camera.setParameters(cameraParameters);
            texture = new SurfaceTexture(0);
            camera.setPreviewTexture((SurfaceTexture) texture);
            camera.startPreview();
        } catch (IOException ex) {
            ex.printStackTrace();
        } catch (RuntimeException ex) {
            Log.e(TAG, "Camera not found: " + ex);
        }
    }

    @Override
    public void stop() {
        try {
            if (camera != null) {
                camera.setPreviewCallback(null);
                camera.stopPreview();
                camera.release();
                camera = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        callback.cameraReleased();
    }

    @Override
    public void onError(int error, Camera camera) {
        if (this.camera != null) {
            this.camera.release();
            this.camera = null;
        }
    }

    private Camera.Size getCameraSize(int desiredWidth, int desiredHeight) {
        for (Camera.Size size : cameraParameters.getSupportedPreviewSizes()) {
            if (size.width==desiredWidth && size.height==desiredHeight) {
                return size;
            }
        }
        return cameraParameters.getSupportedPreviewSizes().get(0);
    }

    private int getCamera() {
        try {
            int numberOfCameras = Camera.getNumberOfCameras();
            final Camera.CameraInfo cameraInfo = new Camera.CameraInfo();

            for (int cameraId = 0; cameraId < numberOfCameras; cameraId++) {
                Camera.getCameraInfo(cameraId, cameraInfo);

                if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                    return cameraId;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return -1;
    }

    @Override
    public int getCameraOrientation() {
        Camera.CameraInfo cameraInfo = new Camera.CameraInfo();

        int cameraId = getCamera();

        if (cameraId != -1) {
            Camera.getCameraInfo(cameraId, cameraInfo);
            return cameraInfo.orientation;
        } else {
            throw new RuntimeException("The getCamera function failed to return a valid camera ID. The image sensor rotation could therefore not be evaluated.");
        }
    }
}
