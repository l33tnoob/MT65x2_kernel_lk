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

package com.mediatek.omacp.message;

import android.content.Context;
import android.graphics.Typeface;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.text.style.StyleSpan;

import com.mediatek.omacp.R;
import com.mediatek.omacp.parser.ApplicationClass;
import com.mediatek.omacp.parser.ApplicationClass.AppAuth;
import com.mediatek.omacp.parser.ApplicationClass.Resource;
import com.mediatek.omacp.parser.NapdefClass;
import com.mediatek.omacp.parser.ProxyClass;
import com.mediatek.omacp.utils.MTKlog;

import java.util.ArrayList;

public class OmacpMessageUtils {

    private static final String TAG = "Omacp/OmacpMessageUtils";

    private static final boolean DEBUG = true;

    // constants for AppIds
    static final String BROWSER_APPID = "w2";

    static final String MMS_APPID = "w4";

    static final String DM_APPID = "w7";

    static final String SMTP_APPID = "25";

    static final String POP3_APPID = "110";

    static final String IMAP4_APPID = "143";

    static final String RTSP_APPID = "554";

    static final String SUPL_APPID = "ap0004";

    static final String MMS_2_APPID = "ap0005";

    static final String APN_APPID = "apn";

    static final String DS_APID = "w5";

    static final String IMPS_APPID = "wA";

    static String sEmailAccountName = null;

    static String sInboundEmailSetting = null;

    static String sOutboundEmailSetting = null;

    static String sEmailInboundType = null;

    private static final int APNTYPEBROWSER = 1;

    private static final int APNTYPEMMS = 2;

    private static final int APNTYPESUPL = 3;

    public static ArrayList<String> getValidApplicationNameSet(Context context,
            ArrayList<ApplicationClass> apList, ArrayList<NapdefClass> napList) {
        ArrayList<String> list = new ArrayList<String>();

        if (apList != null) {
            int size = apList.size();
            for (int i = 0; i < size; i++) {
                getOneValidApplicationNameSet(context, apList, list, i);
            }
        }

        if (napList != null && !napList.isEmpty()) {
            list.add(context.getString(R.string.apn_app_name));
        }

        return list;
    }

    private static void getOneValidApplicationNameSet(Context context,
            ArrayList<ApplicationClass> apList, ArrayList<String> list, int index) {
        ApplicationClass application = apList.get(index);
        String appId = application.mAppid;
        String name = null;
        if (application.mAppid.equalsIgnoreCase(SMTP_APPID)
                || application.mAppid.equalsIgnoreCase(POP3_APPID)
                || application.mAppid.equalsIgnoreCase(IMAP4_APPID)) {
            name = context.getString(R.string.email_app_name);
        } else if (application.mAppid.equalsIgnoreCase(MMS_APPID)) {
            // remove invalid mms setting if it only has mmsc, because it moved to apn
            if (application.mCm != null) {
                name = getAppName(context, appId);
            }
        } else if (application.mAppid.equalsIgnoreCase(MMS_2_APPID)) {
            if (application.mCm != null || application.mRm != null || application.mMs != null
                    || application.mPcAddr != null || application.mMa != null) {
                // if ap0005 mms setting only has mmsc, then ignore it,
                // because it has been moved to apn
                name = getAppName(context, appId);
            }

        } else {
            name = getAppName(context, appId);
        }
        if (name != null && !list.contains(name)) {
            list.add(name);
        }
    }

    public static String getAppName(Context context, String appId) {
        String name = null;
        if (appId.equalsIgnoreCase(MMS_APPID)) {
            name = context.getString(R.string.mms_app_name);
        } else if (appId.equalsIgnoreCase(MMS_2_APPID)) {
            name = context.getString(R.string.mms_app_name);
        } else if (appId.equalsIgnoreCase(BROWSER_APPID)) {
            name = context.getString(R.string.browser_app_name);
        } else if (appId.equalsIgnoreCase(APN_APPID)) {
            name = context.getString(R.string.apn_app_name);
        } else if (appId.equalsIgnoreCase(IMAP4_APPID)) {
            name = context.getString(R.string.email_app_name);
        } else if (appId.equalsIgnoreCase(POP3_APPID)) {
            name = context.getString(R.string.email_app_name);
        } else if (appId.equalsIgnoreCase(SMTP_APPID)) {
            name = context.getString(R.string.email_app_name);
        } else if (appId.equalsIgnoreCase(DM_APPID)) {
            name = context.getString(R.string.dm_app_name);
        } else if (appId.equalsIgnoreCase(SUPL_APPID)) {
            name = context.getString(R.string.agps_app_name);
        } else if (appId.equalsIgnoreCase(RTSP_APPID)) {
            name = context.getString(R.string.rtsp_app_name);
        } else if (appId.equalsIgnoreCase(DS_APID)) {
            name = context.getString(R.string.ds_app_name);
        } else if (appId.equalsIgnoreCase(IMPS_APPID)) {
            name = context.getString(R.string.imps_app_name);
        } else {
            MTKlog.e(TAG, "OmacpMessageUtils getAppName unknown app.");
        }
        return name;
    }

    public static String getSummary(Context context, String savedSummary) {
        if (DEBUG) {
            MTKlog.i(TAG, "OmacpMessageUtils savedSummary is : " + savedSummary);
        }

        String summary = context.getString(R.string.application_label);
        String messageSavedSummary = savedSummary;
        while (messageSavedSummary != null && messageSavedSummary.length() > 0) {
            int sep = messageSavedSummary.indexOf(",");
            String appId = null;
            if (sep >= 0) {
                appId = messageSavedSummary.substring(0, sep);

                if (messageSavedSummary.length() > sep + 1) {
                    messageSavedSummary = messageSavedSummary.substring(sep + 1);
                } else {
                    messageSavedSummary = null;
                }

                String name = getAppName(context, appId);
                if (null != name) {
                    summary += name;
                    summary += ", ";
                }
            }
        }

        if (summary.equalsIgnoreCase(context.getString(R.string.application_label))) {
            MTKlog.e(TAG, "OmacpMessageUtils summary is null.");
            summary = null;
        } else {
            summary = summary.substring(0, summary.length() - 2);
        }

        return summary;
    }

