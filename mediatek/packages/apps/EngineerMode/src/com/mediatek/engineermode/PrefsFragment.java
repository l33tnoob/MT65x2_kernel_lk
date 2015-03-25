/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;
import android.provider.Settings;
import android.content.ContentResolver;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.cip.CipUtil;
import com.mediatek.xlog.Xlog;
import java.io.File;

public class PrefsFragment extends PreferenceFragment {

    private static final String TAG = "EM/PrefsFragment";
    private static final int FRAGMENT_RES[] = { R.xml.telephony,
            R.xml.connectivity, R.xml.hardware_testing, R.xml.location,
            R.xml.log_and_debugging, R.xml.others, };
    private static final String INNER_LOAD_INDICATOR_FILE = 
    	"/system/etc/system_update/address.xml";

    private int mXmlResId;
    private static final int MTK_NFC_CHIP_TYPE_MSR3110 = 0x01;
    private static final int MTK_NFC_CHIP_TYPE_MT6605 = 0x02;
    
    /**
     * Default empty constructor
     */
    public PrefsFragment() {

    }

    /**
     * Set this fragment resource
     * 
     * @param resIndex
     *            Resource ID
     */
    public void setResource(int resIndex) {
        mXmlResId = resIndex;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Load preferences from xml.
        addPreferencesFromResource(FRAGMENT_RES[mXmlResId]);
        PreferenceScreen screen = getPreferenceScreen();

        if (!FeatureOption.MTK_DT_SUPPORT) {
            removePreference(screen, "dualtalk_network_info");
            removePreference(screen, "dualtalk_bandmode");
        }

        // Duplicate with Network Selecting, remove them
        removePreference(screen, "digital_standard");
        if (ModemCategory.getModemType() == ModemCategory.MODEM_TD) {
            removePreference(screen, "rat_mode");
        }

        if (!FeatureOption.MTK_DT_SUPPORT) {
            removePreference(screen, "dualtalk_network_select");
        } else {
            removePreference(screen, "network_select");
        }

        // if
        // (NfcAdapter.getDefaultAdapter(getActivity().getApplicationContext())
        // == null) {
        // removePreference(screen, "nfc");
        // }
        // it's ok
        int versionCode = Settings.Global.getInt(getActivity().getContentResolver(),"nfc_controller_code",-1);
        if(FRAGMENT_RES[mXmlResId] == R.xml.connectivity) {
            switch(versionCode) {
                case MTK_NFC_CHIP_TYPE_MSR3110 :
                    Xlog.i(TAG, "MSR3110 nfc chip, call nfc");
                    removePreference(screen, "hqanfc");
                    break;
                case MTK_NFC_CHIP_TYPE_MT6605 :
                    Xlog.i(TAG, "MT6605 nfc chip, call hqanfc");
                    removePreference(screen, "nfc");
                    break;
                default:
                    Xlog.i(TAG, "no nfc chip support");
                    removePreference(screen, "hqanfc");
                    removePreference(screen, "nfc");
                    break;
            }
        }
        if(!FeatureOption.MTK_NFC_SUPPORT) {
            removePreference(screen, "nfc_dta");
        }
        /*if (!FeatureOption.MTK_LOG2SERVER_APP) {
            removePreference(screen, "log2server");
        }*/

        if (FeatureOption.EVDO_DT_SUPPORT || !FeatureOption.MTK_DT_SUPPORT
                || SystemProperties.getInt("ril.external.md", 0) == 0) {
            removePreference(screen, "ext_md_logger");
        }

        if (!FeatureOption.MTK_SMSREG_APP) {
            removePreference(screen, "device_manager");
        }

        if (FeatureOption.MTK_BSP_PACKAGE) {
            removePreference(screen, "auto_answer");
        }

        if (ChipSupport.isFeatureSupported(ChipSupport.MTK_FM_SUPPORT)) {
            if (!ChipSupport.isFeatureSupported(ChipSupport.MTK_FM_TX_SUPPORT)) {
                removePreference(screen, "fm_transmitter");
            }
        } else {
            removePreference(screen, "fm_receiver");
            removePreference(screen, "fm_transmitter");
        }

        // AGPS is not ready if MTK_AGPS_APP isn't defined
        if (!ChipSupport.isFeatureSupported(ChipSupport.MTK_AGPS_APP)
                || !ChipSupport.isFeatureSupported(ChipSupport.MTK_GPS_SUPPORT)) {
            removePreference(screen, "location_basedservice");
        }

        if (!ChipSupport.isFeatureSupported(ChipSupport.MTK_GPS_SUPPORT)) {
            removePreference(screen, "ygps");
            removePreference(screen, "cw_test");
        }

        // MATV is not ready if HAVE_MATV_FEATURE isn't defined
        if (!ChipSupport.isFeatureSupported(ChipSupport.HAVE_MATV_FEATURE)) {
            removePreference(screen, "matv");
        }

        // BT is not ready if MTK_BT_SUPPORT isn't defined
        if (!ChipSupport.isFeatureSupported(ChipSupport.MTK_BT_SUPPORT)) {
            removePreference(screen, "bluetooth");
        }

        // wifi is not ready if MTK_WLAN_SUPPORT isn't defined
        if (!ChipSupport.isFeatureSupported(ChipSupport.MTK_WLAN_SUPPORT)) {
            removePreference(screen, "wifi");
        }

        if (!isVoiceCapable() || isWifiOnly()) {
            removePreference(screen, "auto_answer");
            removePreference(screen, "repeat_call_test");
            removePreference(screen, "video_telephony");
        }

        if (!FeatureOption.MTK_VT3G324M_SUPPORT) {
            removePreference(screen, "video_telephony");
        }

        if (isWifiOnly()) {
            removePreference(screen, "GPRS");
            removePreference(screen, "Modem");
            removePreference(screen, "NetworkInfo");
            removePreference(screen, "Baseband");
            removePreference(screen, "SIMMeLock");
            removePreference(screen, "BandMode");
            removePreference(screen, "RAT Mode");
            removePreference(screen, "SWLA");
            removePreference(screen, "ModemTest");
        }

        // if it single sim, then the flow is the same as before
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            /**
             * if it is Gemini, then the flow is : it start a TabActivity, then
             * the TabActivity will start sim1 or sim2 simLock activity Intent
             * to launch SIM lock TabActivity
             */
            // intent.setComponent(new
            // ComponentName("com.android.simmelock","com.android.simmelock.TabLockList"));
            removePreference(screen, "simme_lock1");
        } else {
            // Intent to launch SIM lock settings
            // intent.setComponent(new
            // ComponentName("com.android.simmelock","com.android.simmelock.LockList"));
            removePreference(screen, "simme_lock2");
        }
        Xlog.i(TAG, "ChipSupport.getChip(): " + ChipSupport.getChip());
        if (ChipSupport.MTK_6589_SUPPORT > ChipSupport.getChip()) {
            removePreference(screen, "de_sense");
            removePreference(screen, "camera89");
        } else {
            removePreference(screen, "camera");
        }

