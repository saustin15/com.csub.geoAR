package com.csub.geoAR.util;

import android.support.annotation.NonNull;

import java.io.Serializable;
import java.util.List;

public class SampleCategory implements Serializable {

    private final List<SampleData> samples;
    private final String name;

    public SampleCategory(@NonNull List<SampleData> samples, @NonNull String name) {
        this.samples = samples;
        this.name = name;
    }

    public List<SampleData> getSamples() {
        return samples;
    }

    public String getName() {
        return name;
    }
}
