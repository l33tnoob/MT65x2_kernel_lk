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

package com.mediatek.common.agps;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.util.Log;
import android.util.Xml;

public class MtkAgpsProfileManager {

    private List<MtkAgpsProfile> mAgpsProfileList = new ArrayList<MtkAgpsProfile>();
    private boolean mDisableAfterReboot = false;
    private int mSiMode = 1;
    private boolean mCaEnable = true;
    private boolean mNiRequest = true;
    private boolean mAgpsEnable = true;
    private int mLogFileMaxNum = 10;
    private int mCpGeminiPrefSim = 1;
    private boolean mRoamingEnable = false;
    private String mDefaultProfileName = null;
    private boolean mCpUpSelection = false; //true=CP, false=UP
    private int mNotifyTimeout = 8; //0~20
    private int mVerifyTimeout = 8; //0~20
    private boolean mEcidStatus = false;
    private boolean mPmtk997_5 = false;
    private boolean mGpevt = false;
    private int mSuplVersion = 1;
    private int mDedicatedAPN = 0;
    private byte[] mEpcMolrLppPayload = new byte[0];

    private List<MtkAgpsCdmaProfile> mCdmaProfileList = new ArrayList<MtkAgpsCdmaProfile>();
    private String mDefaultCdmaProfile = null;
    private int mEvdoPrefer = 0; //o: wcdma prefer, 1:cdma prefer, 2.cdma force

    public int getEvdoPrefer() {
        return mEvdoPrefer;
    }

    public MtkAgpsCdmaProfile getDefaultCdmaProfile() {
        for(MtkAgpsCdmaProfile profile : mCdmaProfileList) {
            if(profile.mName.equals(mDefaultCdmaProfile)) {
                return profile;
            }
        }
        return null;
    }

    public List<MtkAgpsCdmaProfile> getAllCdmaProfile() {
        return mCdmaProfileList;
    }


    public MtkAgpsProfile getDefaultProfile() {
        for(MtkAgpsProfile profile : mAgpsProfileList) {
            if(profile.name.equals(mDefaultProfileName)) {
                return profile;
            }
        }
        return null;
    }

    public List<MtkAgpsProfile> getAllProfile() {
        return mAgpsProfileList;
    }

    public boolean getDisableAfterRebootStatus() {
        return mDisableAfterReboot;
    }

    public boolean getNiStatus() {
        return mNiRequest;
    }

    public boolean getAgpsStatus() {
        return mAgpsEnable;
    }

    public boolean getRoamingStatus() {
        return mRoamingEnable;
    }

    public int getCpPreferSim() {
        return mCpGeminiPrefSim;
    }

    // true=CP, false=UP
    public boolean getCpUpSelection() {
        return mCpUpSelection;
    }

    public int getNotifyTimeout() {
        return mNotifyTimeout;
    }

    public int getVerifyTimeout() {
        return mVerifyTimeout;
    }

    public int getSiMode() {
        return mSiMode;
    }

    public boolean getCaStatus() {
        return mCaEnable;
    }

    public boolean getEcidStatus() {
        return mEcidStatus;
    }

    public boolean getPmtk997_5() {
        return mPmtk997_5;
    }

    public boolean getGpevt() {
        return mGpevt;
    }

    public int getSuplVersion() {
        return mSuplVersion;
    }

    public int getDedicatedAPN() {
        return mDedicatedAPN;
    }

    public byte[] getEpcMolrLppPayload() {
        return mEpcMolrLppPayload;
    }

