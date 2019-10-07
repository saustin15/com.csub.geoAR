package com.csub.geoAR.advanced.plugins.input;


import android.Manifest;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.camera2.*;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import java.util.Arrays;

@TargetApi(22)
public class WikitudeCamera2 implements Camera {

    private static final String TAG = "WikitudeCamera2";

    private final Object cameraClosedLock = new Object();
    private CameraManager manager;
    private Context context;
    private final CameraCallback callback;
    private int frameWidth;
    private int frameHeight;
    private CameraCaptureSession cameraCaptureSession;
    private CameraDevice cameraDevice;
    private ImageReader imageReader;
    private boolean closeCalled;
    private HandlerThread backgroundThread;
    private Handler backgroundHandler;

    private CameraCaptureSession.StateCallback sessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(CameraCaptureSession session) {
            synchronized (cameraClosedLock) {
                if (!closeCalled) {
                    cameraCaptureSession = session;
                    try {
                        session.setRepeatingRequest(createCaptureRequest(), null, backgroundHandler);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                } else {
                    closeCalled = false;
                }
            }
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
            if (closeCalled) {
                closeCalled = false;
            }
        }
    };
    private CameraDevice.StateCallback cameraStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(CameraDevice camera) {
            synchronized (cameraClosedLock) {
                if (!closeCalled) {
                    cameraDevice = camera;
                    try {
                        cameraDevice.createCaptureSession(Arrays.asList(imageReader.getSurface()), sessionStateCallback, backgroundHandler);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                } else {
                    closeCalled = false;
                }
            }
        }

        @Override
        public void onDisconnected(CameraDevice camera) {
            Log.e(TAG, "Callback function onDisconnected called.");
        }

        @Override
        public void onError(CameraDevice camera, int error) {
            if (closeCalled) {
                closeCalled = false;
            }

            String errorString = "";
            switch (error) {
                case ERROR_CAMERA_DEVICE:
                    errorString = "ERROR_CAMERA_DEVICE received, indicating that the camera device has encountered a fatal error.";
                    break;
                case ERROR_CAMERA_DISABLED:
                    errorString = "ERROR_CAMERA_DISABLED received, indicating that the camera device could not be opened due to a device policy.";
                    break;
                case ERROR_CAMERA_IN_USE:
                    errorString = "ERROR_CAMERA_IN_USE received, indicating that the camera device is in use already.";
                    break;
                case ERROR_CAMERA_SERVICE:
                    errorString = "ERROR_CAMERA_SERVICE received, indicating that the camera service has encountered a fatal error.";
                    break;
                case ERROR_MAX_CAMERAS_IN_USE:
                    errorString = "ERROR_MAX_CAMERAS_IN_USE received, indicating that the camera device could not be opened because there are too many other open camera devices.";
                    break;
            }

            Log.e(TAG, "Callback function onError called." + errorString);
        }
    };

    public WikitudeCamera2(Context context, CameraCallback callback, int frameWidth, int frameHeight) {
        this.context = context;
        this.callback = callback;

        this.frameWidth = frameWidth;
        this.frameHeight = frameHeight;

        manager = (CameraManager) this.context.getSystemService(Context.CAMERA_SERVICE);
        callback.fieldOfViewChanged((float) getCameraFieldOfViewInternal());

        closeCalled = false;
    }

