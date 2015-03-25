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
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;


public class VoiceCommon extends VoiceCommandBusiness {

    public VoiceCommon(IMessageDispatcher dispatcher,
            ConfigurationManager cfgMgr, Handler handler) {
        super(dispatcher, cfgMgr, handler);
        // TODO Auto-generated constructor stub
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_COMMON_KEYWORD:
            errorid = sendProcessKeyWord(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_COMMON_COMMANDPATH:
            errorid = sendProcessCommandPath(message);
            break;
        case VoiceCommandListener.ACTION_VOICE_COMMON_PROCSTATE:
            errorid = sendProcessState(message);
            break;
        default:
            break;
        }
        return errorid;
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        // TODO Auto-generated method stub
        return 0;
    }

    /*
     * Send the the keyword message to the apps process via IPC
     * 
     * @param message
     * 
     * @return
     */
    private int sendProcessKeyWord(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        String[] keyword = mCfgMgr.getKeyWord(message.mPkgName);

        message.mExtraData = DataPackage
                .packageResultInfo(
                        VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                        keyword, null);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /*
     * Send the the command path message to the apps process via IPC
     *
     * @param message
     *
     * @return
     */
    private int sendProcessCommandPath(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        String path = mCfgMgr.getCommandPath(message.mPkgName);
        message.mExtraData = DataPackage
                    .packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                            path, null);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /*
     * Send the process state to the apps process viar IPC
     * 
     * @param message
     * 
     * @return
     */
    private int sendProcessState(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        boolean isEnalbe = mCfgMgr.isProcessEnable(message.mPkgName);
        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, isEnalbe,
                null);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }
}
