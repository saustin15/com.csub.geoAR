package com.csub.geoAR.advanced.plugins;

import com.csub.geoAR.advanced.ArchitectViewExtension;
import com.wikitude.architect.ArchitectView;
import com.wikitude.common.plugins.PluginManager;
import com.csub.geoAR.R;

import android.app.Activity;
import android.util.Log;
import android.widget.Toast;

/**
 * This Extension is the java counterpart of the 13_PluginsAPI_1_QR&Barcode AR-Experience.
 * It registers a native Plugin which uses camera frames provided by the Wikitude SDK for
 * QR- and Barcode detection and will modify the AR-Experience from the native code to display it.
 */
public class QrPluginExtension extends ArchitectViewExtension {

    private static final String TAG = "QrPluginExtension";

    public QrPluginExtension(Activity activity, ArchitectView architectView) {
        super(activity, architectView);
    }

    @Override
    public void onPostCreate() {
        /*
         * Registers the plugin with the name "barcode".
         * The library containing the native plugin is libwikitudePlugins.so.
         */
        architectView.registerNativePlugins("wikitudePlugins", "barcode", new PluginManager.PluginErrorCallback() {
            @Override
            public void onRegisterError(int errorCode, String errorMessage) {
                Toast.makeText(activity, R.string.error_loading_plugins, Toast.LENGTH_SHORT).show();
                Log.e(TAG, "Plugin failed to load. Reason: " + errorMessage);
            }
        });
    }
}
