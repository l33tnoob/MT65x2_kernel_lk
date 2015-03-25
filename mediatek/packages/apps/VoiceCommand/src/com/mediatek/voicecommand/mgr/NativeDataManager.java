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
package com.mediatek.voicecommand.mgr;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.adapter.JNICommandAdapter;
import com.mediatek.voicecommand.adapter.JNITestAdapter;
import com.mediatek.voicecommand.business.VoiceCommandBusiness;
import com.mediatek.voicecommand.business.VoiceContacts;
import com.mediatek.voicecommand.business.VoiceRecognize;
import com.mediatek.voicecommand.business.VoiceServiceInternal;
import com.mediatek.voicecommand.business.VoiceTraining;
import com.mediatek.voicecommand.business.VoiceUI;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class NativeDataManager extends VoiceDataManager implements
        IMessageDispatcher {

    private IVoiceAdapter mjniAdapter;
    private IMessageDispatcher mUpDispatcher;
    private VoiceCommandBusiness mVoiceUI;
    private VoiceCommandBusiness mVoiceTraining;
    private VoiceCommandBusiness mVoiceRecognize;
    // Used to deal with the logic happened in the Service itself
    private VoiceCommandBusiness mVoiceServiceInternal;
    private VoiceCommandBusiness mVoiceContacts;

    public NativeDataManager(VoiceCommandManagerStub service) {
        super(service);

        mjniAdapter = new JNICommandAdapter(this);
//        mjniAdapter = new JNITestAdapter(this,service.mConfigManager);
        mVoiceUI = new VoiceUI(this, service.mConfigManager, mHandler,
                mjniAdapter);
        mVoiceTraining = new VoiceTraining(this, service.mConfigManager,
                mHandler, mjniAdapter);
        mVoiceRecognize = new VoiceRecognize(this, service.mConfigManager,
                mHandler, mjniAdapter);
        mVoiceContacts = new VoiceContacts(this, service.mConfigManager,
                mHandler, mjniAdapter, service.mContext);
        mVoiceServiceInternal = new VoiceServiceInternal(this,
                service.mConfigManager, mHandler, mjniAdapter);

        // TODO Auto-generated constructor stub
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_HEADSET_PLUG);
        service.mContext.registerReceiver(mHeadsetPlugReceiver, filter);
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
                mVoiceUI.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
                mVoiceTraining.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
                mVoiceRecognize.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
                mVoiceContacts.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;
            case VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME:
                handleAsyncContactsMsg((String[])msg.obj);
                break;
            case VoiceCommandBusiness.ACTION_MAIN_VOICE_SERVICE:
            case VoiceCommandBusiness.ACTION_MAIN_VOICE_BROADCAST:
                mVoiceServiceInternal
                        .handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;
            default:
                // do nothing
                break;
            }
        }
    };

    /*
     * Send all contacts name to next dispatcher
     *
     * @param contactsNameList contacts name list
     */
    public void handleAsyncContactsMsg(String[] contactsNames) {
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME;
        Bundle bundle = DataPackage.packageSendInfo(contactsNames, null);
        message.mExtraData = bundle;
        mVoiceContacts.handleAsyncVoiceMessage(message);
    }

    /*
     * Send message to native via JNICommandAdapter
     */
    @Override
    public int dispatchMessageDown(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mMainAction) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            errorid = mVoiceUI.handleSyncVoiceMessage(message);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            errorid = mVoiceTraining.handleSyncVoiceMessage(message);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
            errorid = mVoiceRecognize.handleSyncVoiceMessage(message);
            break;
        case VoiceCommandBusiness.ACTION_MAIN_VOICE_SERVICE:
            errorid = mVoiceServiceInternal.handleSyncVoiceMessage(message);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            errorid = mVoiceContacts.handleSyncVoiceMessage(message);
            break;
        default:
            // do nothing here
            break;
        }
        return errorid;
    }

    @Override
    public int dispatchMessageUp(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mMainAction) {
        case VoiceCommandBusiness.ACTION_MAIN_VOICE_SERVICE:
            if (message.mSubAction == VoiceCommandBusiness.ACTION_VOICE_SERVICE_SELFEXIT) {
                mService.mContext.unregisterReceiver(mHeadsetPlugReceiver);
                mVoiceContacts.handleDataRelease();
            }
            errorid = mVoiceServiceInternal.handleSyncVoiceMessage(message);
            break;
        default:
            errorid = mUpDispatcher.dispatchMessageUp(message);
            break;
        }
        return errorid;
    }

    @Override
    public void setDownDispatcher(IMessageDispatcher dispatcher) {
        // TODO Auto-generated method stub
        // Don't need next dispatcher because this dispatcher send message to
        // native directly
    }

    @Override
    public void setUpDispatcher(IMessageDispatcher dispatcher) {
        // TODO Auto-generated method stub
        mUpDispatcher = dispatcher;
    }

    private BroadcastReceiver mHeadsetPlugReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
                VoiceMessage msg = new VoiceMessage();
                msg.mMainAction = VoiceCommandBusiness.ACTION_MAIN_VOICE_BROADCAST;
                 if(intent.getIntExtra("state", 0) == 0){
                     msg.mSubAction = VoiceCommandBusiness.ACTION_VOICE_BROADCAST_HEADSETPLUGOUT;
                 }else {
                     msg.mSubAction = VoiceCommandBusiness.ACTION_VOICE_BROADCAST_HEADSETPLUGIN;
                 }
                 mVoiceServiceInternal.handleSyncVoiceMessage(msg);
        }
    };
}
