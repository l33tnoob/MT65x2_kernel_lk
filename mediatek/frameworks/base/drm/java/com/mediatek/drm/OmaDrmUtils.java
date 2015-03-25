/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.drm;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.drm.DrmInfo;
import android.drm.DrmInfoStatus;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.OnScanCompletedListener;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Log;

import java.io.BufferedInputStream;
import java.util.ArrayList;
import java.io.UnsupportedEncodingException;

public class OmaDrmUtils {
    private static final String TAG = "OmaDrmUtils";
    private static final Uri[] CID_URIS = new Uri[] {
        MediaStore.Audio.Media.INTERNAL_CONTENT_URI,
        MediaStore.Images.Media.INTERNAL_CONTENT_URI,
        MediaStore.Video.Media.INTERNAL_CONTENT_URI,
        MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
        MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
        MediaStore.Video.Media.EXTERNAL_CONTENT_URI
    };

    private static final boolean OMA_DRM_EXTEND_SUFFIX;

    static {
        // check system property tho determine if it has set Extend-Suffix
        String drmExtendSuffix = System.getProperty("drm.extend.suffix", "no");
        OMA_DRM_EXTEND_SUFFIX =
            drmExtendSuffix.equals("true") || drmExtendSuffix.equals("yes") || drmExtendSuffix.equals("1");
    }


    /**
     * Get action type according to the mime of drm media content.
     * The mime may start with "audio/", "video/", "image/"
     *
     * @param mime MIME type.
     * @return OmaDrmStore.Action.DISPLAY for image/; OmaDrmStore.Action.PLAY for audio/, video/
     */
    public static int getMediaActionType(String mime) {
        if (mime.startsWith(OmaDrmStore.MimePrefix.IMAGE)) {
            return OmaDrmStore.Action.DISPLAY;
        } else if (mime.startsWith(OmaDrmStore.MimePrefix.AUDIO)
                   || mime.startsWith(OmaDrmStore.MimePrefix.VIDEO)) {
            return OmaDrmStore.Action.PLAY;
        }

        return OmaDrmStore.Action.PLAY; // otherwise PLAY is returned.
    }

