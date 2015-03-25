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

package com.mediatek.rcse.plugin.phone;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Message;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.decoder.NativeH264Decoder;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * This class defined as a proxy activity for file transfer
 */
public class RichcallProxyActivity extends Activity {
    private static final String TAG = "RichcallProxyActivity";
    /* package */static final String IMAGE_SHARING_SELECTION = "com.mediatek.rcse.plugin.phone.IMAGE_SHARING_SELECTION";
    /* package */static final String CONTACT = "contact";
    /* package */static final String CONTACT_DISPLAYNAME = "contactDisplayname";
    /* package */static final String SESSION_ID = "sessionId";
    /* package */static final String VIDEO_TYPE = "videotype";
    /* package */static final String VIDEO_WIDTH = "videowidth";
    /* package */static final String VIDEO_HEIGHT = "videoheight";
    /* package */static final String MEDIA_TYPE = "mediatype";
    private static final String GALLERY_TYPE = "image/*";
    private static final String GALLERY_VIDEO_TYPE = "video/*";
    private static final int SUPPORT_WIDTH = 176;
    private static final int SUPPORT_HEIGHT = 144;
    private static final String SUPPORT_FORMAT_H264 = "avc";
    private Uri mCameraTempFileUri = null;
    private static final boolean FORMAT_SUPPORTED = true;
    private static final boolean FORMAT_NOT_SUPPORTED = false;
    private static boolean isFormatSupported = FORMAT_SUPPORTED;
    public static final int REQUEST_CODE_CAMERA = 10;
    public static final int REQUEST_CODE_GALLERY = 11;
    private static final int REQUEST_CODE_VIDEO = 12;
    /* package */static final String IMAGE_NAME = "filename";
    /* package */static final String IMAGE_SIZE = "filesize";
    /* package */static final String IMAGE_TYPE = "filetype";
	static final String THUMBNAIL_TYPE = "thumbnail";
    private static final File SDCARDDIEFILE = Environment.getExternalStorageDirectory();
    private static final String SLASH = "/";
    private static final String RCSE_FILE_DIR = SDCARDDIEFILE + SLASH + "Joyn";
    private static final String RCSE_TEMP_FILE_DIR = RCSE_FILE_DIR + SLASH + "temp";
    private static final String RCSE_TEMP_FILE_NAME_HEADER = "tmp_joyn_";
    private static final String JPEG_SUFFIX = ".jpg";
    public static final String SELECT_TYPE = "selectionType";
    public static final String SELECT_TYPE_GALLERY = "Gallery";
    public static final String SELECT_TYPE_CAMERA = "Camera";
    public static final String SELECT_TYPE_VIDEO = "Video";
    
