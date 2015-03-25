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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.videoplayer;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.PriorityQueue;
import java.util.Set;

public class ThumbnailCache {
    private static final String TAG = "ThumbnailCache";
    private final HashMap<Long, ThumbnailEntity> mThumbnailEntities = new HashMap<Long, ThumbnailEntity>();
    private final ArrayList<ThumbnailStateListener> mListeners = new ArrayList<ThumbnailStateListener>();
    private Handler mNewRequestHandler = null; // Sub thread
    private final Handler mDoneRequestUiHandler = new DoneRequestUiHandler(); // UI thread
    private static Looper sLooper;
    private static final PriorityQueue<Request> REQUEST_QUEUE =
            new PriorityQueue<Request>(10, Request.getComparator()); // Priority queue for async request
    private static final int TASK_REQUEST_DONE = 1;
    private static final int TASK_REQUEST_NEW = 2;
    private long mPrioritySeed = 0;
    private Request mCurrentRequest = null;
    private ThumbnailBuilder mThumbnailBuilder = null;

    public ThumbnailCache(final Context context) {
        //mContext = context;
        mThumbnailBuilder = new ThumbnailBuilder(context);
        initTask();
    }

    private void initTask() {
        MtkLog.v(TAG, "initTask()");
        mPrioritySeed = 0;
        synchronized (ThumbnailCache.class) {
            if (sLooper == null) {
                final HandlerThread t = new HandlerThread("cached-thumbnail-thread",
                        android.os.Process.THREAD_PRIORITY_BACKGROUND);
                t.start();
                sLooper = t.getLooper();
            }
        }
        mNewRequestHandler = new NewRequestHandler(sLooper);
    }

    private void clearTask() {
        MtkLog.v(TAG, "clearTask()");
        mPrioritySeed = 0;
        synchronized (REQUEST_QUEUE) {
            REQUEST_QUEUE.clear();
        }
        mDoneRequestUiHandler.removeMessages(TASK_REQUEST_DONE);
        mNewRequestHandler.removeMessages(TASK_REQUEST_NEW);
        mNewRequestHandler = null;
        mThumbnailBuilder.cancelThumbnailFromDb();
        if (sLooper != null) {
            sLooper.quit();
            sLooper = null;
        }
    }

    //will be loaded if sdcard is in phone
    public Bitmap getCachedThumbnail(final long id, final long dateModified,
            final boolean support3D, final boolean request) {
        MtkLog.v(TAG, "getCachedThumbnail(" + id + ", " + dateModified + ", " + request + ")");
        ThumbnailEntity thumbnailEntity = null;
        synchronized (mThumbnailEntities) {
            thumbnailEntity = mThumbnailEntities.get(id);
        }
        if (request) {
            if (thumbnailEntity == null) {
                thumbnailEntity = createNewRequest(id, dateModified, support3D); 
            } else {
                updateRequest(id, dateModified, support3D, thumbnailEntity);
            }
            MtkLog.v(TAG, "getCachedThumbnail() async load the drawable for " + id + " size()=" + REQUEST_QUEUE.size());
        } 
        // Bind View twice for one item, one thumbnailEntity is null, one is not.
        Bitmap result = null;
        if (thumbnailEntity == null || thumbnailEntity.getThumbnail() == null) {
            result = mThumbnailBuilder.getDefaultThumbnailWith3D(support3D);
        } else {
            result = thumbnailEntity.getThumbnail();
        }
        MtkLog.v(TAG, "getCachedThumbnail() id " + id + " size()=" + REQUEST_QUEUE.size() + ", return " + result);
        return result;
    }

