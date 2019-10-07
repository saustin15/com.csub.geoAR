package com.csub.geoAR.util;

import com.wikitude.common.camera.CameraSettings;
import com.wikitude.common.devicesupport.Feature;
import com.csub.geoAR.SimpleArActivity;
import com.csub.geoAR.SimpleGeoArActivity;
import com.csub.geoAR.advanced.ArchitectViewExtensionActivity;

import java.util.EnumSet;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Context;
import android.support.annotation.NonNull;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import static com.csub.geoAR.util.DefaultConfigs.DEFAULT_CAMERA_2_ENABLED;

public class SampleJsonParser {

    private static final String CATEGORY_NAME = "category_name";
    private static final String CATEGORY_SAMPLES = "samples";
    private static final String SAMPLE_NAME = "name";
    private static final String SAMPLE_PATH = "path";
    private static final String REQUIRED = "requiredFeatures";
    private static final String REQUIRED_GEO = "geo";
    private static final String REQUIRED_IMAGE_TRACKING = "image_tracking";
    private static final String REQUIRED_INSTANT_TRACKING = "instant_tracking";
    private static final String REQUIRED_OBJECT_TRACKING = "object_tracking";
    private static final String STARTUP_CONFIGURATION = "startupConfiguration";
    private static final String CAMERA_POSITION = "camera_position";
    private static final String CAMERA_POSITION_FRONT = "front";
    private static final String CAMERA_POSITION_BACK = "back";
    private static final String CAMERA_RESOLUTION = "camera_resolution";
    private static final String CAMERA_RESOLUTION_AUTO = "auto";
    private static final String CAMERA_RESOLUTION_SD = "sd_640x480";
    private static final String CAMERA_2_ENABLED = "camera_2_enabled";
    private static final String EXTENSION_REQUIRED = "required_extensions";

    private SampleJsonParser() { }

    @NonNull
    public static List<SampleCategory> getCategoriesFromJsonString(@NonNull String jsonString) {
        final List<SampleCategory> list = new ArrayList<>();

        try {
            JSONArray categoryArray = new JSONArray(jsonString);
            for (int i = 0; i < categoryArray.length(); i++) {
                final JSONObject category = categoryArray.getJSONObject(i);

                final String category_name = category.getString(CATEGORY_NAME);
                final JSONArray samplesArray = category.getJSONArray(CATEGORY_SAMPLES);

                final List<SampleData> sampleData = new ArrayList<>();

                for (int u = 0; u < samplesArray.length(); u++) {
                    final JSONObject sample = samplesArray.getJSONObject(u);

                    final String name = sample.getString(SAMPLE_NAME);
                    final String path = sample.getString(SAMPLE_PATH);

                    final JSONArray requiredFeatures = sample.getJSONArray(REQUIRED);
                    final EnumSet<Feature> features = parseRequiredFeatures(requiredFeatures);

                    final JSONObject startupConfig = sample.getJSONObject(STARTUP_CONFIGURATION);

                    final CameraSettings.CameraPosition cameraPosition = parseCameraPositionFromConfig(startupConfig);
                    final CameraSettings.CameraResolution cameraResolution = parseCameraResolutionFromConfig(startupConfig);
                    final boolean camera2Enabled = parseCamera2EnabledFromConfig(startupConfig);
                    List<String> extensions = parseActivityExtensions(sample);
                    Class activityClass;

                    if (extensions != null) {
                        activityClass = ArchitectViewExtensionActivity.class;
                    } else if (features.contains(Feature.GEO)) {
                        activityClass = SimpleGeoArActivity.class;
                    } else {
                        activityClass = SimpleArActivity.class;
                    }

                    final SampleData data = new SampleData.Builder(name, path)
                            .activityClass(activityClass)
                            .extensions(extensions)
                            .arFeatures(features)
                            .cameraPosition(cameraPosition)
                            .cameraResolution(cameraResolution)
                            .camera2Enabled(camera2Enabled)
                            .build();

                    sampleData.add(data);
                }
                list.add(new SampleCategory(sampleData, category_name));
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }

        return list;
    }

    private static List<String> parseActivityExtensions(@NonNull JSONObject sample) {
        try {
            if (sample.has(EXTENSION_REQUIRED)) {
                final JSONArray extensions = sample.getJSONArray(EXTENSION_REQUIRED);

                final List<String> parsedExtensions = new ArrayList<>();

                for (int i = 0; i < extensions.length(); i++) {
                    String extension = extensions.getString(i);
                    parsedExtensions.add(extension);
                }
                return parsedExtensions;
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    private static EnumSet<Feature> parseRequiredFeatures(@NonNull JSONArray requiredFeatures) {
        try {
            if (requiredFeatures.length() > 0) {
                EnumSet<Feature> features = EnumSet.noneOf(Feature.class);
                for (int i = 0; i < requiredFeatures.length(); i++) {
                    final String feature = requiredFeatures.getString(i);

                    switch (feature) {
                        case REQUIRED_GEO: {
                            features.add(Feature.GEO);
                            break;
                        }
                        case REQUIRED_IMAGE_TRACKING: {
                            features.add(Feature.IMAGE_TRACKING);
                            break;
                        }
                        case REQUIRED_INSTANT_TRACKING: {
                            features.add(Feature.INSTANT_TRACKING);
                            break;
                        }
                        case REQUIRED_OBJECT_TRACKING: {
                            features.add(Feature.OBJECT_TRACKING);
                            break;
                        }
                    }
                }
                return features;
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }
        return DefaultConfigs.DEFAULT_AR_FEATURES;
    }

    private static CameraSettings.CameraPosition parseCameraPositionFromConfig(@NonNull JSONObject startupConfig) {
        try {
            if (startupConfig.has(CAMERA_POSITION)){
                final String position = startupConfig.getString(CAMERA_POSITION);
                switch (position) {
                    case CAMERA_POSITION_BACK:
                        return CameraSettings.CameraPosition.BACK;
                    case CAMERA_POSITION_FRONT:
                        return CameraSettings.CameraPosition.FRONT;
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    private static CameraSettings.CameraResolution parseCameraResolutionFromConfig(@NonNull JSONObject startupConfig) {
        try {
            if (startupConfig.has(CAMERA_RESOLUTION)){
                final String position = startupConfig.getString(CAMERA_RESOLUTION);
                switch (position) {
                    case CAMERA_RESOLUTION_AUTO:
                        return CameraSettings.CameraResolution.AUTO;
                    case CAMERA_RESOLUTION_SD:
                        return CameraSettings.CameraResolution.SD_640x480;
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    private static boolean parseCamera2EnabledFromConfig(@NonNull JSONObject startupConfig) {
        try {
            if (startupConfig.has(CAMERA_2_ENABLED)){
                final String enabled = startupConfig.getString(CAMERA_2_ENABLED);
                return Boolean.valueOf(enabled);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return DEFAULT_CAMERA_2_ENABLED;
    }

    @NonNull
    public static String loadStringFromAssets(@NonNull Context context, @NonNull String path) {

        try {
            final InputStream inputStream = context.getAssets().open(path);
            final int size = inputStream.available();
            final byte[] buffer = new byte[size];
            inputStream.read(buffer);
            inputStream.close();

            return new String(buffer, "UTF-8");

        } catch (IOException e) {
            e.printStackTrace();
        }
        return "";
    }
}
