package com.csub.geoAR.util.adapters;

import com.csub.geoAR.util.SampleCategory;
import com.csub.geoAR.util.urllauncher.UrlLauncherSettingsActivity;
import com.csub.geoAR.util.urllauncher.UrlLauncherStorageActivity;
import com.csub.geoAR.R;

import android.content.Context;
import android.content.Intent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class UrlLauncherStorageListAdapter extends BaseAdapter {

    private final Context context;
    private final SampleCategory category;

    public UrlLauncherStorageListAdapter(Context context, SampleCategory category) {
        this.context = context;
        this.category = category;
    }

    @Override
    public int getCount() {
        return category.getSamples().size();
    }

    @Override
    public Object getItem(int position) {
        return category.getSamples().get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            final LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.url_launcher_list_row, null);
        }

        final TextView name = convertView.findViewById(R.id.url_list_row_name);
        name.setText(category.getSamples().get(position).getName());

        final ImageView editView = convertView.findViewById(R.id.url_list_row_edit);
        editView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(context, UrlLauncherSettingsActivity.class);
                intent.putExtra(UrlLauncherStorageActivity.URL_LAUNCHER_SAMPLE_CATEGORY, category);
                intent.putExtra(UrlLauncherStorageActivity.URL_LAUNCHER_EDIT_SAMPLE_ID, position);
                context.startActivity(intent);
            }
        });

        final ImageView deleteView = convertView.findViewById(R.id.url_list_row_delete);
        deleteView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                category.getSamples().remove(position);
                notifyDataSetChanged();
            }
        });
        return convertView;
    }

    public SampleCategory getCategory() {
        return category;
    }
}