    public void insertProfile(MtkAgpsProfile profile) {
        int i = 0;
        boolean isNewOne = true;
        if(profile == null) {
            log("WARNING: insertPorifle the profile is null");
            return;
        }

        for(; i < mAgpsProfileList.size(); i ++) {
            if(profile.code.equals(mAgpsProfileList.get(i).code)) {
                //modify profile without inserting new one
                MtkAgpsProfile p = mAgpsProfileList.get(i);

                p.name              = profile.name;
                p.addr              = profile.addr;
                p.port              = profile.port;
                p.tls               = profile.tls;
                p.code              = profile.code;
                p.backupSlpNameVar  = profile.backupSlpNameVar;
                p.showType          = profile.showType;
                p.addrType          = profile.addrType;
                p.providerId        = profile.providerId;
                p.defaultApn        = profile.defaultApn;
                p.optionApn         = profile.optionApn;
                p.optionApn2        = profile.optionApn2;
                p.appId             = profile.appId;
                p.mccMnc            = profile.mccMnc;

                isNewOne = false;
                break;
            }
        }
        if(isNewOne) {
            //new profile
            mAgpsProfileList.add(profile);
        }
    }

    public void dumpFile(String path) {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(path));
            log("==== dumpFile path=" + path + " ====");
            String line = null;
            while ((line=reader.readLine()) != null) {
                log("dumpFile=" + line);
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if(reader != null)
                    reader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void updateAgpsProfile(String path) {
        XmlPullParser parser = null;
        InputStream is = null;
        try {
            int eventType;
            String name;
            String attrName;
            String attrValue;

            parser = Xml.newPullParser();
            is = new FileInputStream(path);

            parser.setInput(is, "utf-8");
            do {
                parser.next();
                eventType = parser.getEventType();
                name = parser.getName();
                int count = parser.getAttributeCount();
                //log("====== eventType=" + eventType + " name=" + name + " count=" + count + " =========");
                if(eventType != XmlPullParser.START_TAG)
                    continue;
                if(name.equals("agps_profile")) {
                    MtkAgpsProfile agpsProfile = new MtkAgpsProfile();
                    for(int i = 0; i < count; i ++) {
                        attrName = parser.getAttributeName(i);
                        attrValue = parser.getAttributeValue(i);
                        //log(" i=" + i + " name=" + attrName + " value=" + attrValue);

                        if(attrName.equals("address")) {        // 1
                            agpsProfile.addr = attrValue;
                        } else if(attrName.equals("slp_name")) {    // 2
                            agpsProfile.name = attrValue;
                        } else if(attrName.equals("port")) {        // 3
                            agpsProfile.port = Integer.valueOf(attrValue);
                        } else if(attrName.equals("tls")) {         // 4
                            agpsProfile.tls = Integer.valueOf(attrValue);
                        } else if(attrName.equals("show_type")) {   // 5
                            agpsProfile.showType = Integer.valueOf(attrValue);
                        } else if(attrName.equals("code")) {        // 6
                            agpsProfile.code = attrValue;
                        } else if(attrName.equals("backup_slp_name_var")) { // 7
                            agpsProfile.backupSlpNameVar = attrValue;
                        } else if(attrName.equals("provider_id")) { // 8
                            agpsProfile.providerId = attrValue;
                        } else if(attrName.equals("default_apn")) { // 9
                            agpsProfile.defaultApn = attrValue;
                        } else if(attrName.equals("address_type")) { // 10
                            agpsProfile.addrType= attrValue;
                        } else if(attrName.equals("optional_apn")) { // 11
                            agpsProfile.optionApn = attrValue;
                        } else if(attrName.equals("optional_apn_2")) { // 12
                            agpsProfile.optionApn2= attrValue;
                        } else if(attrName.equals("app_id")) { // 13
                            agpsProfile.appId= attrValue;
                        } else if(attrName.equals("mcc_mnc")) { // 14
                            agpsProfile.mccMnc= attrValue;
                        }
                    }
                    mAgpsProfileList.add(agpsProfile);

                } else if(name.equals("cdma_profile")) {
                    MtkAgpsCdmaProfile cdmaProfile = new MtkAgpsCdmaProfile();
                    for(int i = 0; i < count; i ++) {
                        attrName = parser.getAttributeName(i);
                        attrValue = parser.getAttributeValue(i);

                        if(attrName.equals("name")) {
                            cdmaProfile.mName = attrValue;
                        } else if(attrName.equals("mcp_enable")) {
                            cdmaProfile.mMcpEnable = attrValue.equals("yes")?1:0;
                        } else if(attrName.equals("mcp_addr")) {
                            cdmaProfile.mMcpAddr = attrValue;
                        } else if(attrName.equals("mcp_port")) {
                            cdmaProfile.mMcpPort = Integer.valueOf(attrValue);
                        } else if(attrName.equals("pde_addr_valid")) {
                            cdmaProfile.mPdeAddrValid = attrValue.equals("yes")?1:0;
                        } else if(attrName.equals("pde_ip_type")) {
                            cdmaProfile.mPdeIpType = Integer.valueOf(attrValue);
                        } else if(attrName.equals("pde_ip4_addr")) {
                            cdmaProfile.mPdeIp4Addr = attrValue;
                        } else if(attrName.equals("pde_ip6_addr")) {
                            cdmaProfile.mPdeIp6Addr = attrValue;
                        } else if(attrName.equals("pde_port")) {
                            cdmaProfile.mPdePort = Integer.valueOf(attrValue);
                        } else if(attrName.equals("pde_url_valid")) {
                            cdmaProfile.mPdeUrlValid = attrValue.equals("yes")?1:0;
                        } else if(attrName.equals("pde_url_addr")) {
                            cdmaProfile.mPdeUrlAddr = attrValue;
                        }
                    }
                    mCdmaProfileList.add(cdmaProfile);

                } else if(name.equals("agps_conf_para")) {
                    for(int i = 0; i < count; i ++) {
                        attrName = parser.getAttributeName(i);
                        attrValue = parser.getAttributeValue(i);

                        if(attrName.equals("disable_after_reboot")) {
                            mDisableAfterReboot = attrValue.equals("yes")?true:false;
                        } else if(attrName.equals("ni_request")) {
                            mNiRequest = attrValue.equals("yes")?true:false;
                        } else if(attrName.equals("agps_enable")) {
                            mAgpsEnable = attrValue.equals("yes")?true:false;
                        } else if(attrName.equals("log_file_max_num")) {
                            mLogFileMaxNum = Integer.valueOf(attrValue);
                        } else if(attrName.equals("cp_gemini_pref_sim")) {
                            mCpGeminiPrefSim = Integer.valueOf(attrValue);
                            if(mCpGeminiPrefSim <= 0 || mCpGeminiPrefSim > 4) {
                                mCpGeminiPrefSim = 1;
                            }
                        } else if(attrName.equals("roaming")) {
                            mRoamingEnable = attrValue.equals("yes")?true:false;
                        } else if(attrName.equals("default_profile")) {
                            mDefaultProfileName = attrValue;
                        } else if(attrName.equals("cp_up_selection")) {
                            mCpUpSelection = attrValue.equals("cp")?true:false;
                        } else if(attrName.equals("notify_timeout")) {
                            mNotifyTimeout = Integer.valueOf(attrValue);
                            if(mNotifyTimeout > 20 || mNotifyTimeout < 0) {
                                mNotifyTimeout = 8;
                            }
                        } else if(attrName.equals("verify_timeout")) {
                            mVerifyTimeout = Integer.valueOf(attrValue);
                            if(mVerifyTimeout > 20 || mVerifyTimeout < 0) {
                                mVerifyTimeout = 8;
                            }
                        } else if(attrName.equals("ca_enable")) {
                            mCaEnable = attrValue.equals("yes")?true:false;
                        } else if(attrName.equals("si_mode")) {
                            if(attrValue.equals("ma")) {
                                mSiMode = 0;
                            } else {
                                mSiMode = 1;
                            }
                        } else if(attrName.equals("ecid_enable")) {
                            mEcidStatus = attrValue.equals("yes")?true:false;

                        } else if(attrName.equals("cp_auto_reset")) {
                            mPmtk997_5 = attrValue.equals("yes")?true:false;

                        } else if(attrName.equals("gpevt")) {
                            mGpevt = attrValue.equals("yes")?true:false;


                        } else if(attrName.equals("supl_version")) {
                            mSuplVersion = Integer.valueOf(attrValue);
                            if(mSuplVersion > 2 || mSuplVersion <= 0) {
                                mSuplVersion = 1;
                            }


                        } else if(attrName.equals("cdma_agps_preferred")) {
                            mEvdoPrefer = Integer.valueOf(attrValue);
                            if(mEvdoPrefer > 2 || mEvdoPrefer <= 0) {
                                mEvdoPrefer = 0;
                            }

                        } else if(attrName.equals("default_cdma_profile")) {
                            mDefaultCdmaProfile = attrValue;


                        } else if(attrName.equals("dedicated_supl_apn")) {
                            mDedicatedAPN = Integer.valueOf(attrValue);
                            if(mDedicatedAPN > 2 || mDedicatedAPN < 0) {
                                mDedicatedAPN = 0;
                            }

                        } else if(attrName.equals("epc_molr_lpp_payload")) {
                            String lppPayload = attrValue;
                            if(lppPayload != null) {
                                mEpcMolrLppPayload = hexStringToByteArray(lppPayload);
                            }
                        }

                    }
                }

            } while(eventType != XmlPullParser.END_DOCUMENT);

        } catch (FileNotFoundException e) {
            e.printStackTrace();
            dumpFile(path);
        } catch (XmlPullParserException e) {
            e.printStackTrace();
            dumpFile(path);
        } catch (IOException e) {
            e.printStackTrace();
            dumpFile(path);
        } catch (Exception e) {
            e.printStackTrace();
            dumpFile(path);
        } finally {
            try {
                if(is != null)
                    is.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }//end of public void updateAgpsProfile(String path)

    private byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                                 + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }

    public String toString() {
        String tmp = new String();
        tmp = "mDisableAfterReboot=[" + mDisableAfterReboot + "] mNiRequest=[" + mNiRequest +
                "] mAgpsEnable=[" + mAgpsEnable + "] mSiMode=[" + mSiMode + "] mLogFileMaxNum=[" + mLogFileMaxNum +
                "] mCpGeminiPrefSim=[" + mCpGeminiPrefSim + "] mRoamingEnable=[" + mRoamingEnable + "] mCaEnable=[" + mCaEnable +
                "] mDefaultProfileName=[" + mDefaultProfileName + "] mCpUpSelection=[" + mCpUpSelection +
                "] mNotifyTimeout=[" + mNotifyTimeout + "] mVerifyTimeout=[" + mVerifyTimeout +
                "] mEcidStatus=[" + mEcidStatus + "] mPmtk997_5=[" + mPmtk997_5 + "] mGpevt=[" + mGpevt +
                "] mSuplVersion=[" + mSuplVersion + "] mEvdoPrefer=[" + mEvdoPrefer +
                "] mDefaultCdmaProfile=[" + mDefaultCdmaProfile + "] mDedicatedAPN=[" + mDedicatedAPN + 
                "] mEpcMolrLppPayload.length=[" + mEpcMolrLppPayload.length + "]\n";

        tmp += "============ AGPS Profile num=" + mAgpsProfileList.size() + " ===========\n";
        for(MtkAgpsProfile profile : mAgpsProfileList) {
            tmp += profile + "\n";
        }
        tmp += "============ CDMA AGPS Profile num=" + mCdmaProfileList.size() + " ===========\n";
        for(MtkAgpsCdmaProfile profile: mCdmaProfileList) {
            tmp += profile + "\n";
        }
        return tmp;
    }

    private void log(String msg) {
        Log.d("[MtkAgpsManagerService]", msg);
    }
}
