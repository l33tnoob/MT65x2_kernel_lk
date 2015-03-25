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

package com.mediatek.filemanager.utils;

import android.text.TextUtils;

import com.mediatek.filemanager.FileInfo;
import com.mediatek.filemanager.MountPointManager;
import com.mediatek.filemanager.service.FileManagerService;
import com.mediatek.filemanager.service.FileManagerService.OperationEventListener;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.util.List;


/**
 * A utilize class for basic file operations.
 */
public final class FileUtils {
    private static final String TAG = "FileUtils";
    public static final String UNIT_B = "B";
    public static final String UNIT_KB = "KB";
    public static final String UNIT_MB = "MB";
    public static final String UNIT_GB = "GB";
    public static final String UNIT_TB = "TB";
    private static final int UNIT_INTERVAL = 1024;
    private static final double ROUNDING_OFF = 0.005;
    private static final int DECIMAL_NUMBER = 100;

    /**
     * This method check the file name is valid.
     * 
     * @param fileName the input file name
     * @return valid or the invalid type
     */
    public static int checkFileName(String fileName) {
        if (TextUtils.isEmpty(fileName) || fileName.trim().length() == 0) {
            return OperationEventListener.ERROR_CODE_NAME_EMPTY;
        } else {
            try {
                int length = fileName.getBytes("UTF-8").length;
                // int length = fileName.length();
                LogUtils.d(TAG, "checkFileName: " + fileName + ",lenth= " + length);
                if (length > FileInfo.FILENAME_MAX_LENGTH) {
                    LogUtils.d(TAG, "checkFileName,fileName is too long,len=" + length);
                    return OperationEventListener.ERROR_CODE_NAME_TOO_LONG;
                } else {
                    return OperationEventListener.ERROR_CODE_NAME_VALID;
                }
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
                return OperationEventListener.ERROR_CODE_NAME_EMPTY;
            }
        }
    }

    /**
     * This method gets extension of certain file.
     * 
     * @param fileName name of a file
     * @return Extension of the file's name
     */
    public static String getFileExtension(String fileName) {
        if (fileName == null) {
            return null;
        }
        String extension = null;
        final int lastDot = fileName.lastIndexOf('.');
        if ((lastDot >= 0)) {
            extension = fileName.substring(lastDot + 1).toLowerCase();
        }
        return extension;
    }

    /**
     * This method gets name of certain file from its path.
     * 
     * @param absolutePath the file's absolute path
     * @return name of the file
     */
    public static String getFileName(String absolutePath) {
        int sepIndex = absolutePath.lastIndexOf(MountPointManager.SEPARATOR);
        if (sepIndex >= 0) {
            return absolutePath.substring(sepIndex + 1);
        }
        return absolutePath;

    }

    /**
     * This method gets path to directory of certain file(or folder).
     * 
     * @param filePath path to certain file
     * @return path to directory of the file
     */
    public static String getFilePath(String filePath) {
        int sepIndex = filePath.lastIndexOf(MountPointManager.SEPARATOR);
        if (sepIndex >= 0) {
            return filePath.substring(0, sepIndex);
        }
        return "";

    }

    /**
     * This method generates a new suffix if a name conflict occurs, ex: paste a file named
     * "stars.txt", the target file name would be "stars(1).txt"
     * 
     * @param file the conflict file
     * @return a new name for the conflict file
     */

