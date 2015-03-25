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

package com.mediatek.rcse.settings;

import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.text.TextUtils;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

import java.util.HashMap;
import java.util.Map;

import javax.sip.ListeningPoint;

public class ProvisionProfileSettings extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {
    public static final int IPTEL_ACCOUNT = 0;
    public static final int TMOBILE_ACCOUNT = 1;
    public static final int VODADONE_ACCOUNT = 2;
	 public static final int TELEFONICA_ACCOUNT = 3;
	 public static final int ORANGE_ACCOUNT = 4;
	 
    // Fix issue: Video sharing can not set up, when the issue is fixed
    // SIP_OVER_TLS_ENABLED should be set to true
    public static final boolean SIP_OVER_TLS_ENABLED = false;

    private static class RcseAccountConfiguration {
        private String mAccountName = null;
        private String mAccountPassword = null;
        private String mAccountDisplayName = null;
        private String mDomain = null;
        private String mWifiSipProxyPort = null;
        private String mWifiSipProxyAddress = null;
        private String mMobileSipProxyPort = null;
        private String mMobileSipProxyAddress = null;
        private String mSipDefaultProtocolForMobile = null;
        private String mSipDefaultProtocolForWifi = null;
        private String mConferenceUri = null;
        private String mAutoConfigMode = null;
        private String mFtHttpServer = null;
        private String mFtHttpLogin=null;
        private String mFtHttpPassword=null;
    }

    private static final String TAG = "ProvisionProfileSettings";
    private static final String EDIT_ACCOUNT = "edit_account";
    private static final String[] TMOBILE_ACCOUNTS =
            new String[] {
                    "+8618608045697", "+8615110260744", "+8618811045090", "+8618603000343",
                    "+8613693695126", "+8613668151500", "+8613668151501", "+8618981714926",
                    "+8618981714927", "+8618200540563", "+8618200540564", "+8618980928189",
                    "+8618980928190", "+8615390086547", "+8615390086548", "+8613679077019",
                    "+8613679077020", "+8618228023994", "+8618228023995"
            };
    private static final String[] TMOBILE_DISPLAYNAMES =
            new String[] {
                    "Liwen Chang", "Bruce Lee", "Mi Tang", "Meijia Li", "Yuanyuan Li", "Alexy Li",
                    "Alexy Li1", "Elieen Yuan", "Elieen Yuan1", "Huan Feng", "Huan Feng1", "Free",
                    "Fred", "Wenhuai", "Tesla", "Xun Han", "Xun Han1", "Zhanxiang Tang",
                    "Zhanxiang Tang1"
            };
    private static final String[] VODAFONE_ACCOUNTS =
            new String[] {
                    "+34200000246", "+34200000247", "+34200000248", "+34200000249", "+34200000250",
                    "+34200000251", "+34200000252", "+34200000253", "+34200000254", "+34200000255",
                    "+34200000267", "+34200000268", "+34200000269", "+34200000270", "+34200000271",
                    "+34200000272", "+34200000273", "+34200000274", "+34200000275", "+34200000276"
            };
    
   
    private static final String[] VODAFONE_DISPLAYNAMES =
            new String[] {
                    "dummy1", "dummy2", "dummy3", "dummy4", "dummy5", "dummy6", "dummy7", "dummy8",
                    "dummy9", "dummy10", "dummy11", "dummy12", "dummy13", "dummy14", "dummy15",
                    "dummy16", "dummy17", "dummy18", "dummy19", "dummy20"
            };
    public static final String[] ACCOUNTS =
            new String[] {
                    /*
                     * "+8618608045697", "+8615110260744", "+8618811045090",
                     * "+8618603000343", "+8613693695126", "+8613668151500",
                     * "+8613668151501", "+8618981714926", "+8618981714927",
                     * "+8618200540563", "+8618200540564", "+8618980928189",
                     * "+8618980928190", "+8615390086547", "+8615390086548",
                     * "+8613679077019", "+8613679077020", "+8618228023994",
                     * "+8618228023995", "freehurui", "fredhurui", "kenshin",
                     * "aslan", "eyes", "taoqigui", "huanhuan", "berrydog",
                     */"+34200000246", "+34200000247", "+34200000248", "+34200000249",
                    "+34200000250", "+34200000251", "+34200000252", "+34200000253", "+34200000254",
                    "+34200000255", "+34200000267", "+34200000268", "+34200000269", "+34200000270",
                    "+34200000271", "+34200000272", "+34200000273", "+34200000274", "+34200000275",
                    "+34200000276" ,"+34680070013","+34680070014","+34680070015","+34670070025","+34670070026","+34670070027","+34670070028", "+34670070071" ,"+33679403191", "+33679404886", "+33679403505"
            };

