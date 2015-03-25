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

package com.mediatek.bluetooth.util;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.net.Uri;
import android.os.Environment;
import android.os.PowerManager;
import android.os.StatFs;
import android.os.SystemProperties;
import android.os.Vibrator;
import android.os.PowerManager.WakeLock;
import android.os.storage.StorageManager;
import com.mediatek.storage.StorageManagerEx;

import com.mediatek.activity.MessageActivity;
import com.mediatek.bluetooth.BluetoothReceiver;
import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.R;

public class SystemUtils {

    // constants for file manager operations
    // private static final Uri FM_CONTENT_URI = Uri.parse( "content://com.mediatek.filemanager.provider" );
    // private static final String FM_KEY_PATH = "path";
    private static final String FM_SELECTION = "Received File";

    private static final String SDCARD = "sdcard";

    /**
     * get "Received File" path from file-manager and return default config (/sdcard/bluetooth) if error occurred
     * 
     * @param context
     * @return
     */
    public static String getReceivedFilePath(Context context) {

        // query file-manager
        /*
         * Cursor cursor = context.getContentResolver().query( FM_CONTENT_URI, new String[]{ FM_KEY_PATH }, FM_SELECTION,
         * null, null ); String result = null; try { // get path from file manager if( cursor != null && cursor.moveToFirst()
         * ){ result = cursor.getString( cursor.getColumnIndex( FM_KEY_PATH ) ); } // use default setting if error if( result
         * == null ){ StorageManager sm = (StorageManager)context.getSystemService( Context.STORAGE_SERVICE ); result =
         * sm.getDefaultPath() + "/" + Options.DEFAULT_STORAGE_FOLDER; } return result; } finally { if( cursor != null )
         * cursor.close(); }
         */

        // FileManagerProvider is removed, check "Received File" folder directly by file path
        String result = null;
        //StorageManager sm = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        //result = sm.getDefaultPath() + "/" + FM_SELECTION;
        result = StorageManagerEx.getDefaultPath() + "/" + FM_SELECTION;

        File storageDir = new File(result);
        if (!storageDir.isDirectory()) {

            //result = sm.getDefaultPath() + "/" + Options.DEFAULT_STORAGE_FOLDER;
            result = StorageManagerEx.getDefaultPath() + "/" + Options.DEFAULT_STORAGE_FOLDER;
        }

        return result;
    }

    public static String getMountPointPath() {
		String path = null;
        String ret = null;
		try {
			path = SystemProperties.get("internal_sd_path");
		} catch (IllegalArgumentException e) {
            BtLog.w("IllegalArgumentException when getInternalStoragePath:" + e);
			path = "/storage/";
		}

        int idx = path.lastIndexOf("/") + 1;

        ret = path.substring(0, idx);
        return ret;
    }

    /**
     * get the writable available size of default directory
     */
    public static long getReceivedFilePathAvailableSize(Context context) {

        return SystemUtils.getStorageAvailableSize(context, SystemUtils.getReceivedFilePath(context));
    }

    /**
     * get the corresponding external-storage-directory ( "/mnt/sdcard" or "/mnt/sdcard/sdcard2" ) for the specified
     * filename.
     * 
     * @param filename
     * @return null when no external storage directory matches the filename
     */
    public static File getExternalStorageDirectory(Context context, String filename) {

        String result = null;
        if (filename != null) {

            // loop all external storage dir and find out the correct one
            int currMatch = 0;
            filename = filename.toLowerCase();
            StorageManager sm = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
            String[] esds = sm.getVolumePaths();
            for (String dir : esds) {
                // check dir and keep the longest one: "/mnt/sdcard/sdcard2" over "/mnt/sdcard"
                if (filename.startsWith(dir) && dir.length() > currMatch) {
                    result = dir;
                    currMatch = dir.length();
                }
            }
        }
        if (Options.LL_DEBUG) {
            BtLog.d("SystemUtils.getExternalStorageDirectory(): [" + filename + "]=>[" + result + "]");
        }
        if (result == null) {
            return null;
        }
        return new File(result);
    }

    /**
     * check whether the external storage for the given filename is under "MOUNTED" state (writable). this function will
     * exclude "UN-MOUNTING" from the original "MOUNTED" state but we need to call startMonitor() before EJECT event. if the
     * given filename is not in any external storage directory => return false
     * 
     * @param filename
     * @return
     */
    public static boolean isExternalStorageMounted(Context context, String filename) {

        // map filename to external-storage-directory
        File storageDir = SystemUtils.getExternalStorageDirectory(context, filename);
        if (storageDir == null) {

            BtLog.w("StorageMonitor.isStorageMounted(): can't find stroage dir for[" + filename + "]");
            return false;
        }

        // get storage state from Environment API
        String storageDirPath = storageDir.getAbsolutePath();
        StorageManager sm = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        String state = sm.getVolumeState(storageDirPath);
        if (Options.LL_DEBUG) {
            BtLog.d("SystemUtils.isExternalStorageMounted(): [" + filename + "]=>[" + state + "]");
        }

        // when state is mounted, we need to make sure is not under "un-mounting" state
        if (Environment.MEDIA_MOUNTED.equals(state)) {

            if (Options.LL_DEBUG) {
                BtLog.d("SystemUtils.isExternalStorageMounted(): check unmounting[" + storageDirPath + "]["
                        + (BluetoothReceiver.isPathMounted(storageDirPath)) + "]");
            }
            return BluetoothReceiver.isPathMounted(storageDirPath);
        }
        return false;
    }

