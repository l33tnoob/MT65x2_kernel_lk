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

package com.mediatek.bluetooth.opp.mmi;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.MediaStore;
import android.provider.MediaStore.MediaColumns;
import android.provider.OpenableColumns;

import com.mediatek.bluetooth.opp.adp.OppConstants;
import com.mediatek.bluetooth.util.BtLog;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

public class UriDataUtils {

    private static final String OPPC_PUSH_TEMP_FILENAME = "__OPPC_PuSH_";

    private static final String CALENDAR_AUTHORITY = "com.mediatek.calendarimporter";

    public static UriData getUriData(Context context, Uri uri) {

        Cursor cursor = null;
        UriData result = null;

        String scheme = uri.getScheme();

        // File Uri - example: file:///sdcard/bluetooth/001.png
        if ("file".equals(scheme)) {

            BtLog.d("getUriData() - File: " + uri);

            result = new UriData(uri);
            result.setData(uri.getPath());
            result.setName(uri.getLastPathSegment());
            result.setSize(new File(uri.getPath()).length());
            return result;
        } else if ("content".equals(scheme)) {

            BtLog.d("getUriData() - Content: " + uri);

            String authority = uri.getAuthority();
            String[] projection;

            // Contacts
            if (ContactsContract.AUTHORITY.equals(authority) || CALENDAR_AUTHORITY.equals(authority)) {

                projection = new String[] {
                        OpenableColumns.SIZE, OpenableColumns.DISPLAY_NAME
                };
            } else { // Media

                projection = new String[] {
                        OpenableColumns.SIZE, OpenableColumns.DISPLAY_NAME, MediaColumns.DATA
                };
            }

            try {
                // query from content provider
                cursor = context.getContentResolver().query(uri, projection, null, null, null);

                if (cursor == null || !cursor.moveToFirst()) {

                    BtLog.w("getUriData() - no query result for content uri: " + uri);
                    return null;
                }

                result = new UriData(uri);
                result.setName(cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)));
                result.setSize(cursor.getLong(cursor.getColumnIndex(OpenableColumns.SIZE)));

                // Contacts
                // We need convert contact filename for ALPS00277849, use
                // date-time format as new vcard file name
                if (ContactsContract.AUTHORITY.equals(authority) || CALENDAR_AUTHORITY.equals(authority) ||
                            cursor.getString(cursor.getColumnIndex(MediaColumns.DATA)) == null) {

                    if (ContactsContract.AUTHORITY.equals(authority) || CALENDAR_AUTHORITY.equals(authority)) {
                        // result.setName( result.getName() );
                        result.setName(getPushTempFileName(authority));
                    }
                    result.setData(context.getApplicationContext().getFileStreamPath(OPPC_PUSH_TEMP_FILENAME).getPath());

                    if (result.getSize() == 0) {

                        InputStream in = null;
                        try {
                            in = context.getContentResolver().openInputStream(uri);
                            if (in != null) {

                                result.setSize(in.available());
                            }
                        } catch (FileNotFoundException ex) {

                            BtLog.e("getUriData() - get Contact vCard fail: " + ex.getMessage());
                        } catch (IOException ex) {
                            BtLog.e("getUriData() - get Contact vCard fail: " + ex.getMessage());
                        } finally {

                            if (in != null) {
                                try {
                                    in.close();
                                } catch (IOException ex) {

                                    BtLog.e("getUriData() - fail to close input-stream for uri[" + uri + "]");
                                }
                            }
                        }
                    }
                } else {
                    // Media
                    result.setData(cursor.getString(cursor.getColumnIndex(MediaColumns.DATA)));
                }
                return result;
            } finally {

                if (cursor != null) {
                    cursor.close();
                }
            }
        } else if (OppClientActivity.LINK_SHARE_URI_SCHEME.equals(scheme)) {

            BtLog.d("getUriData() - LinkShare: " + uri);
            String text = uri.getSchemeSpecificPart();
            String subject = uri.getFragment();
            Uri tempFileUri = Utils.createContextFileForText(context, subject, text);
            if (tempFileUri != null) {

                result = new UriData(uri);
                result.setData(tempFileUri.getPath());
                result.setName(tempFileUri.getLastPathSegment());
                result.setSize(new File(tempFileUri.getPath()).length());
            }
            return result;
        } else {
            BtLog.e("getUriData() - unsupported uri: " + uri);
            return null;
        }
    }

    /**
     * create data file and setup file size ( in UriData object )
     *
     * @param context
     * @param uriData
     * @return
     */
    public static boolean openUriData(Context context, Uri uri, String dataPath) {

        String scheme = uri.getScheme();

        // File Uri - example: file:///sdcard/bluetooth/001.png
        if ("file".equals(scheme)) {

            // already exist => check available
            return true;
        } else if ("content".equals(scheme)) {

            String authority = uri.getAuthority();

            // Media
            if (MediaStore.AUTHORITY.equals(authority)) {

                // already exist => check available
                return true;
            } else if (ContactsContract.AUTHORITY.equals(authority) // Contacts
                    || CALENDAR_AUTHORITY.equals(authority)
                    || dataPath.endsWith(OPPC_PUSH_TEMP_FILENAME)) {

                // create temp file for vCard content
                try {
                    // open vCard content / temp file and then copy
                    InputStream in = context.getContentResolver().openInputStream(uri);
                    if (in == null) {
                        BtLog.e("openUriData() - open Contact file fail: openInputStream(" + uri + ") return null");
                        return false;
                    }
                    FileOutputStream out = context.getApplicationContext().openFileOutput(OPPC_PUSH_TEMP_FILENAME,
                            Context.MODE_PRIVATE);
                    UriDataUtils.copyStreamContent(in, out);
                    return true;
                } catch (FileNotFoundException ex) {

                    BtLog.e("openUriData() - open Contact file fail: " + ex.getMessage());
                    return false;
                } catch (IOException ex) {
                    return false;
                }
            } else {
                BtLog.w("openUriData() - unsupported content uri: " + uri);
                return false;
            }
        } else if (OppClientActivity.LINK_SHARE_URI_SCHEME.equals(scheme)) {

            BtLog.d("getUriData() - LinkShare: " + uri);
            String text = uri.getSchemeSpecificPart();
            String subject = uri.getFragment();
            Uri tempFileUri = Utils.createContextFileForText(context, subject, text);
            return (tempFileUri != null);
        } else {
            BtLog.w("openUriData() - unsupported uri: " + uri);
            return false;
        }
    }

    /**
     * copy the content from InputStream to OutputStream
     *
     * @param in
     * @param out
     * @return
     * @throws IOException
     */
    public static long copyStreamContent(InputStream in, OutputStream out) throws IOException {

        int read;
        long total = 0;
        byte[] buf = new byte[OppConstants.VCARD_BUF_SIZE];

        while ((read = in.read(buf)) != -1) {

            out.write(buf, 0, read);
            total += read;
        }
        in.close();
        out.close();
        return total;
    }

    /**
     * close the opened task data ( open by oppcOpenTaskData )
     *
     * @param task
     * @return success or failure
     */
    public static void closeUriData(Context context, Uri uri, String dataPath) {

        String scheme = uri.getScheme();

        if ("content".equals(scheme)) {

            String authority = uri.getAuthority();

            // Contacts & Calendar
            if (ContactsContract.AUTHORITY.equals(authority) || CALENDAR_AUTHORITY.equals(authority)
                    || dataPath.endsWith(OPPC_PUSH_TEMP_FILENAME)) {
                context.getApplicationContext().deleteFile(OPPC_PUSH_TEMP_FILENAME);
            }
        }
    }

    /**
     * get temp file name for contact/calender objects
     *
     * @param authority
     * @return temp file name
     */
    public static String getPushTempFileName(String authority) {

        String tempFileName = null;

        SimpleDateFormat tempFormatter = new SimpleDateFormat("yyyyMMdd_hhmmss");
        Date curDate = new Date(System.currentTimeMillis());
        String timeStr = tempFormatter.format(curDate);

        // Contacts - such as 20120508_132345.vcf
        if (ContactsContract.AUTHORITY.equals(authority)) {

            tempFileName = timeStr + ".vcf";
        } else if (CALENDAR_AUTHORITY.equals(authority)) { // Calendar - such as 20120508_132345.vcs

            tempFileName = timeStr + ".vcs";
        } else {

            tempFileName = timeStr;
        }

        return tempFileName;
    }
}