        if (!FeatureOption.MTK_FD_SUPPORT) {
            removePreference(screen, "fast_dormancy");
        }
        
        File innerLoadIndicator = new File(INNER_LOAD_INDICATOR_FILE);
        if (!innerLoadIndicator.exists()) {
        	removePreference(screen, "system_update");
        }
        if (!ChipSupport.isChipInSet(ChipSupport.CHIP_657X_SERIES_NEW)) {
        	removePreference(screen, "deep_idle");
        	removePreference(screen, "sleep_mode");
        	removePreference(screen, "dcm");
        	removePreference(screen, "pll_cg");
        	removePreference(screen, "cpu_dvfs");
        	removePreference(screen, "mcdi_setting");
        }

        if(!FeatureOption.MTK_CDS_EM_SUPPORT){
            removePreference(screen, "cds_information");
        }

        Preference pref = (Preference) findPreference("cmas");
        if (pref != null && !isActivityAvailable(pref.getIntent())) {
            removePreference(screen, "cmas");
        }

        if(!FeatureOption.MTK_WORLD_PHONE){
           removePreference(screen, "world_phone");
        }

        if (!FeatureOption.EVDO_DT_SUPPORT) {
            removePreference(screen, "saber");
        }
        
        String mOptr = SystemProperties.get("ro.operator.optr");
        if (!"OP01".equals(mOptr)) {
            removePreference(screen, "ConfigureCheck2_Send_Test");
            removePreference(screen, "ConfigureCheck2_Self_Test");
        }

