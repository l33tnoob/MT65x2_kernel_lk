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

package com.mediatek.imsp;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import com.hissage.api.NmsIpMessageApi;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.FeatureId;
import com.mediatek.mms.ipmessage.ServiceManager;
import com.mediatek.xlog.Xlog;

import java.lang.ref.SoftReference;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Provide service management related interface
 */
public class ServiceManagerExt extends ServiceManager {
    private static final String TAG = "imsp/ServiceManagerExt";

    private NotificationReceiver mNotificationReceiver = null;
    // private SoftReference<SNmsSimInfo> mSimInfoCache = new
    // SoftReference<SNmsSimInfo>(null);
    private static Map<Integer, SoftReference<SNmsSimInfo>> mNmsSimInfoCacheMap = new LinkedHashMap<Integer, SoftReference<SNmsSimInfo>>();

    public ServiceManagerExt(Context context) {
        super(context);
    }

    public int getIpMessageServiceId() {
        Xlog.d(TAG, "getIpMessageServiceId");
        return IpMessageConsts.IpMessageServiceId.ISMS_SERVICE;
    }

    public void startIpService() {
        Xlog.d(TAG, "startIpService, context=" + mContext);
        NmsIpMessageApi.getInstance(mContext).nmsStartIpService(mContext);

        if (mNotificationReceiver == null) {
            mNotificationReceiver = NotificationReceiver.getInstance();
            IntentFilter filter = new IntentFilter();
            filter.addAction(NmsIpMessageConsts.NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION);
            filter.addAction(NmsIpMessageConsts.NMS_INTENT_SERVICE_READY);
            filter.addAction(NmsIpMessageConsts.NmsImStatus.NMS_IM_STATUS_ACTION);
            filter.addAction(NmsIpMessageConsts.NmsIpMessageStatus.NMS_MESSAGE_STATUS_ACTION);
            filter.addAction(NmsIpMessageConsts.NmsIpMessageStatus.NMS_READEDBURN_TIME_ACTION);
            filter.addAction(NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION);
            filter.addAction(NmsIpMessageConsts.NmsSaveHistory.NMS_ACTION_DOWNLOAD_HISTORY);
            filter.addAction(NmsIpMessageConsts.NmsUpdateGroupAction.NMS_UPDATE_GROUP);
            filter.addAction(NmsIpMessageConsts.NmsUpdateSystemContactAction.NMS_UPDATE_CONTACT);
            filter.addAction(NmsIpMessageConsts.NmsSimInfoChanged.NMS_SIM_INFO_ACTION);
            filter.addAction(NmsConsts.NmsIntentStrId.NMS_MMS_RESTART_ACTION);
            filter.addAction(NmsIntentStrId.NMS_REG_STATUS);
            filter.addAction(NmsIpMessageConsts.NmsStoreStatus.NMS_STORE_STATUS_ACTION);
            mContext.registerReceiver(mNotificationReceiver, filter);
        }
    }

    public boolean serviceIsReady() {
        Xlog.d(TAG, "serviceIsReady");
        return NmsIpMessageApi.getInstance(mContext).nmsServiceIsReady();
    }

    public static void refreshNmsSimInfo(Context context, int simId) {
        Xlog.d(TAG, "refreshNmsSimInfo, sim Id = " + simId);
        SNmsSimInfo nmsSimInfo = null;
        SoftReference<SNmsSimInfo> nmsSimInfoCache = null;
        synchronized (mNmsSimInfoCacheMap) {
            if (mNmsSimInfoCacheMap.containsKey(simId)) {
                mNmsSimInfoCacheMap.remove(simId);
            }
            try {
                nmsSimInfo = NmsIpMessageApi.getInstance(context).nmsGetSimInfoViaSimId(simId);
                if (nmsSimInfo != null) {
                    nmsSimInfoCache = new SoftReference<SNmsSimInfo>(nmsSimInfo);
                    mNmsSimInfoCacheMap.put(simId, nmsSimInfoCache);
                }
            } catch (OutOfMemoryError ex) {
                // fall through and return a null bitmap. The callers can handle
                // a null
            }
        }
    }

