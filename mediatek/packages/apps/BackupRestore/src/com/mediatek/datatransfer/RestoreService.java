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

package com.mediatek.datatransfer;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Notification;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

import com.mediatek.datatransfer.RestoreEngine.OnRestoreDoneListner;
import com.mediatek.datatransfer.ResultDialog.ResultEntity;
import com.mediatek.datatransfer.modules.Composer;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.NotifyManager;
import com.mediatek.datatransfer.utils.Constants.State;

public class RestoreService extends Service implements ProgressReporter, OnRestoreDoneListner {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreService";

    public interface OnRestoreStatusListener {
        public void onComposerChanged(final int type, final int num);

        public void onProgressChanged(Composer composer, int progress);

        public void onRestoreEnd(boolean bSuccess, ArrayList<ResultEntity> resultRecord);

        public void onRestoreErr(IOException e);
    }

    public static class RestoreProgress {
        int mType;
        int mMax;
        int mCurNum;
    }

    private RestoreBinder mBinder = new RestoreBinder();
    private int mState;
    private RestoreEngine mRestoreEngine;
    private ArrayList<ResultEntity> mResultList;
    private RestoreProgress mCurrentProgress = new RestoreProgress();
    private OnRestoreStatusListener mRestoreStatusListener;
    private ArrayList<ResultEntity>mAppResultList;
    private String mFileName = "";
    HashMap<Integer, ArrayList<String>> mParasMap = new HashMap<Integer, ArrayList<String>>();

    @Override
    public IBinder onBind(Intent intent) {
        MyLogger.logI(CLASS_TAG, "onbind");
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        super.onUnbind(intent);
        MyLogger.logI(CLASS_TAG, "onUnbind");
        return true;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        moveToState(State.INIT);
        MyLogger.logI(CLASS_TAG, "onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        MyLogger.logI(CLASS_TAG, "onStartCommand");
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        stopForeground(true);
        MyLogger.logI(CLASS_TAG, "onDestroy");
        if (mRestoreEngine != null && mRestoreEngine.isRunning()) {
            mRestoreEngine.setOnRestoreEndListner(null);
            mRestoreEngine.cancel();
        }
    }

    public void moveToState(int state) {
        synchronized (this) {
            mState = state;
        }
    }

    private int getRestoreState() {
        synchronized (this) {
            return mState;
        }
    }

    public class RestoreBinder extends Binder {
        public int getState() {
            return getRestoreState();
        }
        
        public void setRestoreModelList(ArrayList<Integer> list){
            reset();
            if (mRestoreEngine == null) {
                mRestoreEngine = RestoreEngine
                .getInstance(RestoreService.this, RestoreService.this);
            }
            mRestoreEngine.setRestoreModelList(list);
        }
        
        public void setRestoreItemParam(int itemType, ArrayList<String> paraList){
            mParasMap.put(itemType, paraList);
            mRestoreEngine.setRestoreItemParam(itemType, paraList);
        }
        
        public ArrayList<String> getRestoreItemParam(int itemType){
            return mParasMap.get(itemType);
        }

        public boolean startRestore(String fileName) {
            startForeground(1, new Notification());
            if (mRestoreEngine == null) {
                MyLogger.logE(CLASS_TAG, "startRestore Error: engine is not initialed");
                return false;
            }
            mRestoreEngine.setOnRestoreEndListner(RestoreService.this);
            mRestoreEngine.startRestore(fileName);
            mFileName = fileName;
            moveToState(State.RUNNING);
            return true;
        }

        public void pauseRestore() {
            moveToState(State.PAUSE);
            if (mRestoreEngine != null) {
                mRestoreEngine.pause();
            }
            MyLogger.logD(CLASS_TAG, "pauseRestore");
        }

        public void continueRestore() {
            moveToState(State.RUNNING);
            if (mRestoreEngine != null) {
                mRestoreEngine.continueRestore();
            }
            MyLogger.logD(CLASS_TAG, "continueRestore");
        }

        public void cancelRestore() {
            moveToState(State.CANCELLING);
            if (mRestoreEngine != null) {
                mRestoreEngine.cancel();
            }
            MyLogger.logD(CLASS_TAG, "cancelRestore");
        }

        public void reset() {
            moveToState(State.INIT);
            if (mResultList != null) {
                mResultList.clear();
            }
            if(mAppResultList != null){
                mAppResultList.clear();
            }
            if(mParasMap != null){
                mParasMap.clear();
            }
        }

        public RestoreProgress getCurRestoreProgress() {
            return mCurrentProgress;
        }

        public void setOnRestoreChangedListner(OnRestoreStatusListener listener) {
            mRestoreStatusListener = listener;
        }

        public ArrayList<ResultEntity> getRestoreResult() {
            return mResultList;
        }
        
        public ArrayList<ResultEntity> getAppRestoreResult(){
            return mAppResultList;
        }
    }

    public void onStart(Composer iComposer) {
        mCurrentProgress.mType = iComposer.getModuleType();
        mCurrentProgress.mMax = iComposer.getCount();
        mCurrentProgress.mCurNum = 0;
        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onComposerChanged(mCurrentProgress.mType, mCurrentProgress.mMax);
        }
        
        if (mCurrentProgress.mMax != 0) {
            NotifyManager.getInstance(RestoreService.this).setMaxPercent(mCurrentProgress.mMax);
        }
    }

    public void onOneFinished(Composer composer, boolean result) {

        mCurrentProgress.mCurNum++;
        if(composer.getModuleType() == ModuleType.TYPE_APP){
            if(mAppResultList == null){
                mAppResultList = new ArrayList<ResultEntity>();
            }
            ResultEntity entity = new ResultEntity(ModuleType.TYPE_APP, 
                                    result ? ResultEntity.SUCCESS : ResultEntity.FAIL);
            entity.setKey(mParasMap.get(ModuleType.TYPE_APP).get(mCurrentProgress.mCurNum-1));
            mAppResultList.add(entity);
        }
        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onProgressChanged(composer, mCurrentProgress.mCurNum);
        }
        
        if (mCurrentProgress.mMax != 0) {
            NotifyManager.getInstance(RestoreService.this).showRestoreNotification(
                    ModuleType.getModuleStringFromType(this, composer.getModuleType()), mFileName,
                    composer.getModuleType(), mCurrentProgress.mCurNum);
        }
    }

    public void onEnd(Composer composer, boolean result) {

        if (mResultList == null) {
            mResultList = new ArrayList<ResultEntity>();
        }
        ResultEntity item = new ResultEntity(composer.getModuleType(), 
                    result ? ResultEntity.SUCCESS : ResultEntity.FAIL);
        mResultList.add(item);
    }

    public void onErr(IOException e) {

        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onRestoreErr(e);
        }
    }

    public void onFinishRestore(boolean bSuccess) {
        moveToState(State.FINISH);
        if (mRestoreStatusListener != null) {
            mRestoreStatusListener.onRestoreEnd(bSuccess, mResultList);
        }
//        NotifyManager.getInstance(RestoreService.this).clearNotification();
        NotifyManager.getInstance(this).showFinishNotification(NotifyManager.NOTIFY_RESTORING,true);
    }
}