    public static String[] GSM_NUMBERS =
        new String[] {
                "+919560699228", "+918586928441", "+918586928440","+919711808988"
        };

    private static final Map<String, RcseAccountConfiguration> RCSE_ACCOUNT_MAP =
            new HashMap<String, RcseAccountConfiguration>();
    private static final String IPTEL_DOMAIN = "iptel.org";
    private static final String TMOBILE_DOMAIN = "voiceservice.homeip.net";
    private static final String VODAFONE_DOMAIN = "ims.vodafone.es";
    private static final String ORANGE_DOMAIN = "rcs.lannion-e.com";
    private static final HashMap<String, Integer> ACCOUNT_TYPE_MAP = new HashMap<String, Integer>();
    private static int sAccountType;
    //private static final String FT_HTTP_SERVER = "http://62.87.37.92";
    private static final String FT_HTTP_SERVER = "https://80.58.30.68:8447";

    /** telefonica network use--------------------------------------------------------------------- */
    public static final String TELEFONICA_DOMAIN = "movistar.es";
    public static final String TELEFONICA_CSCF_ADDRESS = "pcscf.rcs.mnc007.mcc214.pub.3gppnetwork.org";
    /**now,just use  constant for the port, this value maybe changed.Now dummy account use wifi network and tcp mode.*/
    public static final String TELEFONICA_PORT_TCP = "5070";
    public static final String TELEFONICA_REIGSTER_IP = "80.58.57.168";
    public static final String TELEFONICA_CONFERENCE = "sip:rcseconferenceuri@d1im1.movistar.es";
    private static final int TYPE_TELEFONICA = 2;
    

    /** follow the Telefonica new account,2013-03-18 email from wendy 
     * +34680070010 ~ +34680070015*/
    private static final String[] TELEFONICA_ACCOUNTS = new String[]{
 "+34680070013","+34680070014","+34680070015","+34670070025","+34670070026","+34670070027","+34670070028","+34670070071"     
    };
    private static final String[] TELEFONICA_PASSWORD = new String[]{
        "680070013","680070014","680070015","670070025","670070026","670070027","670070028","670070071"
    };
    private static final String[] TELEFONICA_DISPLAYNAMES = new String[] {
        "Telefonica70013","Telefonica70014","Telefonica70015","Telefonica70025","Telefonica70026","Telefonica70027","Telefonica70028","Telefonica70071"
    };
    /** telefonica network use--------------------------------------------------------------------- */
    private static final String[] ORANGE_ACCOUNTS =
            new String[] {
    	            "+33679403191", "+33679404886", "+33679403505"
            };


    public static int getAccountType() {
        return sAccountType;
    }