    private SNmsSimInfo getNmsSimInfo(int simId) {
        SNmsSimInfo nmsSimInfo = null;
        SoftReference<SNmsSimInfo> nmsSimInfoCache = mNmsSimInfoCacheMap.get(simId);
        if (nmsSimInfoCache == null || nmsSimInfoCache.get() == null) {
            Xlog.d(TAG, "return nmsSimInfo from isms service");
            synchronized (mNmsSimInfoCacheMap) {
                try {
                    nmsSimInfo = NmsIpMessageApi.getInstance(mContext).nmsGetSimInfoViaSimId(simId);
                    if (nmsSimInfo != null) {
                        nmsSimInfoCache = new SoftReference<SNmsSimInfo>(nmsSimInfo);
                        mNmsSimInfoCacheMap.put(simId, nmsSimInfoCache);
                    }
                } catch (OutOfMemoryError ex) {
                    // fall through and return a null bitmap. The callers can
                    // handle a null
                }
            }
        } else {
            Xlog.d(TAG, "return nmsSimInfo from cache");
            nmsSimInfo = nmsSimInfoCache.get();
        }

        return nmsSimInfo;
    }

    public boolean isActivated() {
        return isActivated(NmsConsts.SINGLE_CARD_SIM_ID);
    }

    public boolean isActivated(int simId) {
        Xlog.d(TAG, "isActivated, sim id = " + simId);
        SNmsSimInfo simInfo = getNmsSimInfo(simId);
        if (null == simInfo) {
            return false;
        }
        return simInfo.status < SNmsSimInfo.NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED ? false
                : true;
    }

    public boolean isEnabled() {
        return isEnabled(NmsConsts.SINGLE_CARD_SIM_ID);
    }

    public boolean isEnabled(int simId) {
        Xlog.d(TAG, "isEnabled, sim id = " + simId);
        SNmsSimInfo simInfo = getNmsSimInfo(simId);
        if (null == simInfo) {
            return false;
        }
        return simInfo.status == SNmsSimInfo.NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED ? true
                : false;
    }

    public int getActivationStatus() {
        return getActivationStatus(NmsConsts.SINGLE_CARD_SIM_ID);
    }

    public int getActivationStatus(int simId) {
        Xlog.d(TAG, "getActivationStatus, sim id = " + simId);
        return NmsIpMessageApi.getInstance(mContext).nmsGetActivationStatus(simId);
    }

    public void enableIpService() {
        enableIpService(NmsConsts.SINGLE_CARD_SIM_ID);
    }

    public void enableIpService(int simId) {
        Xlog.d(TAG, "enableIpService, sim id = " + simId);
        NmsIpMessageApi.getInstance(mContext).nmsEnableIpService(simId);
    }

    public void disableIpService() {
        disableIpService(NmsConsts.SINGLE_CARD_SIM_ID);
    }

    public void disableIpService(int simId) {
        Xlog.d(TAG, "disableIpService, sim id = " + simId);
        NmsIpMessageApi.getInstance(mContext).nmsDisableIpService(simId);
    }

    public boolean isFeatureSupported(int featureId) {
        switch (featureId) {
        case FeatureId.CHAT_SETTINGS:
        case FeatureId.APP_SETTINGS:
        case FeatureId.ACTIVITION:
        case FeatureId.ACTIVITION_WIZARD:
        case FeatureId.ALL_LOCATION: // list all location messages in
                                     // activity.
        case FeatureId.ALL_MEDIA: // list all media messages in activity.
        case FeatureId.MEDIA_DETAIL: // displaying media detail info.
        case FeatureId.GROUP_MESSAGE:
        case FeatureId.CONTACT_SELECTION:
        case FeatureId.SKETCH:
        case FeatureId.LOCATION:
        case FeatureId.TERM:
        case FeatureId.SAVE_CHAT_HISTORY:
        case FeatureId.SAVE_ALL_HISTORY:
        case FeatureId.SHARE_CHAT_HISTORY:
        case FeatureId.SHARE_ALL_HISTORY:
        case FeatureId.IPMESSAGE_SERVICE_CENTER:
        case FeatureId.IPMESSAGE_ACTIVATE_PROMPT:
        case FeatureId.PRIVATE_MESSAGE:
        case FeatureId.PARSE_EMO_WITHOUT_ACTIVATE:
            return true;
        case FeatureId.READEDBURN:
            if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
                return true;
            } else {
                return false;
            }
        default:
            return false;
        }
    }

    public void checkDefaultSmsAppChanged() {
        NmsIpMessageApi.getInstance(mContext).nmsCheckDefaultSmsAppChanged();
    }
}