    /**
     * get the writable available size for the given filename please make sure the directory is already exist.
     * 
     * @param filename
     * @return -1 when no storage is available
     */
    public static long getStorageAvailableSize(Context context, String filename) {

        // check external storage state (if it is)
        File externalStorage = SystemUtils.getExternalStorageDirectory(context, filename);
        if (externalStorage != null && !SystemUtils.isExternalStorageMounted(context, filename)) {

            // a not "MOUNTED" external storage => no available size
            return -1;
        }

        // check storage size
        File ff = new File(filename);
        if (!ff.isDirectory()) {

            BtLog.d("SystemUtils.getStorageAvailableSize(): change to parent folder");
            ff = ff.getParentFile();
            if (ff == null)
                return -1; // no parent folder
        }
        StatFs stat = new StatFs(ff.getPath());
        // -4 for file system block for new file
        return ((long) stat.getBlockSize()) * ((long) (stat.getAvailableBlocks() - 4));
    }

    // private static final int MAX_FILEPATH_LENGTH = 256;
    private static final int MAX_FILENAME_LENGTH = 255;

    /**
     * append '_n' when file exists
     * 
     * @param expectedFilePath
     * @return
     */
    public static File createNewFileForSaving(String expectedFilePath) {

        return SystemUtils.createNewFileForSaving(0, expectedFilePath);
    }

    private static File createNewFileForSaving(int level, String expectedFilePath) {
		
		int innerLevel = level;
		String innerFilePath = expectedFilePath;
		while( true ){
			
			// check expected-filename
			if( innerFilePath== null ){
			
				BtLog.w("SystemUtils.createNewFileForSaving(): null expected filename!");
				return null;
			}
			
			// create default file object for checking
			File file = new File( innerFilePath );
			String filename = file.getName();
			
			// for normal case performance
			if( filename.getBytes().length <= MAX_FILENAME_LENGTH && !file.exists() ) {
			
				try {
					file.createNewFile();
					return file;
				} catch( Exception ex ) {
			
					BtLog.e("create new file error: [" + file + "]", ex);
					return null;
				}
			}
			
			// parsing the filename: [basename] + ["_" + counter] + ["." + extension]
			int extIndex = filename.lastIndexOf('.');
			int sufIndex = filename.lastIndexOf('_');
			// extension
			String extension = (extIndex > -1) ? filename.substring(extIndex) : "";
			
			// don't change the original filename
			if( innerLevel == 0 )
				sufIndex = -1;
			
			// counter
			if( sufIndex > -1 ) {
				String counter = (extIndex > -1) ? filename.substring(sufIndex + 1, extIndex) : filename.substring(sufIndex + 1);
				try {
					innerLevel = Integer.parseInt( counter );
				} catch( NumberFormatException ex ) {
			
					// no a valid counter string
					sufIndex = -1;
				}
			}
			// basename
			String basename;
			if( sufIndex > -1 ) {
				basename = filename.substring( 0, sufIndex );
			} else if( extIndex > -1 ) {
				basename = filename.substring( 0, extIndex );
			} else {
				basename = filename;
			}
			
			// check filename length
			if( filename.getBytes().length > MAX_FILENAME_LENGTH ) {
				
				String suffix = filename.substring( basename.length() );
				int maxBasename = MAX_FILENAME_LENGTH - suffix.getBytes().length;
				filename = SystemUtils.trimFilenameWithLength( filename, maxBasename );
				file = new File( file.getParentFile(), filename + suffix );
				BtLog.w("filename is too long and be trimed: [" + file + "]");
			}
			
			// check exists  
			if( file.exists() ) {
				
				innerLevel++;
				StringBuilder buffer = new StringBuilder( filename.length()).append(basename).append('_').append(innerLevel).append( extension );
				file = new File( file.getParentFile(), buffer.toString() );
				//return SystemUtils.createNewFileForSaving(level, file.getAbsolutePath());    
				//change with while loop for stack overflow issue
				innerFilePath = file.getAbsolutePath();
				continue;
			} else {
				try {
					
					file.createNewFile();
					return file;
				} catch (Exception ex) {
					
					BtLog.e("create new file error: [" + file + "]", ex);
					return null;
				}
			}
		}
	}

