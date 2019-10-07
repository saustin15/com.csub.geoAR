package com.csub.geoAR.fragments;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;

import com.wikitude.common.devicesupport.Feature;
import com.csub.geoAR.util.SampleData;
import com.csub.geoAR.R;

/**
 * Activity used to start AR Fragments.
 */
public class UrlLauncherActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_url_launcher);

        // SampleData is created in the UrlLauncherStorageActivity.
        final SampleData sampleData = (SampleData) getIntent().getSerializableExtra(SimpleArFragment.INTENT_EXTRAS_KEY_SAMPLE);

        // Selects fragment type based on if Geo AR is used.
        Fragment fragment;
        if (sampleData.getArFeatures().contains(Feature.GEO)) {
            fragment = new SimpleGeoArFragment();
        } else {
            fragment = new SimpleArFragment();
        }

        // Adds the SampleData to Fragment arguments.
        final Bundle args = new Bundle();
        args.putSerializable(SimpleArFragment.INTENT_EXTRAS_KEY_SAMPLE, sampleData);
        fragment.setArguments(args);

        final FragmentTransaction fragmentTransaction = this.getSupportFragmentManager().beginTransaction();
        fragmentTransaction.replace(R.id.mainFragement, fragment);
        fragmentTransaction.commit();
    }

}
