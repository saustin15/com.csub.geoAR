package com.csub.geoAR.advanced.plugins.input;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.Nullable;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;

import java.nio.ByteBuffer;

final class FrameInputPluginModule implements CameraCallback {

    private static final int FRAME_WIDTH = 640;
    private static final int FRAME_HEIGHT = 480;

    private final long nativeHandle;
    private final Camera camera;
    private final Display display;

    @Nullable private HandlerThread backgroundThread;
    @Nullable private Handler backgroundHandler;


    FrameInputPluginModule(Context context, long nativeHandle) {
        this.nativeHandle = nativeHandle;

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP_MR1) {
            camera = new WikitudeCamera2(context, this, FRAME_WIDTH, FRAME_HEIGHT);
        } else {
            camera = new WikitudeCamera(this, FRAME_WIDTH, FRAME_HEIGHT);
        }

        WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        if (windowManager == null) {
            throw new IllegalStateException("Unable to get the WindowManager from the given context.");
        }

        display = windowManager.getDefaultDisplay();
    }

    void start() {
        camera.start();
        if (backgroundThread == null) {
            backgroundThread = new HandlerThread("FrameInputPluginModule");
            backgroundThread.start();
            backgroundHandler =  new Handler(backgroundThread.getLooper());
        }

        backgroundHandler.post(new Runnable() {
            private int lastOrientation = -1;
            private int cameraOrientation = camera.getCameraOrientation();

            @Override
            public void run() {
                int rotation = display.getRotation();
                float orientation = 0;
                if (rotation != lastOrientation) {
                    switch (rotation) {
                        case Surface.ROTATION_0:
                            orientation = 0f;
                            break;
                        case Surface.ROTATION_90:
                            orientation = 90f;
                            break;
                        case Surface.ROTATION_180:
                            orientation = 180f;
                            break;
                        case Surface.ROTATION_270:
                            orientation = 270f;
                            break;
                    }
                    lastOrientation = rotation;

                    float camToSurfaceAngle = cameraOrientation - orientation;
                    if (camToSurfaceAngle < 0) {
                        camToSurfaceAngle += 360;
                    }

                    if (nativeHandle != 0) {
                        nativeCameraToSurfaceAngleChanged(nativeHandle, camToSurfaceAngle);
                    }
                }
                backgroundHandler.postDelayed(this, 50);
            }
        });
    }

    void stop() {
        if (backgroundThread != null) {
            backgroundThread.quitSafely();
            try {
                backgroundThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                backgroundThread = null;
                backgroundHandler = null;
            }
        }
        camera.stop();
    }


    @Override
    public void notifyNewCameraFrameYUV420888(final ByteBuffer luminanceData, final ByteBuffer chromaBlueData, final ByteBuffer chromaRedData, final int rowStrideLuminance, final int
            pixelStrideChroma, final int rowStrideChroma) {
        if (nativeHandle != 0) {
            nativeNotifyNewCameraFrameYUV420888(
                    nativeHandle,
                    luminanceData,
                    chromaBlueData,
                    chromaRedData,
                    rowStrideLuminance,
                    pixelStrideChroma,
                    rowStrideChroma
            );
        }
    }

    @Override
    public void notifyNewCameraFrameNV21(final byte[] data) {
        if (nativeHandle != 0) {
            nativeNotifyNewCameraFrameNV21(nativeHandle, data);
        }
    }

    @Override
    public void fieldOfViewChanged(final float fov) {
        if (nativeHandle != 0) {
            nativeFieldOfViewChanged(nativeHandle, fov);
        }
    }

    @Override
    public void cameraReleased() {
        if (nativeHandle != 0) {
            nativeCameraReleased(nativeHandle);
        }
    }

    private native void nativeNotifyNewCameraFrameYUV420888(long nativeHandle, ByteBuffer luminanceData, ByteBuffer chromaBlueData, ByteBuffer chromaRedData,int rowStrideLuminance, int pixelStrideChroma,
                                                            int rowStrideChroma);
    private native void nativeNotifyNewCameraFrameNV21(long nativeHandle, byte[] data);
    private native void nativeFieldOfViewChanged(long nativeHandle, float fov);
    private native void nativeCameraToSurfaceAngleChanged(long nativeHandle, float angle);
    private native void nativeCameraReleased(long nativeHandle);

}
