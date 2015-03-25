/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.orangelabs.rcs.core.ims.service.presence.pidf.geoloc;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * The class is used to obtain UE's location information with GPS technology
 */
public class GPSLocationService extends Service {

    /**
     * Get the LocationManager
     */
    private LocationManager mLocationManager = (LocationManager) AndroidFactory
            .getApplicationContext().getSystemService(Context.LOCATION_SERVICE);

    /**
     * Used to obtain location when update
     */
    private Location mCurrentLocation;
    
    /**
     * The interval used to Check whether the new location fix is newer or older 
     */

    private static final int CHECK_INTERVAL = 1000 * 30;

    /**
     * The logger
     */
    private Logger mLogger = Logger.getLogger(this.getClass().getName());

    public class GPSLocationBinder extends Binder {
        /**
         * Return the current class
         * 
         * @return GPSLocationService
         */
        public GPSLocationService getService() {
            return GPSLocationService.this;
        }
    }

    /**
     * create a instance of LocalBinder and return it when the onBind function
     * is called
     */
    private final IBinder mBinder = new GPSLocationBinder();

    @Override
    public IBinder onBind(Intent intent) {
        // set the listener
        mLocationManager
                .requestLocationUpdates(LocationManager.GPS_PROVIDER, 5000, 0, mGpsListener);
        return mBinder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mLogger.debug("GPSLocation Service is onCreate!");
    }

    @Override
    public void onStart(Intent intent, int startId) {
        super.onStart(intent, startId);
        mLogger.debug("GPSLocation Service is onStart!");
    }

    @Override
    public boolean onUnbind(Intent intent) {
        if (mGpsListener != null) {
            // remove the listener
            mLocationManager.removeUpdates(mGpsListener);
            mGpsListener = null;
        }
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        mLogger.debug("GPSLocation Service is onDestroy!");
        super.onDestroy();
    }

    /**
     * log out the obtained location
     * 
     * @param location
     */
    private void showLocation(Location location) {
        mLogger.debug("Latitude:" + location.getLatitude() + ";" + "Longitude:"
                + location.getLongitude() + ";" + "Accuracy:" + location.getAccuracy());
    }

    /**
     * Judge the GPS whether opened
     * 
     * @return GPS is opened than return true ,or false
     */
    public boolean isGPSOpened() {
        return mLocationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
    }

    /**
     * The LocationListener for GPS Provider
     */
    private LocationListener mGpsListener = new LocationListener() {

        @Override
        public void onLocationChanged(Location location) {
            // Called when a new location is found by the location provider.
            mLogger.debug("Got New Location of provider:" + location.getProvider());
            if (mCurrentLocation != null) {
                if (isBetterLocation(location, mCurrentLocation)) {
                    mLogger.debug("It's a better location");
                    mCurrentLocation = location;
                    showLocation(location);
                } else {
                    mLogger.debug("The new location is not good!");
                }
            } else {
                mLogger.debug("It's first location");
                if (location != null) {
                    mCurrentLocation = location;
                    showLocation(location);
                }
            }
        }

        @Override
        public void onProviderDisabled(String provider) {
            mLogger.debug("onProviderDisabled");
        }

        @Override
        public void onProviderEnabled(String provider) {
            mLogger.debug("onProviderEnabled");
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            mLogger.debug("onStatusChanged");
        }

    };

    /**
     * Obtain the location with GPS
     * 
     * @return Return the location information with the values saved in this
     *         object
     */
    public Location getGPSLocation() {
        mLogger.debug("The Location is : " + mCurrentLocation);

        // use GPS to get location 
        if (mLocationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            Location location = mLocationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
            if (location != null && isBetterLocation(location, mCurrentLocation)) {
                mCurrentLocation = location;
            }
        }
        mLogger.debug("The location is got used GPSProvider is : " + mCurrentLocation);
        return mCurrentLocation;
    }

    protected boolean isBetterLocation(Location location, Location currentBestLocation) {
        if (currentBestLocation == null) {
            return true;
        }

        // Check whether the new location fix is newer or older
        long timeDelta = location.getTime() - currentBestLocation.getTime();
        boolean isSignificantlyNewer = timeDelta > CHECK_INTERVAL;
        boolean isSignificantlyOlder = timeDelta < -CHECK_INTERVAL;
        boolean isNewer = timeDelta > 0;

        // If it's been more than two minutes since the current location,
        // use the new location because the user has likely moved
        if (isSignificantlyNewer) {
            return true;
            // If the new location is more than two minutes older, it must
            // be worse
        } else if (isSignificantlyOlder) {
            return false;
        }

        // Check whether the new location fix is more or less accurate
        int accuracyDelta = (int) (location.getAccuracy() - currentBestLocation.getAccuracy());
        boolean isLessAccurate = accuracyDelta > 0;
        boolean isMoreAccurate = accuracyDelta < 0;
        boolean isSignificantlyLessAccurate = accuracyDelta > 200;

        // Check if the old and new location are from the same provider
        boolean isFromSameProvider = isSameProvider(location.getProvider(), currentBestLocation
                .getProvider());

        // Determine location quality using a combination of timeliness and
        // accuracy
        if (isMoreAccurate) {
            return true;
        } else if (isNewer && !isLessAccurate) {
            return true;
        } else if (isNewer && !isSignificantlyLessAccurate && isFromSameProvider) {
            return true;
        }
        return false;
    }

    /**
     * Checks whether two providers are the same
     * 
     * @param provider1 the first compare target 
     * @param provider2 the second compare target 
     * @return if equal return true,or false
     */
    private boolean isSameProvider(String provider1, String provider2) {
        if (provider1 == null) {
            return provider2 == null;
        }
        return provider1.equals(provider2);
    }
}
