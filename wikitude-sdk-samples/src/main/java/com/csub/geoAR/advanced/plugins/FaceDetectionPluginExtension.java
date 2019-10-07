package com.csub.geoAR.advanced.plugins;

import com.csub.geoAR.advanced.ArchitectViewExtension;
import com.wikitude.architect.ArchitectView;
import com.wikitude.common.plugins.PluginManager;
import com.csub.geoAR.R;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.widget.Toast;

/**
 * This Extension is the java counterpart of the 13_PluginsAPI_2_FaceDetection AR-Experience.
 * It registers a native Plugin which uses camera frames provided by the Wikitude SDK for
 * face detection and will render the augmentations in java.
 */
public class FaceDetectionPluginExtension extends ArchitectViewExtension {

    private static final String TAG = "FaceDetectionPlugin";

    public FaceDetectionPluginExtension(final Activity activity, final ArchitectView architectView) {
        super(activity, architectView);
    }

    @Override
    public void onPostCreate() {
        /*
         * Registers the plugin with the name "face_detection".
         * The library containing the native plugin is libwikitudePlugins.so.
         */
        architectView.registerNativePlugins("wikitudePlugins", "face_detection", new PluginManager.PluginErrorCallback() {
            @Override
            public void onRegisterError(int errorCode, String errorMessage) {
                Toast.makeText(activity, R.string.error_loading_plugins, Toast.LENGTH_SHORT).show();
                Log.e(TAG, "Plugin failed to load. Reason: " + errorMessage);
            }
        });

        passContext(this.activity);
    }

    private native void passContext(Context context);
}