    static {
        ACCOUNT_TYPE_MAP.put(IPTEL_DOMAIN, IPTEL_ACCOUNT);
        ACCOUNT_TYPE_MAP.put(TMOBILE_DOMAIN, TMOBILE_ACCOUNT);
        ACCOUNT_TYPE_MAP.put(VODAFONE_DOMAIN, VODADONE_ACCOUNT);
		ACCOUNT_TYPE_MAP.put(TELEFONICA_DOMAIN, TELEFONICA_ACCOUNT);
		ACCOUNT_TYPE_MAP.put(ORANGE_DOMAIN, ORANGE_ACCOUNT);

        RcseAccountConfiguration account = null;
        int length = TMOBILE_ACCOUNTS.length;
        for (int i = 0; i < length; ++i) {
            account = new RcseAccountConfiguration();
            account.mAccountName = TMOBILE_ACCOUNTS[i];
            account.mAccountPassword = "12345";
            account.mAccountDisplayName = TMOBILE_DISPLAYNAMES[i];
            account.mDomain = TMOBILE_DOMAIN;
            account.mWifiSipProxyPort = "5081";
            account.mWifiSipProxyAddress = "voiceservice.homeip.net";
            account.mMobileSipProxyPort = "5081";
            account.mMobileSipProxyAddress = "voiceservice.homeip.net";
            account.mSipDefaultProtocolForMobile = ListeningPoint.TCP;
            account.mSipDefaultProtocolForWifi = ListeningPoint.TCP;
            account.mConferenceUri = "sip:+491000055501@voiceservice.homeip.net";
            account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
            RCSE_ACCOUNT_MAP.put(TMOBILE_ACCOUNTS[i], account);
        }
        
        
        length = ORANGE_ACCOUNTS.length;
        for (int i = 0; i < length; ++i) {
            account = new RcseAccountConfiguration();
            account.mAccountName = ORANGE_ACCOUNTS[i];
            account.mAccountPassword = "imt30imt30";
            account.mAccountDisplayName = ORANGE_ACCOUNTS[i];
            account.mDomain = ORANGE_DOMAIN;
            account.mWifiSipProxyPort = "5081";
            account.mWifiSipProxyAddress = "rcs.lannion-e.com";
            account.mMobileSipProxyPort = "5080";
            account.mMobileSipProxyAddress = "rcs.lannion-e.com";
            account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
            account.mSipDefaultProtocolForWifi = ListeningPoint.TCP;
            account.mConferenceUri = "sip:Conference-Factory@rcs.lannion-e.com";
            account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
            account.mFtHttpServer= "https://ftcontentserver.rcs.mnc001.mcc208.pub.3gppnetwork.org/rcse-hcs/upload";
            account.mFtHttpLogin= ORANGE_ACCOUNTS[i] + "@rcs.lannion-e.com" ;
            account.mFtHttpPassword= "imt30imt30";
            RCSE_ACCOUNT_MAP.put(ORANGE_ACCOUNTS[i], account);
        }
        
        length = VODAFONE_ACCOUNTS.length;
        for (int i = 0; i < length; ++i) {
            account = new RcseAccountConfiguration();
            account.mAccountName = VODAFONE_ACCOUNTS[i];
            account.mAccountPassword =
                    VODAFONE_ACCOUNTS[i].substring(8, VODAFONE_ACCOUNTS[i].length());
            Logger.v(TAG, "password = " + account.mAccountPassword);
            account.mAccountDisplayName = VODAFONE_DISPLAYNAMES[i];
            account.mDomain = VODAFONE_DOMAIN;
            account.mMobileSipProxyPort = "5095";
            account.mMobileSipProxyAddress = "62.87.89.70";
            account.mSipDefaultProtocolForMobile = ListeningPoint.TCP;
            if (SIP_OVER_TLS_ENABLED) {
                Logger.d(TAG, "SIPoTLS enabled, if use wifi then connect to tls server");
                account.mWifiSipProxyPort = "5096";
                account.mWifiSipProxyAddress = "212.166.182.154";
                account.mSipDefaultProtocolForWifi = ListeningPoint.TLS;
            } else {
                Logger.d(TAG, "SIPoTLS disabled, if use wifi then connect to tcp server");
                account.mWifiSipProxyPort = "5095";
                account.mWifiSipProxyAddress = "62.87.89.70";
                account.mSipDefaultProtocolForWifi = ListeningPoint.TCP;
            }
            account.mConferenceUri = "sip:conference-uri@ims.vodafone.es";
            account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
            account.mFtHttpServer= "https://80.58.30.68:8447";
            RCSE_ACCOUNT_MAP.put(VODAFONE_ACCOUNTS[i], account);
        }

        length = TELEFONICA_ACCOUNTS.length;
        
        for (int i = 0; i < length; ++i) {
            account = new RcseAccountConfiguration();
            account.mAccountName = TELEFONICA_ACCOUNTS[i];
            account.mAccountPassword =TELEFONICA_PASSWORD[i];
            Logger.v(TAG, "password = " + account.mAccountPassword);
            account.mAccountDisplayName = TELEFONICA_DISPLAYNAMES[i];
            account.mDomain = TELEFONICA_DOMAIN;
            account.mMobileSipProxyPort = TELEFONICA_PORT_TCP;            
           
            account.mMobileSipProxyAddress = "195.53.251.96";
            account.mWifiSipProxyAddress = "195.53.251.96";
            
            account.mSipDefaultProtocolForMobile = ListeningPoint.TCP;
            account.mWifiSipProxyPort = TELEFONICA_PORT_TCP;            
            account.mSipDefaultProtocolForWifi = ListeningPoint.TCP;
            account.mConferenceUri = "sip:rcseconferenceuri@d1im1.movistar.es";
            account.mAutoConfigMode = ""+RcsSettingsData.NO_AUTO_CONFIG;
            account.mFtHttpServer= "https://80.58.30.68:8447";
            account.mFtHttpLogin= "670999999@TMELAB";
            account.mFtHttpPassword= "6709999999";
            RCSE_ACCOUNT_MAP.put(TELEFONICA_ACCOUNTS[i], account);
        }

        account = new RcseAccountConfiguration();
        account.mAccountName = "fredhurui";
        account.mAccountPassword = "123456";
        account.mAccountDisplayName = "fred";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);