    public static SpannableStringBuilder getSettingsDetailInfo(Context context,
            ArrayList<ApplicationClass> apList, ArrayList<NapdefClass> napList,
            ArrayList<ProxyClass> pxList) {
        SpannableStringBuilder spannableDetailInfo = new SpannableStringBuilder();
        StringBuilder mmsSetting = new StringBuilder();
        if (apList != null) {
            int size = apList.size();
            for (int i = 0; i < size; i++) {
                ApplicationClass application = apList.get(i);
                if (DEBUG) {
                    MTKlog.i(TAG, "OmacpMessageUtils getSettingsDetailInfo application is : "
                            + application);
                }
                // append application's parameters
                appendApplicationParams(context, application, spannableDetailInfo, mmsSetting);
            }
            // append mms detail info to spannableDetailInfo
            appendMmsInfo(context, spannableDetailInfo, mmsSetting);
            // append email detail info to spannableDetailInfo
            appendEmailInfo(context, spannableDetailInfo);
        }
        if (napList != null) { // Apn
            SpannableStringBuilder apnInfo = getApnSettingInfo(context, napList, apList, pxList);
            if (apnInfo != null) {
                if (spannableDetailInfo.length() > 0) {
                    spannableDetailInfo.append("\n");
                }
                spannableDetailInfo.append(apnInfo);
            }
        }
        if (DEBUG) {
            MTKlog.d(TAG, "OmacpMessageUtils getSettingsDetailInfo info is : "
                    + spannableDetailInfo.toString());
        }
        return spannableDetailInfo;
    }

    private static void appendApplicationParams(Context context, ApplicationClass application,
            SpannableStringBuilder spannableDetailInfo, StringBuilder mmsSetting) {
        if (application.mAppid.equalsIgnoreCase(BROWSER_APPID)) { // Browser
            SpannableStringBuilder browserInfo = getBrowserSettingInfo(context, application);
            appendSpanableDetailInfo(browserInfo, spannableDetailInfo);
        } else if ((application.mAppid.equalsIgnoreCase(MMS_APPID) || application.mAppid
                .equalsIgnoreCase(MMS_2_APPID)) && mmsSetting.length() == 0) {
            getMmsSettingsInfo(context, application, mmsSetting);
        } else if (application.mAppid.equalsIgnoreCase(DM_APPID)) { // DM
            SpannableStringBuilder dmInfo = getDMSettingInfo(context, application);
            appendSpanableDetailInfo(dmInfo, spannableDetailInfo);
        } else if (application.mAppid.equalsIgnoreCase(SMTP_APPID)
                || application.mAppid.equalsIgnoreCase(POP3_APPID)
                || application.mAppid.equalsIgnoreCase(IMAP4_APPID)) {
            // add email settings
            getEmailSetting(context, application);
        } else if (application.mAppid.equalsIgnoreCase(RTSP_APPID)) {
            SpannableStringBuilder rtspInfo = getRtspSettingInfo(context, application);
            appendSpanableDetailInfo(rtspInfo, spannableDetailInfo);
        } else if (application.mAppid.equalsIgnoreCase(SUPL_APPID)) {
            SpannableStringBuilder suplInfo = getSuplSettingInfo(context, application);
            appendSpanableDetailInfo(suplInfo, spannableDetailInfo);
        } else if (application.mAppid.equalsIgnoreCase(DS_APID)) {
            SpannableStringBuilder dsInfo = getDsSettingInfo(context, application);
            appendSpanableDetailInfo(dsInfo, spannableDetailInfo);
        } else if (application.mAppid.equalsIgnoreCase(IMPS_APPID)) {
            SpannableStringBuilder impsInfo = getImpsSettingInfo(context, application);
            appendSpanableDetailInfo(impsInfo, spannableDetailInfo);
        } else {
            MTKlog.e(TAG, "OmacpMessageUtils getSettingsDetailInfo appid unknown.");
        }
    }

    private static void appendSpanableDetailInfo(SpannableStringBuilder appInfo,
            SpannableStringBuilder spannableDetailInfo) {
        if (appInfo != null) {
            if (spannableDetailInfo.length() > 0) {
                spannableDetailInfo.append("\n");
            }
            spannableDetailInfo.append(appInfo);
        }
    }

    private static void getEmailSetting(Context context, ApplicationClass application) {
        if (application.mAppid.equalsIgnoreCase(SMTP_APPID)) {
            if (sEmailAccountName == null && application.mProviderId != null) {
                sEmailAccountName = application.mProviderId;
            }
            if (sOutboundEmailSetting != null) {
                return;
            }
            sOutboundEmailSetting = getEmailSettingInfo(context, application);
            if (application.mAppauth.size() == 0) {
                return;
            }
            sOutboundEmailSetting += "\n";
            sOutboundEmailSetting += context.getString(R.string.email_need_sign_label);
            if (!OmacpApplicationCapability.sEmailOutboundAuthType) {
                sOutboundEmailSetting += context.getString(R.string.info_unsupport);
            } else if (application.mAppauth.get(0).mAauthtype != null) {

                sOutboundEmailSetting += context.getString(R.string.email_need_sign_yes);
            } else {
                sOutboundEmailSetting += context.getString(R.string.email_need_sign_no);
            }
        } else {
            if (sEmailAccountName == null && application.mProviderId != null) {
                sEmailAccountName = application.mProviderId;
            }
            if (sEmailInboundType == null && application.mAppid.equalsIgnoreCase(POP3_APPID)) {
                sEmailInboundType = context.getString(R.string.email_pop3_app_name);
            } else if (sEmailInboundType == null
                    && application.mAppid.equalsIgnoreCase(IMAP4_APPID)) {
                sEmailInboundType = context.getString(R.string.email_imap4_app_name);
            }
            if (sInboundEmailSetting == null) {
                sInboundEmailSetting = getEmailSettingInfo(context, application);
            }
        }
    }