    public static String trimFilenameWithLength(String filename, int maxLength) {

        BtLog.d("SystemUtils.trimFilenameWithLength(): [" + filename + "][" + maxLength + "]");

        // check parameter
        if (filename == null)
            return "";

        // valid case -> return directly
        if (filename.getBytes().length <= maxLength) {

            return filename;
        }

        // need to trim filename
        int currentEnd = filename.length();
        do {
            filename = filename.substring(0, --currentEnd);
        } while (filename.getBytes().length > maxLength);

        return filename;
    }

    /**
     * open object with appropriate application or display error dialog.
     * 
     * @param context
     * @param filename
     * @param mimeType
     * @return
     */
    public static Intent getOpenFileIntent(Context context, String filename, String mimeType) {

        // invalid parameters
        // if( filename == null || mimeType == null ){
        if (filename == null) {

            BtLog.w("invalid parameters for getOpenFileIntent(): filename=" + filename + " mimetype=" + mimeType);
            return MessageActivity.createIntent(context, context.getString(R.string.bt_share_openfile_title), context
                    .getString(R.string.bt_share_openfile_message_invalid_param, filename, mimeType), context
                    .getString(R.string.bt_share_openfile_confirm));
        }

        // resource exists
        File file = new File(filename);
        if (!file.exists()) {

            return MessageActivity.createIntent(context, context.getString(R.string.bt_share_openfile_title), context
                    .getString(R.string.bt_share_openfile_message_resource_not_found, filename), context
                    .getString(R.string.bt_share_openfile_confirm));
        }

        // supported data type
        Uri path = Uri.parse(filename);
        path = (path.getScheme() == null) ? Uri.fromFile(file) : path;
        if (!isSupportedDataType(context, path, mimeType)) {

            return MessageActivity.createIntent(context, context.getString(R.string.bt_share_openfile_title), context
                    .getString(R.string.bt_share_openfile_message_unsupported_type, mimeType), context
                    .getString(R.string.bt_share_openfile_confirm));
        } else {

            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setDataAndType(path, mimeType);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            return intent;
        }
    }

    /**
     * check if the data type is supported by system
     * 
     * @param context
     * @param data
     * @param mimeType
     * @return
     */
    public static boolean isSupportedDataType(Context context, Uri data, String mimeType) {

        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setDataAndType(data, mimeType);
        List<ResolveInfo> list = context.getPackageManager()
                .queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY);
        if (list.size() == 0) {

            BtLog.d("cannot find proper Activity to handle Intent: mime[" + mimeType + "], data[" + data + "]");
            return false;
        }
        return true;
    }

    /**
     * Check Bluetooth is allowed to be used under current config Currently not used in app package -> should be used in
     * Settings (BluetoothAdapter.ACTION_REQUEST_ENABLE)
     */
    // public static boolean isBluetoothAllowed( Context context){
    //
    // // get content resolver
    // ContentResolver resolver = context.getContentResolver();
    //
    // // check airplane-mode config
    // boolean isAirplaneModeOn = Settings.System.getInt(resolver, Settings.System.AIRPLANE_MODE_ON, 0) == 1;
    // if( !isAirplaneModeOn ) return true;
    //
    // String airplaneModeRadios = Settings.System.getString(resolver, Settings.System.AIRPLANE_MODE_RADIOS);
    // boolean isAirplaneSensitive = airplaneModeRadios == null ? true : airplaneModeRadios.contains(
    // Settings.System.RADIO_BLUETOOTH );
    // if( !isAirplaneSensitive ) return true;
    //
    // String airplaneModeToggleableRadios = Settings.System.getString(resolver,
    // Settings.System.AIRPLANE_MODE_TOGGLEABLE_RADIOS);
    // boolean isAirplaneToggleable = airplaneModeToggleableRadios == null ? false :
    // airplaneModeToggleableRadios.contains(Settings.System.RADIO_BLUETOOTH);
    // if( isAirplaneToggleable ) return true;
    //
    // return false;
    // }

    public static void startVibration(Context context, long milliseconds) {

        Vibrator v = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (v == null) {

            BtLog.w("startVibration error: cna't find VIBRATOR_SERVICE");
            return;
        }
        v.vibrate(milliseconds);
    }

    public static void startVibration(Context context, long[] pattern, int repeat) {

        Vibrator v = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (v == null) {

            BtLog.w("startVibration error: cna't find VIBRATOR_SERVICE");
            return;
        }
        v.vibrate(pattern, repeat);
    }

    public static void stopVibration(Context context) {

        Vibrator v = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (v == null) {

            BtLog.w("stopVibration error: can't find VIBRATOR_SERVICE");
            return;
        }
        v.cancel();
    }

    public static void sound(Context context) {

        // MediaPlayer mp = MediaPlayer.create( context, R.raw.sound_file_1 );
        // mp.start();
    }

    public static void screen(Context context, long milliseconds) {

        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        WakeLock wl = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "SystemUtilsScreenLock");
        wl.acquire(milliseconds);
        // wl.release();
    }
}
