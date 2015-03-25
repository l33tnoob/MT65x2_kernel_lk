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

import android.content.Context;
import android.content.CursorLoader;
import android.content.Loader;
import android.content.Loader.OnLoadCompleteListener;
import android.database.Cursor;
import android.os.AsyncTask;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;

import com.mediatek.rcse.api.Logger;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;


public class PhotoLoaderManager implements OnLoadCompleteListener<Cursor> {
    private static final String TAG = "PhotoLoaderManager";
    private static volatile PhotoLoaderManager sInstance = null;

    static final String[] PROJECTION =
            {
                    Data._ID, Data.MIMETYPE, Data.DATA1, Data.DATA2, Data.DISPLAY_NAME,
                    Contacts.SORT_KEY_PRIMARY
            };
    private static final long LOAD_THROTTLE = 3000;

    private static final List<OnPhotoChangedListener> LISTENER_LIST =
            new CopyOnWriteArrayList<OnPhotoChangedListener>();

    public static void initialize(Context context) {
        if (null != sInstance) {
            return;
        } else {
            sInstance = new PhotoLoaderManager(context);
        }
    }

    public static void addListener(OnPhotoChangedListener listener) {
        Logger.d(TAG, "addListener() entry listener is " + listener);
        LISTENER_LIST.add(listener);
        Logger.d(TAG, "addListener() exit LISTENER_LIST is " + LISTENER_LIST + "size is " + LISTENER_LIST.size());
    }

    public static void removeListener(OnPhotoChangedListener listener) {
        Logger.d(TAG, "removeListener() entry listener is " + listener);
        LISTENER_LIST.remove(listener);
        Logger.d(TAG, "removeListener() entry LISTENER_LIST is " + LISTENER_LIST);
    }

    private CursorLoader mLoader = null;

    private PhotoLoaderManager(Context context) {
        mLoader = new CursorLoader(context, Data.CONTENT_URI, PROJECTION, null, null, null);
        mLoader.setUpdateThrottle(LOAD_THROTTLE);
        mLoader.registerListener(0, this);
        mLoader.startLoading();
    }

    @Override
    public void onLoadComplete(Loader<Cursor> loader, Cursor data) {
        Logger.d(TAG, "onLoadComplete() entry");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                for (OnPhotoChangedListener listener : LISTENER_LIST) {
                    Logger.d(TAG, "onLoadComplete the listener is " + listener);
                    listener.onPhotoChanged();
                }
            }
        });
    }

    public interface OnPhotoChangedListener {
        void onPhotoChanged();
    }
}
