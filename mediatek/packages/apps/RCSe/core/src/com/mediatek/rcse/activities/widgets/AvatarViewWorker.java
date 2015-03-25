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

package com.mediatek.rcse.activities.widgets;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.ContactsContract.Contacts;
import android.text.TextUtils;
import android.util.Log;
import android.widget.ImageView;
import android.widget.QuickContactBadge;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.service.ImageLoader.OnLoadImageFinishListener;

import com.orangelabs.rcs.R;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;

public class AvatarViewWorker implements PhotoLoaderManager.OnPhotoChangedListener {

    public static final String TAG = "AvatarViewWorker";

    private static final Handler UI_HANDLER = new Handler(Looper.getMainLooper());

    private static final String[] PROJECTION = new String[] {
            Contacts._ID, Contacts.LOOKUP_KEY, Contacts.PHOTO_URI
    };
    private QueryHandler mCurrentQueryHandler = null;
    private String mCurrentNumber = null;
    private static final Map<String, WeakReference<Bitmap>> DEFAULT_AVATAR_MAP =
            new ConcurrentHashMap<String, WeakReference<Bitmap>>();
    private Context mContext;
    private ImageView mImage = null;
    private QuickContactBadge mBadge = null;
    private AsyncListener mCurrentListener = null;
    private TreeSet<String> mNumberList = new TreeSet<String>();

    private class AsyncListener implements OnLoadImageFinishListener {
        private Object mKey = null;
        private boolean mIsEnabled = true;

        AsyncListener(Object key) {
            mKey = key;
        }

        @Override
        public void onLoadImageFished(Bitmap image) {
            Logger.d(TAG, "onLoadImageFished() entry");
            if (mIsEnabled) {
                mImage.setImageBitmap(image);
            } else {
                Logger.d(TAG, "onLoadImageFished() mIsEnabled" + mIsEnabled);
            }
            if (this == mCurrentListener) {
                mCurrentListener = null;
            }
        }

        /**
         * Used to disable query data
         */
        public void destroy() {
            ImageLoader.interrupt(mKey);
        }

        /**
         * Used to disable query result
         */
        public void disable() {
            Logger.d(TAG, "disable() entry!");
            mIsEnabled = false;
        }
    }

    private class QueryHandler extends AsyncQueryHandler {

        private static final String TAG = "QueryHandler";
        private ContentResolver mResolver = null;
        private boolean mIsEnabled = true;
        private String mNumber = null;
        private static final int EVENT_ARG_QUERY = 1;

        public QueryHandler(Context context, String number) {
            super(context.getContentResolver());
            mNumber = number;
            mResolver = context.getContentResolver();
        }

        /**
         * Used to disable query data
         */
        public void disable() {
            Logger.d(TAG, "disable entry!");
            mIsEnabled = false;
        }

        /**
         * Extend the WorkerHandler to load image after query the contact uri
         */
        protected class ExtWorkerHandler extends WorkerHandler {
            private static final String TAG = "ExtWorkerHandler";

            public ExtWorkerHandler(Looper looper) {
                super(looper);
            }

            @Override
            public void handleMessage(Message msg) {
                final ContentResolver resolver = mResolver;
                if (resolver == null) {
                    Logger.e(TAG, "handleMessage() mResolver is null");
                    return;
                }
                WorkerArgs args = (WorkerArgs) msg.obj;

                int token = msg.what;
                int event = msg.arg1;
                if (EVENT_ARG_QUERY == event) {
                    Cursor cursor = null;
                    cursor = resolver.query(args.uri, args.projection, args.selection, args.selectionArgs, args.orderBy);
                    // Calling getCount() causes the cursor window to be
                    // filled,
                    // which will make the first access on the main
                    // thread a lot faster.
                    if (cursor != null) {
                        cursor.getCount();
                        parsePhoto(cursor);
                    } else {
                        Logger.e(TAG, "handleMessage() cursor is null");
                    }
                    args.result = cursor;
                } else {
                    Logger.e(TAG, "handleMessage() unsupported event received: " + event);
                    return;
                }

                // passing the original token value back to the caller
                // on top of the event values in arg1.
                Message reply = args.handler.obtainMessage(token);
                reply.obj = args;
                reply.arg1 = msg.arg1;

                Log.d(TAG, "WorkerHandler.handleMsg: msg.arg1=" + msg.arg1 + ", reply.what="
                        + reply.what);

                reply.sendToTarget();
            }

        }