    public static File genrateNextNewName(File file) {
        String parentDir = file.getParent();
        String fileName = file.getName();
        String ext = "";
        int newNumber = 0;
        if (file.isFile()) {
            int extIndex = fileName.lastIndexOf(".");
            if (extIndex != -1) {
                ext = fileName.substring(extIndex);
                fileName = fileName.substring(0, extIndex);
            }
        }

        if (fileName.endsWith(")")) {
            int leftBracketIndex = fileName.lastIndexOf("(");
            if (leftBracketIndex != -1) {
                String numeric = fileName.substring(leftBracketIndex + 1, fileName.length() - 1);
                if (numeric.matches("[0-9]+")) {
                    LogUtils.v(TAG, "Conflict folder name already contains (): " + fileName
                            + "thread id: " + Thread.currentThread().getId());
                    try {
                        newNumber = Integer.parseInt(numeric);
                        newNumber++;
                        fileName = fileName.substring(0, leftBracketIndex);
                    } catch (NumberFormatException e) {
                        LogUtils.e(TAG, "Fn-findSuffixNumber(): " + e.toString());
                    }
                }
            }
        }
        StringBuffer sb = new StringBuffer();
        sb.append(fileName).append("(").append(newNumber).append(")").append(ext);
        if (FileUtils.checkFileName(sb.toString()) < 0) {
            return null;
        }
        return new File(parentDir, sb.toString());
    }

    /**
     * This method converts a size to a string
     * 
     * @param size the size of a file
     * @return the string represents the size
     */
    public static String sizeToString(long size) {
        String unit = UNIT_B;
        if (size < DECIMAL_NUMBER) {
            LogUtils.d(TAG, "sizeToString(),size = " + size);
            return Long.toString(size) + " " + unit;
        }
        
        unit = UNIT_KB;
        double sizeDouble = (double) size / (double) UNIT_INTERVAL;
        if (sizeDouble > UNIT_INTERVAL) {
            sizeDouble = (double) sizeDouble / (double) UNIT_INTERVAL;
            unit = UNIT_MB;
        }
        if (sizeDouble > UNIT_INTERVAL) {
            sizeDouble = (double) sizeDouble / (double) UNIT_INTERVAL;
            unit = UNIT_GB;
        }
        if (sizeDouble > UNIT_INTERVAL) {
            sizeDouble = (double) sizeDouble / (double) UNIT_INTERVAL;
            unit = UNIT_TB;
        }

        // Add 0.005 for rounding-off.
        long sizeInt = (long) ((sizeDouble + ROUNDING_OFF) * DECIMAL_NUMBER); // strict to two
        // decimal places
        double formatedSize = ((double) sizeInt) / DECIMAL_NUMBER;
        LogUtils.d(TAG, "sizeToString(): " + formatedSize + unit);

        if (formatedSize == 0) {
            return "0" + " " + unit;
        } else {
            return Double.toString(formatedSize) + " " + unit;
        }
    }

    /**
     * This method gets the MIME type from multiple files (order to return: image->video->other)
     * 
     * @param service service of FileManager
     * @param currentDirPath the current directory
     * @param files a list of files
     * @return the MIME type of the multiple files
     */
    public static String getMultipleMimeType(FileManagerService service, String currentDirPath,
            List<FileInfo> files) {
        String mimeType = null;

        for (FileInfo info : files) {
            mimeType = info.getFileMimeType(service);
            if ((null != mimeType)
                    && (mimeType.startsWith("image/") || mimeType.startsWith("video/"))) {
                break;
            }
        }

        if (mimeType == null || mimeType.startsWith("unknown")) {
            mimeType = FileInfo.MIMETYPE_UNRECOGNIZED;
        }
        LogUtils.d(TAG, "Multiple files' mimetype is " + mimeType);
        return mimeType;
    }

    /**
     * This method checks weather extension of certain file(not folder) is changed.
     * 
     * @param newFilePath path to file before modified.(Here modify means rename).
     * @param oldFilePath path to file after modified.
     * @return true for extension changed, false for not changed.
     */
    public static boolean isExtensionChange(String newFilePath, String oldFilePath) {
        File oldFile = new File(oldFilePath);
        if (oldFile.isDirectory()) {
            return false;
        }
        String origFileExtension = FileUtils.getFileExtension(oldFilePath);
        String newFileExtension = FileUtils.getFileExtension(newFilePath);
        if (((origFileExtension != null) && (!origFileExtension.equals(newFileExtension)))
                || ((newFileExtension != null) && (!newFileExtension.equals(origFileExtension)))) {
            return true;
        }
        return false;
    }
}
