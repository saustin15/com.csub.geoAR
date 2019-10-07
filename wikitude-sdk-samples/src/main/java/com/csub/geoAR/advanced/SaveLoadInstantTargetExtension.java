package com.csub.geoAR.advanced;

import com.wikitude.architect.ArchitectJavaScriptInterfaceListener;
import com.wikitude.architect.ArchitectView;
import com.csub.geoAR.R;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

public class SaveLoadInstantTargetExtension extends ArchitectViewExtension implements ArchitectJavaScriptInterfaceListener {

    private static final String TAG = SaveLoadInstantTargetExtension.class.getSimpleName();

    private final File instantTargetSaveFile;
    private final File savedAugmentationsFile;

    public SaveLoadInstantTargetExtension(final Activity activity, final ArchitectView architectView) {
        super(activity, architectView);

        // The saved files can be usually found in /sdcard/Android/data/com.wikitude.sdksamples/ (getExternalFilesDir)
        instantTargetSaveFile = new File(activity.getExternalFilesDir(null), "SavedInstantTarget.wto");
        savedAugmentationsFile = new File(activity.getExternalFilesDir(null), "SavedAugmentations.json");
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
     * @param jsonObject jsonObject passed in AR.platform.sendJSONObject
     */
    @Override
    public void onJSONObjectReceived(final JSONObject jsonObject) {
        try {
            switch (jsonObject.getString("action")) {
                case "save_current_instant_target":
                    saveAugmentations(jsonObject.getString("augmentations"));
                    saveCurrentInstantTarget();
                    break;
                case "load_existing_instant_target":
                    loadExistingInstantTarget();
                    break;
            }
        } catch (JSONException e) {
            activity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(activity, R.string.error_parsing_json, Toast.LENGTH_LONG).show();
                }
            });
        }
    }

    private void loadExistingInstantTarget() {
        architectView.callJavascript(String.format("World.loadExistingInstantTargetFromUrl(\"%s\", %s)", instantTargetSaveFile.getAbsolutePath(), loadAugmentations()));
    }

    private void saveCurrentInstantTarget() {
        architectView.callJavascript(String.format("World.saveCurrentInstantTargetToUrl(\"%s\")", instantTargetSaveFile.getAbsolutePath()));
    }

    private void saveAugmentations(String data) {
        try (FileOutputStream stream = new FileOutputStream(savedAugmentationsFile)) {
            stream.write(data.getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Could not save augmentations.", e);
        }
    }

    private String loadAugmentations() {
        int length = (int) savedAugmentationsFile.length();

        byte[] bytes = new byte[length];
        String jsonString;

        try (FileInputStream in = new FileInputStream(savedAugmentationsFile)) {
            in.read(bytes);
            jsonString = new String(bytes);
        } catch (IOException e) {
            jsonString = "";
        }
        return jsonString;
    }
}
