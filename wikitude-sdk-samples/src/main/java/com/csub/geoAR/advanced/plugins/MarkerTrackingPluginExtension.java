package com.csub.geoAR.advanced.plugins;

import com.csub.geoAR.advanced.ArchitectViewExtension;
import com.wikitude.architect.ArchitectView;
import com.wikitude.common.plugins.PluginManager;
import com.csub.geoAR.R;

import android.app.Activity;
import android.util.Log;
import android.widget.Toast;

/**
 * This Extension is the java counterpart of the 13_PluginsAPI_5_MarkerTracking AR-Experience.
 * It registers a native Plugin which uses camera frames provided by the Wikitude SDK for
 * aruco marker detection and will render the augmentations using the Wikitude positionables api.
 */
public class MarkerTrackingPluginExtension extends ArchitectViewExtension {

    private static final String TAG = "MarkerTrackingPlugin";

    public MarkerTrackingPluginExtension(Activity activity, ArchitectView architectView) {
        super(activity, architectView);
    }

    @Override
    public void onPostCreate() {
        /*
         * Registers the plugin with the name "markertracking".
         * The library containing the native plugin is libwikitudePlugins.so.
         */
        architectView.registerNativePlugins("wikitudePlugins", "markertracking", new PluginManager.PluginErrorCallback() {
            @Override
            public void onRegisterError(int errorCode, String errorMessage) {
                Toast.makeText(activity, R.string.error_loading_plugins, Toast.LENGTH_SHORT).show();
                Log.e(TAG, "Plugin failed to load. Reason: " + errorMessage);
            }
        });
    }
}
