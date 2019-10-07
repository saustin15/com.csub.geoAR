package com.csub.geoAR.advanced;

import com.csub.geoAR.util.location.LocationProvider;
import com.wikitude.architect.ArchitectView;
import com.csub.geoAR.R;

import android.app.Activity;
import android.hardware.SensorManager;
import android.location.Location;
import android.location.LocationListener;
import android.os.Bundle;
import android.widget.Toast;

public class GeoExtension extends ArchitectViewExtension implements LocationListener {

    /**
     * Very basic location provider to enable location updates.
     * Please note that this approach is very minimal and we recommend to implement a more
     * advanced location provider for your app. (see https://developer.android.com/training/location/index.html)
     */
    private LocationProvider locationProvider;

    /**
     * Error callback of the LocationProvider, noProvidersEnabled is called when neither location over GPS nor
     * location over the network are enabled by the device.
     */
    private final LocationProvider.ErrorCallback errorCallback = new LocationProvider.ErrorCallback() {
        @Override
        public void noProvidersEnabled() {
            Toast.makeText(activity, R.string.no_location_provider, Toast.LENGTH_LONG).show();
        }
    };

    /**
     * The ArchitectView.SensorAccuracyChangeListener notifies of changes in the accuracy of the compass.
     * This can be used to notify the user that the sensors need to be recalibrated.
     *
     * This listener has to be registered after onCreate and unregistered before onDestroy in the ArchitectView.
     */
    private final ArchitectView.SensorAccuracyChangeListener sensorAccuracyChangeListener = new ArchitectView.SensorAccuracyChangeListener() {
        @Override
        public void onCompassAccuracyChanged(int accuracy) {
            if ( accuracy < SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM) { // UNRELIABLE = 0, LOW = 1, MEDIUM = 2, HIGH = 3
                Toast.makeText(activity, R.string.compass_accuracy_low, Toast.LENGTH_LONG ).show();
            }
        }
    };

    private LocationListener locationListenerExtension;

    public GeoExtension(Activity activity, ArchitectView architectView) {
        super(activity, architectView);
    }


    @Override
    public void onCreate() {
        locationProvider = new LocationProvider(activity, this, errorCallback);
    }

    @Override
    public void onResume() {
        locationProvider.onResume();
        /*
         * The SensorAccuracyChangeListener has to be registered to the Architect view after ArchitectView.onCreate.
         * There may be more than one SensorAccuracyChangeListener.
         */
        architectView.registerSensorAccuracyChangeListener(sensorAccuracyChangeListener);
    }

    @Override
    public void onPause() {
        locationProvider.onPause();
        // The SensorAccuracyChangeListener has to be unregistered from the Architect view before ArchitectView.onDestroy.
        architectView.unregisterSensorAccuracyChangeListener(sensorAccuracyChangeListener);
    }

    /**
     * The ArchitectView has to be notified when the location of the device
     * changed in order to accurately display the Augmentations for Geo AR.
     *
     * The ArchitectView has two methods which can be used to pass the Location,
     * it should be chosen by whether an altitude is available or not.
     */
    @Override
    public void onLocationChanged(Location location) {
        float accuracy = location.hasAccuracy() ? location.getAccuracy() : 1000;
        if (location.hasAltitude()) {
            architectView.setLocation(location.getLatitude(), location.getLongitude(), location.getAltitude(), accuracy);
        } else {
            architectView.setLocation(location.getLatitude(), location.getLongitude(), accuracy);
        }
        if (locationListenerExtension != null){
            locationListenerExtension.onLocationChanged(location);
        }
    }

    /**
     * The very basic LocationProvider setup of this sample app does not handle the following callbacks
     * to keep the sample app as small as possible. They should be used to handle changes in a production app.
     */
    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {}

    @Override
    public void onProviderEnabled(String provider) {}

    @Override
    public void onProviderDisabled(String provider) {}

    public void setLocationListenerExtension(LocationListener extension) {
        locationListenerExtension = extension;
    }
}