        private void parsePhoto(Cursor cursor) {
            Bitmap avatar = null;
            if (cursor.moveToFirst()) {
                Long id = cursor.getLong(cursor.getColumnIndex(Contacts._ID));
                String key = cursor.getString(cursor.getColumnIndex(Contacts.LOOKUP_KEY));
                String photoUriString = cursor.getString(cursor.getColumnIndex(Contacts.PHOTO_URI));
                if (photoUriString != null) {
                    Uri photoUri = null;
                    photoUri = Uri.parse(photoUriString);
                    avatar = ImageLoader.requestImage(photoUri);
                } else {
                    Logger.e(TAG, "parsePhotoUri(), photoUriString is null, number is " + mNumber);
                }
                Logger.d(TAG, "parsePhotoUri(),the id is " + id + " and the key is " + key);
            } else {
                Logger.d(TAG, "parsePhotoUri(), cursor.moveToFirst() is false");
            }
            final Bitmap realAvatar = avatar;
            UI_HANDLER.post(new Runnable() {
                @Override
                public void run() {
                    if (mIsEnabled) {
                        onLoadAvatarComplete(realAvatar);
                    } else {
                        Logger
                                .d(TAG,
                                        "parsePhoto, UI Handler, mIsEnabled is false, no need to update view!");
                    }
                }
            });
        }

        @Override
        protected Handler createHandler(Looper looper) {
            Logger.d(TAG, "createHandler() create ExtWorkerHandler");
            return new ExtWorkerHandler(looper);
        }

        protected void onLoadAvatarComplete(Bitmap avatar) {
            if (mIsEnabled) {
                Logger.d(TAG, "onLoadAvatarComplete() set avatar for " + mNumber);
                if (null != avatar) {
                    Logger.d(TAG, "onLoadAvatarComplete() avatar is null,"
                            + " set to default, number is " + mNumber);
                    mImage.setImageBitmap(avatar);
                    DEFAULT_AVATAR_MAP.put(mNumber, new WeakReference<Bitmap>(avatar));
                    checkDefaultAvatar();
                } else {
                    Logger.d(TAG, "onLoadAvatarComplete, avatar is null!");
                    mImage.setImageResource(R.drawable.default_header);
                    DEFAULT_AVATAR_MAP.remove(mNumber);
                }
            } else {
                Logger.e(TAG, "onLoadAvatarComplete() mCurrentQueryHandler has been reset, "
                        + "discard avatar for " + mNumber);
            }
        }

        private void checkDefaultAvatar() {
            Collection<WeakReference<Bitmap>> bitmapReferences = DEFAULT_AVATAR_MAP.values();
            ArrayList<WeakReference<Bitmap>> toBeDeletedArrayList = null;
            for (WeakReference<Bitmap> bitmapReference : bitmapReferences) {
                if (bitmapReference.get() == null) {
                    if (toBeDeletedArrayList == null) {
                        toBeDeletedArrayList = new ArrayList<WeakReference<Bitmap>>();
                    }
                    toBeDeletedArrayList.add(bitmapReference);
                }
            }
            if (null != toBeDeletedArrayList) {
                bitmapReferences.removeAll(toBeDeletedArrayList);
            } else {
                Logger.d(TAG, "checkDefaultAvatar, toBeDeletedArrayList is null!");
            }
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            if (mIsEnabled) {
                if (cursor != null) {
                    new AssignContactTask().execute(cursor);
                } else {
                    Logger.e(TAG, "onQueryComplete(), cursor is null");
                }
            } else {
                if (cursor != null) {
                    cursor.close();
                }
                Logger.e(TAG, "onQueryComplete() mCurrentQueryHandler has been reset, "
                        + "discard current query for " + mNumber);
            }

        }

        private class AssignContactTask extends AsyncTask<Cursor, Void, Uri> {
            @Override
            protected Uri doInBackground(Cursor... params) {
                Cursor cursor = params[0];
                try {
                    if (mIsEnabled) {
                        if (cursor.moveToFirst()) {
                            Long id = cursor.getLong(cursor.getColumnIndex(Contacts._ID));
                            String key =
                                    cursor.getString(cursor.getColumnIndex(Contacts.LOOKUP_KEY));
                            Uri uri = Contacts.getLookupUri(id, key);
                            Logger.d(TAG, "doInBackground(),the id is " + id + " and the key is "
                                    + key + ",the contactUri is " + uri);
                            return uri;
                        } else {

                            Logger.d(TAG, "doInBackground(), it's a stranger! number = " + mNumber);
                            return null;
                        }
                    } else {
                        Logger.e(TAG, "doInBackground() mCurrentQueryHandler has been reset, "
                                + "discard current query for " + mNumber);
                        return null;
                    }
                } finally {
                    if (cursor != null) {
                        cursor.close();
                    }
                }
            }

