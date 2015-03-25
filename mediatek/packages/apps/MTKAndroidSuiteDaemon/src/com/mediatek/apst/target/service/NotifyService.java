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

package com.mediatek.apst.target.service;

import android.app.IntentService;
import android.content.Intent;

import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.event.Event;
import com.mediatek.apst.target.event.EventDispatcher;
import com.mediatek.apst.target.event.ISimStateListener;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.command.sysinfo.SimDetailInfo;
import com.mediatek.apst.util.entity.message.Message;

public class NotifyService extends IntentService {

    private Boolean mSimOK;
    private Boolean mSim1OK;
    private Boolean mSim2OK;
    private Boolean mSim3OK;
    private Boolean mSim4OK;
    public static final String SIM_ID = "simid";

    /**
     * Constructor.
     */
    public NotifyService() {
        super("NotifyService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Debugger.logI(new Object[] { intent },
                "NotifyService --> onHandleIntent");
        if (intent != null) {
            onSimStatusChange(intent);
        } else {
            Debugger.logE(new Object[] { intent }, "intent is null");
        }
        Debugger.logI(new Object[] { intent },
                "NotifyService --> onHandleIntent End");
    }

    /**
     * @param intent
     */
    private void onSimStatusChange(Intent intent) {
        String strAction = intent.getStringExtra("Action");
        if (null != strAction) {
            Debugger.logI(new Object[] { intent }, "strAction:" + strAction);
        }
        boolean isSimInfoChanged = false;
        boolean isSim1InfoChanged = false;
        boolean isSim2InfoChanged = false;
        boolean isSim3InfoChanged = false;
        boolean isSim4InfoChanged = false;
        if (Intent.SIM_SETTINGS_INFO_CHANGED.equals(strAction)) {
            if (Config.MTK_GEMINI_SUPPORT) {
                long simId = intent.getLongExtra(SIM_ID, -1);
                SimDetailInfo simInfo = Global.getSimInfoById((int) simId);
                if (simInfo != null) {
                    int slotId = simInfo.getSlotId();
                    if (0 == slotId) {
                        isSim1InfoChanged = true;
                    } else if (1 == slotId) {
                        isSim2InfoChanged = true;
                    } else if (2 == slotId) {
                        isSim3InfoChanged = true;
                    } else if (3 == slotId) {
                        isSim4InfoChanged = true;
                    }
                }

            } else {
                isSimInfoChanged = true;
            }
        }
        if (Config.MTK_GEMINI_SUPPORT) {
            int sim1State = SystemInfoProxy.getSimState(Message.SIM1_ID);
            int sim2State = SystemInfoProxy.getSimState(Message.SIM2_ID);
            int sim3State = SystemInfoProxy.getSimState(Message.SIM3_ID);
            int sim4State = SystemInfoProxy.getSimState(Message.SIM4_ID);
            
            boolean sim1OK = SystemInfoProxy.isSimAccessible(sim1State);
            boolean sim2OK = SystemInfoProxy.isSimAccessible(sim2State);
            boolean sim3OK = SystemInfoProxy.isSimAccessible(sim3State);
            boolean sim4OK = SystemInfoProxy.isSimAccessible(sim4State);
            
            SimDetailInfo sim1Info = Global.getSimInfoBySlot(Message.SIM1_ID);
            SimDetailInfo sim2Info = Global.getSimInfoBySlot(Message.SIM2_ID);
            SimDetailInfo sim3Info = Global.getSimInfoBySlot(Message.SIM3_ID);
            SimDetailInfo sim4Info = Global.getSimInfoBySlot(Message.SIM4_ID);
            
            if (null == mSim1OK || mSim1OK != sim1OK
                    || isSim1InfoChanged) {
                EventDispatcher.dispatchSimStateChangedEvent(new Event().put(
                        ISimStateListener.STATE, sim1State).put(
                        ISimStateListener.SIM_ID, Message.SIM1_ID).put(
                        ISimStateListener.SIM_INFO, sim1Info).put(
                        ISimStateListener.SIM_INFO_FLAG, isSim1InfoChanged));
            }
            if (null == mSim2OK || mSim2OK != sim2OK
                    || isSim2InfoChanged) {
                EventDispatcher.dispatchSimStateChangedEvent(new Event().put(
                        ISimStateListener.STATE, sim2State).put(
                        ISimStateListener.SIM_ID, Message.SIM2_ID).put(
                        ISimStateListener.SIM_INFO, sim2Info).put(
                        ISimStateListener.SIM_INFO_FLAG, isSim2InfoChanged));
            }
            if (null == mSim3OK || mSim3OK != sim3OK
                    || isSim1InfoChanged) {
                EventDispatcher.dispatchSimStateChangedEvent(new Event().put(
                        ISimStateListener.STATE, sim3State).put(
                        ISimStateListener.SIM_ID, Message.SIM3_ID).put(
                        ISimStateListener.SIM_INFO, sim3Info).put(
                        ISimStateListener.SIM_INFO_FLAG, isSim3InfoChanged));
            }
            if (null == mSim4OK || mSim4OK != sim4OK
                    || isSim4InfoChanged) {
                EventDispatcher.dispatchSimStateChangedEvent(new Event().put(
                        ISimStateListener.STATE, sim4State).put(
                        ISimStateListener.SIM_ID, Message.SIM4_ID).put(
                        ISimStateListener.SIM_INFO, sim4Info).put(
                        ISimStateListener.SIM_INFO_FLAG, isSim4InfoChanged));
            }
            mSim1OK = sim1OK;
            mSim2OK = sim2OK;
            mSim3OK = sim3OK;
            mSim4OK = sim4OK;
        } else {
        	int simState = SystemInfoProxy.getSimState(Message.SIM_ID);
                        
            boolean simOK = SystemInfoProxy.isSimAccessible(simState);
                        
            SimDetailInfo simInfo = Global.getSimInfoBySlot(Message.SIM_ID);

            if (null == mSimOK || mSimOK != simOK
                    || isSimInfoChanged) {
                EventDispatcher.dispatchSimStateChangedEvent(new Event().put(
                        ISimStateListener.STATE, simState).put(
                        ISimStateListener.SIM_ID, Message.SIM_ID).put(
                        ISimStateListener.SIM_INFO, simInfo).put(
                        ISimStateListener.SIM_INFO_FLAG, isSimInfoChanged));
            }
            mSimOK = simOK;
        }

    }
}
