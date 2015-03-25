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
package com.mediatek.voicecommand.service;

import android.content.Context;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log; //import com.android.server.Watchdog;

import com.mediatek.common.voicecommand.IVoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandManagerService;
import com.mediatek.voicecommand.business.VoiceCommandBusiness;
import com.mediatek.voicecommand.mgr.AppDataManager;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.NativeDataManager;
import com.mediatek.voicecommand.mgr.ServiceDataManager;
import com.mediatek.voicecommand.mgr.VoiceMessage;

public final class VoiceCommandManagerStub extends
        IVoiceCommandManagerService.Stub implements IVoiceCommandManagerService { // ,
                                                                                  // Watchdog.Monitor
                                                                                  // {

    public static final String TAG = "VCmdMgrService";
    static final boolean DBG = false;

    public final Context mContext;
    AppDataManager mAppDataManager;
    ServiceDataManager mServiceDataManager;
    NativeDataManager mNativeDataManager;

    public ConfigurationManager mConfigManager;

    // VoiceDataManager dataManager

    public VoiceCommandManagerStub(Context context) {

        super();
        Log.i(TAG, "VoiceCommandManagerService Constructor start !"
                + context.getPackageName());
        mContext = context;

        mConfigManager = ConfigurationManager.getInstance(context);

        mAppDataManager = new AppDataManager(this);
        mServiceDataManager = new ServiceDataManager(this);
        mNativeDataManager = new NativeDataManager(this);
        // set the order of dispatcher;
        mAppDataManager.setDownDispatcher(mServiceDataManager);
        mServiceDataManager.setUpDispatcher(mAppDataManager);
        mServiceDataManager.setDownDispatcher(mNativeDataManager);
        mNativeDataManager.setUpDispatcher(mServiceDataManager);

        // Watchdog.getInstance().addMonitor(this);
        Log.i(TAG, "VoiceCommandManagerService Constructor End !");

    }

    @Override
    public int registerListener(String pkgName, IVoiceCommandListener listener)
            throws RemoteException {
        // TODO Auto-generated method stub

        Log.i(TAG, pkgName + " register listener " + listener);

        return mAppDataManager.registerListener(pkgName,
                Binder.getCallingUid(), Binder.getCallingPid(), listener);

    }

    @Override
    public int unregisterListener(String pkgName, IVoiceCommandListener listener)
            throws RemoteException {
        Log.i(TAG, pkgName + " unregister listener " + listener);
        // TODO Auto-generated method stub
        return mAppDataManager.unRegisterListener(pkgName, Binder
                .getCallingUid(), Binder.getCallingPid(), listener, false);
    }

    @Override
    public IBinder asBinder() {
        // TODO Auto-generated method stub
        return this;
    }

    @Override
    public int sendCommand(String pkgName, int mainAction, int subAction,
            Bundle extraData) throws RemoteException {
        // TODO Auto-generated method stub
        Log.i(TAG, pkgName + " sendCommand " + "mainAction=" + mainAction
                + " subAction=" + subAction);
        VoiceMessage message = new VoiceMessage();
        message.mPkgName = pkgName;
        message.mMainAction = mainAction;
        message.mSubAction = subAction;
        message.mExtraData = extraData;
        message.pid = Binder.getCallingPid();
        message.uid = Binder.getCallingUid();

        return mAppDataManager.dispatchMessageDown(message);

    }

    /*
     * We need to release the native memory when destroyed
     */
    public void release() {
        // Release memory from native to app
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandBusiness.ACTION_MAIN_VOICE_SERVICE;
        message.mSubAction = VoiceCommandBusiness.ACTION_VOICE_SERVICE_SELFEXIT;
        mNativeDataManager.dispatchMessageUp(message);
        mConfigManager.release();
    }

    // public static VoiceCommandManagerService main(Context context) {
    // VCMThread thread = new VCMThread(context);
    // thread.start();
    //
    // synchronized (thread) {
    // while (thread.mService == null) {
    // try {
    // thread.wait();
    // } catch (InterruptedException e) {
    // }
    // }
    // return thread.mService;
    // }
    // }

    // // We need to create the thread for async operation
    // static class VCMThread extends Thread {
    // VoiceCommandManagerService mService;
    // private final Context mContext;
    //
    // public VCMThread(Context context) {
    // super("VoiceCommandManagerService");
    // mContext = context;
    // }
    //
    // @Override
    // public void run() {
    // Looper.prepare();
    // VoiceCommandManagerService service = new
    // VoiceCommandManagerService(mContext);
    // android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_BACKGROUND);
    // android.os.Process.setCanSelfBackground(false);
    //
    // synchronized (this) {
    // mService = service;
    // notifyAll();
    // }
    //
    // Looper.loop();
    // }
    // }

    // @Override
    // public void monitor() {
    // // TODO Auto-generated method stub
    // synchronized(mAppDataManager){};
    // }

}
