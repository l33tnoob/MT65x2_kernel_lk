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

package com.mediatek.smsreg;

import android.content.Context;
import android.os.Build;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.custom.CustomProperties;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.List;

class SmsBuilder {
    private String mTAG = "SmsReg/SmsBuilder";
    private TelephonyManagerEx mTeleMgr = null;    
    private int mSimId = 0;
    private ConfigInfoGenerator mConfigInfo;
    private String mModelName = null;

    SmsBuilder(Context context) {
        mTeleMgr = TelephonyManagerEx.getDefault();
        mConfigInfo = XMLGenerator.getInstance(SmsRegConst.getConfigPath());
        if (mConfigInfo == null) {
            Xlog.e(mTAG, "get XMLGenerator instance failed!");
        }
    }

    String getSmsContent(ConfigInfoGenerator configGenerator, int simCard) {
        mSimId = simCard;
        Xlog.i(mTAG, "SimId = " + mSimId);
        if (mSimId < 0) {
            Xlog.e(mTAG, "SimId is not valid!");
            return null;
        }
        List<SmsInfoUnit> smsInfoList = configGenerator.getSmsInfoList();
        if (smsInfoList == null) {
            Xlog.e(mTAG, "there is no sms segment in config file!");
            return null;
        }
        String smsContext = "";
        for (int i = 0; i < smsInfoList.size(); i++) {
            SmsInfoUnit smsUnit = smsInfoList.get(i);
            String smsUnitContent = smsUnit.getContent();
            Xlog.i(mTAG, "smsUnit = " + smsUnitContent);
            // if the smsUnit content is null, then return null message and DO
            // NOT to register
            String smsContent = getContentInfo(configGenerator, smsUnitContent);
            if (smsContent != null) {
                String prefix = smsUnit.getPrefix();
                String postfix = smsUnit.getPostfix();
                if (prefix != null) {
                    smsContext += prefix;
                }
                smsContext += smsContent;
                if (postfix != null) {
                    smsContext += postfix;
                }
            } else {
                Xlog.w(mTAG, "The smsUnit [" + smsUnitContent
                        + "] content is null");
                smsContext = null;
                break;
            }
        }
        Xlog.i(mTAG, "sms context: " + smsContext);
        return smsContext;
    }

    public String getContentInfo(ConfigInfoGenerator configGenerator,
            String command) {
        if (command.equals("getimsi")) {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                Xlog.i(mTAG, "get imsi by mSimId " + mSimId);
                return mTeleMgr.getSubscriberId(mSimId);
            } else {
                return mTeleMgr.getSubscriberId(0);
            }
        } else if (command.equals("getimei")) {
            /* Use IMEI of one sim slot as IMEI of device. I think it's 
             * weird that one device have two IMEI. 
             */
            String imei = mTeleMgr.getDeviceId(PhoneConstants.GEMINI_SIM_1);
            Xlog.i(mTAG, "return IMEI (single/gemini) " + imei);
            return imei;
        } else if (command.equals("getversion")) {
            Xlog.i(mTAG, "return version " + Build.DISPLAY);
            return Build.DISPLAY;
        } else if (command.equals("getproduct")) {
            mModelName = CustomProperties.getString(CustomProperties.MODULE_DM,
                CustomProperties.MODEL, "MTK");
            Xlog.i(mTAG, " return product('MTK' is for test): " + mModelName);
            return mModelName;
        } else if (command.equals("getvendor")) {
            String man = CustomProperties.getString(CustomProperties.MODULE_DM,
                    CustomProperties.MANUFACTURER, "MTK1");
            Xlog.i(mTAG, " return product('MTK1' is for test): " + man);
            return man;
        } else if (command.equals("getOem")) {
            String oemName = configGenerator.getOemName();
            if (oemName == null) {
                // Here should use the system api to get oem name;
                return null;
            }
            return oemName;
        } else {
            Xlog.w(mTAG, "The wrong command");
        }
        return null;
    }
}
