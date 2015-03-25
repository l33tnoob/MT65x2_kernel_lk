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

package com.mediatek.datatransfer.modules;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore.Images;
import android.provider.MediaStore;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.FileUtils;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

public class PictureBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/PictureBackupComposer";

    private static final String[] mProjection = new String[] { MediaStore.Images.Media._ID,
            MediaStore.Images.Media.DATA };
    private static final Uri[] mPictureUriArray = { Images.Media.INTERNAL_CONTENT_URI,
            Images.Media.EXTERNAL_CONTENT_URI };
    private Cursor[] mPictureCursorArray = { null, null };

    private ArrayList<String> mFileNameList = null;

    public PictureBackupComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_PICTURE;
    }

    public int getCount() {
        int count = 0;
        for (Cursor cur : mPictureCursorArray) {
            if (cur != null && !cur.isClosed() && cur.getCount() > 0) {
                count += cur.getCount();
            }
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    public boolean init() {
        boolean result = false;
        for (int i = 0; i < mPictureCursorArray.length; ++i) {
            if (mPictureUriArray[i] == Images.Media.EXTERNAL_CONTENT_URI) {
                String path = SDCardUtils.getStoragePath(mContext);
                if (path != null && !path.trim().equals("")) {
                    String externalSDPath = "%"
                            + path.subSequence(0, path.lastIndexOf(File.separator)) + "%";
                    mPictureCursorArray[i] = mContext.getContentResolver().query(
                            mPictureUriArray[i],
                            mProjection, MediaStore.Images.Media.DATA + " not like ?",
                            new String[] {
                                externalSDPath
                            }, null);
                }
            } else {
                mPictureCursorArray[i] = mContext.getContentResolver().query(mPictureUriArray[i],
                        mProjection, null, null, null);
            }

            if (mPictureCursorArray[i] != null) {
                mPictureCursorArray[i].moveToFirst();
                result = true;
            }
        }

        mFileNameList = new ArrayList<String>();

        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + getCount());
        return result;
    }

    @Override
    public boolean isAfterLast() {
        boolean result = true;
        for (Cursor cur : mPictureCursorArray) {
            if (cur != null && !cur.isAfterLast()) {
                result = false;
                break;
            }
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    @Override
    public boolean implementComposeOneEntity() {
        boolean result = false;
        for (int i = 0; i < mPictureCursorArray.length; ++i) {
            if (mPictureCursorArray[i] != null && !mPictureCursorArray[i].isAfterLast()) {
                int dataColumn = mPictureCursorArray[i]
                        .getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
                String data = mPictureCursorArray[i].getString(dataColumn);

                String destnationFileName = null;
                try {
                    String tmpName = mParentFolderPath + File.separator + ModulePath.FOLDER_PICTURE +
                        data.subSequence(data.lastIndexOf(File.separator), data.length()).toString();
                    destnationFileName = getDestinationName(tmpName);
                } catch (StringIndexOutOfBoundsException e) {
                    MyLogger.logD(CLASS_TAG, "data OutOfBoundsException:data" + data);
                } catch(Exception e) {
                    e.printStackTrace();
                }

                if (destnationFileName != null) {
                    try {
                        copyFile(data, destnationFileName);
                        mFileNameList.add(destnationFileName);
                        result = true;
                    } catch (IOException e) {
                        if (super.mReporter != null) {
                            super.mReporter.onErr(e);
                        }
                        MyLogger.logD(CLASS_TAG, "copy file fail");
                    }
                }
                MyLogger.logD(CLASS_TAG, "pic:" + data + ",destName:" + destnationFileName);
                mPictureCursorArray[i].moveToNext();
                break;
            }
        }

        return result;
    }

    private String getDestinationName(String name) {
        if (!mFileNameList.contains(name)) {
            return name;
        } else {
            return rename(name);
        }
    }

    private String rename(String name) {
        String tmpName;
        int id = name.lastIndexOf(".");
        int id2, leftLen;
        for (int i = 1; i < (1 << 12); ++i) {
            leftLen = 255 - (1 + Integer.toString(i).length() + name.length() - id);
            id2 = id <= leftLen ? id : leftLen;
            tmpName = name.subSequence(0, id2) + "~" + i + name.subSequence(id, name.length());
            if (!mFileNameList.contains(tmpName)) {
                return tmpName;
            }
        }

        return null;
    }

    /**
     * Describe <code>onStart</code> method here.
     *
     */
    public final void onStart() {
        super.onStart();
        if(getCount() > 0) {
            File path = new File(mParentFolderPath + File.separator + ModulePath.FOLDER_PICTURE);
            if (path.exists()) {
                deleteFolder(path);
            }

            path.mkdirs();
        }

    }

    public void onEnd() {
        super.onEnd();
        if (mFileNameList != null && mFileNameList.size() > 0) {
            FileUtils.scanPathforMediaStore(mParentFolderPath + File.separator
                    + ModulePath.FOLDER_PICTURE, mContext);
            mFileNameList.clear();
        }

        for (Cursor cur : mPictureCursorArray) {
            if (cur != null) {
                cur.close();
                cur = null;
            }
        }
    }

    private void deleteFolder(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                int count = mContext.getContentResolver().delete(MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                                                                 MediaStore.Images.Media.DATA + " like ?",
                                                                 new String[] { file.getAbsolutePath() });
                MyLogger.logD(CLASS_TAG, "deleteFolder():" + count + ":" + file.getAbsolutePath());
                file.delete();
            } else if (file.isDirectory()) {
                File files[] = file.listFiles();
                for (int i = 0; i < files.length; ++i) {
                    this.deleteFolder(files[i]);
                }
            }

            file.delete();
        }
    }


    private void copyFile(String srcFile, String destFile) throws IOException {
        try {
            File f1 = new File(srcFile);
            if (f1.exists() && f1.isFile()) {
                InputStream inStream = new FileInputStream(srcFile);
                FileOutputStream outStream = new FileOutputStream(destFile);
                byte[] buf = new byte[1024];
                int byteRead = 0;
                while ((byteRead = inStream.read(buf)) != -1) {
                    outStream.write(buf, 0, byteRead);
                }
                outStream.flush();
                outStream.close();
                inStream.close();
            }
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
