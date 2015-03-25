/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.voicecommand.business;

import android.content.Context;
import android.database.ContentObserver;
import android.os.Handler;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class VoiceContacts extends VoiceCommandBusiness {

    private IVoiceAdapter mJniAdapter;
    private Context mContext;
    private ContentObserver mVoiceContactsObserver;
    private IMessageDispatcher mDispatcher;

    public VoiceContacts(IMessageDispatcher dispatcher,
            ConfigurationManager cfgMgr, Handler handler,
            IVoiceAdapter adapter, Context context) {
        super(dispatcher, cfgMgr, handler);
        mDispatcher = dispatcher;
        mJniAdapter = adapter;
        // TODO Auto-generated constructor stub
        mContext = context;
        mVoiceContactsObserver = new VoiceContactsObserver(mContext, handler);
        if (FeatureOption.MTK_VOICE_CONTACT_SEARCH_SUPPORT) {
            mContext.getContentResolver().registerContentObserver(
                    VoiceContactsObserver.CONTACTS_URI, true,
                    mVoiceContactsObserver);
        }
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_START:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_STOP:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ENABLE:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_DISABLE:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_INTENSITY:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SELECTED:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SEARCHCNT:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ORIENTATION:
            errorid = sendMessageToHandler(message);
            break;
        default:
            break;
        }
        return errorid;
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (!FeatureOption.MTK_VOICE_CONTACT_SEARCH_SUPPORT) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_VOICEINVALID;
            Log.i(VoiceCommandManagerStub.TAG,
                    "Voice Contacts feature is off, can not handle message");
            sendMessageToApps(message, errorid);
            return errorid;
        }

        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_START:
            errorid = handleContactsStart(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_STOP:
            errorid = handleContactsStop(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ENABLE:
            errorid = handleContactsEnable(message, true);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_DISABLE:
            errorid = handleContactsEnable(message, false);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_INTENSITY:
            handleContactsIntensity(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SELECTED:
            handleContactsSelected(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME:
            handleContactsName(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SEARCHCNT:
            handleContactsSearchCnt(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ORIENTATION:
            handleContactsOrientation(message);
            break;
        default:
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
            break;
        }

        return errorid;
    }

    private int handleContactsStart(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCfgMgr.isProcessEnable(message.mPkgName)) {
            if (message.mExtraData == null) {
                errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SENDINVALID;
            } else {
                int screenOrientation = message.mExtraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
                errorid = mJniAdapter.startVoiceContacts(message.mPkgName,
                        message.pid, screenOrientation);
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_PROCESSOFF;
        }

        sendMessageToApps(message, errorid);
        return errorid;
    }

    private int handleContactsStop(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCfgMgr.isProcessEnable(message.mPkgName)) {
            errorid = mJniAdapter.stopVoiceContacts(message.mPkgName,
                    message.pid);
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_PROCESSOFF;
        }
        sendMessageToApps(message, errorid);
        return errorid;
    }

    private int handleContactsEnable(VoiceMessage message, boolean isEnable) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Boolean isCurEnable = mCfgMgr.isProcessEnable(message.mPkgName);
        if (!(isCurEnable & isEnable)) {
            errorid = mCfgMgr.updateFeatureEnable(message.mPkgName, isEnable) ? VoiceCommandListener.VOICE_NO_ERROR
                    : VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
            if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
                sendMessageToApps(message, errorid);
            }
        }

        if (isEnable) {
            handleContactsStart(message);
        } else {
            handleContactsStop(message);
        }

        return errorid;
    }

    private int handleContactsIntensity(VoiceMessage message) {
        int intensity = mJniAdapter.getNativeIntensity();

        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, intensity, 0);
        mDispatcher.dispatchMessageUp(message);
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    private int handleContactsSelected(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SENDINVALID;
        } else {
            String selectedName = message.mExtraData
                    .getString(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            errorid = mJniAdapter.sendContactsSelected(selectedName);
        }

        sendMessageToApps(message, errorid);
        return errorid;
    }

    /*
     * Send all contacts name to next dispatcher
     *
     * @param contactsNameList contacts name list
     */
    private int handleContactsName(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SENDINVALID;
        } else {
            String modelpath = mCfgMgr.getContactsModelFile();
            String contactsdbPath = mCfgMgr.getContactsdbFilePath();
            if (modelpath == null || contactsdbPath == null) {
                Log.i(VoiceCommandManagerStub.TAG,
                        "handleContactsName error modelpath=" + modelpath
                                + ", contactsdbPath=" + contactsdbPath);
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            } else {
                String[] allContactsNames = message.mExtraData
                        .getStringArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
                errorid = mJniAdapter.sendContactsName(modelpath,
                        contactsdbPath, allContactsNames);
            }
        }
        return errorid;
    }

    private int handleContactsSearchCnt(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SENDINVALID;
        } else {
            int searchCnt = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            errorid = mJniAdapter.sendContactsSearchCnt(searchCnt);
        }

        sendMessageToApps(message, errorid);
        return errorid;
    }

    private int handleContactsOrientation(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SENDINVALID;
        } else {
            int orientation = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            errorid = mJniAdapter.sendContactsOrientation(orientation);
        }

        sendMessageToApps(message, errorid);
        return errorid;
    }

    /*
     * release all variables when service is destroy.
     */
    public void handleDataRelease() {
        if (FeatureOption.MTK_VOICE_CONTACT_SEARCH_SUPPORT) {
            mContext.getContentResolver().unregisterContentObserver(
                    mVoiceContactsObserver);
        }
    }
}