    @Override
    public void start() {
        try {
            startBackgroundThread();

            if (Build.VERSION.SDK_INT >= 23 && context.checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                Log.e(TAG, "Camera Permission has been denied by the user. Aborting initialization.");
                throw new SecurityException();
            }

            imageReader = ImageReader.newInstance(frameWidth, frameHeight, ImageFormat.YUV_420_888, 2);
            manager.openCamera(getCamera(), cameraStateCallback, backgroundHandler);
            imageReader.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
                @Override
                public void onImageAvailable(final ImageReader reader) {
                    Image image = reader.acquireLatestImage();

                    if (image == null) {
                        return;
                    }

                    Image.Plane[] planes = image.getPlanes();
                    if (planes != null) {

                        int rowStrideLuminance = planes[0].getRowStride();
                        int pixelStrideChroma = planes[1].getPixelStride();
                        int rowStrideChroma = planes[1].getRowStride();

                        callback.notifyNewCameraFrameYUV420888(
                                planes[0].getBuffer(),
                                planes[1].getBuffer(),
                                planes[2].getBuffer(),
                                rowStrideLuminance,
                                pixelStrideChroma,
                                rowStrideChroma
                        );
                    }

                    image.close();
                }
            }, backgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void stop() {
        synchronized (cameraClosedLock) {
            closeCalled = true;
            try {
                if (cameraCaptureSession != null && cameraDevice != null) {
                    closeCalled = false;
                }

                if (cameraCaptureSession != null) {
                    cameraCaptureSession.abortCaptures();
                    cameraCaptureSession.close();
                    cameraCaptureSession = null;
                }

                if (cameraDevice != null) {
                    cameraDevice.close();
                    cameraDevice = null;
                }

                if (imageReader != null) {
                    imageReader.close();
                    imageReader = null;
                }
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
        stopBackgroundThread();
        callback.cameraReleased();
    }

    private String getCamera() {
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics cameraCharacteristics = manager.getCameraCharacteristics(cameraId);

                int cameraOrientation = cameraCharacteristics.get(CameraCharacteristics.LENS_FACING);
                if (cameraOrientation == CameraCharacteristics.LENS_FACING_BACK) {
                    return cameraId;
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        return null;
    }

    private double getCameraFieldOfViewInternal() {
        try {
            for (String cameraId : manager.getCameraIdList()) {
                CameraCharacteristics cameraCharacteristics = manager.getCameraCharacteristics(cameraId);

                int cameraOrientation = cameraCharacteristics.get(CameraCharacteristics.LENS_FACING);
                if (cameraOrientation == CameraCharacteristics.LENS_FACING_BACK) {
                    float sensorWidth = cameraCharacteristics.get(CameraCharacteristics.SENSOR_INFO_PHYSICAL_SIZE).getWidth();
                    float focalLength = cameraCharacteristics.get(CameraCharacteristics.LENS_INFO_AVAILABLE_FOCAL_LENGTHS)[0];
                    return Math.toDegrees(2 * Math.atan(0.5 * sensorWidth / focalLength));
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        return 0.0f;
    }

    @Override
    public int getCameraOrientation() {
        try {
            if (manager.getCameraIdList().length == 0) {
                throw new RuntimeException("The camera manager returned an empty list of available cameras. The image sensor rotation could not be evaluated.");
            } else {
                for (String cameraId : manager.getCameraIdList()) {
                    CameraCharacteristics cameraCharacteristics = manager.getCameraCharacteristics(cameraId);

                    Integer sensorOrientation = cameraCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                    if (sensorOrientation != null) {
                        return sensorOrientation;
                    }
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        // 90, 180, 270, 360 are valid values
        // as this function return an angle in degrees that is used to rotate the camera image
        // a visually easily perceivable values is chosen. Using -1 might go unnoticed as
        // a rotation this small is visually insignificant.
        return 45;
    }

    private CaptureRequest createCaptureRequest() {
        try {
            CaptureRequest.Builder builder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
            builder.addTarget(imageReader.getSurface());
            return builder.build();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        return null;
    }

    private void startBackgroundThread() {
        if (backgroundThread == null) {
            backgroundThread = new HandlerThread("CameraThread");
            backgroundThread.start();
            backgroundHandler = new Handler(backgroundThread.getLooper());
        } else {
            Log.w(TAG, "startBackgroundThread: background thread is already running");
        }
    }

    private void stopBackgroundThread() {
        if (backgroundThread != null) {
            backgroundThread.quitSafely();
            try {
                backgroundThread.join();
            } catch (InterruptedException e) {
                Log.e(TAG, "stopBackgroundThread: interruption while safely stopping the thread", e);
            } finally {
                backgroundThread = null;
                backgroundHandler = null;
            }
        } else {
            Log.w(TAG, "stopBackgroundThread: trying to stop non-existing thread");
        }
    }
}