    private void updateRequest(final long id, final long dateModified,
            boolean support3D, ThumbnailEntity thumbnailEntity) {
        MtkLog.d(TAG, "updateRequest() id:" + id + ", dateModified: "+ dateModified + ", thumbnailEntity:"+thumbnailEntity);
        if (thumbnailEntity != null &&
                (thumbnailEntity.getDateModified() != dateModified || thumbnailEntity.isSupport3D() != support3D)) {
            thumbnailEntity.setType(ThumbnailBuilder.TYPE_NEED_LOAD);
        } else {
            // Most case come here, type will be not ThumbnailBuilder.TYPE_NEED_LOAD
        }
        if(thumbnailEntity.getType() == ThumbnailBuilder.TYPE_NEED_LOAD) {
            mPrioritySeed++;
            synchronized (REQUEST_QUEUE) {
                //check is processing or not
                if (!isProcessing(id, dateModified)) {
                    Request oldRequestParam = getOldRequest(id);
                    if (oldRequestParam != null) {
                        MtkLog.d(TAG, "updateRequest() updateOldRequest priority:" + (-mPrioritySeed));
                        oldRequestParam.setPriority(-mPrioritySeed);
                        oldRequestParam.getThumbnailEntity().setDateModified(dateModified);
                        oldRequestParam.getThumbnailEntity().setSupport3D(support3D);
                        if (REQUEST_QUEUE.remove(oldRequestParam)) {
                            REQUEST_QUEUE.add(oldRequestParam);//re-order the queue
                        }
                    }
                }
            }
        }
    }

    private ThumbnailEntity createNewRequest(final long id,
            final long dateModified, final boolean support3D) {
        MtkLog.d(TAG, "createNewRequest() id: " + id + ",dateModified:" + dateModified + ", support3D:" + support3D);
        synchronized (REQUEST_QUEUE) {
            mPrioritySeed++;
            ThumbnailEntity thumbnailEntity = createThumbnailEntity(id, dateModified, support3D);
            mThumbnailEntities.put(id, thumbnailEntity);
            final Request request = new Request(id, -mPrioritySeed, thumbnailEntity);
            REQUEST_QUEUE.add(request);

            mNewRequestHandler.sendEmptyMessage(TASK_REQUEST_NEW);
            return thumbnailEntity;
        }
    }

    private ThumbnailEntity createThumbnailEntity(final long id, final long dateModified, final boolean support3D) {
        ThumbnailEntity thumbnailEntity = new ThumbnailEntity(mThumbnailBuilder.getDefaultThumbnailWith3D(support3D),
                    ThumbnailBuilder.TYPE_NEED_LOAD, dateModified, support3D);
        MtkLog.v(TAG, "createThumbnailEntity(" + id + ", " + dateModified + ", " + support3D + ") return " + thumbnailEntity);
        return thumbnailEntity;
    }

    private Request getOldRequest(final long id) {
        Request oldRequest = null;
        for (final Request one : REQUEST_QUEUE) {
            if (one.getRowId() == id) {
                oldRequest = one;
                break;
            }
        }
        MtkLog.v(TAG, "getOldRequest(" + id + ") return " + oldRequest);
        return oldRequest;
    }

    private boolean isProcessing(final long id, final long dateModified) {
        boolean processing = false;
        if (mCurrentRequest != null) {
            synchronized (mCurrentRequest) {
                if (mCurrentRequest.getRowId() == id && mCurrentRequest.getThumbnailEntity().getDateModified() == dateModified) {
                    processing = true;
                }
            }
        }
        MtkLog.v(TAG, "isProcessing(" + id + ", " +  dateModified + ") return " + processing);
        return processing;
    }

    public void clear() {
        MtkLog.v(TAG, "clear()");
        clearTask();
        mListeners.clear();
        synchronized (mThumbnailEntities) {
            final Set<Long> keys = mThumbnailEntities.keySet();
            for (final Long key : keys) {
                final ThumbnailEntity thumbnailEntity = mThumbnailEntities.get(key);
                mThumbnailBuilder.recycle(thumbnailEntity);
            }
            mThumbnailEntities.clear();
        }
        MtkLog.v(TAG, "clear() finished");
    }
    
    public boolean addListener(final ThumbnailStateListener listener) {
        return mListeners.add(listener);
    }
    
    public boolean removeListener(final ThumbnailStateListener listener) {
        return mListeners.remove(listener);
    }

    public interface ThumbnailStateListener {
        //will be called if requested drawable state is changed.
        void onChanged(long rowId, int type, Bitmap thumbnail);
    }

