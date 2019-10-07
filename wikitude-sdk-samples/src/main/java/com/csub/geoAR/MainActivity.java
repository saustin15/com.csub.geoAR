package com.csub.geoAR;

import com.csub.geoAR.util.PermissionUtil;
import com.csub.geoAR.util.SampleCategory;
import com.csub.geoAR.util.SampleJsonParser;
import com.csub.geoAR.util.adapters.SamplesExpendableListAdapter;
import com.csub.geoAR.util.urllauncher.UrlLauncherStorageActivity;
import com.wikitude.architect.ArchitectView;
import com.wikitude.common.CallStatus;
import com.wikitude.common.permission.PermissionManager;
import com.wikitude.common.util.SDKBuildInformation;
import com.csub.geoAR.util.SampleData;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ExpandableListView;
import android.widget.Toast;

import java.util.Arrays;
import java.util.List;

/**
 * The MainActivity is used to display the list of samples and handles the runtime
 * permissions for the sample activities.
 */
public class MainActivity extends AppCompatActivity implements ExpandableListView.OnChildClickListener {

    private static final String sampleDefinitionsPath = "samples/samples.json";
    private static final int EXPANDABLE_INDICATOR_START_OFFSET = 60;
    private static final int EXPANDABLE_INDICATOR_END_OFFSET = 30;

    private final PermissionManager permissionManager = ArchitectView.getPermissionManager();
    private ExpandableListView listView;
    private List<SampleCategory> categories;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final String json = SampleJsonParser.loadStringFromAssets(this, sampleDefinitionsPath);
        categories = SampleJsonParser.getCategoriesFromJsonString(json);

        for (final SampleCategory category : categories) {
            for (final SampleData data : category.getSamples()) {
                CallStatus status = ArchitectView.isDeviceSupporting(this, data.getArFeatures());
                if (status.isSuccess()) {
                    data.isDeviceSupporting(true, "");
                } else {
                    data.isDeviceSupporting(false, status.getError().getMessage());
                }
            }
        }

        final SamplesExpendableListAdapter adapter = new SamplesExpendableListAdapter(this, categories);

        listView = findViewById(R.id.listView);
        moveExpandableIndicatorToRight();
        listView.setOnChildClickListener(this);
        listView.setAdapter(adapter);

        final Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (listView != null) moveExpandableIndicatorToRight();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    public boolean onChildClick(ExpandableListView parent, View v, int groupPosition, int childPosition, long id) {
        final SampleData sampleData = categories.get(groupPosition).getSamples().get(childPosition);
        final String[] permissions = PermissionUtil.getPermissionsForArFeatures(sampleData.getArFeatures());

        if(!sampleData.getIsDeviceSupporting()) {
            showDeviceMissingFeatures(sampleData.getIsDeviceSupportingError());
        } else {
            permissionManager.checkPermissions(MainActivity.this, permissions, PermissionManager.WIKITUDE_PERMISSION_REQUEST, new PermissionManager.PermissionManagerCallback() {
                @Override
                public void permissionsGranted(int requestCode) {
                    final Intent intent = new Intent(MainActivity.this, sampleData.getActivityClass());
                    intent.putExtra(SimpleArActivity.INTENT_EXTRAS_KEY_SAMPLE, sampleData);
                    intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    startActivity(intent);
                }

                @Override
                public void permissionsDenied(@NonNull String[] deniedPermissions) {
                    Toast.makeText(MainActivity.this, getString(R.string.permissions_denied) + Arrays.toString(deniedPermissions), Toast.LENGTH_SHORT).show();
                }

                @Override
                public void showPermissionRationale(final int requestCode, @NonNull String[] strings) {
                    final AlertDialog.Builder alertBuilder = new AlertDialog.Builder(MainActivity.this);
                    alertBuilder.setCancelable(true);
                    alertBuilder.setTitle(R.string.permission_rationale_title);
                    alertBuilder.setMessage(getString(R.string.permission_rationale_text) + Arrays.toString(permissions));
                    alertBuilder.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            permissionManager.positiveRationaleResult(requestCode, permissions);
                        }
                    });

                    AlertDialog alert = alertBuilder.create();
                    alert.show();
                }
            });
        }
        return false;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        ArchitectView.getPermissionManager().onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    private void moveExpandableIndicatorToRight() {
        final DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int width = metrics.widthPixels;

        if(android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.JELLY_BEAN_MR2) {
            listView.setIndicatorBounds(width - dpToPx(EXPANDABLE_INDICATOR_START_OFFSET), width - dpToPx(EXPANDABLE_INDICATOR_END_OFFSET));
            listView.setIndicatorBounds(width - dpToPx(EXPANDABLE_INDICATOR_START_OFFSET), width - dpToPx(EXPANDABLE_INDICATOR_END_OFFSET));
        } else {
            listView.setIndicatorBoundsRelative(width - dpToPx(EXPANDABLE_INDICATOR_START_OFFSET), width - dpToPx(EXPANDABLE_INDICATOR_END_OFFSET));
            listView.setIndicatorBoundsRelative(width - dpToPx(EXPANDABLE_INDICATOR_START_OFFSET), width - dpToPx(EXPANDABLE_INDICATOR_END_OFFSET));
        }
    }

    private int dpToPx(int dp) {
        final float scale = getResources().getDisplayMetrics().density;
        return (int) (dp * scale + 0.5f);
    }

    public void launchCustomUrl(MenuItem item) {
        final Intent intent = new Intent(this, UrlLauncherStorageActivity.class);
        startActivity(intent);
    }

    public void showSdkBuildInformation(MenuItem item) {
        final SDKBuildInformation sdkBuildInformation = ArchitectView.getSDKBuildInformation();
        new AlertDialog.Builder(MainActivity.this)
                .setTitle(R.string.build_information_title)
                .setMessage(
                        getString(R.string.build_information_config) + sdkBuildInformation.getBuildConfiguration() + "\n" +
                        getString(R.string.build_information_date) + sdkBuildInformation.getBuildDate() + "\n" +
                        getString(R.string.build_information_number) + sdkBuildInformation.getBuildNumber() + "\n" +
                        getString(R.string.build_information_version) + ArchitectView.getSDKVersion()
                )
                .show();
    }

    public void showDeviceMissingFeatures(String errorMessage) {
        new AlertDialog.Builder(MainActivity.this)
            .setTitle(R.string.device_missing_features)
            .setMessage(errorMessage)
            .show();
    }
}