    private static void appendEmailInfo(Context context, SpannableStringBuilder spannableDetailInfo) {
        if (sOutboundEmailSetting != null || sInboundEmailSetting != null) {
            if (spannableDetailInfo.length() > 0) {
                spannableDetailInfo.append("\n");
            }
            int before = spannableDetailInfo.length();
            // append "application: + ap name"
            spannableDetailInfo.append(context.getString(R.string.application_label));
            spannableDetailInfo.append(context.getString(R.string.email_app_name));
            spannableDetailInfo.setSpan(new ForegroundColorSpan(android.graphics.Color.MAGENTA),
                    before, spannableDetailInfo.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
            spannableDetailInfo.setSpan(new StyleSpan(Typeface.BOLD), before,
                    spannableDetailInfo.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
            if (!OmacpApplicationCapability.sEmail) {
                spannableDetailInfo.append("\n");
                spannableDetailInfo.append(context.getString(R.string.info_unsupport));
            } else {
                // account name
                if (sEmailAccountName != null) {
                    spannableDetailInfo.append("\n");
                    spannableDetailInfo.append(context.getString(R.string.email_account_label));
                    if (!OmacpApplicationCapability.sEmailProviderId) {
                        spannableDetailInfo.append(context.getString(R.string.info_unsupport));
                    } else {
                        spannableDetailInfo.append(sEmailAccountName);
                    }
                }
                // outbound setting info
                if (sOutboundEmailSetting != null) {
                    spannableDetailInfo.append("\n");

                    before = spannableDetailInfo.length();
                    spannableDetailInfo.append(context.getString(R.string.email_smtp_app_name));
                    spannableDetailInfo.setSpan(new StyleSpan(Typeface.BOLD), before,
                            spannableDetailInfo.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
                    spannableDetailInfo.append(sOutboundEmailSetting);
                }
                // inbound setting info
                if (sInboundEmailSetting != null && sEmailInboundType != null) {
                    spannableDetailInfo.append("\n");
                    before = spannableDetailInfo.length();
                    spannableDetailInfo.append(sEmailInboundType);
                    spannableDetailInfo.setSpan(new StyleSpan(Typeface.BOLD), before,
                            spannableDetailInfo.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
                    spannableDetailInfo.append(sInboundEmailSetting);
                }
            }
            resetEmailStaticSetting();
        }
    }

    private static void appendMmsInfo(Context context, SpannableStringBuilder spannableDetailInfo,
            StringBuilder mmsSetting) {
        if (mmsSetting.length() > 0) {
            if (spannableDetailInfo.length() > 0) {
                spannableDetailInfo.append("\n");
            }
            addApplicationLabel(context, spannableDetailInfo, MMS_APPID);
            if (!OmacpApplicationCapability.sMms) {
                spannableDetailInfo.append("\n");
                spannableDetailInfo.append(context.getString(R.string.info_unsupport));
            } else {
                spannableDetailInfo.append(mmsSetting.toString());
            }
        }
    }

    private static void getMmsSettingsInfo(Context context, ApplicationClass application,
            StringBuilder mmsSetting) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils addMmsSettingsInfo application is null.");
            return;
        }
        // if w4 mms setting only has mmsc, then ignore it, because it has been
        // moved to apn
        if (application.mAppid.equalsIgnoreCase(MMS_APPID) && application.mCm == null) {
            MTKlog.e(TAG, "OmacpMessageUtils invalid w4 mms setting.");
            return;
        }
        // if ap0005 mms setting only has mmsc, then ignore it, because it has
        // been moved to apn
        if (application.mAppid.equalsIgnoreCase(MMS_2_APPID) && application.mCm == null
                && application.mRm == null && application.mMs == null
                && application.mPcAddr == null && application.mMa == null) {
            MTKlog.e(TAG, "OmacpMessageUtils invalid ap0005 mms setting.");
            return;
        }
        // cm
        if (application.mCm != null) {
            mmsSetting.append(getElement(context, context.getString(R.string.mms_cm_label),
                    OmacpApplicationCapability.sMmsCm, application.mCm));
        }
        // rm
        if (application.mRm != null) {
            mmsSetting.append(getElement(context, context.getString(R.string.mms_rm_label),
                    OmacpApplicationCapability.sMmsRm, application.mRm));
        }
        // ms
        if (application.mMs != null) {
            mmsSetting.append(getElement(context, context.getString(R.string.mms_ms_label),
                    OmacpApplicationCapability.sMmsMs, application.mMs));
        }
        // pc addr
        if (application.mPcAddr != null) {
            mmsSetting.append(getElement(context, context.getString(R.string.mms_pc_addr_label),
                    OmacpApplicationCapability.sMmsPcAddr, application.mPcAddr));
        }
        // ma
        if (application.mMa != null) {
            mmsSetting.append(getElement(context, context.getString(R.string.mms_ma_label),
                    OmacpApplicationCapability.sMmsMa, application.mMa));
        }
    }

    private static String getElement(Context context, String label, boolean capability, String value) {
        String element = "\n";
        element += label;
        if (!capability) {
            element += context.getString(R.string.info_unsupport);
        } else {
            element += value;
        }
        return element;
    }

    private static void addApplicationLabel(Context context,
            SpannableStringBuilder spannableDetailInfo, String appId) {
        if (spannableDetailInfo == null) {
            MTKlog.e(TAG, "OmacpMessageUtils addApplicationLabel info is null.");
            return;
        }

        int before = spannableDetailInfo.length();
        // append "application: + ap name"
        spannableDetailInfo.append(context.getString(R.string.application_label));
        spannableDetailInfo.append(getAppName(context, appId));
        spannableDetailInfo.setSpan(new ForegroundColorSpan(android.graphics.Color.MAGENTA),
                before, spannableDetailInfo.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
        spannableDetailInfo.setSpan(new StyleSpan(Typeface.BOLD), before,
                spannableDetailInfo.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
    }

    private static String getEmailSettingInfo(Context context, ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils getEmailInboundSettingInfo application is null.");
            return null;
        }
        boolean isOutbound = false;
        if (application.mAppid.equalsIgnoreCase(SMTP_APPID)) {
            isOutbound = true;
        }
        StringBuilder emailInfo = new StringBuilder();
        if (application.mName != null) {
            emailInfo.append(getElement(context,
                    context.getString(R.string.email_setting_name_label),
                    OmacpApplicationCapability.sEmailSettingName, application.mName));
        }
        // append user name
        appendEmailUserInfo(context, application, emailInfo, isOutbound);
        // append server information
        appendEmailServerInfo(context, application, emailInfo, isOutbound);
        // append outBound information
        appendEmailOutBoundInfo(context, application, emailInfo, isOutbound);
        return emailInfo.toString();
    }

    private static void appendEmailUserInfo(Context context, ApplicationClass application,
            StringBuilder emailInfo, boolean isOutbound) {
        if (application == null) {
            return;
        }
        if (application.mAppauth.size() != 0) {
            if (application.mAppauth.get(0).mAauthname != null) {
                emailInfo.append(getEmailElement(context,
                        context.getString(R.string.user_name_label),
                        OmacpApplicationCapability.sEmailOutboundUserName,
                        OmacpApplicationCapability.sEmailInboundUserName, isOutbound,
                        application.mAppauth.get(0).mAauthname));
                // password
                if (application.mAppauth.get(0).mAauthsecret != null) {
                    emailInfo.append(getEmailElement(context,
                            context.getString(R.string.password_label),
                            OmacpApplicationCapability.sEmailOutboundPassword,
                            OmacpApplicationCapability.sEmailInboundPassword, isOutbound,
                            application.mAppauth.get(0).mAauthsecret));
                }
            }
        }
    }

    private static void appendEmailServerInfo(Context context, ApplicationClass application,
            StringBuilder emailInfo, boolean isOutbound) {
        if (application.mAppaddr.size() != 0 && application.mAppaddr.get(0).mAddr != null) {
            emailInfo.append(getEmailElement(context,
                    context.getString(R.string.server_address_label),
                    OmacpApplicationCapability.sEmailOutboundAddr,
                    OmacpApplicationCapability.sEmailInboundAddr, isOutbound,
                    application.mAppaddr.get(0).mAddr));
        } else if (application.mAddr.size() != 0 && application.mAddr.get(0) != null) {
            emailInfo.append(getEmailElement(context,
                    context.getString(R.string.server_address_label),
                    OmacpApplicationCapability.sEmailOutboundAddr,
                    OmacpApplicationCapability.sEmailInboundAddr, isOutbound,
                    application.mAddr.get(0)));
        }
        if (application.mAppaddr.get(0).mPort.size() != 0
                && application.mAppaddr.get(0).mPort.get(0).mPortnbr != null) {
            emailInfo.append(getEmailElement(context,
                    context.getString(R.string.port_number_label),
                    OmacpApplicationCapability.sEmailOutboundPortNumber,
                    OmacpApplicationCapability.sEmailInboundPortNumber, isOutbound,
                    application.mAppaddr.get(0).mPort.get(0).mPortnbr));
        }
        if (application.mAppaddr.get(0).mPort.size() != 0
                && application.mAppaddr.get(0).mPort.get(0).mService.size() != 0
                && application.mAppaddr.get(0).mPort.get(0).mService.get(0) != null) {
            emailInfo.append(getEmailElement(context,
                    context.getString(R.string.email_secure_label),
                    OmacpApplicationCapability.sEmailOutboundSecure,
                    OmacpApplicationCapability.sEmailInboundSecure, isOutbound,
                    application.mAppaddr.get(0).mPort.get(0).mService.get(0)));
        }
    }

    private static void appendEmailOutBoundInfo(Context context, ApplicationClass application,
            StringBuilder emailInfo, boolean isOutbound) {
        if (isOutbound) {
            if (application.mFrom != null) {
                emailInfo.append(getElement(context, context.getString(R.string.email_from_label),
                        OmacpApplicationCapability.sEmailFrom, application.mFrom));
            }
            if (application.mRtAddr != null) {
                emailInfo.append(getElement(context,
                        context.getString(R.string.email_rt_address_label),
                        OmacpApplicationCapability.sEmailRtAddr, application.mRtAddr));
            }
        }
    }

    private static String getEmailElement(Context context, String label, boolean outCapability,
            boolean inCapability, boolean isOutbound, String value) {
        String element = "\n";
        element += label;
        if (isOutbound) {
            if (outCapability) {
                element += value;
            } else {
                element += context.getString(R.string.info_unsupport);
            }
        } else {
            if (inCapability) {
                element += value;
            } else {
                element += context.getString(R.string.info_unsupport);
            }
        }
        return element;
    }

    private static SpannableStringBuilder getRtspSettingInfo(Context context,
            ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils getRtspSettingInfo application is null.");
            return null;
        }
        SpannableStringBuilder rtspInfo = new SpannableStringBuilder();
        StringBuilder rtspSettings = new StringBuilder();
        if (!OmacpApplicationCapability.sRtsp) {
            addApplicationLabel(context, rtspInfo, application.mAppid);
            rtspInfo.append("\n");
            rtspInfo.append(context.getString(R.string.info_unsupport));
        } else {
            // server id
            if (application.mProviderId != null) {
                rtspSettings.append(getElement(context,
                        context.getString(R.string.server_id_label),
                        OmacpApplicationCapability.sRtspProviderId, application.mProviderId));
            }
            // server name
            if (application.mName != null) {
                rtspSettings.append(getElement(context, context.getString(R.string.name_label),
                        OmacpApplicationCapability.sRtspName, application.mName));
            }
            if (application.mMaxBandwidth != null) {
                rtspSettings.append(getElement(context,
                        context.getString(R.string.rtsp_max_bandwidth_label),
                        OmacpApplicationCapability.sRtspMaxBandwidth, application.mMaxBandwidth));
            }
            if (application.mNetinfo != null) {
                for (int p = 0; p < application.mNetinfo.size(); p++) {
                    rtspSettings.append(getElement(context,
                            context.getString(R.string.rtsp_netinfo_label),
                            OmacpApplicationCapability.sRtspNetInfo, application.mNetinfo.get(p)));
                }
            }
            if (application.mMaxUdpPort != null) {
                rtspSettings.append(getElement(context,
                        context.getString(R.string.rtsp_max_udp_port_label),
                        OmacpApplicationCapability.sRtspMaxUdpPort, application.mMaxUdpPort));
            }
            if (application.mMinUdpPort != null) {
                rtspSettings.append(getElement(context,
                        context.getString(R.string.rtsp_min_udp_port_label),
                        OmacpApplicationCapability.sRtspMinUdpPort, application.mMinUdpPort));
            }
            if (rtspSettings.length() > 0) {
                addApplicationLabel(context, rtspInfo, application.mAppid);
                rtspInfo.append(rtspSettings);
            }
        }
        return rtspInfo;
    }

    private static SpannableStringBuilder getImpsSettingInfo(Context context,
            ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils getDsSettingInfo application is null.");
            return null;
        }

        SpannableStringBuilder impsInfo = new SpannableStringBuilder();
        StringBuilder impsSettings = new StringBuilder();
        if (!OmacpApplicationCapability.sImps) {
            addApplicationLabel(context, impsInfo, application.mAppid);
            impsInfo.append("\n");
            impsInfo.append(context.getString(R.string.info_unsupport));
        } else {
            appendImpsServerInfo(context, application, impsSettings);
            if (impsSettings.length() > 0) {
                addApplicationLabel(context, impsInfo, application.mAppid);
                impsInfo.append(impsSettings);
            }
        }
        return impsInfo;
    }

    private static void appendImpsServerInfo(Context context, ApplicationClass application,
            StringBuilder impsSettings) {
        // append Imps Server basic information.
        appendImpsServerBaseInfo(context, application, impsSettings);
        // addr type
        if (application.mAppaddr.size() != 0 && application.mAppaddr.get(0).mAddrtype != null) {
            impsSettings.append(getElement(context,
                    context.getString(R.string.server_addr_type_label),
                    OmacpApplicationCapability.sImpsAddressType,
                    application.mAppaddr.get(0).mAddrtype));
        }
        // append Imps Server auth information.
        appendImpsServerAuthInfo(context, application, impsSettings);
        // services
        if (application.mServices != null) {
            impsSettings.append(getElement(context,
                    context.getString(R.string.imps_services_label),
                    OmacpApplicationCapability.sImpsServices, application.mServices));
        }
        // cid prefix
        if (application.mCidprefix != null) {
            impsSettings.append(getElement(context,
                    context.getString(R.string.imps_cid_prefix_label),
                    OmacpApplicationCapability.sImpsClientIdPrefix, application.mCidprefix));
        }
    }

    private static void appendImpsServerBaseInfo(Context context, ApplicationClass application,
            StringBuilder impsSettings) {
        // server id
        if (application.mProviderId != null) {
            impsSettings.append(getElement(context, context.getString(R.string.server_id_label),
                    OmacpApplicationCapability.sImpsProviderId, application.mProviderId));
        }
        // server name
        if (application.mName != null) {
            impsSettings.append(getElement(context, context.getString(R.string.server_name_label),
                    OmacpApplicationCapability.sImpsServerName, application.mName));
        }
        // content type
        if (application.mAaccept != null) {
            impsSettings.append(getElement(context, context.getString(R.string.content_type_label),
                    OmacpApplicationCapability.sImpsContentType, application.mAaccept));
        }
        // server address
        if (application.mAppaddr.size() != 0 && application.mAppaddr.get(0).mAddr != null) {
            impsSettings.append(getElement(context,
                    context.getString(R.string.server_address_label),
                    OmacpApplicationCapability.sImpsServerAddress,
                    application.mAppaddr.get(0).mAddr));
        } else if (application.mAddr.size() != 0 && application.mAddr.get(0) != null) {
            impsSettings.append(getElement(context,
                    context.getString(R.string.server_address_label),
                    OmacpApplicationCapability.sImpsServerAddress, application.mAddr.get(0)));
        }
    }

    private static void appendImpsServerAuthInfo(Context context, ApplicationClass application,
            StringBuilder impsSettings) {
     // auth
        if (application.mAppauth != null && application.mAppauth.size() != 0) {
            AppAuth auth = application.mAppauth.get(0);
            if (auth.mAauthlevel != null) {
                impsSettings.append(getElement(context,
                        context.getString(R.string.auth_level_label),
                        OmacpApplicationCapability.sImpsAuthLevel, auth.mAauthlevel));
            }

            if (auth.mAauthname != null) {
                impsSettings.append(getElement(context,
                        context.getString(R.string.user_name_label),
                        OmacpApplicationCapability.sImpsAuthName, auth.mAauthname));
            }

            if (auth.mAauthsecret != null) {
                impsSettings.append(getElement(context, context.getString(R.string.password_label),
                        OmacpApplicationCapability.sImpsAuthSecret, auth.mAauthsecret));
            }
        }
    }

    private static SpannableStringBuilder getDsSettingInfo(Context context,
            ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils getDsSettingInfo application is null.");
            return null;
        }

        SpannableStringBuilder info = new SpannableStringBuilder();
        StringBuilder dsSettings = new StringBuilder();
        if (!OmacpApplicationCapability.sDs) {
            addApplicationLabel(context, info, application.mAppid);
            info.append("\n");
            info.append(context.getString(R.string.info_unsupport));
            return info;
        } else {
            // Add Ds Server params
            getDsServerInfo(context, application, dsSettings);
            // auth
            getDsAuthInfo(context, application, dsSettings);
            // resource databases
            getDsResourceInfo(context, application, dsSettings);
            if (dsSettings.length() > 0) {
                addApplicationLabel(context, info, application.mAppid);
                info.append(dsSettings);
            }
            return info;
        }
    }