    private static final String FILE_SCHEMA = "file://";
    private static final String CONTENT_SCHEMA = "content://";
    private static final String VCARD_SCHEMA = "content://com.android.contacts/contacts/as_vcard";
    private static final String ALL_VCARD_SCHEMA =
            "content://com.android.contacts/contacts/as_multi_vcard/";
    private static final String READABLE_RIGHT = "r";
    private static final String VCARD_SUFFIX = ".vcf";
    private static final String VCALENDAR_SCHEMA = "content://com.mediatek.calendarimporter/";
    private static final String VCALENDAR_DATA_TYPE = "text/x-vcalendar";
    private static final String VCALENDAR_SUFFIX = ".vcs";
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Logger.v(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String selectionType = intent.getStringExtra(SELECT_TYPE);
        if (SELECT_TYPE_GALLERY.equalsIgnoreCase(selectionType)) {
            startGallery();
        } else if (SELECT_TYPE_CAMERA.equalsIgnoreCase(selectionType)) {
            startCamera();
        } else if (SELECT_TYPE_VIDEO.equalsIgnoreCase(selectionType)) {
            startVideoGallery();
        } else {
            Logger.v(TAG, "intent is of unkown type");
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        new ImageProcessingAsyncTask(requestCode, resultCode, data).execute();
    }

    private class ImageProcessingAsyncTask extends AsyncTask<Void, Void, String> {
        private int mRequestCode = -1;
        private int mResultCode = -1;
        private Intent mData = null;
        private long mDuration = 0;
        private String mEncoding = "h264";
        private int mWidth = 0;
        private int mHeight = 0;

        public ImageProcessingAsyncTask(int requestCode, int resultCode, Intent data) {
            mRequestCode = requestCode;
            mResultCode = resultCode;
            mData = data;
        }
        
        private String checkSelectedImageFile(String fileName, int which) {
            File file = new File(fileName);
            long fileSize = file.length();
            long maxFileSize = RcsSettings.getInstance().getMaxImageSharingSize() * 1024;
            long warningFileSize = RcsSettings.getInstance()
                    .getWarningMaxImageTransferSize() * 1024;
            boolean shouldWarning = false;
            boolean shouldRepick = false;
            if (warningFileSize != 0 && fileSize >= warningFileSize) {
                shouldWarning = true;
            }
            if (maxFileSize != 0 && fileSize >= maxFileSize) {
                shouldRepick = true;
            }
            Logger.d(TAG, "checkSelectedImageFile() maxFileSize: " + maxFileSize
                    + " warningFileSize: " + warningFileSize + " shouldWarning: " + shouldWarning
                    + " shouldRepick: " + shouldRepick);
            if (shouldRepick) {
                sendRepicDialogIntent(which);
                return null;
            } else if (shouldWarning) {
                Context context = AndroidFactory.getApplicationContext();
                if (context != null) {
                    boolean remind = RcsSettings.getInstance().restoreRemindWarningLargeImageFlag();
                    Logger.w(TAG, "checkSelectedImageFile() remind: " + remind);
                    if (remind) {
                        sendWarnDialogIntent(fileName);
                        return null;
                    } else {
                        return fileName;
                    }
                }else{
                    return fileName;
                }
            } else {
                return fileName;
            }
        }



        private String getFileNameFromFileSchema(String uri){
            return uri.substring(FILE_SCHEMA.length(), uri.length());
        }
        
        private String getFileNameFromContentSchema(Uri uri,int entry_type) {
            Cursor cursor = null;
            if(entry_type == REQUEST_CODE_GALLERY){
                cursor = RichcallProxyActivity.this.getContentResolver().query(uri,
                    new String[] {
                        MediaStore.Images.ImageColumns.DATA
                    }, null, null, null);
            }else if(entry_type == REQUEST_CODE_VIDEO){
                cursor = RichcallProxyActivity.this.getContentResolver().query(uri,
                        new String[] {
                        MediaStore.Video.VideoColumns.DATA,
                        MediaStore.Video.VideoColumns.DURATION
                }, null, null, null);
            }
            String fileName = null;
            if (cursor != null) {
                cursor.moveToFirst();
                fileName = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
                cursor.close();
            }
            return fileName;
        }
        
        private String getFileNameFromVcard(Uri uri) {
            String fileName = RCSE_TEMP_FILE_DIR + System.currentTimeMillis() + VCARD_SUFFIX;
            try {
                AssetFileDescriptor fd = RichcallProxyActivity.this.getContentResolver()
                        .openAssetFileDescriptor(uri, READABLE_RIGHT);
                FileInputStream fis = fd.createInputStream();
                byte[] data = new byte[fis.available()];
                fis.read(data);
                fis.close();
                File dir = new File(RCSE_TEMP_FILE_DIR);
                if (!dir.exists()) {
                    if (dir.mkdir()) {
                        Logger.e(TAG, "getFileNameFromVcard()-create dir failed");
                        File file = new File(fileName);
                        file.setWritable(true);
                        file.setReadable(true);
                        FileOutputStream fos = new FileOutputStream(file);
                        fos.write(data);
                        fos.close();
                    }
                }
            } catch (FileNotFoundException fileNotFoundException) {
                Logger.e(TAG, "getFileNameFromVcard()-fileNotFoundException");
                fileNotFoundException.printStackTrace();
                fileName = null;
            } catch (IOException iOException) {
                Logger.e(TAG, "getFileNameFromVcard()-iOException while accessing the stream");
                iOException.printStackTrace();
                fileName = null;
            } finally {
                return fileName;
            }
        }
        
        private String getFileNameFromVcalendar(Uri uri) {
            String fileName = RCSE_TEMP_FILE_DIR + System.currentTimeMillis() + VCALENDAR_SUFFIX;
            try {
                AssetFileDescriptor fd = RichcallProxyActivity.this.getContentResolver()
                        .openAssetFileDescriptor(uri, READABLE_RIGHT);
                FileInputStream fis = fd.createInputStream();
                byte[] data = new byte[fis.available()];
                fis.read(data);
                fis.close();
                File dir = new File(RCSE_TEMP_FILE_DIR);
                if (!dir.exists()) {
                    if (dir.mkdir()) {
                        File file = new File(fileName);
                        file.setWritable(true);
                        file.setReadable(true);
                        FileOutputStream fos = new FileOutputStream(file);
                        fos.write(data);
                        fos.close();
                    }
                }
            } catch (FileNotFoundException fileNotFoundException) {
                Logger.e(TAG, "getFileNameFromVcalendar()-fileNotFoundException");
                fileNotFoundException.printStackTrace();
                fileName = null;
            } catch (IOException iOException) {
                Logger.e(TAG, "getFileNameFromVcalendar()-iOException while accessing the stream");
                iOException.printStackTrace();
                fileName = null;
            } finally {
                return fileName;
            }
        }
        
        private String getFileFullPathFromUri(Uri uri,int entry_type) {
            String fileName = null;
            if (uri != null) {
                String uriString = Uri.decode(uri.toString());
                Logger.d(TAG, "getFileFullPathFromUri()-The uri is:[" + uriString + "]");
                if (uriString != null && uriString.startsWith(FILE_SCHEMA)) {
                    fileName = getFileNameFromFileSchema(uriString);
                } else if (uriString != null && uriString.startsWith(CONTENT_SCHEMA)) {
                    fileName = getFileNameFromContentSchema(uri, entry_type);
                } else if (uriString != null
                        && (uriString.startsWith(VCARD_SCHEMA) || uriString
                                .startsWith(ALL_VCARD_SCHEMA))) {
                    fileName = getFileNameFromVcard(uri);
                } else if (uriString != null && uriString.startsWith(VCALENDAR_SCHEMA)) {
                    fileName = getFileNameFromVcalendar(uri);
                } else {
                    Logger.e(TAG, "getFileFullPathFromUri()-uriString = " + uriString
                            + ",is not start with file:// and content://");
                }
            }
            return fileName;
        }

		private String checkSelectedVideoFile(String fileFullName) {
           /* Logger.d(TAG, "checkSelectedFile() entry fileFullName = "
                            + fileFullName);
                            MediaExtractor extractor = new MediaExtractor();
                            extractor.setDataSource(fileFullName);
                            for (int i = 0; i < extractor.getTrackCount(); i++) {
                                MediaFormat format = extractor.getTrackFormat(i);
                                int result = 0;
                                String mime = format.getString(MediaFormat.KEY_MIME);
                                if (mime.startsWith("video/")) {
						            if(mime.indexOf(SUPPORT_FORMAT_H264) != -1){
						                mEncoding = SUPPORT_FORMAT_H264;
						                mWidth = format.getInteger(MediaFormat.KEY_WIDTH);
						                mHeight = format.getInteger(MediaFormat.KEY_HEIGHT);
										if(mDuration == 0){
										    mDuration = format.getLong(MediaFormat.KEY_DURATION)/1000;
										}
						                result = 1;
						            }
                                }
                            }

                            extractor.release();
                            extractor = null;
            Logger.d(TAG, "checkSelectedFile() exit fileFullName = "
                            + fileFullName);
            return fileFullName;*/
return null;
        }

        @Override
        protected String doInBackground(Void... params) {
            String fileFullName = null;
            Uri uri = null;
            if (mResultCode == RESULT_CANCELED) {
                Logger.d(TAG, "ImageProcessingAsyncTask mResultCode is RESULT_CANCELED");
                return null;
                        }
            switch(mRequestCode){
            case REQUEST_CODE_CAMERA : 
            {
                fileFullName = getFileFullPathFromUri(mCameraTempFileUri, REQUEST_CODE_CAMERA);
                if(fileFullName != null){
                    fileFullName = checkSelectedImageFile(fileFullName, REQUEST_CODE_CAMERA);
                }else{
                    return null;
                }
                break;
            }
            case REQUEST_CODE_GALLERY : 
            {
                if (mData != null) {
                        // Get image filename
                        uri = mData.getData();
                        if(uri != null){
                            fileFullName = getFileFullPathFromUri(uri, REQUEST_CODE_GALLERY);
                            if(fileFullName != null){
                                fileFullName = checkSelectedImageFile(fileFullName, REQUEST_CODE_GALLERY);
                            }else{
                                return null;
                        }
                        }else{
                            return null;
                    }
                } else {
                    Logger.w(TAG, "mData is null,that is onActivityResult() get a null intent");
                }
                    break;
            }
            case REQUEST_CODE_VIDEO :
            {
                if (mData != null) {
                        // Get video filename
                        uri = mData.getData();
                        if(uri != null){
                            fileFullName = getFileFullPathFromUri(uri, REQUEST_CODE_VIDEO);
                            if(fileFullName != null){
                                fileFullName = checkSelectedVideoFile(fileFullName);
                            }else{
                                return null;
                            }
                        }else{
                            return null;
                        }
            } else {
                        Logger.w(TAG, "mData is null,that is onActivityResult() get a null intent");
                    }
                    break;
            }
            default :
            {
                Logger.w(TAG, "unkown result");
                break;
            }
            }
            return fileFullName;
        }

        @Override
        protected void onPostExecute(String fileName) {
            Logger.v(TAG, "ImageProcessingAsyncTask onPostExecute() entry, with fileName = "
                    + fileName);
            if (mResultCode != RESULT_CANCELED) {
                switch (mRequestCode) {
                    case REQUEST_CODE_VIDEO:
                        if (fileName == null) {
                        if(isFormatSupported)
                        {
                            Toast.makeText(getApplicationContext(), R.string.video_sharing_not_support, Toast.LENGTH_SHORT)
                                    .show();
                        }
                        else
                        {
                            isFormatSupported = FORMAT_SUPPORTED;
                            Toast.makeText(getApplicationContext(), R.string.video_file_format_not_support, Toast.LENGTH_SHORT)
                            .show();
                        }
                        }
                        break;
                    case REQUEST_CODE_GALLERY:
                    case REQUEST_CODE_CAMERA:
                    if (fileName != null) {
                        startImageFile(fileName);
                    }
                        break;
                    default:
                        break;
                }
            } else {
                Logger.d(TAG, "onPostExecute, user canceled!");
            }
            finish();
        }
    }

    private boolean createDirectory(String path) {
        File dir = new File(path);
        if (!dir.exists()) {
            if (!dir.mkdirs()) {
                return false;
            }
        }
        return true;
    }

    private void startCamera() {
        Logger.d(TAG, "startCamera entry");
        if (this.createDirectory(RCSE_FILE_DIR)) {
            Logger.w(TAG, "Create rcse dir success");
        } else {
            Logger.w(TAG, "Create rcse dir failed");
        }
        if (this.createDirectory(RCSE_TEMP_FILE_DIR)) {
            Logger.w(TAG, "Create rcse tmp dir success");
        } else {
            Logger.w(TAG, "Create rcse tmp dir failed");
        }
        mCameraTempFileUri = Uri.fromFile(new File(RCSE_TEMP_FILE_DIR, RCSE_TEMP_FILE_NAME_HEADER
                + String.valueOf(System.currentTimeMillis()) + JPEG_SUFFIX));
        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, mCameraTempFileUri);
        startActivityForResult(intent, REQUEST_CODE_CAMERA);
        Logger.d(TAG, "startCamera exit");
    }

