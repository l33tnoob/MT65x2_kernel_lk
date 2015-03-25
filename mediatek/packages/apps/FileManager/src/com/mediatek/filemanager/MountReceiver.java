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

package com.mediatek.filemanager;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;

import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.OptionsUtils;

import java.util.ArrayList;


public class MountReceiver extends BroadcastReceiver {
    private static final String TAG = "MountReceiver";

    private final MountPointManager mMountPointManager;
    private final ArrayList<MountListener> mMountListenerList = new ArrayList<MountListener>();
    private static final String INTENT_SD_SWAP = "com.mediatek.SD_SWAP";

    public interface MountListener {
        /**
         * This method will be called to do things before MountPoint init.
         */
        // void prepareForMount(String mountPoint);

        /**
         * This method will be called when receive a mounted intent.
         */
        void onMounted(String mountPoint);

        /**
         * This method will be implemented by its class who implements this
         * interface, and called when receive a unMounted intent.
         * 
         * @param mountPoint the path of mount point
         */
        void onUnMounted(String unMountPoint);

        /**
         * This method cancel the current action on the SD card which will be
         * unmounted.
         */
        void onEjected(String unMountPoint);

        /**
         * This method re-load volume info when sd swap on/off
         * 
         */
        void onSdSwap();
    }

    /**
     * This method gets MountPointManager's instance
     */
    public MountReceiver() {
        mMountPointManager = MountPointManager.getInstance();
    }

    /**
     * This method adds listener for activities
     * 
     * @param listener listener of certain activity to respond mounted and
     *            unMounted intent
     */
    public void registerMountListener(MountListener listener) {
        mMountListenerList.add(listener);
    }

    public void unregisterMountListener(MountListener listener) {
        mMountListenerList.remove(listener);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        String mountPoint = null;
        Uri mountPointUri = intent.getData();
        if (mountPointUri != null) {
            mountPoint = mountPointUri.getPath();
        }
        LogUtils.d(TAG, "onReceive: " + action + " mountPoint: " + mountPoint);
        if (INTENT_SD_SWAP.equals(action)) {
            synchronized (this) {
                for (MountListener listener : mMountListenerList) {
                    LogUtils.d(TAG, "onReceive,handle SD_SWAP ");
                    listener.onSdSwap();
                }
            }
        }

        if (mountPoint == null || mountPointUri == null) {
            return;
        }

        if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
            // cancel the current operation before MountPointManager init
            // to avoid concurrently access to mountpoint arraylist.{
            for (MountListener listener : mMountListenerList) {
                listener.onMounted(mountPoint);
            }
        } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
            if (mMountPointManager.changeMountState(mountPoint, false)) {
                for (MountListener listener : mMountListenerList) {
                    listener.onUnMounted(mountPoint);
                }
            }
        } else if (Intent.ACTION_MEDIA_EJECT.equals(action)) {
            for (MountListener listener : mMountListenerList) {
                listener.onEjected(mountPoint);
            }
        }
    }

    /**
     * Register a MountReceiver for context. See
     * {@link Intent.ACTION_MEDIA_MOUNTED} {@link Intent.ACTION_MEDIA_UNMOUNTED}
     * 
     * @param context Context to use
     * @return A mountReceiver
     */
    public static MountReceiver registerMountReceiver(Context context) {
        MountReceiver receiver = new MountReceiver();

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addDataScheme("file");
        context.registerReceiver(receiver, intentFilter);

        if (OptionsUtils.isMtkSDSwapSurpported()) {
           IntentFilter intentFilterSDSwap = new IntentFilter();
           intentFilterSDSwap.addAction(INTENT_SD_SWAP);
           context.registerReceiver(receiver, intentFilterSDSwap);
        }
        return receiver;
    }
}