    private static void getDsServerInfo(Context context, ApplicationClass application,
            StringBuilder settings) {
        // server name
        if (application.mName != null) {
            settings.append(getElement(context, context.getString(R.string.server_name_label),
                    OmacpApplicationCapability.sDsServerName, application.mName));
        }
        // server id
        if (application.mProviderId != null) {
            settings.append(getElement(context, context.getString(R.string.server_id_label),
                    OmacpApplicationCapability.sSuplProviderId, application.mProviderId));
        }
        // server address
        if (application.mAppaddr.size() != 0 && application.mAppaddr.get(0).mAddr != null) {
            settings.append(getElement(context, context.getString(R.string.server_address_label),
                    OmacpApplicationCapability.sDsServerAddress, application.mAppaddr.get(0).mAddr));
        } else if (application.mAddr.size() != 0 && application.mAddr.get(0) != null) {
            settings.append(getElement(context, context.getString(R.string.server_address_label),
                    OmacpApplicationCapability.sDsServerAddress, application.mAddr.get(0)));
        }
        // addr type
        if (application.mAppaddr.size() != 0 && application.mAppaddr.get(0).mAddrtype != null) {
            settings.append(getElement(context, context.getString(R.string.server_addr_type_label),
                    OmacpApplicationCapability.sDsAddressType,
                    application.mAppaddr.get(0).mAddrtype));
        }
        // port number
        if (application.mAddr != null && application.mAppaddr.size() != 0
                && application.mAppaddr.get(0).mPort.size() != 0
                && application.mAppaddr.get(0).mPort.get(0).mPortnbr != null) {
            settings.append(getElement(context, context.getString(R.string.port_number_label),
                    OmacpApplicationCapability.sDsPortNumber,
                    application.mAppaddr.get(0).mPort.get(0).mPortnbr));
        }
    }

