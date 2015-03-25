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

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;

public abstract class VoiceCommandBusiness {

    
    //Used to deal with the event only inside the service
    public static final int ACTION_MAIN_VOICE_SERVICE = 10000;

    //Used to deal with the event from death notification
    public static final int ACTION_VOICE_SERVICE_PROCESSEXIT = 1;

    //Used to deal with the event when service need to be destroyed
    public static final int ACTION_VOICE_SERVICE_SELFEXIT = ACTION_VOICE_SERVICE_PROCESSEXIT + 1;

    //Used to deal with the event Broadcast
    public static final int ACTION_MAIN_VOICE_BROADCAST = 10001;
    //Used to deal with the event when HeadSet plug in
    public static final int ACTION_VOICE_BROADCAST_HEADSETPLUGIN = 1;

    //Used to deal with the event when HeadSet plug in
    public static final int ACTION_VOICE_BROADCAST_HEADSETPLUGOUT = ACTION_VOICE_BROADCAST_HEADSETPLUGIN + 1;

    IMessageDispatcher mDispatcher;
    ConfigurationManager mCfgMgr;
    Handler mHandler;

    public VoiceCommandBusiness(IMessageDispatcher dispatcher,
            ConfigurationManager cfgMgr, Handler handler) {
        mDispatcher = dispatcher;
        mCfgMgr = cfgMgr;
        mHandler = handler;
    }

    /*
     * Some messages need to be handled in main thread , so we need to send the
     * messages from binder thread to main thread
     * 
     * @param message
     * 
     * @return
     */
    public int sendMessageToHandler(VoiceMessage message) {
        Message msg = mHandler.obtainMessage();
        msg.what = message.mMainAction;
        msg.obj = message;
        // send message to service thread
        return mHandler.sendMessage(msg) ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
    }

    /*
     * Encapsulate the result message and send back to apps
     * 
     * @param message
     * 
     * @param errorid
     */
    protected void sendMessageToApps(VoiceMessage message, int errorid) {
        Bundle bundle = null;

        if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
            bundle = DataPackage.packageSuccessResult();
        } else {
            bundle = DataPackage.packageErrorResult(errorid);
        }

        message.mExtraData = bundle;
        // Notify the apps about the execution result
        mDispatcher.dispatchMessageUp(message);
    }

    /*
     * Handle the voice business message and send back the result in the same
     * time in the binder thread
     * 
     * @param message
     * 
     * @return
     */
    public abstract int handleSyncVoiceMessage(VoiceMessage message);

    /*
     * Handle the voice business message and send back the result in the Service
     * handler of main thread
     * 
     * @param message
     * 
     * @return
     */
    public abstract int handleAsyncVoiceMessage(VoiceMessage message);

    /*
     * When Service is need to be destroyed , we need to stop the native
     * business and release the memory
     */
    public void handleDataRelease() {

    }

}
