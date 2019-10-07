package com.csub.geoAR.util.urllauncher;

import com.csub.geoAR.util.SampleCategory;
import com.wikitude.common.devicesupport.Feature;
import com.csub.geoAR.util.SampleData;
import com.csub.geoAR.R;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

import static com.wikitude.common.camera.CameraSettings.CameraFocusMode;
import static com.wikitude.common.camera.CameraSettings.CameraPosition;
import static com.wikitude.common.camera.CameraSettings.CameraResolution;

public class UrlLauncherSettingsActivity extends AppCompatActivity {

    @NonNull private static final String AUTOCOMPLETE_PREFS = "autocomplete_prefs";
    @NonNull private static final String AUTOCOMPLETE_PREFS_HISTORY = "autocomplete_prefs_history";

    @NonNull Set<String> history = new HashSet<>();

    SampleCategory category;
    int editPosition;
    AutoCompleteTextView urlText;
    EditText nameText;
    Switch geoSwitch;
    Switch imageSwitch;
    Switch instantSwitch;
    Switch objectSwitch;
    Switch camera2Switch;
    Spinner focusSpinner;
    Spinner positionSpinner;
    Spinner resolutionSpinner;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.url_launcher_row_settings);


        final SharedPreferences preferences = getSharedPreferences(AUTOCOMPLETE_PREFS, 0);
        history = preferences.getStringSet(AUTOCOMPLETE_PREFS_HISTORY, new HashSet<String>());
        updateAutocompleteSource();

        urlText = findViewById(R.id.url_launcher_url_text);
        nameText = findViewById(R.id.url_launcher_name_edit_text);
        geoSwitch = findViewById(R.id.url_launcher_geo_switch);
        imageSwitch = findViewById(R.id.url_launcher_image_switch);
        instantSwitch = findViewById(R.id.url_launcher_instant_switch);
        objectSwitch = findViewById(R.id.url_launcher_object_switch);
        camera2Switch = findViewById(R.id.url_launcher_camera2_switch);
        focusSpinner = findViewById(R.id.url_launcher_focus_spinner);
        positionSpinner = findViewById(R.id.url_launcher_position_spinner);
        resolutionSpinner = findViewById(R.id.url_launcher_resolution_spinner);

        final Intent intent = getIntent();
        editPosition = intent.getIntExtra(UrlLauncherStorageActivity.URL_LAUNCHER_EDIT_SAMPLE_ID, -1);
        category = (SampleCategory) intent.getSerializableExtra(UrlLauncherStorageActivity.URL_LAUNCHER_SAMPLE_CATEGORY);


        if (editPosition >= 0) {
            final SampleData sampleData = category.getSamples().get(editPosition);
            urlText.setText(sampleData.getPath());
            nameText.setText(sampleData.getName());

            final EnumSet<Feature> arFeatures = sampleData.getArFeatures();
            geoSwitch.setChecked(arFeatures.contains(Feature.GEO));
            imageSwitch.setChecked(arFeatures.contains(Feature.IMAGE_TRACKING));
            instantSwitch.setChecked(arFeatures.contains(Feature.INSTANT_TRACKING));
            objectSwitch.setChecked(arFeatures.contains(Feature.OBJECT_TRACKING));

            camera2Switch.setChecked(sampleData.isCamera2Enabled());

            final CameraFocusMode focusMode = sampleData.getCameraFocusMode();
            int focus;
            switch (focusMode){
                case CONTINUOUS: focus = 0; break;
                case ONCE: focus = 1; break;
                case OFF: focus = 2; break;
                default: focus = 0; break;
            }
            focusSpinner.setSelection(focus);

            final CameraPosition cameraPosition = sampleData.getCameraPosition();
            int position;
            switch (cameraPosition){
                case BACK: position = 0; break;
                case FRONT: position = 1; break;
                default: position = 0; break;
            }
            positionSpinner.setSelection(position);

            final CameraResolution cameraResolution = sampleData.getCameraResolution();
            int resolution;
            switch (cameraResolution){
                case SD_640x480: resolution = 0; break;
                case HD_1280x720: resolution = 1; break;
                case FULL_HD_1920x1080: resolution = 2; break;
                case AUTO: resolution = 3; break;
                default: resolution = 0; break;
            }
            resolutionSpinner.setSelection(resolution);
        }

        urlText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_SEND) {
                    storeSettings();
                }
                return true;
            }
        });

        final Button ok = findViewById(R.id.url_launcher_ok_button);

        ok.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                storeSettings();
            }
        });
    }

    private void storeSettings() {
        final String urlTextString = urlText.getText().toString();
        history.add(urlTextString);
        updateAutocompleteSource();

        final SharedPreferences preferences = getSharedPreferences(AUTOCOMPLETE_PREFS, 0);
        final SharedPreferences.Editor editor = preferences.edit().putStringSet(AUTOCOMPLETE_PREFS_HISTORY, history);
        editor.apply();

        EnumSet<Feature> features = EnumSet.noneOf(Feature.class);
        if (geoSwitch.isChecked()) {
            features.add(Feature.GEO);
        }
        if (imageSwitch.isChecked()) {
            features.add(Feature.IMAGE_TRACKING);
        }
        if (instantSwitch.isChecked()) {
            features.add(Feature.INSTANT_TRACKING);
        }
        if (objectSwitch.isChecked()) {
            features.add(Feature.OBJECT_TRACKING);
        }

        CameraFocusMode focusMode;
        final int focus = (int) focusSpinner.getSelectedItemId();
        switch (focus){
            case 0: focusMode = CameraFocusMode.CONTINUOUS; break;
            case 1: focusMode = CameraFocusMode.ONCE; break;
            case 2: focusMode = CameraFocusMode.OFF; break;
            default: focusMode = CameraFocusMode.CONTINUOUS; break;
        }
        CameraPosition cameraPosition;
        final int position = (int) positionSpinner.getSelectedItemId();
        switch (position){
            case 0: cameraPosition = CameraPosition.BACK; break;
            case 1: cameraPosition = CameraPosition.FRONT; break;
            default: cameraPosition = CameraPosition.BACK; break;
        }
        CameraResolution cameraResolution;
        final int resolution = (int) resolutionSpinner.getSelectedItemId();
        switch (resolution){
            case 0: cameraResolution = CameraResolution.SD_640x480; break;
            case 1: cameraResolution = CameraResolution.HD_1280x720; break;
            case 2: cameraResolution = CameraResolution.FULL_HD_1920x1080; break;
            case 3: cameraResolution = CameraResolution.AUTO; break;
            default: cameraResolution = CameraResolution.SD_640x480; break;
        }

        final SampleData sampleData = new SampleData.Builder(nameText.getText().toString(), urlTextString)
                .arFeatures(features)
                .cameraFocusMode(focusMode)
                .cameraPosition(cameraPosition)
                .cameraResolution(cameraResolution)
                .camera2Enabled(camera2Switch.isEnabled())
                .build();

        if (editPosition >= 0) {
            category.getSamples().set(editPosition, sampleData);
        } else {
            category.getSamples().add(sampleData);
        }

        final Intent intent = new Intent(UrlLauncherSettingsActivity.this, UrlLauncherStorageActivity.class);
        intent.putExtra(UrlLauncherStorageActivity.URL_LAUNCHER_SAMPLE_CATEGORY, category);
        intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
        startActivity(intent);
    }

    private void updateAutocompleteSource() {
        final AutoCompleteTextView urlText = findViewById(R.id.url_launcher_url_text);
        final ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, history.toArray(new String[history.size()]));
        urlText.setAdapter(adapter);
    }
}
