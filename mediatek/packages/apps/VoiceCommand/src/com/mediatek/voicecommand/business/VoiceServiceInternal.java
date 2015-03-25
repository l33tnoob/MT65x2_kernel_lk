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

import android.os.Handler;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;

public class VoiceServiceInternal extends VoiceCommandBusiness {

    private IVoiceAdapter mJniAdapter;

    public VoiceServiceInternal(IMessageDispatcher dispatcher,
            ConfigurationManager cfgMgr, Handler handler, IVoiceAdapter adapter) {
        super(dispatcher, cfgMgr, handler);
        mJniAdapter = adapter;
        // TODO Auto-generated constructor stub
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mMainAction) {
        case ACTION_MAIN_VOICE_SERVICE:
            if (message.mSubAction == ACTION_VOICE_SERVICE_PROCESSEXIT) {
                handleProcessExit(message);
            }
            break;

        case ACTION_MAIN_VOICE_BROADCAST:
            handleHeadsetPlugEvent(message);
            break;

        default:
            break;
        }

        return errorid;
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mMainAction) {
        case ACTION_MAIN_VOICE_SERVICE:
            if (message.mSubAction == ACTION_VOICE_SERVICE_PROCESSEXIT) {
                sendMessageToHandler(message);
            } else if (message.mSubAction == ACTION_VOICE_SERVICE_SELFEXIT) {
                handleDataRelease();
            }
            break;

        case ACTION_MAIN_VOICE_BROADCAST:
            if (mHandler.hasMessages(ACTION_MAIN_VOICE_BROADCAST)) {
                mHandler.removeMessages(ACTION_MAIN_VOICE_BROADCAST);
            }
            sendMessageToHandler(message);
            break;

        default:
            break;
        }

        return errorid;
    }

    /*
     * Notify JNI to stop the voice command business because the app process was
     * died
     *
     * @param message
     */
    private void handleProcessExit(VoiceMessage message) {
        mJniAdapter.stopCurMode(message.mPkgName, message.pid);
    }

    /*
     * Notify JNI to handle HeadsetPlug Event
     *
     * @param message
     */
    private void handleHeadsetPlugEvent(VoiceMessage message) {
        if (message.mSubAction == ACTION_VOICE_BROADCAST_HEADSETPLUGIN) {
            mJniAdapter.setCurHeadsetMode(true);
        } else {
            mJniAdapter.setCurHeadsetMode(false);
        }
    }

    @Override
    public void handleDataRelease() {
        mJniAdapter.release();
    }

}
