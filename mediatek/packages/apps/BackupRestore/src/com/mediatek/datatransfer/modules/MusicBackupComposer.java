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
import android.provider.MediaStore.Audio;

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

/**
 * Describe class <code>MusicBackupComposer</code> here.
 * 
 * @author
 * @version 1.0
 */
public class MusicBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MusicBackupComposer";
    private ArrayList<String> mNameList;
    private static final Uri[] mMusicUriArray = {
        //Audio.Media.INTERNAL_CONTENT_URI,
        Audio.Media.EXTERNAL_CONTENT_URI
    };
    private Cursor[] mMusicCursorArray = {
        //null,
        null };

    private static final String[] mProjection = new String[] { Audio.Media._ID, Audio.Media.DATA };

    /**
     * Creates a new <code>MusicBackupComposer</code> instance.
     * 
     * @param context
     *            a <code>Context</code> value
     */
    public MusicBackupComposer(Context context) {
        super(context);
    }

    /**
     * Describe <code>init</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean init() {
        boolean result = false;
        for (int i = 0; i < mMusicCursorArray.length; ++i) {
            if (mMusicUriArray[i] == Audio.Media.EXTERNAL_CONTENT_URI) {
                String path = SDCardUtils.getStoragePath(mContext);
                if (path != null && !path.trim().equals("")) {
                    String externalSDPath = "%"
                            + path.subSequence(0, path.lastIndexOf(File.separator)) + "%";
                    mMusicCursorArray[i] = mContext.getContentResolver().query(mMusicUriArray[i],
                            mProjection, Audio.Media.DATA + " not like ?",
                            new String[] {
                                externalSDPath
                            }, null);
                }
            } else {
                mMusicCursorArray[i] = mContext.getContentResolver().query(mMusicUriArray[i],
                        mProjection, null, null, null);
            }
            if (mMusicCursorArray[i] != null) {
                mMusicCursorArray[i].moveToFirst();
                result = true;
            }
        }

        mNameList = new ArrayList<String>();
        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + getCount());
        return result;
    }

    /**
     * Describe <code>getModuleType</code> method here.
     * 
     * @return an <code>int</code> value
     */
    public final int getModuleType() {
        return ModuleType.TYPE_MUSIC;
    }

    /**
     * Describe <code>getCount</code> method here.
     * 
     * @return an <code>int</code> value
     */
    public final int getCount() {
        int count = 0;
        for (Cursor cur : mMusicCursorArray) {
            if (cur != null && !cur.isClosed() && cur.getCount() > 0) {
                count += cur.getCount();
            }
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    /**
     * Describe <code>isAfterLast</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean isAfterLast() {
        boolean result = true;
        for (Cursor cur : mMusicCursorArray) {
            if (cur != null && !cur.isAfterLast()) {
                result = false;
                break;
            }
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    /**
     * Describe <code>implementComposeOneEntity</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean implementComposeOneEntity() {
        boolean result = false;
        for (int i = 0; i < mMusicCursorArray.length; ++i) {
            if (mMusicCursorArray[i] != null && !mMusicCursorArray[i].isAfterLast()) {
                int dataColumn = mMusicCursorArray[i].getColumnIndexOrThrow(Audio.Media.DATA);
                String data = mMusicCursorArray[i].getString(dataColumn);
                String destFileName = null;
                try {
                    String tmpName = mParentFolderPath + File.separator + ModulePath.FOLDER_MUSIC +
                        data.subSequence(data.lastIndexOf(File.separator), data.length()).toString();
                    destFileName = getDestinationName(tmpName);
                    if (destFileName != null) {
                        try {
                            copyFile(data, destFileName);
                            mNameList.add(destFileName);
                            result = true;
                        } catch (IOException e) {
                            if (super.mReporter != null) {
                                super.mReporter.onErr(e);
                            }
                            MyLogger.logD(CLASS_TAG, MyLogger.MUSIC_TAG + "copy file fail");
                        }
                    }

                    MyLogger.logD(CLASS_TAG, data + ",destFileName:" + destFileName);
                } catch (StringIndexOutOfBoundsException e) {
                    MyLogger.logE(CLASS_TAG, MyLogger.MUSIC_TAG
                            + " StringIndexOutOfBoundsException");
                    e.printStackTrace();
                } catch(Exception e) {
                    e.printStackTrace();
                }


                mMusicCursorArray[i].moveToNext();
                break;
            }
        }

        return result;
    }

    /**
     * Describe <code>onStart</code> method here.
     * 
     */
    @Override
    public void onStart() {
        super.onStart();
        if(getCount() > 0) {
            File path = new File(mParentFolderPath + File.separator + ModulePath.FOLDER_MUSIC);
            if (path.exists()) {
                deleteFolder(path);
            }

            path.mkdirs();
        }
    }

    /**
     * Describe <code>onEnd</code> method here.
     * 
     */
    @Override
    public void onEnd() {
        super.onEnd();
        if (mNameList != null && mNameList.size() > 0) {
            FileUtils.scanPathforMediaStore(mParentFolderPath + File.separator
                    + ModulePath.FOLDER_MUSIC, mContext);
            mNameList.clear();
        }

        for (Cursor cur : mMusicCursorArray) {
            if (cur != null) {
                cur.close();
                cur = null;
            }
        }
    }


    /**
     * Describe <code>getDestinationName</code> method here.
     *
     * @param name a <code>String</code> value
     * @return a <code>String</code> value
     */
    private String getDestinationName(String name) {
        if (!mNameList.contains(name)) {
            return name;
        } else {
            return rename(name);
        }
    }

    /**
     * Describe <code>rename</code> method here.
     * 
     * @param name
     *            a <code>String</code> value
     * @return a <code>String</code> value
     */
    private String rename(String name) {
        String tmpName;
        int id = name.lastIndexOf(".");
        int id2, leftLen;
        for (int i = 1; i < (1 << 12); ++i) {
            leftLen = 255 - (1 + Integer.toString(i).length() + name.length() - id);
            id2 = id <= leftLen ? id : leftLen;
            tmpName = name.subSequence(0, id2) + "~" + i + name.subSequence(id, name.length());
            if (!mNameList.contains(tmpName)) {
                return tmpName;
            }
        }

        return null;
    }

    private void deleteFolder(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                int count = mContext.getContentResolver().delete(Audio.Media.EXTERNAL_CONTENT_URI,
                                                                 Audio.Media.DATA + " like ?", new String[] { file.getAbsolutePath() });
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
