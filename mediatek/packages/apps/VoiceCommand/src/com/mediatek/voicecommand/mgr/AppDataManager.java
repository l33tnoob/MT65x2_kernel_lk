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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.app.ActivityManager;
import android.app.ActivityManagerNative;
import android.app.IActivityManager;

import com.mediatek.common.voicecommand.IVoiceCommandListener;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.business.VoiceCommandBusiness;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class AppDataManager extends VoiceDataManager implements
        IMessageDispatcher {

    private static final String SYSTEM_PROCESS = "android";
    final IActivityManager mActivityManager;

    // ListenerRecord mCurTopListener;
    HashMap<String, ListenerRecord> mListenerCollection = new HashMap<String, ListenerRecord>();

    private IMessageDispatcher mDownDispatcher;
    private int[] mNativeAction = { VoiceCommandListener.ACTION_MAIN_VOICE_UI,
            VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE,
            VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING };

    public AppDataManager(VoiceCommandManagerStub service) {
        super(service);

        mActivityManager = ActivityManagerNative.getDefault();
        // TODO Auto-generated constructor stub
    }

    /*
     * check is the process illegal in AMS
     *
     * @param pkgName
     *
     * @param uid
     *
     * @param pid
     *
     * @return
     */
    private int checkProcessIllegal(String pkgName, int uid, int pid) {

        int errorid = mService.mConfigManager.isAllowProcessRegister(pkgName);
        ArrayList<String> processNameList = mService.mConfigManager.getProcessName(pkgName);

        if (processNameList != null /*&& !processName.equals(SYSTEM_PROCESS)*/
                && errorid == VoiceCommandListener.VOICE_NO_ERROR) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;

            try {
                List<ActivityManager.RunningAppProcessInfo> processList = mActivityManager
                        .getRunningAppProcesses();

                for (ActivityManager.RunningAppProcessInfo runningInfo : processList) {

                    if (runningInfo.uid == uid
                            && runningInfo.pid == pid
                            && processNameList.contains(runningInfo.processName)) {
                        errorid = VoiceCommandListener.VOICE_NO_ERROR;
                        break;
                    }
                }

            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                e.printStackTrace();
            }
        }

        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.e(VoiceCommandManagerStub.TAG, pkgName + " uid=" + uid
                    + " pid=" + pid + " register fail errorid=" + errorid);
        }
        return errorid;
    }

    private ListenerRecord checkProcessRegister(String pkgName, int uid, int pid) {
        ListenerRecord record = mListenerCollection.get(pkgName);
        if (record != null && record.mUid == uid && record.mPid == pid) {
            return record;
        }

        return null;
    }

    /*
     * check if the process use the correct main action and sub action
     *
     * @param pkgName
     *
     * @param mainAction
     *
     * @param subAction
     *
     * @return
     */
    private boolean checkActionPermission(String pkgName, int mainAction,
            int subAction) {
        // return true;

        return mService.mConfigManager.hasOperationPermission(pkgName,
                mainAction);
    }

    public int registerListener(String pkgName, int uid, int pid,
            IVoiceCommandListener listener) {
        synchronized (mListenerCollection) {
            // check from AMS whether the process(pkgname) is illegal
            int errorid = mService.mConfigManager.isCfgPrepared() ? checkProcessIllegal(
                    pkgName, uid, pid)
                    : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;

            if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {

                ListenerRecord record = checkProcessRegister(pkgName, uid, pid);

                if (record == null) {
                    record = new ListenerRecord();
                    record.mPackageName = pkgName;
                    record.mPid = pid;
                    record.mUid = uid;
                }

                if (!record.addListener(listener)) {
                    Log.i(VoiceCommandManagerStub.TAG, pkgName
                            + " has registered the listener " + listener);
                } else {
                    // registher the death notification
                    try {
                        listener.asBinder().linkToDeath(record, 0);
                        mListenerCollection.put(pkgName, record);
                    } catch (RemoteException e) {
                        // TODO Auto-generated catch block
                        errorid = VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
                        record.deleteListener(listener);
                        e.printStackTrace();
                    }
                }
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    Log.i(VoiceCommandManagerStub.TAG, pkgName
                            + " register successfully");
                }
            } else {
                Log.i(VoiceCommandManagerStub.TAG, pkgName
                        + " register fail errorid=" + errorid);
            }
            Runtime.getRuntime().gc();
            return errorid;
        }
    }

    public int unRegisterListener(String pkgName, int uid, int pid,
            IVoiceCommandListener listener, boolean isCheckedProcess) {
        // check from AMS that the process(pkgname) is illegal
        int errorid = isCheckedProcess ? VoiceCommandListener.VOICE_NO_ERROR
                : checkProcessIllegal(pkgName, uid, pid);
        synchronized (mListenerCollection) {
            if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                ListenerRecord record = checkProcessRegister(pkgName, uid, pid);
                if (record != null) {
                    if (record.deleteListener(listener)) {
                        if (record.mCurListener == null) {
                            unRegisterListenerLocked(record);
                        }
                    } else {
                        Log.i(VoiceCommandManagerStub.TAG, pkgName
                                + " didn't register the listener " + listener);
                    }
                } else {
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_UNREGISTER;
                    Log.i(VoiceCommandManagerStub.TAG, pkgName
                            + " didn't register in unRegisterListener");
                }
            }
            Runtime.getRuntime().gc();
            return errorid;
        }
    }

    /*
     * Remove the listener from the list and unlink the death from the app's
     * processes
     *
     * @param record
     *
     * @param isProcessDied
     */
    private void unRegisterListenerLocked(ListenerRecord record) {
        Log
                .i(VoiceCommandManagerStub.TAG,
                        "unRegisterListenerLocked Remove record "
                                + record.mPackageName);
        mListenerCollection.remove(record.mPackageName);
        // if the app has started the voice ui ,recognization or training,we
        // need to stop the native voice capture .
        if (mService.mConfigManager.containOperationPermission(
                record.mPackageName, mNativeAction)) {
            VoiceMessage msg = new VoiceMessage();
            msg.mPkgName = record.mPackageName;
            msg.pid = mService.mConfigManager.getProcessID(record.mPackageName);
            msg.mMainAction = VoiceCommandBusiness.ACTION_MAIN_VOICE_SERVICE;
            msg.mSubAction = VoiceCommandBusiness.ACTION_VOICE_SERVICE_PROCESSEXIT;
            // Notify VoiceServiceInternal to stop capture
            mDownDispatcher.dispatchMessageDown(msg);
        }
    }

    /*
     * Send message to next dispatcher
     */
    @Override
    public int dispatchMessageDown(VoiceMessage message) {
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        synchronized (mListenerCollection) {
            ListenerRecord record = checkProcessRegister(message.mPkgName,
                    message.uid, message.pid);
            if (record == null) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_UNREGISTER;
                Log.i(VoiceCommandManagerStub.TAG, message.mPkgName
                        + " didn't register in dispatchMessageDown");
            } else if (!checkActionPermission(message.mPkgName,
                    message.mMainAction, message.mSubAction)) {
                // Will this case happen
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
                Log.i(VoiceCommandManagerStub.TAG, message.mPkgName
                        + " has no permission");
                Bundle bundle = DataPackage.packageErrorResult(errorid);
                message.mExtraData = bundle;
                dispatchMessageUp(message);

            } else {
                errorid = mDownDispatcher.dispatchMessageDown(message);
            }
        }
        return errorid;
    }

    /*
     * Send message to Apps
     */
    @Override
    public int dispatchMessageUp(VoiceMessage message) {

        Log.i(VoiceCommandManagerStub.TAG,
                "dispatchMessageUp in AppDataManager" + " mainAction="
                        + message.mMainAction + " subAction="
                        + message.mSubAction);
        // TODO Auto-generated method stub
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        // filter out the process which can receive the message
        synchronized (mListenerCollection) {
            ListenerRecord record = mListenerCollection.get(message.mPkgName);

            if (record != null && record.mCurListener != null) {
                try {
                    record.mCurListener.onVoiceCommandNotified(
                            message.mMainAction, message.mSubAction,
                            message.mExtraData);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    // e.printStackTrace();
                    record.deleteListener(record.mCurListener);
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_NOTIFYFAIL;
                }
            } else {
                Log
                        .i(VoiceCommandManagerStub.TAG,
                                "dispatchMessageUp in AppDataManager can't find the listener record");
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_UNREGISTER;
            }
        }

        return errorid;
    }

    @Override
    public void setDownDispatcher(IMessageDispatcher dispatcher) {
        // TODO Auto-generated method stub
        mDownDispatcher = dispatcher;
    }

    @Override
    public void setUpDispatcher(IMessageDispatcher dispatcher) {
        // TODO Auto-generated method stub
        // Do not need up dispatcher ,because this manager send message to apps
        // directly
    }

    class ListenerRecord implements IBinder.DeathRecipient {

        ArrayList<IVoiceCommandListener> mListenerList = new ArrayList<IVoiceCommandListener>();

        IVoiceCommandListener mCurListener = null;
        /*
         * The name of the process which access voice command service
         */
        String mPackageName;

        int mPid;

        int mUid;

        @Override
        public void binderDied() {
            synchronized (mListenerCollection) {
                // TODO Auto-generated method stub
                Log.i(VoiceCommandManagerStub.TAG, mPackageName + " has died ");
                mListenerList.clear();
                unRegisterListenerLocked(ListenerRecord.this);
            }
        }

        /*
         * Add the process listener ,if exist , return false
         *
         * @param listener
         *
         * @return
         */
        public boolean addListener(IVoiceCommandListener listener) {
            mCurListener = listener;

            int size = mListenerList.size();

            for (int i = 0; i < size; i++) {
                if (mListenerList.get(i).asBinder() == listener.asBinder()) {
                    return false;
                }
            }
            mListenerList.add(listener);
            return true;
        }

        /*
         * Delete the process listener
         *
         * @param listener
         *
         * @return
         */
        public boolean deleteListener(IVoiceCommandListener listener) {
            boolean isDeleted = false;
            int size = mListenerList.size();
            for (int i = 0; i < size; i++) {
                if (mListenerList.get(i).asBinder() == listener.asBinder()) {
                    isDeleted = true;
                    size--;
                    mListenerList.remove(i);
                }
            }
            if (isDeleted) {
                listener.asBinder().unlinkToDeath(ListenerRecord.this, 0);
                if (size > 0) {
                    mCurListener = mListenerList.get(size - 1);
                } else {
                    mCurListener = null;
                }
            }
            return isDeleted;
        }

    }

}

