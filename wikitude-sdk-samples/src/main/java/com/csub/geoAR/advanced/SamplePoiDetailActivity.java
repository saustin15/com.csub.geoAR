package com.csub.geoAR.advanced;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

import com.csub.geoAR.R;

public class SamplePoiDetailActivity extends Activity {

    public static final String EXTRAS_KEY_POI_ID = "id";
    public static final String EXTRAS_KEY_POI_TITILE = "title";
    public static final String EXTRAS_KEY_POI_DESCR = "description";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_poidetail);

        final Bundle extras = getIntent().getExtras();
        ((TextView) findViewById(R.id.poi_detail_id_field_text_view)).setText(extras.getString(EXTRAS_KEY_POI_ID));
        ((TextView) findViewById(R.id.poi_detail_name_field_text_view)).setText(extras.getString(EXTRAS_KEY_POI_TITILE));
        ((TextView) findViewById(R.id.poi_detail_description_field_text_view)).setText(extras.getString(EXTRAS_KEY_POI_DESCR));
    }
}