    private static void getDsAuthInfo(Context context, ApplicationClass application,
            StringBuilder settings) {
        if (application.mAppauth != null && application.mAppauth.size() != 0) {
            AppAuth auth = application.mAppauth.get(0);
            if (auth.mAauthlevel != null) {
                settings.append(getElement(context, context.getString(R.string.auth_level_label),
                        OmacpApplicationCapability.sDsAuthLevel, auth.mAauthlevel));
            }

            if (auth.mAauthtype != null) {
                settings.append(getElement(context, context.getString(R.string.auth_type_label),
                        OmacpApplicationCapability.sDsAuthType, auth.mAauthtype));
            }

            if (auth.mAauthname != null) {
                settings.append(getElement(context, context.getString(R.string.user_name_label),
                        OmacpApplicationCapability.sDsAuthName, auth.mAauthname));
            }

            if (auth.mAauthsecret != null) {
                settings.append(getElement(context, context.getString(R.string.password_label),
                        OmacpApplicationCapability.sDsAuthSecret, auth.mAauthsecret));
            }

            if (auth.mAauthdata != null) {
                settings.append(getElement(context, context.getString(R.string.auth_data_label),
                        OmacpApplicationCapability.sDsAuthData, auth.mAauthdata));
            }
        }
    }

    private static void getDsResourceInfo(Context context, ApplicationClass application,
            StringBuilder settings) {
        ArrayList<Resource> resourceList = application.mResource;
        for (int j = 0; j < resourceList.size(); j++) {
            Resource resource = resourceList.get(j);

            StringBuilder database = new StringBuilder();

            if (resource.mUri != null) {
                // database name
                if (resource.mName != null) {
                    database.append(getElement(context,
                            context.getString(R.string.ds_database_name_label),
                            OmacpApplicationCapability.sDsDatabaseName, resource.mName));
                }
                // database uri
                database.append(getElement(context,
                        context.getString(R.string.ds_database_url_label),
                        OmacpApplicationCapability.sDsDatabaseUrl, resource.mUri));

                // content type
                if (resource.mAaccept != null) {
                    database.append(getElement(context,
                            context.getString(R.string.content_type_label),
                            OmacpApplicationCapability.sDsDatabaseContentType, resource.mAaccept));
                }
                // auth type
                if (resource.mAauthtype != null) {
                    database.append(getElement(context,
                            context.getString(R.string.auth_type_label),
                            OmacpApplicationCapability.sDsDatabaseAuthType, resource.mAauthtype));
                }
                // auth name
                if (resource.mAauthname != null) {
                    database.append(getElement(context,
                            context.getString(R.string.user_name_label),
                            OmacpApplicationCapability.sDsDatabaseAuthName, resource.mAauthname));
                }
                // auth secret
                if (resource.mAauthsecret != null) {
                    database.append(getElement(context, context.getString(R.string.password_label),
                            OmacpApplicationCapability.sDsDatabaseAuthSecret, resource.mAauthsecret));
                }
                // cliuri
                if (resource.mCliuri != null) {
                    database.append(getElement(context,
                            context.getString(R.string.ds_cliuri_label),
                            OmacpApplicationCapability.sDsClientDatabaseUrl, resource.mCliuri));
                }
                // sync type
                if (resource.mSynctype != null) {
                    database.append(getElement(context,
                            context.getString(R.string.ds_sync_type_label),
                            OmacpApplicationCapability.sDsSyncType, resource.mSynctype));
                }
                if (database.length() > 0) {
                    settings.append(database);
                }
            }
        }
    }

