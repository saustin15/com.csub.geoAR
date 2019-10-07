package com.csub.geoAR.util.urllauncher;

import com.csub.geoAR.util.PermissionUtil;
import com.csub.geoAR.util.SampleCategory;
import com.wikitude.architect.ArchitectView;
import com.wikitude.common.permission.PermissionManager;
import com.csub.geoAR.fragments.SimpleArFragment;
import com.csub.geoAR.fragments.UrlLauncherActivity;
import com.csub.geoAR.util.SampleData;
import com.csub.geoAR.util.adapters.UrlLauncherStorageListAdapter;
import com.csub.geoAR.R;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Arrays;


public class UrlLauncherStorageActivity extends AppCompatActivity implements AdapterView.OnItemClickListener {

    public static final String URL_LAUNCHER_SAMPLE_CATEGORY = "urlLauncherSampleCategory";
    public static final String URL_LAUNCHER_EDIT_SAMPLE_ID = "urlLauncherSampleCategoryId";

    private static final String STORAGE_FILE = "urlLauncherSampleCategoryList";
    private static final String TAG = UrlLauncherStorageActivity.class.getSimpleName();

    private final PermissionManager permissionManager = ArchitectView.getPermissionManager();

    private SampleCategory sampleCategory = new SampleCategory(new ArrayList<SampleData>(), URL_LAUNCHER_SAMPLE_CATEGORY);
    private UrlLauncherStorageListAdapter adapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_url_launcher_storage);

        final Toolbar toolbar = findViewById(R.id.toolbar);
        toolbar.setTitle(R.string.url_launcher_title);
        setSupportActionBar(toolbar);
    }

    @Override
    protected void onResume() {
        super.onResume();

        final Intent intent = getIntent();
        if (intent.hasExtra(URL_LAUNCHER_SAMPLE_CATEGORY)) {
            sampleCategory = (SampleCategory) intent.getSerializableExtra(URL_LAUNCHER_SAMPLE_CATEGORY);
        } else {
            try {
                final FileInputStream fis = openFileInput(STORAGE_FILE);
                final ObjectInputStream is = new ObjectInputStream(fis);
                final SampleCategory categoryFromFs = (SampleCategory) is.readObject();
                is.close();
                fis.close();

                if (categoryFromFs != null) {
                    sampleCategory = categoryFromFs;
                }
            } catch (IOException | ClassNotFoundException e) {
                Log.w(TAG, "Could not load saved list data.");
            }
        }

        final ListView listView = findViewById(R.id.url_list_storage_view);
        listView.setOnItemClickListener(this);
        adapter = new UrlLauncherStorageListAdapter(this, sampleCategory);
        listView.setAdapter(adapter);
    }

    @Override
    protected void onPause() {
        super.onPause();
        sampleCategory = adapter.getCategory();

        try {
            final FileOutputStream fos = openFileOutput(STORAGE_FILE, Context.MODE_PRIVATE);
            final ObjectOutputStream os = new ObjectOutputStream(fos);
            os.writeObject(sampleCategory);
            os.close();
            fos.close();
        } catch (IOException e) {
            Log.w(TAG, "Could not store list data.");
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
    }

    public void addUrl(View view) {
        final Intent intent = new Intent(UrlLauncherStorageActivity.this, UrlLauncherSettingsActivity.class);
        intent.putExtra(URL_LAUNCHER_SAMPLE_CATEGORY, sampleCategory);
        startActivity(intent);
    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, final int position, long id) {
        final SampleData sampleData = sampleCategory.getSamples().get(position);
        final String[] permissions = PermissionUtil.getPermissionsForArFeatures(sampleData.getArFeatures());

        permissionManager.checkPermissions(UrlLauncherStorageActivity.this, permissions, PermissionManager.WIKITUDE_PERMISSION_REQUEST, new PermissionManager.PermissionManagerCallback() {
            @Override
            public void permissionsGranted(int requestCode) {
                final Intent intent = new Intent(UrlLauncherStorageActivity.this, UrlLauncherActivity.class);
                intent.putExtra(SimpleArFragment.INTENT_EXTRAS_KEY_SAMPLE, sampleData);
                startActivity(intent);
            }

            @Override
            public void permissionsDenied(@NonNull String[] deniedPermissions) {
                Toast.makeText(UrlLauncherStorageActivity.this, getString(R.string.permissions_denied) + Arrays.toString(deniedPermissions), Toast.LENGTH_SHORT).show();
            }

            @Override
            public void showPermissionRationale(final int requestCode, @NonNull String[] strings) {
                final AlertDialog.Builder alertBuilder = new AlertDialog.Builder(UrlLauncherStorageActivity.this);
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
}