        if ("OP09".equals(mOptr)) { // For CT
            removePreference(screen, "network_select");
            removePreference(screen, "dualtalk_network_select");
        } else {
            removePreference(screen, "cdma_network_select");
        }

        if (!FeatureOption.MTK_DEVREG_APP) {
            removePreference(screen, "device_register");
        }
        
        if (!FeatureOption.MTK_WFD_SUPPORT) {
            removePreference(screen, "wfd_settings");
        }

        if (!FeatureOption.MTK_LTE_DC_SUPPORT) {
            removePreference(screen, "lte_config");
        }
        if (!FeatureOption.MTK_LTE_SUPPORT) {
            removePreference(screen, "lte_network_info");
        }
        removePreference(screen, "lte_network_mode");

        if (!CipUtil.isCipSupported()) {
            removePreference(screen, "cip");
        }

        if (UserHandle.MU_ENABLED && UserManager.supportsMultipleUsers()) {
            // Remove all items used phone instance
            removePreference(screen, "bandmode");
            removePreference(screen, "te_dev_tool");
            removePreference(screen, "cds_information");
            removePreference(screen, "cfu");
            removePreference(screen, "fast_dormancy");
            removePreference(screen, "gprs");
            removePreference(screen, "hspa_info");
            removePreference(screen, "mobile_data_prefer");
            removePreference(screen, "modem_test");
            removePreference(screen, "modem_warning");
            removePreference(screen, "network_info");
            removePreference(screen, "dualtalk_network_info");
            removePreference(screen, "network_select");
            removePreference(screen, "cdma_network_select");
            removePreference(screen, "dualtalk_network_select");
            removePreference(screen, "rat_mode");
            removePreference(screen, "rf_desense_test");
            removePreference(screen, "swla");
            removePreference(screen, "simme_lock1");
            removePreference(screen, "simme_lock2");
            removePreference(screen, "world_phone");
            removePreference(screen, "lte_config");
            removePreference(screen, "lte_network_mode");
            removePreference(screen, "lte_network_info");
            removePreference(screen, "lte_tool");
        }

        if(!FeatureOption.MTK_DFO_RESOLUTION_SUPPORT){
           removePreference(screen, "dfo");
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        PreferenceScreen screen = getPreferenceScreen();
        int count = screen.getPreferenceCount();
        for (int i = 0; i < count; i++) {
            Preference pre = screen.getPreference(i);
            if (null != pre) {
                Intent intent = pre.getIntent();
                pre.setEnabled(isActivityAvailable(intent));
            }
        }
    }

    private void removePreference(PreferenceScreen prefScreen, String prefId) {
        Preference pref = (Preference) findPreference(prefId);
        if (pref != null) {
            prefScreen.removePreference(pref);
        }
    }

    private boolean isVoiceCapable() {
        TelephonyManager telephony = (TelephonyManager) getActivity()
                .getSystemService(Context.TELEPHONY_SERVICE);
        boolean bVoiceCapable = (telephony != null && telephony
                .isVoiceCapable());
        Xlog.i(TAG, "sIsVoiceCapable : " + bVoiceCapable);
        return bVoiceCapable;
    }

    private boolean isWifiOnly() {
        ConnectivityManager connManager = (ConnectivityManager) getActivity()
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        boolean bWifiOnly = false;
        if (null != connManager) {
            bWifiOnly = !connManager
                    .isNetworkSupported(ConnectivityManager.TYPE_MOBILE);
        }
        return bWifiOnly;
    }

    private boolean isActivityAvailable(Intent intent) {
        return (null != getActivity().getPackageManager().resolveActivity(
                intent, 0));
    }
}