    private class NewRequestHandler extends Handler {
        private static final String TAG = "NewRequestHandler";
        
        public NewRequestHandler(Looper looper) {
            super(looper);
        }
        
        @Override
        public void handleMessage(final Message msg) {
            MtkLog.v(TAG, "mTaskHandler.handleMessage(" + msg + ") this=" + this);
            if (msg.what == TASK_REQUEST_NEW) {
                synchronized (REQUEST_QUEUE) {
                    mCurrentRequest = REQUEST_QUEUE.poll();
                }
                if (mCurrentRequest == null) {
                    MtkLog.w(TAG, "wrong request, has request but no task params.");
                    return;
                }
                final Request request = mCurrentRequest;//currentRequest may be cleared by other thread.
                //recheck the drawable is exists or not.
                final long id = request.getRowId();
                ThumbnailEntity thumbnailEntity = null;
                synchronized (mThumbnailEntities) {
                    thumbnailEntity = mThumbnailEntities.get(id);
                }
                if (thumbnailEntity == null) {
                    MtkLog.w(TAG, "cached drawable was delete. may for clear.");
                    return;
                }
                //load or reload the preview
                if (thumbnailEntity.getType() == ThumbnailBuilder.TYPE_NEED_LOAD) {
                    Bitmap originThumbnail = mThumbnailBuilder.getThumbnailFromDb(id);
                    if (originThumbnail != null) {
                        int width = mThumbnailBuilder.getmDefaultThumbnail().getWidth();
                        int height = mThumbnailBuilder.getmDefaultThumbnail().getHeight();
                        Bitmap thumbnail = Bitmap.createScaledBitmap(originThumbnail, width, height, true);
                        if (thumbnailEntity.isSupport3D()) {
                            // Copy to avoid IllegalStateException: Immutable bitmap passed to Canvas constructor
                            thumbnail = thumbnail.copy(Bitmap.Config.ARGB_8888, true);
                            thumbnail = mThumbnailBuilder.overlay3DImpl(thumbnail);
                        }
                        thumbnailEntity.setThumbnail(thumbnail);
                        thumbnailEntity.setType(ThumbnailBuilder.TYPE_LOADED_HAS_PREVIEW);
                    } else {
                        thumbnailEntity.setThumbnail(null);
                        thumbnailEntity.setType(ThumbnailBuilder.TYPE_LOADED_NO_PREVIEW);
                    }
                }
                if (request != mCurrentRequest) {
                    MtkLog.w(TAG, "current request was changed by other thread. task=" + request
                            + ", currentRequest=" + mCurrentRequest);
                    return;
                }
                request.setThumbnailEntity(thumbnailEntity);
                final Message doneMessage = mDoneRequestUiHandler.obtainMessage(TASK_REQUEST_DONE);
                doneMessage.obj = request;
                doneMessage.sendToTarget();
                MtkLog.v(TAG, "mTaskHandler.handleMessage() send done. " + mCurrentRequest + " this=" + this);
                synchronized (REQUEST_QUEUE) {
                    mCurrentRequest = null;
                }
            }
        }
    }
    
    private class DoneRequestUiHandler extends Handler {
        @Override
        public void handleMessage(final Message msg) {
            MtkLog.v(TAG, "handleMessage(" + msg + ")");
            if (msg.what == TASK_REQUEST_DONE && (msg.obj instanceof Request)) {
                final Request param = (Request) msg.obj;
                final ThumbnailEntity thumbnailEntity = param.getThumbnailEntity();
                for (final ThumbnailStateListener listener : mListeners) {
                    listener.onChanged(param.getRowId(), thumbnailEntity.getType(), thumbnailEntity.getThumbnail());
                }
            }
        }
    }
    
    public int getDefaultThumbnailWidth() {
        return mThumbnailBuilder.getmDefaultThumbnail().getWidth();
    }
    
    public int getDefaultThumbnailHeight() {
        return mThumbnailBuilder.getmDefaultThumbnail().getHeight();
    }
}
