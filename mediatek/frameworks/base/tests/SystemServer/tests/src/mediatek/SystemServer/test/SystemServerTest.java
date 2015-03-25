/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package mediatek.SystemServer.test;
import android.bluetooth.BluetoothAdapter;
import android.test.AndroidTestCase;
import android.os.SystemProperties;
import android.os.ServiceManager;
import android.content.pm.IPackageManager;
import android.content.Context;
import android.util.Slog;
import com.android.internal.os.BinderInternal;
import java.util.ArrayList;
import java.util.Collections;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;

// For the tag of services
import com.mediatek.common.featureoption.FeatureOption;
import android.server.BluetoothProfileManagerService;
import android.service.dreams.DreamService;

public class SystemServerTest extends AndroidTestCase {
    private static final String TAG = "SystemServerTest";
    /*
     *  The default snooze delay: 1 seconds
     */
    private final long SNOOZE_DELAY = 1 * 1000L;
    private boolean mBootCompleted = false;
    private ArrayList<String> mSrvInitList = new ArrayList<String>();

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mBootCompleted = "1".equals(SystemProperties.get("dev.bootcomplete", "0"));
    }

    /**
    /* Test Case 1:
    /* Purpose: Check all services in the ServiceManager are active
    **/
    public void testServiceManager() throws Exception {
        Slog.v(TAG, "testServiceManager()");
        boolean bResult = false;

        do {
            if(false == mBootCompleted) {
                Slog.e(TAG, "Must test this test case after boot completed!");
                break;
            }
            // Query services from ServiceManager
            String[] srvList = ServiceManager.listServices();
            if(null == srvList) {
                Slog.e(TAG, "Service list is empty!");
                break;
            }
            Slog.v(TAG, "Total number of services in ServiceManager: " + srvList.length);

            // Check each services
            for(String srvTag : srvList) {
                Slog.v(TAG, "check service: " + srvTag);
                if(null == ServiceManager.checkService(srvTag)) {
                    break;
                }
            }
            bResult = true;
        } while(false);

        assertTrue(bResult);
    }

    /**
    /* Test Case 2:
    /* Purpose: Check all services are initialized in system server (Only in SystemServer.java) successfully.
    **/
    public void testServiceInitStatus() throws Exception {
        Slog.v(TAG, "testServiceInitStatus()");
        boolean bResult = true;

        do {
            if(false == mBootCompleted) {
                Slog.e(TAG, "Must test this test case after boot completed!");
                bResult = false;
                break;
            }
            // Prepare the service list to check
            prepareSystemServerSrvInitList(); // in SystemServer.java
            prepareOtherSrvInitList(); // Other important service

            Slog.v(TAG, "Total number of test services: " + mSrvInitList.size());
            // Check each services
            for(String srvTag : mSrvInitList) {
                Slog.v(TAG, "Check service: " + srvTag);
                if(null == ServiceManager.checkService(srvTag)) {
                    Slog.w(TAG, srvTag + " didn't register in ServiceManager successfully!");
                    bResult = false;
                }
            }
        } while(false);
        assertTrue(bResult);
    }

    private void prepareOtherSrvInitList() {
        // Other Important service
        mSrvInitList.add("package");
        mSrvInitList.add("activity");
        mSrvInitList.add("phone");
        mSrvInitList.add("isms");
    }

    private void prepareSystemServerSrvInitList() {
        Slog.v(TAG, "prepareSrvInitList() Serivce in SystemServer");
        mSrvInitList.add("entropy");
        mSrvInitList.add(Context.POWER_SERVICE);
        mSrvInitList.add(Context.DISPLAY_SERVICE);
        mSrvInitList.add("telephony.registry");
        mSrvInitList.add("telephony.registry2");
        /*
        if(true == FeatureOption.MTK_GEMINI_3SIM_SUPPORT || true == FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
            mSrvInitList.add("telephony.registry3");
            if(true == FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                mSrvInitList.add("telephony.registry4");
            }
        }
        */
        mSrvInitList.add(Context.USER_SERVICE);
        mSrvInitList.add(Context.ACCOUNT_SERVICE);
        mSrvInitList.add("battery");
        mSrvInitList.add("vibrator");
        mSrvInitList.add(Context.ALARM_SERVICE);
        mSrvInitList.add(Context.WINDOW_SERVICE);
        mSrvInitList.add(Context.INPUT_SERVICE);
        if(true == FeatureOption.MTK_BT_SUPPORT) {
            mSrvInitList.add(BluetoothAdapter.BLUETOOTH_MANAGER_SERVICE);
            if (true == FeatureOption.MTK_BT_PROFILE_MANAGER) {
                mSrvInitList.add(BluetoothProfileManagerService.BLUETOOTH_PROFILEMANAGER_SERVICE);
            }
        }
        mSrvInitList.add(Context.INPUT_METHOD_SERVICE);
        mSrvInitList.add(Context.ACCESSIBILITY_SERVICE);
        mSrvInitList.add("mount");
        mSrvInitList.add("lock_settings");
        mSrvInitList.add(Context.DEVICE_POLICY_SERVICE);
        mSrvInitList.add(Context.STATUS_BAR_SERVICE);
        mSrvInitList.add(Context.CLIPBOARD_SERVICE);
        mSrvInitList.add(Context.NETWORKMANAGEMENT_SERVICE);
        mSrvInitList.add(Context.TEXT_SERVICES_MANAGER_SERVICE);
        mSrvInitList.add(Context.NETWORK_STATS_SERVICE);
        mSrvInitList.add(Context.NETWORK_POLICY_SERVICE);
        mSrvInitList.add(Context.WIFI_P2P_SERVICE);
        mSrvInitList.add(Context.WIFI_SERVICE);
        mSrvInitList.add(Context.CONNECTIVITY_SERVICE);
        mSrvInitList.add(Context.NSD_SERVICE);
        //mSrvInitList.add(Context.THROTTLE_SERVICE);
        mSrvInitList.add(Context.UPDATE_LOCK_SERVICE);
        mSrvInitList.add(Context.NOTIFICATION_SERVICE);
        mSrvInitList.add("devicestoragemonitor");
        mSrvInitList.add(Context.LOCATION_SERVICE);
        mSrvInitList.add(Context.COUNTRY_DETECTOR);
        mSrvInitList.add(Context.SEARCH_SERVICE);

        Slog.v(TAG, "FeatureOption.MTK_BSP_PACKAGE:" + FeatureOption.MTK_BSP_PACKAGE);

        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            mSrvInitList.add(Context.SEARCH_ENGINE_SERVICE);
        }
        mSrvInitList.add(Context.DROPBOX_SERVICE);
        mSrvInitList.add(Context.WALLPAPER_SERVICE);
        mSrvInitList.add(Context.AUDIO_SERVICE);
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            mSrvInitList.add(Context.AUDIOPROFILE_SERVICE);
        }
        mSrvInitList.add(Context.USB_SERVICE);
        mSrvInitList.add(Context.SERIAL_SERVICE);
        mSrvInitList.add(Context.BACKUP_SERVICE);
        mSrvInitList.add(Context.APPWIDGET_SERVICE);
        if(true == FeatureOption.MTK_AGPS_APP && true == FeatureOption.MTK_GPS_SUPPORT) {
            mSrvInitList.add(Context.MTK_AGPS_SERVICE);
        }
        mSrvInitList.add("diskstats");
        mSrvInitList.add("samplingprofiler");
        mSrvInitList.add("commontime_management");
        mSrvInitList.add(DreamService.DREAM_SERVICE);
    }

    /**
    /* Test Case 3:
    /* Purpose: Check the total time to boot system server
    **/
    public void testBootupTime() {
        final float BOOT_TIME_THRESHOLD = 50000.0f; // The worstest case(until JB2) takes 41 seconds.
        final String SPLIT_TOKEN = " : ";
        Slog.v(TAG, "testSystemServerBootupTime()");
        boolean bResult = false;
        boolean bMTPROF_disable = "1".equals(SystemProperties.get("ro.mtprof.disable"));
        String recordString = null;
        Float startTime = 0.0f;
        Float endTime = 0.0f;

        try {
            if(!bMTPROF_disable) {
                String lineStr = null;
                FileReader fr = new FileReader("/proc/bootprof");
                BufferedReader br = new BufferedReader(fr);
                lineStr = br.readLine();
                while(null != lineStr) {
                    // Ignore head and tail lines
                    if(false == lineStr.contains(" : ")) {
                        lineStr = br.readLine();
                        continue;
                    }
                    lineStr = lineStr.trim();
                    String[] data = lineStr.split(SPLIT_TOKEN);
                    Float theTime = Float.valueOf(data[0]);
                    // Get start time
                    if(true == lineStr.contains("Android:SysServerInit_START")) {
                        startTime = theTime;
                    }
                    // Get end time
                    if(true == lineStr.contains("Android:SysServerInit_END")) {
                        endTime = theTime;
                    }
                    Slog.v(TAG, lineStr);
                    lineStr = br.readLine();
                }
                br.close();
                fr.close();
            } else {
                Slog.e(TAG, "Bootprof is disabled!");
            }

            // Start check boot time
            do {
                if(startTime == 0.0f || endTime == 0.0f) {
                    Slog.e(TAG, "Bootprof file is not complete!");
                    break;
                }
                if(startTime >= endTime) {
                    Slog.e(TAG, "Bootprof file has problem! startTime: " + startTime + " endTime: " + endTime);
                    break;
                }
                float diff = endTime - startTime;

                Slog.v(TAG, "Threadhold: " + BOOT_TIME_THRESHOLD);
                if(diff <= BOOT_TIME_THRESHOLD) {
                    Slog.v(TAG, "Take " + diff + " (ms) booting up");
                    bResult = true;
                } else {
                    Slog.e(TAG, "Take too much time (ms): " + diff);
                }
            }
            while(false);
        } catch (FileNotFoundException e) {
            Slog.e(TAG, "Failure open /proc/bootprof, not found!", e);
        } catch (java.io.IOException e) {
            Slog.e(TAG, "Failure open /proc/bootprof entry", e);
        } catch (Exception e) {
            Slog.e(TAG, "Test exception!", e);
        }
        assertTrue(bResult);
    }
}
