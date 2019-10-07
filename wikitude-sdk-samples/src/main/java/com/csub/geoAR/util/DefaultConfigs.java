package com.csub.geoAR.util;

import com.csub.geoAR.SimpleArActivity;
import com.wikitude.common.devicesupport.Feature;

import java.util.EnumSet;

import static com.wikitude.common.camera.CameraSettings.*;

public class DefaultConfigs {

    private DefaultConfigs() {}

    // Defaults configuration for samples
    public static final Class DEFAULT_ACTIVITY = SimpleArActivity.class;
    public static final CameraPosition DEFAULT_CAMERA_POSITION = CameraPosition.DEFAULT;
    public static final CameraResolution DEFAULT_CAMERA_RESOLUTION = CameraResolution.SD_640x480;
    public static final CameraFocusMode DEFAULT_CAMERA_FOCUS_MODE = CameraFocusMode.CONTINUOUS;
    public static final boolean DEFAULT_CAMERA_2_ENABLED = true;
    public static final EnumSet<Feature> DEFAULT_AR_FEATURES = EnumSet.allOf(Feature.class);

}
