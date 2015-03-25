/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.service;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.provider.ContactsContract.Contacts;
import android.provider.MediaStore.Images;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.SoftReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This is a utility class, which is used to load image asynchronously
 */
public final class ImageLoader {
    public static final String TAG = "ImageLoader";

    private static final ImageLoader INSTANCE = new ImageLoader();

    private static final String[] PROJECTION = new String[] {
            Contacts._ID, Contacts.LOOKUP_KEY, Contacts.PHOTO_URI
    };
    private static final Bitmap DEFAULT_BITMAP;
    private static final Bitmap BLANK_BITMAP;
    static {
        Resources resource = null;
        try {
            resource = AndroidFactory.getApplicationContext().getPackageManager()
                    .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (android.content.pm.PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        if (null != resource) {
            Logger.d(TAG, "ImageLoader resource is not null");
            DEFAULT_BITMAP = BitmapFactory.decodeResource(resource, R.drawable.default_header);
            BLANK_BITMAP = BitmapFactory.decodeResource(resource, R.drawable.contact_blank_avatar);
        } else {
            Logger.d(TAG, "ImageLoader resource is  null");
            DEFAULT_BITMAP = null;
            BLANK_BITMAP = null;
        }
    }

    /**
     * Caller application needs to implement this interface to retrieve the
     * target Bitmap from ImageLoader
     */
    public interface OnLoadImageFinishListener {
        /**
         * This method will be called on the main thread after the image has
         * been loaded successfully
         * 
         * @param image The target image caller application wants
         */
        void onLoadImageFished(Bitmap image);
    }

    /**
     * Get a image at a specific file path
     * 
     * @param path The path of the image file
     * @return The Bitmap instance if the image has already been decoded, null
     *         if the image has not been decoded
     */
    public static Bitmap getImage(Object key) {
        Logger.d(TAG, "getImage() the key is " + key);
        if (getInstance().isImageLoaded(key)) {
            Logger.d(TAG, "getImage() the image " + key
                    + " has already been loaded, just return it");
            return getInstance().getExistingImage(key);
        } else {
            Logger.d(TAG, "getImage() the image " + key + " has not been loaded, just return null");
            return null;
        }
    }

    /**
     * Try to get a bitmap if it has been loaded or request to load it if it has
     * not been loaded, this calling will force synchronized loading
     * 
     * @param key The key of this image
     * @return The Bitmap instance if the image has already been decoded, null
     *         if the image has not been decoded
     */
    public static Bitmap requestImage(Object key) {
        Logger.d(TAG, "requestImage() the key is " + key);
        return requestImage(key, null);
    }

    /**
     * Try to get a bitmap if it has been loaded or request to load it if it has
     * not been loaded
     * 
     * @param key The key of this image
     * @param listener The callback listener user to notify the image has been
     *            loaded successfully, null indicates that the called needs
     *            synchronized loading
     * @return The Bitmap instance if the image has already been decoded, null
     *         if the image has not been decoded
     */
    public static Bitmap requestImage(Object key, OnLoadImageFinishListener listener) {
        Logger.d(TAG, "requestImage() the key is " + key + " the listener is " + listener);
        if (key instanceof TreeSet<?>) {
            new LoadParticipantsImageTask((TreeSet<String>) key, listener);
            if (getInstance().isParticipantsImageLoaded(key)) {
                Logger.d(TAG, "requestImage() the image " + key
                        + " has already been loaded, just return it");
                return getInstance().getExistingImage(key);
            } else {
                Logger.d(TAG, "requestImage() the image " + key
                        + " has not been loaded, just return null and request to load it");
                return null;
            }
        } else if (getInstance().isImageLoaded(key)) {
            Logger.d(TAG, "requestImage() the image " + key
                    + " has already been loaded, just return it");
            return getInstance().getExistingImage(key);
        } else {
            if (null != listener) {
                Logger.d(TAG, "requestImage() the image " + key
                        + " has not been loaded, just return null and request to load it");
                getInstance().loadImage(key, listener);
                return null;
            } else {
                Logger.d(TAG, "requestImage() the image " + key
                        + " has not been loaded, sync load the requested image and return it");
                return getInstance().syncLoadImage(key);
            }
        }
    }

    public static void interrupt() {
        Logger.d(TAG, "interrupt() called");
        LoadImageTask.interrupt();
    }

    public static void interrupt(Object key) {
        Logger.d(TAG, "interrupt() called");
        if (key instanceof TreeSet<?>) {
            LoadParticipantsImageTask.interrupt((TreeSet<String>) key);
        } else {
            LoadImageTask.interrupt(key);
        }
    }

    public static ImageLoader getInstance() {
        return INSTANCE;
    }

    public void clearImageMap() {
        if (mImageMap != null) {
            mImageMap.clear();
        }
    }

    private Map<Object, BitmapReference> mImageMap =
            new ConcurrentHashMap<Object, BitmapReference>();
    private Map<TreeSet<String>, BitmapReference> mParticipantsImageMap =
            new ConcurrentHashMap<TreeSet<String>, ImageLoader.BitmapReference>();

    private ReferenceQueue<Bitmap> mBitmapReferenceQueue = new ReferenceQueue<Bitmap>();

    private void loadImage(Object key, OnLoadImageFinishListener listener) {
        Logger.d(TAG, "loadImage() key is " + key + ", listener is " + listener);
        generateTask(key, listener);
    }

    private Bitmap syncLoadImage(Object key) {
        Logger.d(TAG, "syncLoadImage() key is " + key);
        LoadImageTask task = generateTask(key, null);
        if (task == null) {
            Logger.w(TAG, "syncLoadImage(),task is null");
            return null;
        }
        Bitmap bitmap = task.syncLoadImage();
        if (null != bitmap) {
            setImage(key, bitmap);
        }
        return bitmap;
    }

    private LoadImageTask generateTask(Object key, OnLoadImageFinishListener listener) {
        // judge key is null, because of selector haven't key
        if (key != null) {
            if (key instanceof String) {
                return new LoadImageFileTask((String) key, listener);
            } else if (key instanceof Uri) {
                return new LoadImageUriTask((Uri) key, listener);
            } else {
                throw new IllegalArgumentException("Unknown key type");
            }
        } else {
            Logger.w(TAG, "generateTask() loadImage, key is null!");
            return null;
        }
    }

    private Bitmap getExistingImage(Object key) {
        BitmapReference reference = null;
        if (key instanceof TreeSet<?>) {
            reference = mParticipantsImageMap.get(key);
        } else {
            reference = mImageMap.get(key);
        }
        if (null != reference) {
            return reference.get();
        } else {
            Logger.e(TAG, "getExistingImage() reference is null, the key is " + key);
            return null;
        }
    }

    private boolean isImageLoaded(Object key) {
        BitmapReference reference = null;
        if (key instanceof TreeSet<?>) {
            reference = mParticipantsImageMap.get(key);
        } else {
            if (key != null) {
                reference = mImageMap.get(key);
            } else {
                Logger.d(TAG, "isImageLoaded() the key is null ");
            }
        }
        clearUnreachableBitmap();
        return (null != reference && null != reference.get());
    }

    private boolean isParticipantsImageLoaded(Object key) {
        BitmapReference reference = mParticipantsImageMap.get(key);
        clearUnreachableBitmap();
        return (null != reference && null != reference.get());
    }

    private void setImage(Object key, Bitmap image) {
        Logger.d(TAG, "setImage() cache bitmap for key: " + key);
        if (key instanceof TreeSet<?>) {
            mParticipantsImageMap.put((TreeSet<String>) key, new BitmapReference(image,
                    mBitmapReferenceQueue, key));
        } else {
            mImageMap.put(key, new BitmapReference(image, mBitmapReferenceQueue, key));
        }
        clearUnreachableBitmap();
    }

    private void clearUnreachableBitmap() {
        BitmapReference reference = null;
        while (null != (reference = (BitmapReference) mBitmapReferenceQueue.poll())) {
            Object key = reference.mKey;
            Logger.d(TAG, "clearUnreachedBitmap () remove unreached Bitmap " + key);
            if (key instanceof TreeSet<?>) {
                mParticipantsImageMap.remove(key);
            } else {
                mImageMap.remove(key);
            }
        }
    }

    /**
     * Use SoftReference to store Bitmap instance of avoid memory leak by
     * clearing unreachable Bitmap
     */
    private static class BitmapReference extends SoftReference<Bitmap> {
        private Object mKey = null;

        public BitmapReference(Bitmap r, ReferenceQueue<? super Bitmap> q, Object key) {
            super(r, q);
            mKey = key;
        }
    }

    /**
     * This asyncTask is used to load image in sequence
     */
    private abstract static class LoadImageTask extends AsyncTask<Void, Void, Bitmap> {
        public static final String TAG = "LoadImageTask";

        private static final Handler MAIN_THREAD_HANDLER = new Handler(Looper.getMainLooper());

        private static final CopyOnWriteArrayList<LoadImageTask> TASK_LIST =
                new CopyOnWriteArrayList<LoadImageTask>();

        private static final ConcurrentHashMap<Object, LoadImageTask> TASK_MAP =
                new ConcurrentHashMap<Object, LoadImageTask>();

        public static void interrupt() {
            Logger.v(TAG, "interrupt()");
            MAIN_THREAD_HANDLER.post(new Runnable() {
                @Override
                public void run() {
                    int size = TASK_LIST.size();
                    if (size > 0) {
                        Logger.i(TAG, "interrupt() the size is " + size);
                        TASK_LIST.get(0).cancel(true);
                        TASK_LIST.clear();
                    } else {
                        Logger.i(TAG, "interrupt() the size is " + size
                                + " and no need to interrupt");
                    }
                }
            });
        }

        public static void interrupt(Object key) {
            Logger.d(TAG, "interrupt() called the key is " + key);
            LoadImageTask task = TASK_MAP.get(key);
            if (task != null) {
                Status status = task.getStatus();
                switch (status) {
                    case PENDING:
                    case FINISHED:
                        TASK_MAP.remove(key);
                        TASK_LIST.remove(task);
                        Logger.d(TAG, "interrupt() the image " + key + " is " + status
                                + ", remove it from LIST and MAP");
                        break;
                    case RUNNING:
                        task.mIsActive = false;
                        Logger.d(TAG, "interrupt() the image " + key
                                + " is running, just make it disabled");
                        break;
                    default:
                        break;
                }
            } else {
                Logger.e(TAG, "interrupt() TASK_MAP doesn't contain the key " + key);
            }
        }

        private OnLoadImageFinishListener mListener = null;
        public Object mKey = null;
        private boolean mIsActive = true;

        private LoadImageTask(Object key, OnLoadImageFinishListener listener) {
            mListener = listener;
            mKey = key;
            if (null != mListener) {
                Logger.d(TAG, "Constructor() key is " + key
                        + " and listener is not null, it's an async task");
                MAIN_THREAD_HANDLER.post(new Runnable() {
                    @Override
                    public void run() {
                        TASK_LIST.add(LoadImageTask.this);
                        TASK_MAP.put(mKey, LoadImageTask.this);
                        if (1 == TASK_LIST.size()) {
                            LoadImageTask.this.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                        }
                    }
                });
            } else {
                Logger.d(TAG, "Constructor() key is " + key
                        + " and listener is null, it's a sync task");
            }
        }

        public Bitmap syncLoadImage() {
            Logger.d(TAG, "syncLoadImage() entry, key is " + mKey);
            return doInBackground(null, null, null);
        }

        @Override
        protected void onPostExecute(Bitmap result) {
            Logger.v(TAG, "onPostExecute() entry: the image is " + mKey);
            super.onPostExecute(result);
            if (mIsActive) {
                if (null != mListener) {
                    ImageLoader.getInstance().setImage(mKey, result);
                    mListener.onLoadImageFished(result);
                }
            } else {
                Logger.d(TAG, "onPostExecute() the loading " + mKey + "has been disabled");
            }

            TASK_MAP.remove(mKey);
            TASK_LIST.remove(this);
            int size = TASK_LIST.size();
            if (size > 0) {
                Logger.v(TAG, "onPostExecute() there is still " + size + " waiting images to load");
                TASK_LIST.get(0).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
            } else {
                Logger.v(TAG, "onPostExecute() there is not any waiting images to load");
            }
        }
    }

    /**
     * This class is used to load image from a image file
     */
    private class LoadImageFileTask extends LoadImageTask {
        public static final String TAG = "LoadImageFileTask";

        private LoadImageFileTask(String path, OnLoadImageFinishListener listener) {
            super(path, listener);
        }

        @Override
        protected Bitmap doInBackground(Void... params) {
            Logger.v(TAG, "doInBackground() Begin to load image: " + mKey);
            Bitmap image = ImageLoader.getImage(mKey);
            if (null == image) {
                Logger.d(TAG, "doInBackground() the file " + mKey + " has not been loaded yet");
                if (com.mediatek.rcse.service.Utils.isFileExist((String)mKey)) {
                    image = ThumbnailUtils.createImageThumbnail((String)mKey, Images.Thumbnails.MINI_KIND);
                    int degrees = Utils.getDegreesRotated((String)mKey);
                    Logger.v(TAG, "rotate the picture " + degrees);
                    if (0 != degrees) {
                        image = Utils.rotate(image, degrees);
                    } else {
                        Logger
                                .d(TAG,
                                        "doInBackground() file degress is zero, so no need to rotate");
                    }
                } else {
                    Logger.e(TAG, "doInBackground() the file " + mKey + " doesn't exist!");
                }
            } else {
                Logger.w(TAG, "doInBackground() the file " + mKey + " has already been loaded");
            }
            Logger.v(TAG, "load " + mKey + " finish");
            return image;
        }
    }

    /**
     * This class is used to load a image from an inputStream in
     * ContactsProvider
     */
    private class LoadImageUriTask extends LoadImageTask {
        public static final String TAG = "LoadImageUriTask";

        private LoadImageUriTask(Uri uri, OnLoadImageFinishListener listener) {
            super(uri, listener);
        }

        @Override
        protected Bitmap doInBackground(Void... params) {
            Logger.v(TAG, "doInBackground() Begin to load image: " + mKey);
            Bitmap image = null;
            if (mKey != null) {
                image = ImageLoader.getImage(mKey);
                if (null == image) {
                    Logger.d(TAG, "doInBackground() the file " + mKey + " has not been loaded yet");
                    ApiManager apiManager = ApiManager.getInstance();
                    if (null != apiManager) {
                        try {
                            image =
                                    android.provider.MediaStore.Images.Media.getBitmap(apiManager
                                            .getContext().getContentResolver(), (Uri)mKey);
                        } catch (FileNotFoundException e) {
                            e.printStackTrace();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    } else {
                        Logger.e(TAG, "doInBackground() ApiManager is null");
                    }
                } else {
                    Logger.w(TAG, "doInBackground() the file " + mKey + " has already been loaded");
                }
            }
            Logger.v(TAG, "load " + mKey + " finish");
            Logger.d(TAG, "doInBackground(), iamge is " + image);
            return image;
        }
    }

    /**
     * This class is used to parse image from each participant image ,it is
     * combined by three sub image.
     */
    private static class LoadParticipantsImageTask extends AsyncTask<Void, Void, List<Bitmap>> {

        public static final String TAG = "LoadParticipantsImageTask";

        private static final int PARTICIPANT_NUM_ZERO = 0;
        private static final int PARTICIPANT_NUM_ONE = 1;
        private static final int PARTICIPANT_NUM_TWO = 2;
        private static final int PARTICIPANT_NUM_THREE = 3;

        private static final Handler MAIN_THREAD_HANDLER = new Handler(Looper.getMainLooper());

        private static final CopyOnWriteArrayList<LoadParticipantsImageTask> TASK_LIST =
                new CopyOnWriteArrayList<LoadParticipantsImageTask>();

        private static final ConcurrentHashMap<TreeSet<?>, LoadParticipantsImageTask> TASK_MAP =
                new ConcurrentHashMap<TreeSet<?>, LoadParticipantsImageTask>();

        public static void interrupt(TreeSet<String> key) {
            Logger.d(TAG, "interrupt() called the key is " + key);
            LoadParticipantsImageTask task = TASK_MAP.get(key);
            if (task != null) {
                Status status = task.getStatus();
                switch (status) {
                    case PENDING:
                    case FINISHED:
                        TASK_MAP.remove(key);
                        TASK_LIST.remove(task);
                        Logger.d(TAG, "interrupt() the image " + key + " is " + status
                                + ", remove it from LIST and MAP");
                        break;
                    case RUNNING:
                        task.mIsActive = false;
                        Logger.d(TAG, "interrupt() the image " + key
                                + " is running, just make it disabled");
                        break;
                    default:
                        break;
                }
            } else {
                Logger.e(TAG, "interrupt() TASK_MAP doesn't contain the key " + key);
            }
        }

        private OnLoadImageFinishListener mListener = null;
        public TreeSet<String> mKey = null;
        private boolean mIsActive = true;

        private LoadParticipantsImageTask(TreeSet<String> key, OnLoadImageFinishListener listener) {
            mListener = listener;
            mKey = key;
            if (null != mListener) {
                Logger.d(TAG, "Constructor() key is " + key
                        + " and listener is not null, it's an async task");
                MAIN_THREAD_HANDLER.post(new Runnable() {
                    @Override
                    public void run() {
                        TASK_LIST.add(LoadParticipantsImageTask.this);
                        TASK_MAP.put(mKey, LoadParticipantsImageTask.this);
                        if (1 == TASK_LIST.size()) {
                            LoadParticipantsImageTask.this
                                    .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                        }
                    }
                });
            } else {
                Logger.d(TAG, "Constructor() key is " + key
                        + " and listener is null, it's a sync task");
            }
        }

        private Bitmap resizeImage(Bitmap bitmap, int w, int h, boolean needRecycle) {
            Logger.d(TAG, "resizeImage() entry ");
            if (null == bitmap) {
                return null;
            }
            Bitmap bitmapOrg = bitmap;
            int width = bitmapOrg.getWidth();
            int height = bitmapOrg.getHeight();
            int newWidth = w;
            int newHeight = h;

            float scaleWidth = ((float) newWidth) / width;
            float scaleHeight = ((float) newHeight) / height;
            Matrix matrix = new Matrix();
            matrix.postScale(scaleWidth, scaleHeight);

            Bitmap resizedBitmap = Bitmap
                    .createBitmap(bitmapOrg, 0, 0, width, height, matrix, true);
            if (needRecycle && !bitmapOrg.isRecycled()) {
                bitmapOrg.recycle();
            }
            return resizedBitmap;
        }

        private Bitmap processBitmaps(List<Bitmap> bitMapList) {
            Logger.d(TAG, "processBitmaps() entry");
            bitMapList = fillBitmapList(mKey.size(), bitMapList);
            Bitmap one = bitMapList.get(PARTICIPANT_NUM_ZERO);
            Bitmap two = bitMapList.get(PARTICIPANT_NUM_ONE);
            Bitmap three = bitMapList.get(PARTICIPANT_NUM_TWO);
            if (one == null || two == null || three == null) {
                Logger.e(TAG, "processBitmaps() one/two/three is/are invalid!");
                return null;
            }

            if (one.getWidth() < 96 || one.getHeight() < 96) {
                one = resizeImage(one, 96, 96, false);
            }

            int block = one.getWidth() / 16;

            one = Bitmap.createBitmap(one, block * 4, 0, block * 9, one.getHeight());
            two = resizeImage(two, block * 7, one.getHeight() / 2, false);
            three = resizeImage(three, block * 7, one.getHeight() / 2, false);

            Bitmap newbmp = Bitmap.createBitmap(one.getWidth() + 1 + two.getWidth(),
                    one.getHeight(), Config.ARGB_8888);
            Canvas canvas = new Canvas(newbmp);
            canvas.drawBitmap(one, 0, 0, null);
            canvas.drawBitmap(two, one.getWidth() + 1, 0, null);
            canvas.drawBitmap(three, one.getWidth() + 1, two.getHeight() + 1, null);
            canvas.save(Canvas.ALL_SAVE_FLAG);
            canvas.restore();
            one.recycle();
            two.recycle();
            three.recycle();
            return newbmp;
        }

        private static List<Bitmap> fillBitmapList(int count, List<Bitmap> bitmapList) {
            Logger.d(TAG, "fillBitmapList() entry count is " + count);
            if (count < 0 || bitmapList == null) {
                Logger.e(TAG, "fillBitmapList() parm is error");
                return null;
            }
            int size = bitmapList.size();
            Logger.d(TAG, "fillBitmapList() bitmapList size is " + size);
            switch (count) {
                case PARTICIPANT_NUM_ZERO:
                    if (size == PARTICIPANT_NUM_ZERO) {
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(BLANK_BITMAP);
                        bitmapList.add(BLANK_BITMAP);
                    } else {
                        Logger.e(TAG, "fillBitmapList() count==0, size>0");
                        return null;
                    }
                    break;
                case PARTICIPANT_NUM_ONE:
                    if (size == PARTICIPANT_NUM_ZERO) {
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(BLANK_BITMAP);
                    } else if (size == PARTICIPANT_NUM_ONE) {
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(BLANK_BITMAP);
                    } else {
                        Logger.e(TAG, "fillBitmapList() count==1, size>1");
                        return null;
                    }
                    break;
                case PARTICIPANT_NUM_TWO:
                    if (size == PARTICIPANT_NUM_ZERO) {
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(DEFAULT_BITMAP);
                    } else if (size == PARTICIPANT_NUM_ONE) {
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(DEFAULT_BITMAP);
                    } else if (size == PARTICIPANT_NUM_TWO) {
                        bitmapList.add(DEFAULT_BITMAP);
                    } else {
                        Logger.e(TAG, "fillBitmapList() count==2, size>2");
                        return null;
                    }
                    break;
                default:
                    Logger.d(TAG, "fillBitmapList() count is " + count);
                    if (size == PARTICIPANT_NUM_ZERO) {
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(DEFAULT_BITMAP);
                    } else if (size == PARTICIPANT_NUM_ONE) {
                        bitmapList.add(DEFAULT_BITMAP);
                        bitmapList.add(DEFAULT_BITMAP);
                    } else if (size == PARTICIPANT_NUM_TWO) {
                        bitmapList.add(DEFAULT_BITMAP);
                    } else if (size == PARTICIPANT_NUM_THREE) {
                        Logger.d(TAG, "fillBitmapList() count>=3, size=3");
                    } else {
                        Logger.e(TAG, "fillBitmapList() count>=3, size>3");
                        return null;
                    }
                    break;
            }
            return bitmapList;
        }

        @Override
        protected void onPostExecute(List<Bitmap> result) {
            Logger.v(TAG, "onPostExecute() entry: the image is " + mKey);
            super.onPostExecute(result);
            if (mIsActive) {
                if (null != mListener) {
                    Bitmap prossedMap = processBitmaps(result);
                    Logger.d(TAG, "onPostExecute the mKey is prossedMap is " + mKey + prossedMap);
                    getInstance().setImage(mKey, prossedMap);
                    mListener.onLoadImageFished(prossedMap);
                } else {
                    Logger.e(TAG, "onPostExecute() result is " + result + " listener is "
                            + mListener);
                }
            } else {
                Logger.d(TAG, "onPostExecute() the loading " + mKey + "has been disabled");
            }
            TASK_MAP.remove(mKey);
            TASK_LIST.remove(this);
            int size = TASK_LIST.size();
            if (size > 0) {
                Logger.v(TAG, "onPostExecute() there is still " + size + " waiting images to load");
                TASK_LIST.get(0).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
            } else {
                Logger.v(TAG, "onPostExecute() there is not any waiting images to load");
            }
        }

        private Bitmap parsePhoto(Cursor cursor) {
            Logger.d(TAG, "parsePhoto() entry");
            Bitmap avatar = null;
            if (cursor.moveToFirst()) {
                Long id = cursor.getLong(cursor.getColumnIndex(Contacts._ID));
                String photoUriString = cursor.getString(cursor.getColumnIndex(Contacts.PHOTO_URI));
                if (photoUriString != null) {
                    Uri photoUri = null;
                    photoUri = Uri.parse(photoUriString);
                    if (INSTANCE.isImageLoaded(photoUri)) {
                        Logger.d(TAG, "parsePhoto() the image " + photoUri
                                + " has already been loaded, just return it");
                        return INSTANCE.getExistingImage(photoUri);
                    } else {
                        ApiManager apiManager = ApiManager.getInstance();
                        if (apiManager != null) {
                            try {
                                avatar = android.provider.MediaStore.Images.Media.getBitmap(
                                        apiManager.getContext().getContentResolver(), photoUri);
                            } catch (FileNotFoundException e) {
                                e.printStackTrace();
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        } else {
                            Logger.e(TAG, "parsePhoto() the apiManager is null");
                        }
                    }
                } else {
                    Logger.e(TAG, "parsePhoto(), photoUriString is null ");
                }
                Logger.d(TAG, "parsePhoto(),the id is " + id);
            } else {
                Logger.d(TAG, "parsePhoto(), cursor.moveToFirst() is false");
            }
            return avatar;
        }

        @Override
        protected List<Bitmap> doInBackground(Void... params) {
            Logger.v(TAG, "doInBackground() Begin to load image: " + mKey);
            ArrayList<Bitmap> bitMapList = new ArrayList<Bitmap>();
            Context context = ApiManager.getInstance().getContext();
            ContentResolver contentResolver = context.getContentResolver();
            if (mKey != null) {
                Bitmap bitMap = null;
                for (String number : mKey) {
                    Uri uri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode(number));
                    Cursor cursor = contentResolver.query(uri, PROJECTION, null, null,
                            Contacts.SORT_KEY_PRIMARY);
                    if (cursor != null) {
                        try {
                            cursor.getCount();
                            bitMap = parsePhoto(cursor);
                            if (bitMap != null) {
                                bitMapList.add(bitMap);
                            } else {
                                Logger.d(TAG, "doInBackground() the bitMap is null");
                            }
                        } finally {
                            cursor.close();
                        }
                    } else {
                        Logger.e(TAG, "doInBackground() cursor is null");
                    }
                    int size = bitMapList.size();
                    if (size == PARTICIPANT_NUM_THREE) {
                        Logger.d(TAG, "doInBackground() the size is " + PARTICIPANT_NUM_THREE);
                        break;
                    }
                    Logger.d(TAG, "doInBackground() the size is " + size);
                }
                return bitMapList;
            } else {
                Logger.e(TAG, "doInBackground() the mKey is null");
                return bitMapList;
            }
        }
    }
}