    private void startGallery() {
        Logger.d(TAG, "startGallery entry");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType(GALLERY_TYPE);
        startActivityForResult(intent, REQUEST_CODE_GALLERY);
        Logger.d(TAG, "startGallery exit");
    }

    private void startVideoGallery() {
        Logger.d(TAG, "startVideoGallery entry");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType(GALLERY_VIDEO_TYPE);
        startActivityForResult(intent, REQUEST_CODE_VIDEO);
        Logger.d(TAG, "startVideoGallery exit");
    }
    

    private void startImageFile(String fileName) {
        if (fileName == null) {
            finish();
            Logger.d(TAG, "onPostExecute fileName is null");
            return;
        }
        Intent intent = new Intent(ImageSharingPlugin.IMAGE_SHARING_START_ACTION);
        intent.putExtra(ImageSharingPlugin.IMAGE_NAME, fileName);
        Context context = AndroidFactory.getApplicationContext();
        if (context == null) {
            Logger
                    .v(TAG,
                            "AndroidFactory.getApplicationContext() return null, so call getApplicationContext instead");
            context = getApplicationContext();
        }
        context.sendBroadcast(intent);
    }
    
    private void sendRepicDialogIntent(int select_type) {
        Intent intent = new Intent(ImageSharingPlugin.IMAGE_SHARING_REPIC_ACTION);
        intent.putExtra(ImageSharingPlugin.SELECT_TYPE, select_type);
        Context context = AndroidFactory.getApplicationContext();
        if (context == null) {
            Logger
            .v(TAG,
                    "AndroidFactory.getApplicationContext() return null, so call getApplicationContext instead");
            context = getApplicationContext();
        }
        context.sendBroadcast(intent);
    }
    
    private void sendWarnDialogIntent(String fileName) {
        if (fileName == null) {
            finish();
            Logger.d(TAG, "sendWarnDialogIntent fileName is null");
            return;
}
        Intent intent = new Intent(ImageSharingPlugin.IMAGE_SHARING_WARN_ACTION);
        intent.putExtra(ImageSharingPlugin.IMAGE_NAME, fileName);
        Context context = AndroidFactory.getApplicationContext();
        if (context == null) {
            Logger
            .v(TAG,
                    "AndroidFactory.getApplicationContext() return null, so call getApplicationContext instead");
            context = getApplicationContext();
        }
        context.sendBroadcast(intent);
    }
}