    private static SpannableStringBuilder getSuplSettingInfo(Context context,
            ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils getSuplSettingInfo application is null.");
            return null;
        }

        SpannableStringBuilder suplInfo = new SpannableStringBuilder();
        StringBuilder suplSettings = new StringBuilder();
        if (!OmacpApplicationCapability.sSupl) {
            addApplicationLabel(context, suplInfo, application.mAppid);
            suplInfo.append("\n");
            suplInfo.append(context.getString(R.string.info_unsupport));
        } else {
            // server id
            if (application.mProviderId != null) {
                suplSettings.append(getElement(context,
                        context.getString(R.string.server_id_label),
                        OmacpApplicationCapability.sSuplProviderId, application.mProviderId));
            }
            // server name
            if (application.mName != null) {
                suplSettings.append(getElement(context,
                        context.getString(R.string.server_name_label),
                        OmacpApplicationCapability.sSuplServerName, application.mName));
            }
            if (application.mAppaddr.size() != 0 && application.mAppaddr.get(0).mAddr != null) {
                suplSettings.append(getElement(context,
                        context.getString(R.string.server_address_label),
                        OmacpApplicationCapability.sSuplServerAddr,
                        application.mAppaddr.get(0).mAddr));
            } else if (application.mAddr.size() != 0 && application.mAddr.get(0) != null) {
                suplSettings.append(getElement(context,
                        context.getString(R.string.server_address_label),
                        OmacpApplicationCapability.sSuplServerAddr, application.mAddr.get(0)));
            }

            // addr type
            if (application.mAppaddr.size() != 0 && application.mAppaddr.get(0).mAddrtype != null) {
                suplSettings.append(getElement(context,
                        context.getString(R.string.server_addr_type_label),
                        OmacpApplicationCapability.sSuplAddrType,
                        application.mAppaddr.get(0).mAddrtype));
            }

            if (suplSettings.length() > 0) {
                addApplicationLabel(context, suplInfo, application.mAppid);
                suplInfo.append(suplSettings);
            }
        }
        return suplInfo;
    }

    private static SpannableStringBuilder getDMSettingInfo(Context context,
            ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils getDMSettingInfo application is null.");
            return null;
        }

        SpannableStringBuilder dmInfo = new SpannableStringBuilder();
        StringBuilder dmSettings = new StringBuilder();
        if (!OmacpApplicationCapability.sDm) {
            addApplicationLabel(context, dmInfo, application.mAppid);
            dmInfo.append("\n");
            dmInfo.append(context.getString(R.string.info_unsupport));
        } else {
            //append DM information
            appendDmSettingsInfo(context, application, dmSettings);
            appendDmIsSupport(context, application, dmSettings);
            if (dmSettings.length() > 0) {
                addApplicationLabel(context, dmInfo, application.mAppid);
                dmInfo.append(dmSettings);
            }
        }
        return dmInfo;
    }

    private static void appendDmSettingsInfo(Context context, ApplicationClass application,
            StringBuilder dmSettings) {
        if (application.mProviderId != null) {
            dmSettings.append(getElement(context, context.getString(R.string.server_id_label),
                    OmacpApplicationCapability.sDmProviderId, application.mProviderId));
        }
        if (application.mName != null) {
            dmSettings.append(getElement(context, context.getString(R.string.server_name_label),
                    OmacpApplicationCapability.sDmServerName, application.mName));
        }
        // append Dm addr params.
        appendDmAddrSettingsInfo(context, application, dmSettings);
        if (application.mAppauth != null && application.mAppauth.size() != 0) {
            // append Dm auth params.
           appendDmAuthSettingsInfo(context, application, dmSettings);
        }
    }

    private static void appendDmAddrSettingsInfo(Context context, ApplicationClass application,
            StringBuilder dmSettings) {
        if (application.mAddr != null && application.mAppaddr.size() != 0
                && application.mAppaddr.get(0).mAddr != null) {
            dmSettings
                    .append(getElement(context, context.getString(R.string.server_address_label),
                            OmacpApplicationCapability.sDmServerAddress,
                            application.mAppaddr.get(0).mAddr));
        } else if (application.mAddr != null && application.mAddr.size() != 0
                && application.mAddr.get(0) != null) {
            dmSettings.append(getElement(context, context.getString(R.string.server_address_label),
                    OmacpApplicationCapability.sDmServerAddress, application.mAddr.get(0)));
        }
        if (application.mAddr != null && application.mAppaddr.size() != 0
                && application.mAppaddr.get(0).mAddrtype != null) {
            dmSettings.append(getElement(context,
                    context.getString(R.string.server_addr_type_label),
                    OmacpApplicationCapability.sDmAddrType, application.mAppaddr.get(0).mAddrtype));
        }
        if (application.mAddr != null && application.mAppaddr.size() != 0
                && application.mAppaddr.get(0).mPort.size() != 0
                && application.mAppaddr.get(0).mPort.get(0).mPortnbr != null) {
            dmSettings.append(getElement(context, context.getString(R.string.port_number_label),
                    OmacpApplicationCapability.sDmPortNumber,
                    application.mAppaddr.get(0).mPort.get(0).mPortnbr));
        }
    }

    private static void appendDmAuthSettingsInfo(Context context, ApplicationClass application,
            StringBuilder dmSettings) {
        AppAuth auth = application.mAppauth.get(0);
        if (auth.mAauthlevel != null) {
            dmSettings.append(getElement(context, context.getString(R.string.auth_level_label),
                    OmacpApplicationCapability.sDmAuthLevel, auth.mAauthlevel));
        }
        if (auth.mAauthtype != null) {
            dmSettings.append(getElement(context, context.getString(R.string.auth_type_label),
                    OmacpApplicationCapability.sDmAuthType, auth.mAauthtype));
        }
        if (auth.mAauthname != null) {
            dmSettings.append(getElement(context, context.getString(R.string.user_name_label),
                    OmacpApplicationCapability.sDmAuthName, auth.mAauthname));
        }
        if (auth.mAauthsecret != null) {
            dmSettings.append(getElement(context, context.getString(R.string.password_label),
                    OmacpApplicationCapability.sDmAuthSecret, auth.mAauthsecret));
        }
        if (auth.mAauthdata != null) {
            dmSettings.append(getElement(context, context.getString(R.string.auth_data_label),
                    OmacpApplicationCapability.sDmAuthData, auth.mAauthdata));
        }
    }

    private static void appendDmIsSupport(Context context, ApplicationClass application,
            StringBuilder dmSettings) {
        if (application.mInit != null) {
            dmSettings.append("\n");
            dmSettings.append(context.getString(R.string.dm_init_label));
            if (!OmacpApplicationCapability.sDmInit) {
                dmSettings.append(context.getString(R.string.info_unsupport));
            } else {
                if (application.mInit.equalsIgnoreCase("1")) {
                    dmSettings.append(context.getString(R.string.yes));
                } else {
                    dmSettings.append(context.getString(R.string.no));
                }
            }
        }
    }

    private static SpannableStringBuilder getBrowserSettingInfo(Context context,
            ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpMessageUtils getBrowserSettingInfo application is null.");
            return null;
        }

        SpannableStringBuilder browserInfo = new SpannableStringBuilder();
        StringBuilder browserSettings = new StringBuilder();
        if (!OmacpApplicationCapability.sBrowser) {
            addApplicationLabel(context, browserInfo, application.mAppid);
            browserInfo.append("\n");
            browserInfo.append(context.getString(R.string.info_unsupport));
        } else {
            // bookmark folder
            if (application.mName != null) {
                browserSettings.append(getElement(context,
                        context.getString(R.string.bookmark_folder_label),
                        OmacpApplicationCapability.sBrowserBookMarkFolder, application.mName));
            }

            ArrayList<Resource> resourceList = application.mResource;
            String homePage = null;
            for (int j = 0; j < resourceList.size(); j++) {
                Resource resource = resourceList.get(j);
                if (resource.mUri != null) {
                    if (resource.mName != null) {
                        browserSettings.append(getElement(context,
                                context.getString(R.string.bookmark_name_label),
                                OmacpApplicationCapability.sBrowserBookMarkName, resource.mName));
                    }
                    browserSettings.append(getElement(context,
                            context.getString(R.string.bookmark_label),
                            OmacpApplicationCapability.sBrowserBookMark, resource.mUri));

                    if (resource.mAauthname != null) {
                        browserSettings.append(getElement(context,
                                context.getString(R.string.user_name_label),
                                OmacpApplicationCapability.sBrowserUserName, resource.mAauthname));
                    }
                    if (resource.mAauthsecret != null) {
                        browserSettings
                                .append(getElement(context,
                                        context.getString(R.string.password_label),
                                        OmacpApplicationCapability.sBrowserPassWord,
                                        resource.mAauthsecret));
                    }
                    if (resource.mStartpage != null && resource.mStartpage.equalsIgnoreCase("1")) {
                        if (homePage == null) {
                            homePage = resource.mUri;
                        }
                    }
                }
            }
            if (homePage != null) {
                browserSettings.append("\n");
                browserSettings.append(context.getString(R.string.homepage_label));
                if (!OmacpApplicationCapability.sBrowserHomePage) {
                    browserSettings.append(context.getString(R.string.info_unsupport));
                } else {
                    browserSettings.append(homePage);
                }
            }
            if (browserSettings.length() > 0) {
                addApplicationLabel(context, browserInfo, application.mAppid);
                browserInfo.append(browserSettings);
            } else {
                addApplicationLabel(context, browserInfo, application.mAppid);
                browserInfo.append(context.getString(R.string.info_unavaliable));
            }
        }
        return browserInfo;
    }

    private static SpannableStringBuilder getApnSettingInfo(Context context,
            ArrayList<NapdefClass> napList, ArrayList<ApplicationClass> apList,
            ArrayList<ProxyClass> pxList) {
        if (napList == null || napList.isEmpty()) {
            MTKlog.e(TAG, "OmacpMessageUtils getApnSettingInfo napList is null or size is 0.");
            return null;
        }

        SpannableStringBuilder apnInfo = new SpannableStringBuilder();
        int napSize = napList.size();
        for (int i = 0; i < napSize; i++) {
            StringBuilder napSettings = new StringBuilder();
            NapdefClass napdefClass = napList.get(i);
            // add Apn name, address, username, password, params to apnInfo
            getAPNBaseParams(context, napdefClass, napSettings, apList);
            // add Apn Proxy and port and MMSC params, apn type, authentication type params to apnInfo
            getAPNProxyParams(context, pxList, napdefClass, napSettings, apList);
            if (!"".equals(napSettings.toString().trim())) {
                if (i > 0) {
                    apnInfo.append("\n");
                }

                addApplicationLabel(context, apnInfo, APN_APPID);
                apnInfo.append(napSettings.toString());
            }
        }
        return apnInfo;
    }

    private static void getAPNBaseParams(Context context, NapdefClass napdefClass,
            StringBuilder napSettings, ArrayList<ApplicationClass> apList) {
        // Name
        if (napdefClass.mName != null) {
            napSettings.append("\n");
            napSettings.append(context.getString(R.string.name_label));
            napSettings.append(napdefClass.mName);
        }
        // APN
        if (napdefClass.mNapaddress != null) {
            napSettings.append("\n");
            napSettings.append(context.getString(R.string.apn_apn_label));
            napSettings.append(napdefClass.mNapaddress);
        }
        // Username
        if (napdefClass.mNapauthinfo.size() != 0
                && napdefClass.mNapauthinfo.get(0).mAuthname != null) {
            napSettings.append("\n");
            napSettings.append(context.getString(R.string.user_name_label));
            napSettings.append(napdefClass.mNapauthinfo.get(0).mAuthname);
        }
        // Password
        if (napdefClass.mNapauthinfo.size() != 0
                && napdefClass.mNapauthinfo.get(0).mAuthsecret != null) {
            napSettings.append("\n");
            napSettings.append(context.getString(R.string.password_label));
            napSettings.append(napdefClass.mNapauthinfo.get(0).mAuthsecret);
        }
    }

    private static class ApnProxyParam {
        String mProxy;

        String mPort;
    }

    private static void getAPNProxyParams(Context context, ArrayList<ProxyClass> pxList,
            NapdefClass napdefClass, StringBuilder napSettings, ArrayList<ApplicationClass> apList) {
        // check ProxyClass params.
        ApnProxyParam apnProxy = new ApnProxyParam();
        int apnType = -1;
        apnType = getProxyClassParams(context, pxList, napdefClass, napSettings, apnProxy, apList);
        String apnTypeString = null;
        // append mms related settings info
        // check whether add mms proxy params
        if (apnType == APNTYPEBROWSER) {
            apnTypeString = "default";
        } else if(apnType == APNTYPEMMS){
            appendMmscProxyParams(context, apList, napSettings, apnProxy);
            apnTypeString = "mms";
        } else if(apnType == APNTYPESUPL){
            apnTypeString = "supl";
        }else{
            apnTypeString = getAPNType(apList);
        }

        // Authentication type
        if (napdefClass.mNapauthinfo.size() != 0
                && napdefClass.mNapauthinfo.get(0).mAuthtype != null) {
            napSettings.append("\n");
            napSettings.append(context.getString(R.string.apn_auth_type_label));
            napSettings.append(napdefClass.mNapauthinfo.get(0).mAuthtype);
        }
        // APN type
        if (apnTypeString != null){
            napSettings.append("\n");
            napSettings.append(context.getString(R.string.apn_type_label));
            napSettings.append(apnTypeString);
        }
    }

    private static void appendMmscProxyParams(Context context, ArrayList<ApplicationClass> apList,
            StringBuilder napSettings, ApnProxyParam apnProxy) {
        String mmsc = null;
        if (apList != null) {
            for (int n = 0; n < apList.size(); n++) {
                if (apList.get(n).mAppid.equalsIgnoreCase(MMS_APPID)
                        || apList.get(n).mAppid.equalsIgnoreCase(MMS_2_APPID)) {
                    if (apList.get(n).mAddr.size() != 0 && apList.get(n).mAddr.get(0) != null) {
                        mmsc = apList.get(n).mAddr.get(0);
                    } else if (apList.get(n).mAppaddr.size() != 0
                            && apList.get(n).mAppaddr.get(0).mAddr != null) {
                        mmsc = apList.get(n).mAppaddr.get(0).mAddr;
                    }
                }
            }
        }
        if (mmsc != null) {
            napSettings.append("\n");
            napSettings.append(context.getString(R.string.mmsc_label));
            napSettings.append(mmsc);
            String proxy = apnProxy.mProxy;
            String port = apnProxy.mPort;
            if (proxy != null && proxy.length() > 0) {
                napSettings.append("\n");
                napSettings.append(context.getString(R.string.apn_mms_proxy_label));
                napSettings.append(proxy);
            }
            if (port != null && port.length() > 0) {
                napSettings.append("\n");
                napSettings.append(context.getString(R.string.apn_mms_port_label));
                napSettings.append(port);
            }
        }
    }

    private static int getProxyClassParams(Context context, ArrayList<ProxyClass> pxList,
            NapdefClass napdefClass, StringBuilder napSettings, ApnProxyParam apnProxy, ArrayList<ApplicationClass> apList) {
        if (pxList == null || pxList.size() == 0) {
            return -1;
        }
        ProxyClass proxyClass = null;
        boolean flag = false;
        int apnType = -1;
        for (int n = 0; n < pxList.size(); n++) {
            proxyClass = pxList.get(n);
            flag = checkPxphysical(proxyClass, napdefClass);
            if (flag) {
                break;
            }
        }
        if(flag){
            for(int appIndex = 0; appIndex < apList.size(); appIndex++ ) {
                ApplicationClass applicationClass = apList.get(appIndex);
                apnType = checkApplicationProxy(proxyClass, applicationClass);
                if (apnType != -1){
                    break;
                }
            }
        }
        if (flag && proxyClass != null) {
            if (proxyClass.mPxphysical.size() != 0) {
                if (proxyClass.mPxphysical.get(0).mPxaddr != null && apnType != APNTYPEMMS) {
                    napSettings.append("\n");
                    napSettings.append(context.getString(R.string.proxy_label));
                    napSettings.append(proxyClass.mPxphysical.get(0).mPxaddr);
                }
                apnProxy.mProxy = proxyClass.mPxphysical.get(0).mPxaddr;

                if (proxyClass.mPxphysical.get(0).mPort.size() != 0) {
                    if (proxyClass.mPxphysical.get(0).mPort.get(0).mPortnbr != null && apnType != APNTYPEMMS) {
                        napSettings.append("\n");
                        napSettings.append(context.getString(R.string.port_number_label));
                        napSettings.append(proxyClass.mPxphysical.get(0).mPort.get(0).mPortnbr);
                    }
                    apnProxy.mPort = proxyClass.mPxphysical.get(0).mPort.get(0).mPortnbr;
                }
            }
        }
        return apnType;
    }

    private static boolean checkPxphysical(ProxyClass proxyClass, NapdefClass napdefClass) {
        if (proxyClass.mPxphysical != null && proxyClass.mPxphysical.size() != 0) {
            ArrayList<String> toNapIdList = proxyClass.mPxphysical.get(0).mToNapid;
            if (toNapIdList.size() == 0) {
                return false;
            }
            for (int m = 0; m < toNapIdList.size(); m++) {
                String toNapId = toNapIdList.get(m);
                if (napdefClass.mNapid.equalsIgnoreCase(toNapId)) {
                    return true;
                }
            }
        }
        return false;
    }

    private static int checkApplicationProxy(ProxyClass proxyClass, ApplicationClass applicationClass){
        if (proxyClass.mPxphysical != null && proxyClass.mPxphysical.size() != 0
                && applicationClass != null) {
            if (applicationClass.mToProxy == null || applicationClass.mToProxy.size() == 0) {
                return -1;
            }
            String toProxy = applicationClass.mToProxy.get(0);
            String toProxyId = proxyClass.mProxyId;
            String appId = applicationClass.mAppid;
            if (toProxy.equals(toProxyId) && appId != null){
                if ((appId.equals(MMS_APPID) || appId.equals(MMS_2_APPID))) {
                    return APNTYPEMMS;
                } else if (appId.equals(BROWSER_APPID)){
                    return APNTYPEBROWSER;
                } else if (appId.equals(SUPL_APPID)) {
                    return APNTYPESUPL;
                }
            }
        }
        return -1;
    }

    public static String getAPNType(ArrayList<ApplicationClass> apList) {
        String apnType = null;
        if (apList != null) {
            for (int n = 0; n < apList.size(); n++) {
                if (apList.get(n).mAppid.equalsIgnoreCase(BROWSER_APPID)) {
                    if (apnType != null) {
                        apnType += ",default";
                    } else {
                        apnType = "default";
                    }
                } else if ((apList.get(n).mAppid.equalsIgnoreCase(MMS_APPID) || apList.get(n).mAppid
                          .equalsIgnoreCase(MMS_2_APPID))
                        && (apnType == null || !apnType.contains("mms"))) {
                    if (apnType != null) {
                        apnType += ",mms";
                    } else {
                        apnType = "mms";
                    }
                } else if (apList.get(n).mAppid.equalsIgnoreCase(SUPL_APPID)) {
                    if (apnType != null) {
                        apnType += ",supl";
                    } else {
                        apnType = "supl";
                    }
                }
            }
        }
        return apnType;
    }

    private static void resetEmailStaticSetting() {
        sEmailAccountName = null;
        sInboundEmailSetting = null;
        sOutboundEmailSetting = null;
        sEmailInboundType = null;
    }

}