        account = new RcseAccountConfiguration();
        account.mAccountName = "freehurui";
        account.mAccountPassword = "123456";
        account.mAccountDisplayName = "free";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
        account.mSipDefaultProtocolForWifi = ListeningPoint.UDP;
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);

        account = new RcseAccountConfiguration();
        account.mAccountName = "kenshin";
        account.mAccountPassword = "kenshin";
        account.mAccountDisplayName = "kenshin";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
        account.mSipDefaultProtocolForWifi = ListeningPoint.UDP;
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);

        account = new RcseAccountConfiguration();
        account.mAccountName = "aslan";
        account.mAccountPassword = "aslan";
        account.mAccountDisplayName = "aslan";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
        account.mSipDefaultProtocolForWifi = ListeningPoint.UDP;
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);

        account = new RcseAccountConfiguration();
        account.mAccountName = "eyes";
        account.mAccountPassword = "007213";
        account.mAccountDisplayName = "eyes";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
        account.mSipDefaultProtocolForWifi = ListeningPoint.UDP;
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);

        account = new RcseAccountConfiguration();
        account.mAccountName = "taoqigui";
        account.mAccountPassword = "54268";
        account.mAccountDisplayName = "taoqigui";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
        account.mSipDefaultProtocolForWifi = ListeningPoint.UDP;
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);

        account = new RcseAccountConfiguration();
        account.mAccountName = "huanhuan";
        account.mAccountPassword = "123456";
        account.mAccountDisplayName = "huanhuan";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
        account.mSipDefaultProtocolForWifi = ListeningPoint.UDP;
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);

        account = new RcseAccountConfiguration();
        account.mAccountName = "berrydog";
        account.mAccountPassword = "123456";
        account.mAccountDisplayName = "berrydog";
        account.mDomain = "iptel.org";
        account.mWifiSipProxyPort = "5060";
        account.mWifiSipProxyAddress = "sip.iptel.org";
        account.mMobileSipProxyPort = "5060";
        account.mMobileSipProxyAddress = "sip.iptel.org";
        account.mSipDefaultProtocolForMobile = ListeningPoint.UDP;
        account.mSipDefaultProtocolForWifi = ListeningPoint.UDP;
        account.mAutoConfigMode = "" + RcsSettingsData.NO_AUTO_CONFIG;
        RCSE_ACCOUNT_MAP.put(account.mAccountName, account);
        String domain = RcsSettings.getInstance().getUserProfileImsDomain();
        if (!TextUtils.isEmpty(domain)) {
            sAccountType = ACCOUNT_TYPE_MAP.get(domain);
        } else {
            Logger.e(TAG, "static field: domain is not a valid value, but " + domain);
        }
    }
    private ListPreference mAccountEdit;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.rcs_settings_provision_profile_preferences);
        setTitle(R.string.rcs_settings_title_provision_profile_settings);

        mAccountEdit = (ListPreference) findPreference(EDIT_ACCOUNT);
        mAccountEdit.setPersistent(false);
        mAccountEdit.setOnPreferenceChangeListener(this);
        RcsSettings.createInstance(getApplicationContext());
        String account = RcsSettings.getInstance().getUserProfileImsUserName();
        if ((account == null) || (account.trim().length() == 0)) {
            account = RcsSettings.getInstance().getUserProfileImsPrivateId();
        }
        mAccountEdit.setEntries(ACCOUNTS);
        mAccountEdit.setEntryValues(ACCOUNTS);
        mAccountEdit.setTitle(account);

    }

    public boolean onPreferenceChange(Preference preference, Object objValue) {
        if (EDIT_ACCOUNT.equals(preference.getKey())) {
            final String name = (String) objValue;
            mAccountEdit.setTitle(name);
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    RcseAccountConfiguration account = RCSE_ACCOUNT_MAP.get(name);
                    if (account == null) {
                        Logger.w(TAG, "when set rcse account, account is null");
                        return null;
                    }
                    String privateIdString = name + "@" + account.mDomain;
                    Logger.v(TAG, "account:" + account.mAccountDisplayName + "," + account.mDomain
                            + "," + account.mWifiSipProxyAddress + "," + account.mWifiSipProxyPort
                            + "," + account.mMobileSipProxyAddress + ","
                            + account.mMobileSipProxyPort);
                    RcsSettings.getInstance().setFtHttpServer(account.mFtHttpServer);
                    RcsSettings.getInstance().setFtHttpLogin(account.mFtHttpLogin);
                    RcsSettings.getInstance().setFtHttpPassword(account.mFtHttpPassword);
                   // RcsSettings.getInstance().setFtHttpLogin("670999999@TMELAB");
                   // RcsSettings.getInstance().setFtHttpPassword("670999999");
                    RcsSettings.getInstance().setUserProfileImsUserName(name);
                    RcsSettings.getInstance().setUserProfileImsDomain(account.mDomain);
                    RcsSettings.getInstance().setUserProfileImsDisplayName(
                            account.mAccountDisplayName);
                    
                    if (privateIdString != null && privateIdString.startsWith("+34")) {
                        privateIdString = privateIdString.substring(1, privateIdString.length());
                    }
                    RcsSettings.getInstance().setUserProfileImsPrivateId(privateIdString);
                    RcsSettings.getInstance().setUserProfileImsPassword(account.mAccountPassword);
                    RcsSettings.getInstance().setImsProxyAddrForMobile(
                            account.mMobileSipProxyAddress);
                    RcsSettings.getInstance().setImsProxyAddrForWifi(account.mWifiSipProxyAddress);
                    RcsSettings.getInstance().setImConferenceUri(account.mConferenceUri);
                    if (account.mMobileSipProxyPort != null) {
                        RcsSettings.getInstance().setImsProxyPortForMobile(
                                Integer.parseInt(account.mMobileSipProxyPort));
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.SIP_DEFAULT_PORT,
                                account.mMobileSipProxyPort);
                    }
                    if (account.mWifiSipProxyPort != null) {
                        RcsSettings.getInstance().setImsProxyPortForWifi(
                                Integer.parseInt(account.mWifiSipProxyPort));
                    }
                    RcsSettings.getInstance().writeParameter(
                            RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_MOBILE,
                            account.mSipDefaultProtocolForMobile);
                    RcsSettings.getInstance().writeParameter(
                            RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_WIFI,
                            account.mSipDefaultProtocolForWifi);
                    sAccountType = ACCOUNT_TYPE_MAP.get(account.mDomain);
                    RcsSettings.getInstance().writeParameter(RcsSettingsData.AUTO_CONFIG_MODE,
                            account.mAutoConfigMode);
                    return null;
                }
            }.execute();
        }
        return true;
    }
}