            @Override
            protected void onPostExecute(Uri result) {
                if (mIsEnabled) {
                    if (null != result) {
                        Logger.d(TAG, "onPostExecute() result is " + result);
                        if (mBadge != null) {
                            mBadge.assignContactUri(result);
                        } else {
                            Logger.d(TAG, "onPostExecute(), mBadge is null!");
                        }
                    } else {
                        Logger.d(TAG, "onPostExecute() result is null, assign the number "
                                + mNumber);
                        if (mBadge != null) {
                            mBadge.assignContactFromPhone(mNumber, true);
                        } else {
                            Logger.d(TAG, "onPostExecute(), mBadge is null!");
                        }
                    }
                    mCurrentNumber = mNumber;
                    if (mCurrentQueryHandler == QueryHandler.this) {
                        mCurrentQueryHandler = null;
                    } else {
                        Logger.d(TAG, "onPostExecute, mCurrentQueryHandler is changed!");
                    }
                } else {
                    Logger.e(TAG, "onPostExecute() mCurrentQueryHandler has been reset, "
                            + "discard current query for " + mNumber);
                }
            }
        }
    }

    public AvatarViewWorker(Context context, ImageView imageView) {
        mContext = context;
        mImage = imageView;
        PhotoLoaderManager.initialize(context);
    }

    public AvatarViewWorker(Context context, QuickContactBadge quickContactBadge) {
        mContext = context;
        mBadge = quickContactBadge;
        mImage = mBadge;
        PhotoLoaderManager.initialize(context);
    }

    /**
     * Called when attach to window
     */
    public void onAttachedToWindow() {
        Logger.d(TAG, "onAttachedToWindow() entry");
        PhotoLoaderManager.addListener(this);
    }

    /**
     * Called when detach from window
     */
    public void onDetachedFromWindow() {
        Logger.d(TAG, "onDetachedFromWindow() entry");
        PhotoLoaderManager.removeListener(this);
    }

    /**
     * Use the contact number to get photo uri
     * 
     * @param number The number or this contact
     */
    public void setAsyncContact(String number) {
        Logger.d(TAG, "setAsyncContact enter! number is " + number);
        if (!TextUtils.isEmpty(number)) {
            if (null != mCurrentQueryHandler) {
                if (number.equals(mCurrentQueryHandler.mNumber)) {
                    Logger.d(TAG, "setAsyncContact, number is equals current query number!");
                    return;
                } else {
                    mCurrentQueryHandler.disable();
                    mCurrentQueryHandler = null;
                }
            } else {
                Logger.d(TAG, "setAsyncContact, mCurrentQueryHandler is null!");
            }
            WeakReference<Bitmap> tempDefault = DEFAULT_AVATAR_MAP.get(number);
            if (tempDefault != null && null != tempDefault.get()) {
                Logger.d(TAG, "set the before avatar default!");
                mImage.setImageBitmap(tempDefault.get());
            } else {
                Logger.d(TAG, "set the default header!");
                mImage.setImageResource(R.drawable.default_header);
            }
            QueryHandler queryHandler = new QueryHandler(mContext, number);
            mCurrentQueryHandler = queryHandler;
            Uri uri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri.encode(number));
            queryHandler
                    .startQuery(0, null, uri, PROJECTION, null, null, Contacts.SORT_KEY_PRIMARY);
        } else {
            Logger.e(TAG, "setAsyncContact, number is null!");
        }
    }

    /**
     * Use the contact numbers to get group image
     * 
     * @param numbers The number or the contacts
     */
    public void setAsyncContact(TreeSet<String> numbers) {
        Logger.d(TAG, "setAsyncContact enter! numbers is " + numbers);
        mNumberList = numbers;
        Bitmap bitMap = requestImage();
        if (bitMap != null) {
            mImage.setImageBitmap(bitMap);
        } else {
            Logger.d(TAG, "setAsyncContact bitMap is null");
        }
    }
    
    private Bitmap requestImage() {
        Logger.d(TAG, "requestImage() entry mKey is " + mNumberList);
        if (null != mCurrentListener && !mCurrentListener.mKey.equals(mNumberList)) {
            mCurrentListener.disable();
            mCurrentListener.destroy();
        }
        mCurrentListener = new AsyncListener(mNumberList);
        return ImageLoader.requestImage(mNumberList, mCurrentListener);
    }

    @Override
    public void onPhotoChanged() {
        UI_HANDLER.post(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "onPhotoChanged() entry");
                if (null != mCurrentNumber) {
                    setAsyncContact(mCurrentNumber);
                } else if (mNumberList != null && !mNumberList.isEmpty()) {
                    setAsyncContact(mNumberList);
                } else {
                    Logger.e(TAG, "onPhotoChanged() mCurrentNumber and mKey is null");
                }
            }
        });
    }
}

