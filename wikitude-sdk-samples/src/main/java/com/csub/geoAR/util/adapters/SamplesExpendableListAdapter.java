package com.csub.geoAR.util.adapters;

import com.csub.geoAR.util.SampleCategory;
import com.csub.geoAR.util.SampleData;
import com.csub.geoAR.R;

import android.content.Context;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.TextView;

import java.util.List;

public class SamplesExpendableListAdapter extends BaseExpandableListAdapter {

    private final Context context;
    private final List<SampleCategory> sampleCategories;

    public SamplesExpendableListAdapter(@NonNull Context context, @NonNull List<SampleCategory> sampleCategories) {
        this.context = context;
        this.sampleCategories = sampleCategories;
    }

    @Override
    public int getGroupCount() {
        return sampleCategories.size();
    }

    @Override
    public int getChildrenCount(int groupPosition) {
        return sampleCategories.get(groupPosition).getSamples().size();
    }

    @Override
    public Object getGroup(int groupPosition) {
        return sampleCategories.get(groupPosition);
    }

    @Override
    public Object getChild(int groupPosition, int childPosition) {
        return sampleCategories.get(groupPosition).getSamples().get(childPosition);
    }

    @Override
    public long getGroupId(int groupPosition) {
        return groupPosition;
    }

    @Override
    public long getChildId(int groupPosition, int childPosition) {
        return childPosition;
    }

    @Override
    public boolean hasStableIds() {
        return false;
    }

    @Override
    public View getGroupView(int groupPosition, boolean expanded, View view, ViewGroup viewGroup) {


        if (view == null) {
            final LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            view = inflater.inflate(R.layout.expand_header, null);
        }

        final String categoryTitle = sampleCategories.get(groupPosition).getName();

        final TextView categoryText = view.findViewById(R.id.expand_text);
        categoryText.setText(categoryTitle);

        if (expanded) {
            view.setBackgroundColor(context.getResources().getColor(R.color.wikitude_primary));
        } else {
            view.setBackgroundColor(context.getResources().getColor(R.color.wikitude_white));
        }

        return view;
    }

    @Override
    public View getChildView(int groupPosition, int childPosition, boolean lastChild, View view, ViewGroup viewGroup) {
        final SampleData sampleData = (SampleData) getChild(groupPosition, childPosition);

        if (view == null) {
            final LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            view = inflater.inflate(R.layout.expand_row, null);
        }

        final TextView sampleText = view.findViewById(R.id.expand_row_text);
        sampleText.setText(sampleData.getName());

        if(sampleData.getIsDeviceSupporting()) {
            view.setBackgroundColor(context.getResources().getColor(R.color.wikitude_white));
        } else {
            view.setBackgroundColor(context.getResources().getColor(R.color.wikitude_is_device_not_supported));
        }

        return view;
    }

    @Override
    public boolean isChildSelectable(int groupPosition, int childPosition) {
        return true;
    }
}
