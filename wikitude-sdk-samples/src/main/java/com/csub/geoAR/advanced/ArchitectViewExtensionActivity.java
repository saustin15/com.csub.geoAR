package com.csub.geoAR.advanced;

import com.wikitude.architect.ArchitectView;
import com.csub.geoAR.SimpleArActivity;
import com.csub.geoAR.advanced.plugins.FaceDetectionPluginExtension;
import com.csub.geoAR.advanced.plugins.MarkerTrackingPluginExtension;
import com.csub.geoAR.advanced.plugins.QrPluginExtension;
import com.csub.geoAR.advanced.plugins.input.CustomCameraExtension;
import com.csub.geoAR.advanced.plugins.input.SimpleInputPluginExtension;
import com.csub.geoAR.util.SampleData;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.util.HashMap;
import java.util.Map;

public class ArchitectViewExtensionActivity extends SimpleArActivity {

    private static final String EXTENSION_APPLICATION_MODEL_POIS = "application_model_pois";
    private static final String EXTENSION_GEO = "geo";
    private static final String EXTENSION_NATIVE_DETAIL = "native_detail";
    private static final String EXTENSION_SCREENSHOT = "screenshot";
    private static final String EXTENSION_SAVE_LOAD_INSTANT_TARGET = "save_load_instant_target";
    private static final String EXTENSION_CUSTOM_CAMERA = "custom_camera";
    private static final String EXTENSION_SIMPLE_INPUT = "simple_input";
    private static final String EXTENSION_FACE_DETECTION = "face_detection";
    private static final String EXTENSION_QR_CODE = "qr_code";
    private static final String EXTENSION_MARKER_TRACKING = "marker_tracking";

    private final Map<String, ArchitectViewExtension> extensions = new HashMap<>();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();

        SampleData sampleData = (SampleData) intent.getSerializableExtra(SimpleArActivity.INTENT_EXTRAS_KEY_SAMPLE);

        for (String extension : sampleData.getExtensions()) {
            switch (extension) {
                case EXTENSION_APPLICATION_MODEL_POIS:
                    extensions.put(EXTENSION_APPLICATION_MODEL_POIS, new PoiDataFromApplicationModelExtension(this, architectView));
                    break;
                case EXTENSION_GEO:
                    extensions.put(EXTENSION_GEO, new GeoExtension(this, architectView));
                    break;
                case EXTENSION_NATIVE_DETAIL:
                    extensions.put(EXTENSION_NATIVE_DETAIL, new NativePoiDetailExtension(this, architectView));
                    break;
                case EXTENSION_SCREENSHOT:
                    extensions.put(EXTENSION_SCREENSHOT, new ScreenshotSaverExtension(this, architectView));
                    break;
                case EXTENSION_SAVE_LOAD_INSTANT_TARGET:
                    extensions.put(EXTENSION_SAVE_LOAD_INSTANT_TARGET, new SaveLoadInstantTargetExtension(this, architectView));
                    break;
                case EXTENSION_CUSTOM_CAMERA:
                    extensions.put(EXTENSION_CUSTOM_CAMERA, new CustomCameraExtension(this, architectView));
                    break;
                case EXTENSION_SIMPLE_INPUT:
                    extensions.put(EXTENSION_SIMPLE_INPUT, new SimpleInputPluginExtension(this, architectView));
                    break;
                case EXTENSION_FACE_DETECTION:
                    extensions.put(EXTENSION_FACE_DETECTION, new FaceDetectionPluginExtension(this, architectView));
                    break;
                case EXTENSION_QR_CODE:
                    extensions.put(EXTENSION_QR_CODE, new QrPluginExtension(this, architectView));
                    break;
                case EXTENSION_MARKER_TRACKING:
                    extensions.put(EXTENSION_MARKER_TRACKING, new MarkerTrackingPluginExtension(this, architectView));
                    break;
            }
        }

        if (extensions.containsKey(EXTENSION_GEO) && extensions.containsKey(EXTENSION_APPLICATION_MODEL_POIS)) {
            PoiDataFromApplicationModelExtension applicationModelExtension = (PoiDataFromApplicationModelExtension) extensions.get(EXTENSION_APPLICATION_MODEL_POIS);
            GeoExtension geoExtension = (GeoExtension) extensions.get(EXTENSION_GEO);
            geoExtension.setLocationListenerExtension(applicationModelExtension);
        }

        for (ArchitectViewExtension extension : extensions.values()) {
            extension.onCreate();
        }
    }

    @Override
    protected void onPostCreate(@Nullable Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);

        for (ArchitectViewExtension extension : extensions.values()) {
            extension.onPostCreate();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        for (ArchitectViewExtension extension : extensions.values()) {
            extension.onResume();
        }
    }

    @Override
    protected void onPause() {
        for (ArchitectViewExtension extension : extensions.values()) {
            extension.onPause();
        }

        super.onPause();
    }

    @Override
    protected void onDestroy() {
        for (ArchitectViewExtension extension : extensions.values()) {
            extension.onDestroy();
        }
        extensions.clear();

        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        ArchitectView.getPermissionManager().onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
