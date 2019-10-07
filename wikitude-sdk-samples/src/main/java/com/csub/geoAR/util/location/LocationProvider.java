package com.csub.geoAR.util.location;

import android.annotation.SuppressLint;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;


/**
 * Very basic location provider to enable location updates.
 * Please note that this approach is very minimal and we recommend to implement a more
 * advanced location provider for your app. (see https://developer.android.com/training/location/index.html)
 */
public class LocationProvider {

	/** location listener called on each location update */
	private final @NonNull LocationListener	locationListener;

	/** callback called when no providers are enabled */
	private final @NonNull ErrorCallback callback;

	/** system's locationManager allowing access to GPS / Network position */
	private final @Nullable LocationManager	locationManager;

	/** location updates should fire approximately every second */
	private static final int LOCATION_UPDATE_MIN_TIME_GPS = 1000;

	/** location updates should fire, even if last signal is same than current one (0m distance to last location is OK) */
	private static final int LOCATION_UPDATE_DISTANCE_GPS = 0;

	/** location updates should fire approximately every second */
	private static final int LOCATION_UPDATE_MIN_TIME_NW = 1000;

	/** location updates should fire, even if last signal is same than current one (0m distance to last location is OK) */
	private static final int LOCATION_UPDATE_DISTANCE_NW = 0;

	/** to faster access location, even use 10 minute old locations on start-up */
	private static final int LOCATION_OUTDATED_WHEN_OLDER_MS = 1000 * 60 * 10;

	/** is gpsProvider and networkProvider enabled in system settings */
	private boolean					gpsProviderEnabled, networkProviderEnabled;

	public LocationProvider(@NonNull final Context context,
	                        @NonNull final LocationListener locationListener,
	                        @NonNull final ErrorCallback callback) {
		super();
		this.locationListener = locationListener;
		this.callback = callback;

		locationManager = (LocationManager)context.getSystemService( Context.LOCATION_SERVICE );

		if (this.locationManager != null) {
			gpsProviderEnabled = this.locationManager.isProviderEnabled( LocationManager.GPS_PROVIDER );
			networkProviderEnabled = this.locationManager.isProviderEnabled( LocationManager.NETWORK_PROVIDER );
		}
	}

	public void onPause() {
		if (this.locationManager != null && (this.gpsProviderEnabled || this.networkProviderEnabled)) {
			this.locationManager.removeUpdates( this.locationListener );
		}
	}

	@SuppressLint("MissingPermission")
	public void onResume() {
		if (this.locationManager != null) {

			// check which providers are available are available
			this.gpsProviderEnabled = this.locationManager.isProviderEnabled( LocationManager.GPS_PROVIDER );
			this.networkProviderEnabled = this.locationManager.isProviderEnabled( LocationManager.NETWORK_PROVIDER );

			// is GPS provider enabled?
			if ( this.gpsProviderEnabled ) {
				final Location lastKnownGPSLocation = this.locationManager.getLastKnownLocation( LocationManager.GPS_PROVIDER );
				if ( lastKnownGPSLocation != null && lastKnownGPSLocation.getTime() > System.currentTimeMillis() - LOCATION_OUTDATED_WHEN_OLDER_MS ) {
					locationListener.onLocationChanged( lastKnownGPSLocation );
				}
				if (locationManager.getProvider(LocationManager.GPS_PROVIDER)!=null) {
					this.locationManager.requestLocationUpdates( LocationManager.GPS_PROVIDER, LOCATION_UPDATE_MIN_TIME_GPS, LOCATION_UPDATE_DISTANCE_GPS, this.locationListener );
				}
			}

			//is Network / WiFi positioning provider available?
			if ( this.networkProviderEnabled ) {
				final Location lastKnownNWLocation = this.locationManager.getLastKnownLocation( LocationManager.NETWORK_PROVIDER );
				if ( lastKnownNWLocation != null && lastKnownNWLocation.getTime() > System.currentTimeMillis() - LOCATION_OUTDATED_WHEN_OLDER_MS ) {
					locationListener.onLocationChanged( lastKnownNWLocation );
				}
				if (locationManager.getProvider(LocationManager.NETWORK_PROVIDER)!=null) {
					this.locationManager.requestLocationUpdates( LocationManager.NETWORK_PROVIDER, LOCATION_UPDATE_MIN_TIME_NW, LOCATION_UPDATE_DISTANCE_NW, this.locationListener );
				}
			}

			// user didn't check a single positioning in the location settings, recommended: handle this event properly in your app, e.g. forward user directly to location-settings, new Intent( Settings.ACTION_LOCATION_SOURCE_SETTINGS )
			if ( !this.gpsProviderEnabled && !this.networkProviderEnabled ) {
				callback.noProvidersEnabled();
			}
		}
	}

	public interface ErrorCallback {
		void noProvidersEnabled();
	}
}