    /**
     * scan with the given content-id (Content Uri) of corresponding drm media file
     *
     * @param context The application context
     * @param drmContentUri The content-id (cid) of the DRM file
     * @param callback OnDrmScanCompletedListener. may be null
     * @return The count of files need to be be scanned
     */
    public static int rescanDrmMediaFiles(Context context, String drmContentUri,
            OnDrmScanCompletedListener callback) {
        Log.v(TAG, "rescanDrmMediaFiles : " + drmContentUri + " with callback " + callback);

        ContentResolver cr = context.getContentResolver();
        String where = "drm_content_uri=?";
        String[] whereArgs = new String[] {drmContentUri};
        ArrayList<String> pathArray = new ArrayList<String>();
        int length = CID_URIS.length;
        for (int i = 0; i < length; i++) {
            Uri uri = CID_URIS[i];
            Cursor cursor = cr.query(uri, new String[] {"_data"}, where, whereArgs, null);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    pathArray.add(cursor.getString(0));
                }
                cursor.close();
            }
        }

        int total = pathArray.size();
        if (total < 1) {
            if (callback != null) {
                callback.onScanCompletedAll(0);
            }
        } else {
            String[] paths = new String[total];
            pathArray.toArray(paths);
            if (callback != null) {
                DrmScanCompletedProxy clientProxy = new DrmScanCompletedProxy(callback, total);
                MediaScannerConnection.scanFile(context, paths, null, clientProxy);
            } else {
                MediaScannerConnection.scanFile(context, paths, null, null);
            }

            int size = pathArray.size();
            for (int i = 0; i < size; i++) {
                Log.v(TAG, "rescanDrmMediaFiles : #" + i + " path=" + pathArray.get(i));
            }
        }

        return total;
    }

    /**
     * when a single DRM file is downloaded / received, scan it.
     *
     * @param context The application context
     * @param file The path name of the DRM file
     * @param callback OnDrmScanCompletedListener. may be null
     * @return The count of files need to be be scanned (will be 1)
     */
    public static int scanDrmMediaFile(Context context, String file,
            OnDrmScanCompletedListener callback) {
        Log.v(TAG, "scanDrmMediaFile : " + file + " with callback " + callback);

        String[] paths = new String[] {file};
        if (callback != null) {
            DrmScanCompletedProxy clientProxy = new DrmScanCompletedProxy(callback, paths.length);
            MediaScannerConnection.scanFile(context, paths, null, clientProxy);
        } else {
            MediaScannerConnection.scanFile(context, paths, null, null);
        }

        return paths.length;
    }

    /**
     * Notifying clients of the result of scanning a requested media file width DRM info.
     */
    public interface OnDrmScanCompletedListener {
        /**
         * Notify the client when the media scanner has finished scanning one file
         *
         * @param path The path of file that has been scanned
         * @param uri The Uri of file added to media database; null if scanning failed
         */
        void onScanCompletedOne(String path, Uri uri);
        /**
         * Notify the client wwhen all the files are scanned
         *
         * @param scannedCount The amount of files which have been scanned
         */
        void onScanCompletedAll(int scannedCount) ;
    }

    private static class DrmScanCompletedProxy implements OnScanCompletedListener {
        private int mScannedCount;
        private int mScanCount;
        private OnDrmScanCompletedListener mClient;

        public DrmScanCompletedProxy(OnDrmScanCompletedListener callback, int scanCount) {
            mScannedCount = 0;
            mScanCount = scanCount;
            mClient = callback;
        }

        public void onScanCompleted(String path, Uri uri) {
            mScannedCount++;
            mClient.onScanCompletedOne(path, uri);
            if (mScannedCount >= mScanCount) {
                mClient.onScanCompletedAll(mScannedCount);
            }
        }
    }

    /**
     * Check the object represented by {uri} and returns DrmProfile
     * The {uri} need to be the following cases: (otherwise it cannot recognize)
     *   1. file path name, starts with file://
     *   2. MediaProvider database record.
     *   3. Query documentsUI uri( "context://com.android.providers.media.documents")
     *
     * Remarks:
     *   all the parameters shall not be null. Otherwise throw IllegalArgumentException
     *
     * @param context Application context
     * @param uri Uri of the object to be checked
     * @param client OmaDrmClient instance
     * @return DrmProfile Containing the brief DRM information
     */
    public static DrmProfile getDrmProfile(Context context, Uri uri, OmaDrmClient client) {
        Log.v(TAG, "getDrmProfile : " + uri);

        if (uri == null || context == null || client == null) {
            Log.e(TAG, "getDrmProfile : invalid parameters client=" + client + " context=" + context + " uri=");
            throw new IllegalArgumentException();
        }

        boolean isDrm = false;
        int method = OmaDrmStore.DrmMethod.METHOD_NONE;

        String scheme = uri.getScheme();
        if (ContentResolver.SCHEME_CONTENT.equals(scheme)) {
                //&& MediaStore.AUTHORITY.equals(uri.getHost())) { // MediaProvider
            String[] projection = new String[] {
                MediaStore.Audio.Media.IS_DRM,
                MediaStore.Audio.Media.DRM_METHOD
            };

            // query MediaProvider db and get profile info
            //The db may have not a field named IS_DRM
            //In such case mark it as a non drm file
            Cursor c = null;
            try {
                c = context.getContentResolver().query(uri, projection, null,
                        null, null);
                if (c != null) {
                    if (c.moveToNext()) {
                        int columnIndex = c
                                .getColumnIndex(MediaStore.Audio.Media.IS_DRM);
                        if (columnIndex != -1) {
                            isDrm = "1".equals(c.getString(columnIndex));
                        }

                        columnIndex = c
                                .getColumnIndex(MediaStore.Audio.Media.DRM_METHOD);
                        if (columnIndex != -1) {
                            method = c.getInt(columnIndex);
                        }
                    }
                }
			} catch (Exception ex) {// becasue provider will throw different exception, so will catch Exception directly.
				Log.w(TAG, ex);
                isDrm = false;
            } finally {
                if (null != c) {
                    c.close();
                    c = null;
                }
            }
        } else if (ContentResolver.SCHEME_FILE.equals(scheme)) { // file path name
            // if the suffix matches
            if (isDrmSuffix(uri.getLastPathSegment())) {
                isDrm = true;
                method = client.getMethod(uri);
            } else {
                isDrm = false;
                // the suffix does not match, but we configured to allow
                // suffix other than .dcf / .dm ... so correct the result
                if (OMA_DRM_EXTEND_SUFFIX) {
                    method = client.getMethod(uri);
                    isDrm = (method != OmaDrmStore.DrmMethod.METHOD_NONE);
                }
            }
        }

        DrmProfile profile = new DrmProfile(method, isDrm);
        Log.v(TAG, "getDrmProfile : isDrm=" + profile.isDrm() + " method=" + profile.getMethod());
        return profile;
    }

    /**
     * Containing the brief DRM information.
     */
    public static class DrmProfile {
        private boolean mIsDrm = false;
        private int mDrmMethod = OmaDrmStore.DrmMethod.METHOD_NONE;
        private String mMimeType = "";

        /*package*/ DrmProfile(int method, boolean isDrm) {
            mDrmMethod = method;
            mIsDrm = isDrm;

            if (method == OmaDrmStore.DrmMethod.METHOD_FL
                    || method == OmaDrmStore.DrmMethod.METHOD_CD
                    || method == OmaDrmStore.DrmMethod.METHOD_FLDCF) {
                mMimeType = OmaDrmStore.DrmObjectMime.MIME_DRM_MESSAGE;
            } else if (method == OmaDrmStore.DrmMethod.METHOD_SD) {
                mMimeType = OmaDrmStore.DrmObjectMime.MIME_DRM_CONTENT;
            }
        }

        public boolean isDrm() {
            return mIsDrm;
        }

        public int getMethod() {
            return mDrmMethod;
        }

        public String getMimeType() {
            return mMimeType;
        }

        public boolean isDrmDcfFile() {
            return OmaDrmUtils.isDrmDcfFile(mMimeType, null);
        }

        public boolean isDrmMsgFile() {
            return OmaDrmUtils.isDrmMsgFile(mMimeType, null);
        }
    }

    /**
     * get the returned information (a string) from DrmInfoStatus instance
     *
     * @param status DrmInfoStatus instance
     * @return The message contained in DrmInfoStatus. may be empty
     */
    public static String getMsgFromInfoStatus(DrmInfoStatus status) {
        byte[] data = status.data.getData();
        String message = "";
        if (null != data) {
            try {
                // the information shall be in format of ASCII string
                message = new String(data, "US-ASCII");
            } catch (UnsupportedEncodingException e) {
                Log.e(TAG, "Unsupported encoding type");
                message = "";
            }
        }

        Log.v(TAG, "getMsgFromInfoStatus : >" + message);
        return message;
    }

    /*package*/ static String convertUriToPath(Context context, Uri uri) {
        Log.v(TAG, "convertUriToPath : " + uri + " @" + context);

        String path = null;
        if (null != uri) {
            String scheme = uri.getScheme();
            if (null == scheme || scheme.equals("") || scheme.equals(ContentResolver.SCHEME_FILE)) {
                path = uri.getPath();
            } else if (scheme.equals("http")) {
                path = uri.toString();
            } else if (scheme.equals(ContentResolver.SCHEME_CONTENT)) {
                String[] projection = new String[] {MediaStore.MediaColumns.DATA};
                Cursor cursor = null;
                try {
                    cursor = context.getContentResolver().query(uri, projection, null, null, null);
                    if (null == cursor || 0 == cursor.getCount() || !cursor.moveToFirst()) {
                        throw new IllegalArgumentException("Given Uri could not be found in media store");
                    }
                    int pathIndex = cursor.getColumnIndexOrThrow(MediaStore.MediaColumns.DATA);
                    path = cursor.getString(pathIndex);
                } catch (SQLiteException e) {
                    throw new IllegalArgumentException("Given Uri is not formatted in a way so that it can be found in media store.");
                } finally {
                    if (null != cursor) {
                        cursor.close();
                    }
                }
            } else {
                throw new IllegalArgumentException("Given Uri scheme is not supported");
            }
        }

        Log.v(TAG, "convertUriToPath : >" + path);
        return path;
    }

    /**
     * this is used when trying to save a DRM protected file (OMA DRM v1.0) on
     * device storage. The general idea is to adjust the file suffix to ".dcf".
     * e.g. DownloadManager sets the destination file path/name when it starts
     * downloading; MMS sets the file path/name when it saves an attachment file
     * to device local storage for further use.
     * Remarks:
     *   1. if the file name ends with ".dm" or ".dcf", the mimeType is ignored.
     *   ".dm" will be replaced with ".dcf"; ".dcf" will remain the same.
     *   2. if the file name does not end with ".dm" nor ".dcf", it will check
     *   the mimeType and add ".dcf" after the file name.
     *
     * @param name The file name. (May be full path name, or file name only)
     * @param mimeType MIME type of the file.
     * @return The Adujusted file name based on the original input.
     */
    public static String getDrmStorageFileName(String name, String mimeType) {
        Log.v(TAG, "getDrmStorageFileName : " + name + " -" + mimeType);

        int index = name.lastIndexOf(".");
        String raw = null;
        String suffix = null;
        if (-1 != index) {
            raw = name.substring(0, index);
            suffix = name.substring(index, name.length());
        } else {
            raw = name;
            suffix = "";
        }
        // we check according to the suffix
        String result = name;
        if (suffix.equalsIgnoreCase(OmaDrmStore.DrmFileExt.EXT_DRM_MESSAGE)) {
            result = raw + OmaDrmStore.DrmFileExt.EXT_DRM_CONTENT;
        } else if (suffix.equalsIgnoreCase(OmaDrmStore.DrmFileExt.EXT_DRM_CONTENT)) {
            result = name;
        } else {
             // when the suffix is not ".dm" nor ".dcf"
            if (mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_DRM_MESSAGE)
                    || mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_DRM_CONTENT)) {
                //(we keep the file name to remain the same if
                // we enables "drm.extend.suffix" system property and OMA_DRM_EXTEND_SUFFIX is true.
                result = OMA_DRM_EXTEND_SUFFIX ?
                    name : (name + OmaDrmStore.DrmFileExt.EXT_DRM_CONTENT);
            }
        }
        return result;
    }

    /**
     * judge from the mimeType whether it is an OMA DRM file
     *
     * @param mimeType MIME type of the object.
     * @return true for a matched OMA DRM v1.0 mime type; false otherwise
     */
    public static boolean isDrmFile(String mimeType) {
        Log.v(TAG, "isDrmFile : " + mimeType);
        return (mimeType != null)
            && (mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_DRM_MESSAGE)
                || mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_DRM_CONTENT)
                || mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_RIGHTS_XML)
                || mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_RIGHTS_WBXML));
    }

    /**
     * judge from the file name suffix whether it is an expected DRM file suffix
     * ".dcf" for DCF (DRM Content Format) file or ".dm" for "DRM Message" file.
     * note that we have the assumption that all the ".dcf"/".dm" files are for
     * OMA DRM case.
     *
     * @param name The file name. (May be full path name, or file name only)
     * @return true for a matched file name suffix; false otherwise
     */
    public static boolean isDrmSuffix(String name) {
        Log.v(TAG, "isDrmSuffix : " + name);
        return (name != null)
            && (name.toLowerCase().endsWith(OmaDrmStore.DrmFileExt.EXT_DRM_CONTENT)
                || name.toLowerCase().endsWith(OmaDrmStore.DrmFileExt.EXT_DRM_MESSAGE));
    }

    /**
     * judge according to the mime type or file name whether it's an OMA DRM
     * "DRM Message" file.
     * Remarks: both the parameter may be null. The mimeType is checked first
     *   and if it matches, file name will not be checked.
     *
     * @param mimeType MIME type of the file.
     * @param name The file name (may be name only, or full path name)
     * @return true if it's an OMA DRM "DRM Message" file; false otherwise
     */
    public static boolean isDrmMsgFile(String mimeType, String name) {
        Log.v(TAG, "isDrmMsgFile : " + name + " -" + mimeType);
        if (null != mimeType) {
            return mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_DRM_MESSAGE);
        }
        if (null != name) {
            return name.toLowerCase().endsWith(OmaDrmStore.DrmFileExt.EXT_DRM_MESSAGE);
        }
        return false;
    }

    /**
     * judge according to the mime type or file name whether it's an OMA DRM
     * DCF (DRM Content Format) file.
     * Remarks: both the parameter may be null. The mimeType is checked first
     *   and if it matches, file name will not be checked.
     *
     * @param mimeType MIME type of the file.
     * @param name The file name (may be name only, or full path name)
     * @return true if it's an OMA DRM DCF file; false otherwise
     */
    public static boolean isDrmDcfFile(String mimeType, String name) {
        Log.v(TAG, "isDrmDcfFile : " + name + " -" + mimeType);
        if (null != mimeType) {
            return mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_DRM_CONTENT);
        }
        if (null != name) {
            return name.toLowerCase().endsWith(OmaDrmStore.DrmFileExt.EXT_DRM_CONTENT);
        }
        return false;
    }

    /**
     * judge according to the mime type or file name whether it's an OMA DRM
     * Rights Object file.
     * Remarks: both the parameter may be null. The mimeType is checked first
     *   and if it matches, file name will not be checked.
     *
     * @param mimeType MIME type of the file.
     * @param name The file name (may be name only, or full path name)
     * @return true if it's an OMA DRM Rights Object file; false otherwise
     */
    public static boolean isDrmRightsFile(String mimeType, String name) {
        Log.v(TAG, "isDrmRightsFile : " + name + " -" + mimeType);
        if (null != mimeType) {
            return mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_RIGHTS_XML)
                || mimeType.equals(OmaDrmStore.DrmObjectMime.MIME_RIGHTS_WBXML);
        }
        if (null != name) {
            return name.toLowerCase().endsWith(OmaDrmStore.DrmFileExt.EXT_RIGHTS_XML)
                || name.toLowerCase().endsWith(OmaDrmStore.DrmFileExt.EXT_RIGHTS_WBXML);
        }
        return false;
    }
}

