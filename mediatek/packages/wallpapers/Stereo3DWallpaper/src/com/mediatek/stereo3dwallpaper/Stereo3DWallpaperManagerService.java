/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.stereo3dwallpaper;

import android.app.Service;
import android.app.WallpaperManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.FileUtils;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.view.Display;
import android.view.WindowManager;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class Stereo3DWallpaperManagerService extends Service {
    private static final String TAG = "Stereo3DWallpaperManagerService";
    private static final int SET_WALLPAPER = 101;
    private static final int RESULT_OK = 102;
    private static final int RESULT_ERROR = 103;
    private static final String WALLPAPER_SERVICE_NAME
        = "com.mediatek.stereo3dwallpaper.Stereo3DPairWallpaperService";
    private static final File WALLPAPER_DIR
        = new File("/data/data/com.mediatek.stereo3dwallpaper/files");
    private static final String LEFT_WALLPAPER = "leftwallpaper";
    private static final String RIGHT_WALLPAPER = "rightwallpaper";
    protected static final File LEFT_WALLPAPER_FILE = new File(WALLPAPER_DIR, LEFT_WALLPAPER);
    protected static final File RIGHT_WALLPAPER_FILE = new File(WALLPAPER_DIR, RIGHT_WALLPAPER);
    protected static final String VISIBILITY_CHANGED
        = "com.mediatek.stereo3dwallpaper.ACTION_VISIBILITY_CHANGED";
    private Bitmap[] mPair = new Bitmap[2]; // left and right bitmaps

    // Handler of incoming messages from clients
    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Stereo3DLog.log(TAG, "Handle message: " + msg.what);

            if (msg.what == SET_WALLPAPER) {
                if (msg.replyTo != null) {
                    Uri uri = (Uri)msg.obj;

                    if (uri == null) {
                        sendResult(msg.replyTo, RESULT_ERROR);
                    } else {
                        setWallpaper(msg.replyTo, uri);
                    }
                }
            }
        }
    }

    final Messenger mMessenger = new Messenger(new IncomingHandler());

    @Override
    public IBinder onBind(Intent intent) {
        return mMessenger.getBinder();
    }

    @Override
    public void onDestroy() {
        Stereo3DLog.log(TAG, "onDestroy");
        release();
        super.onDestroy();
    }

    /**
     * The method releases all bitmap resources
     *
     */
    private void release() {
        for (int i = 0; i < mPair.length; i++) {
            if (mPair[i] != null) {
                mPair[i].recycle();
                mPair[i] = null;
            }
        }
    }

    /**
     * This method sets the wallpaper
     *
     * @param messenger the messenger used to communicate with the client
     * @param uri the URI representing the file
     */
    private void setWallpaper(Messenger messenger, Uri uri) {
        Stereo3DLog.log(TAG, "Set wallpaper by uri: " + uri.toString());

        setScreenDimension();
        int result = decodeFile(uri);

        if (result == RESULT_OK && mPair[0] != null && mPair[1] != null) {
            savePairBitmaps();
            setWallpaperService();
        }

        sendResult(messenger, result);
    }

    /**
     * This method sets the current screen dimension to the utility class
     */
    private void setScreenDimension() {
        WindowManager window = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        Display display = window.getDefaultDisplay();
        Stereo3DUtility.setScreenDimension(display.getWidth(), display.getHeight());
    }

    /**
     * This method decodes the URI and creates left and right bitmaps
     * @param uri the given URI to be decoded
     */
    private int decodeFile(Uri uri) {
        String mimeType = Stereo3DUtility.getMimeType(getContentResolver(), uri);
        Stereo3DLog.log(TAG, "MimeType: " + mimeType);

        if ("image/mpo".equalsIgnoreCase(mimeType)) {
            Stereo3DUtility.decodeMpoUri(getContentResolver(), uri, mPair);
        } else if ("image/jps".equalsIgnoreCase(mimeType) || "image/x-jps".equalsIgnoreCase(mimeType)) {
            Stereo3DUtility.decodeJpsUri(getContentResolver(), uri, mPair);
        } else {
            return RESULT_ERROR;
        }

        return RESULT_OK;
    }

    /**
     * This method saves left and right bitmaps into the internal storage
     */
    private void savePairBitmaps() {
        try {
            saveBitmap(LEFT_WALLPAPER_FILE, mPair[0]);
            saveBitmap(RIGHT_WALLPAPER_FILE, mPair[1]);
        } catch (IOException e) {
            Stereo3DLog.log(TAG, "IOException - Failed to save pair bitmaps");
        }
    }

    /**
     * This method saves a bitmap into the internal storage
     *
     * @param file the file to be opened
     * @param bitmap the bitmap to be saved
     */
    private void saveBitmap(File file, Bitmap bitmap) throws IOException {
        ParcelFileDescriptor fd = openFile(file);

        if (fd == null) {
            return;
        }

        FileOutputStream fos = null;

        try {
            fos = new ParcelFileDescriptor.AutoCloseOutputStream(fd);
            bitmap.compress(Bitmap.CompressFormat.PNG, 90, fos);
        } finally {
            if (fos != null) {
                fos.close();
            }

            fd.close();
        }
    }

    /**
     * The method creates a new ParcelFileDescriptor accessing a given file
     *
     * @param file the file to be opened
     * @return a new ParcelFileDescriptor pointing to the given file
     */
    private ParcelFileDescriptor openFile(File file) {
        try {
            if (!WALLPAPER_DIR.exists()) {
                Stereo3DLog.log(TAG, "WALLPAPER_DIR does not exist, create it");

                boolean success = WALLPAPER_DIR.mkdir();

                if (!success) {
                    Stereo3DLog.log(TAG, "Fail to create WALLPAPER_DIR");
                    return null;
                }

                FileUtils.setPermissions(WALLPAPER_DIR.getPath(),
                                         FileUtils.S_IRWXU | FileUtils.S_IRWXG | FileUtils.S_IXOTH, -1, -1);
            }

            ParcelFileDescriptor fd = ParcelFileDescriptor.open(file,
                                      ParcelFileDescriptor.MODE_CREATE | ParcelFileDescriptor.MODE_READ_WRITE);

            return fd;
        } catch (FileNotFoundException e) {
            Stereo3DLog.log(TAG, "FileNotFoundException - Cannot open a file");
        }

        return null;
    }

    /**
     * The method sets the wallpaper service to notify WallpaperManagerService
     */
    private void setWallpaperService() {
        Stereo3DLog.log(TAG, "Set wallpaper service");

        try {
            WallpaperManager wallpaperManager = WallpaperManager.getInstance(this);
            ComponentName wallpaperComponent = new ComponentName("com.mediatek.stereo3dwallpaper",
                    Stereo3DPairWallpaperService.class.getName());

            wallpaperManager.getIWallpaperManager().setWallpaperComponent(wallpaperComponent);
        } catch (RemoteException e) {
            Stereo3DLog.log(TAG, "RemoteException - Failed to set wallpaper service");
        }
    }

    /**
     * The method sends the result back to the client
     *
     * @param the messenger used to communicate with the client
     */
    private void sendResult(Messenger messenger, int result) {
        Message msg = Message.obtain(null, result);
        try {
            messenger.send(msg);
        } catch (RemoteException e) {
            Stereo3DLog.log(TAG, "RemoteException - Failed to send the result back to the client");
        }
    }
}
