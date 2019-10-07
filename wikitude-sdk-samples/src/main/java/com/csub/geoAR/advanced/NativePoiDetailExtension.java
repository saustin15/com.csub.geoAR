package com.csub.geoAR.advanced;

import com.wikitude.architect.ArchitectJavaScriptInterfaceListener;
import com.wikitude.architect.ArchitectView;
import com.csub.geoAR.R;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.content.Intent;
import android.widget.Toast;

/**
 * This Extension is used for the NativeDetailScreen AR-Experience.
 * It is used to start a new Activity that displays the details of the clicked POI.
 *
 * For this functionality it implements an ArchitectJavaScriptInterfaceListener.
 */
public class NativePoiDetailExtension extends ArchitectViewExtension implements ArchitectJavaScriptInterfaceListener {

    public NativePoiDetailExtension(Activity activity, ArchitectView architectView) {
        super(activity, architectView);
    }

    @Override
    public void onCreate() {
        /*
         * The ArchitectJavaScriptInterfaceListener has to be added to the Architect view after ArchitectView.onCreate.
         * There may be more than one ArchitectJavaScriptInterfaceListener.
         */
        architectView.addArchitectJavaScriptInterfaceListener(this);
    }

    @Override
    public void onDestroy() {
        // The ArchitectJavaScriptInterfaceListener has to be removed from the Architect view before ArchitectView.onDestroy.
        architectView.removeArchitectJavaScriptInterfaceListener(this);
    }

    /**
     * ArchitectJavaScriptInterfaceListener.onJSONObjectReceived is called whenever
     * AR.platform.sendJSONObject is called in the JavaScript code.
     *
     * In this case the jsonObject includes an id, a title and a description which will be
     * parsed and sent to the SamplePoiDetailActivity as intent extras.
     *
     * @param jsonObject jsonObject passed in AR.platform.sendJSONObject
     */
    @Override
    public void onJSONObjectReceived(JSONObject jsonObject) {
        final Intent poiDetailIntent = new Intent(activity, SamplePoiDetailActivity.class);
        try {
            switch (jsonObject.getString("action")) {
                case "present_poi_details":
                    poiDetailIntent.putExtra(SamplePoiDetailActivity.EXTRAS_KEY_POI_ID, jsonObject.getString("id"));
                    poiDetailIntent.putExtra(SamplePoiDetailActivity.EXTRAS_KEY_POI_TITILE, jsonObject.getString("title"));
                    poiDetailIntent.putExtra(SamplePoiDetailActivity.EXTRAS_KEY_POI_DESCR, jsonObject.getString("description"));
                    activity.startActivity(poiDetailIntent);
                    break;
            }

        } catch (JSONException e) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(activity, R.string.error_parsing_json, Toast.LENGTH_LONG).show();
                }
            });
            e.printStackTrace();
        }
    }
}
