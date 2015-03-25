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

package com.mediatek.rcse.service;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Environment;
import android.os.StatFs;
import android.text.TextUtils;

import com.mediatek.rcse.activities.widgets.AsyncGalleryView;
import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.file.FileFactory;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Random;

/**
 * This is the tool class
 */
public class Utils {

    public static final String TAG = "Utils";
    public static final double SIZE_K = 1024;
    public static final double SIZE_M = 1048576;
    public static final int SIZE_TYPE_TOTAL_SIZE = 0;
    public static final int SIZE_TYPE_BYTE = 1;
    public static final int SIZE_TYPE_K_BYTE = 2;
    public static final int SIZE_TYPE_M = 3;
    public static final String BLANK_SPACE = " ";
    public static final int ROTATE_DEGREES_0 = 0;
    public static final int ROTATE_DEGREES_90 = 90;
    public static final int ROTATE_DEGREES_180 = 180;
    public static final int ROTATE_DEGREES_270 = 270;
    public static final String IS_GROUP_CHAT = "isChatGroup";
    public static final String CHAT_TAG = "chatTag";
    public static final String CHAT_PARTICIPANT = "chatParticipant";
    public static final String CHAT_PARTICIPANTS = "chatParticipants";
    public static final String MESSAGE = "message";
    public static final String TYPE = "type";
    public static final String TITLE = "title";
    public static final String CONTENT_VIEW = "contentView";
    private static final int MAX_COMPRESS_QUALITY = 100;
    private static final int SPECIAL_COMPRESS_QUALITY = 30;
    // When a image file size is larger than 50k or iamge file height(width) is
    // larger than 500 then compress using quality 30, in other case compress
    // using quality 100
    //When compress using inSampleSize = width/500
    private static final int MAX_WIDTH_REQUIRED = 500;
    // 50K
    private static final int MAX_SIZE_REQUIRED = 50 * 1024;
    public static final File SDCARDDIEFILE = Environment.getExternalStorageDirectory();
    public static final String SLASH = "/";
    public static final String RCSE_FILE_DIR = SDCARDDIEFILE + SLASH + "Joyn";
    public static final String RCSE_COMPRESSED_FILE_DIR = RCSE_FILE_DIR + SLASH + "compressed";
    public static final String DEFAULT_REMOTE = "defaultRemote";
    public static final Random RANDOM = new Random();

    /** Image */
    public static final String FILE_TYPE_IMAGE = "image";
    /** Audio */
    public static final String FILE_TYPE_AUDIO = "audio";
    /** Video */
    public static final String FILE_TYPE_VIDEO = "video";
    /** Text */
    public static final String FILE_TYPE_TEXT = "text";
    /** Application */
    public static final String FILE_TYPE_APP = "application";
    /**
     * File size.
     */
    public static final String FILE_SIZE = "filesize";
    public static final int GROUP_STATUS_CANSENDMSG = 0;
    public static final int GROUP_STATUS_UNAVIALABLE = 1;
    public static final int GROUP_STATUS_TERMINATED = 2;
    public static final int GROUP_STATUS_REJOINING = 3;
    public static final int GROUP_STATUS_RESTARTING = 4;

    /**
     * This function is to get file size type, byte, K, M...
     * 
     * @param file size
     */
    public static int getSizeType(long fileSize) {
        if (fileSize < SIZE_K) {
            return SIZE_TYPE_BYTE;
        } else if (fileSize < SIZE_M) {
            return SIZE_TYPE_K_BYTE;
        } else {
            return SIZE_TYPE_M;
        }
    }

    /**
     * Get URI of file.
     * 
     * @param filePath The file path.
     * @return The URI of the file.
     */
    public static Uri getFileNameUri(String filePath) {
        Logger.d(TAG, "getFileNameUri() entry, the filePath is " + filePath);
        if (TextUtils.isEmpty(filePath)) {
            return null;
        }
        Uri fileNameUri = null;
        if (!filePath.startsWith(AsyncGalleryView.FILE_SCHEMA)) {
            fileNameUri = Uri.parse(AsyncGalleryView.FILE_SCHEMA + filePath);
        } else {
            fileNameUri = Uri.parse(filePath);
        }
        Logger.d(TAG, "getFileNameUri() exit, the fileNameUri is " + fileNameUri);
        return fileNameUri;
    }

    /**
     * this function is to convert file size to byte, K, M
     * 
     * @param fileSize, the transfer file size
     * @param sizeType, used to handle set progress size
     */
    public static String formatFileSizeToString(long fileSize, int sizeType) {
        ApiManager apiManager = ApiManager.getInstance();
        if (apiManager != null) {
            Context context = apiManager.getContext();
            if (context != null) {
                if (sizeType == SIZE_TYPE_TOTAL_SIZE) {
                    if (fileSize < (SIZE_K)) {
                        return String.valueOf(fileSize) + BLANK_SPACE
                                + context.getString(R.string.file_transfer_bytes);
                    } else if (fileSize < (SIZE_M)) {
                        double sizeK = fileSize / SIZE_K;
                        return String.format("%1$.2f", sizeK) + BLANK_SPACE
                                + context.getString(R.string.file_transfer_k_bytes);
                    } else {
                        double sizeM = fileSize / SIZE_M;
                        return String.format("%1$.2f", sizeM) + BLANK_SPACE
                                + context.getString(R.string.file_transfer_m);
                    }
                } else {
                    switch (sizeType) {
                        case SIZE_TYPE_BYTE:
                            return String.valueOf(fileSize) + BLANK_SPACE
                                    + context.getString(R.string.file_transfer_bytes);
                        case SIZE_TYPE_K_BYTE:
                            double sizeK = fileSize / SIZE_K;
                            return String.format("%1$.2f", sizeK) + BLANK_SPACE
                                    + context.getString(R.string.file_transfer_k_bytes);
                        case SIZE_TYPE_M:
                            double sizeM = fileSize / SIZE_M;
                            return String.format("%1$.2f", sizeM) + BLANK_SPACE
                                    + context.getString(R.string.file_transfer_m);
                        default:
                            return null;
                    }
                }
            } else {
                Logger.e(TAG, "formatFileSizeToString get context is null!");
                return null;
            }
        } else {
            Logger.e(TAG, "formatFileSizeToString, ApiManager is null!");
            return null;
        }
    }

    /**
     * Get file extension of a given file.
     * 
     * @param fileName The file name.
     * @return File extension of the given file name.
     */
    public static String getFileExtension(String fileName) {
        Logger.d(TAG, "getFileExtension() entry, the fileName is " + fileName);
        String extension = null;
        if (TextUtils.isEmpty(fileName)) {
            Logger.d(TAG, "getFileExtension() entry, the fileName is null");
            return null;
        }
        int lastDot = fileName.lastIndexOf(AsyncGalleryView.SEPRATOR);
        extension = fileName.substring(lastDot + AsyncGalleryView.INDEX_OFFSET).toLowerCase();
        Logger.d(TAG, "getFileExtension() entry, the extension is " + extension);
        return extension;
    }

    /**
     * this function is to convert file size to byte, K, M
     * 
     * @param context The context.
     * @param fileSize The transfer file size
     * @param sizeType Used to handle set progress size
     */
    public static String formatFileSizeToString(Context context, long fileSize, int sizeType) {
        if (sizeType == SIZE_TYPE_TOTAL_SIZE) {
            if (fileSize < (SIZE_K)) {
                return String.valueOf(fileSize) + BLANK_SPACE
                        + context.getString(R.string.file_transfer_bytes);
            } else if (fileSize < (SIZE_M)) {
                double sizeK = fileSize / SIZE_K;
                return String.format("%1$.2f", sizeK) + BLANK_SPACE
                        + context.getString(R.string.file_transfer_k_bytes);
            } else {
                double sizeM = fileSize / SIZE_M;
                return String.format("%1$.2f", sizeM) + BLANK_SPACE
                        + context.getString(R.string.file_transfer_m);
            }
        } else {
            switch (sizeType) {
                case SIZE_TYPE_BYTE:
                    return String.valueOf(fileSize) + BLANK_SPACE
                            + context.getString(R.string.file_transfer_bytes);
                case SIZE_TYPE_K_BYTE:
                    double sizeK = fileSize / SIZE_K;
                    return String.format("%1$.2f", sizeK) + BLANK_SPACE
                            + context.getString(R.string.file_transfer_k_bytes);
                case SIZE_TYPE_M:
                    double sizeM = fileSize / SIZE_M;
                    return String.format("%1$.2f", sizeM) + BLANK_SPACE
                            + context.getString(R.string.file_transfer_m);
                default:
                    return null;
            }
        }
    }

    /**
     * Returns the degrees of a picture should be rotated. Notice that do not
     * call this method in ui thread.
     * 
     * @param fileFullPath A picture's full path name.
     * @return the degrees of a picture should be rotated.
     */
    public static int getDegreesRotated(String fileFullPath) {
        int degreesRotated = ROTATE_DEGREES_0;
        int orientation = 0;
        ExifInterface exif;
        try {
            exif = new ExifInterface(fileFullPath);
            orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, 0);
            Logger.v(TAG, "orientation=" + orientation);
            degreesRotated = getExifRotation(orientation);
            Logger.v(TAG, "degreesRotated=" + degreesRotated);
            return degreesRotated;
        } catch (IOException e) {
            Logger.e(TAG, "Construct ExifInterface failed, fileFullPath = " + fileFullPath);
            e.printStackTrace();
            return degreesRotated;
        }
    }

    private static int getExifRotation(int orientation) {
        int degrees = ROTATE_DEGREES_0;
        switch (orientation) {
            case ExifInterface.ORIENTATION_NORMAL:
                degrees = ROTATE_DEGREES_0;
                break;
            case ExifInterface.ORIENTATION_ROTATE_90:
                degrees = ROTATE_DEGREES_90;
                break;
            case ExifInterface.ORIENTATION_ROTATE_180:
                degrees = ROTATE_DEGREES_180;
                break;
            case ExifInterface.ORIENTATION_ROTATE_270:
                degrees = ROTATE_DEGREES_270;
                break;
            default:
                break;
        }
        return degrees;
    }

    /**
     * Rotate a bitmap. Do not call this method on ui thread.
     * 
     * @param bitmap The bitmap you want to rotate.
     * @param degrees The degrees you want to rotate.
     * @return The rotated bitmap.
     */
    public static Bitmap rotate(Bitmap bitmap, int degrees) {
        Logger.v(TAG, "rotate(),degrees = " + degrees);
        Bitmap tmpBitmap = null;
        if (degrees != 0 && bitmap != null) {
            Matrix matrix = new Matrix();
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            Logger.v(TAG, "width = " + width + ",height = " + height);
            matrix.setRotate(degrees, (float) width / 2, (float) height / 2);
            try {
                tmpBitmap = Bitmap.createBitmap(bitmap, 0, 0, width, height, matrix, true);
                if (bitmap != tmpBitmap) {
                    bitmap.recycle();
                }
            } catch (OutOfMemoryError ex) {
                Logger.e(TAG, "We have no memory to rotate. Return the original bitmap");
                ex.printStackTrace();
            }
        }
        return tmpBitmap;
    }

    /**
     * Compress the image
     * 
     * @return The compressed image file full name.
     */
    public static String compressImage(final String originFileName) {
        Logger.d(TAG, "compressImage(): originFileName = " + originFileName);
        BitmapFactory.Options bitmapOptions = new BitmapFactory.Options();
        // To get the bitmap's width and height
        bitmapOptions.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(originFileName, bitmapOptions);
        int originWidht = bitmapOptions.outWidth;
        int originHeight = bitmapOptions.outHeight;

        bitmapOptions.inJustDecodeBounds = false;
        bitmapOptions.inSampleSize = originWidht / MAX_WIDTH_REQUIRED;
        Bitmap bitmap = null;
        try {
            bitmap = BitmapFactory.decodeFile(originFileName, bitmapOptions);
        } catch (OutOfMemoryError e) {
            Logger.e(
                    TAG,
                    "So big the picture, comperss failed by oom. "
                            + e.getMessage());
        }

        if (bitmap == null) {
            Logger.d(TAG, "Decode bitmap failed, bitmap is null.");
            return originFileName;
        }

        int quality = MAX_COMPRESS_QUALITY;
        File file = new File(originFileName);
        long size = file.length();
        Logger.d(TAG, "size = " + size);
        if (originHeight > MAX_WIDTH_REQUIRED || originWidht > MAX_WIDTH_REQUIRED
                || size > MAX_SIZE_REQUIRED) {
            Logger.d(
                    TAG,
                    "The original image width or height is larger than 500, or the original image size is larger than 50");
            quality = SPECIAL_COMPRESS_QUALITY;
        }
        int targetWidht = bitmap.getWidth();
        int targetHeight = bitmap.getHeight();

        Logger.d(TAG, "originWidht = " + originWidht + ", originHeight = " + originHeight
                + ", targetWidht = " + targetWidht + ", targetHeight = " + targetHeight
                + ",bitmapOptions.inSampleSize  = " + bitmapOptions.inSampleSize);
        createRcseFolder();
        String compressedFileName = null;
        int pos = originFileName.lastIndexOf(SLASH);
        int length = originFileName.length();
        // No need to check the pos and length, since it is successfull to
        // create bitmap
        compressedFileName = RCSE_COMPRESSED_FILE_DIR + SLASH
                + originFileName.substring(pos + 1, length);
        File targetFile = new File(compressedFileName);
        FileOutputStream outputStream = null;
        try {
            outputStream = new FileOutputStream(targetFile);
            if (bitmap.compress(Bitmap.CompressFormat.JPEG, quality, outputStream)) {
                outputStream.flush();
                outputStream.close();
            } else {
                Logger.w(TAG, "Compress failed");
                return originFileName;
            }
        } catch (FileNotFoundException e) {
            Logger.e(TAG, "FileNotFoundException:" + e.getMessage());
            return originFileName;
        } catch (IOException e) {
            Logger.e(TAG, "IOException:" + e.getMessage());
            return originFileName;
        } finally {
            bitmap.recycle();
            bitmap = null;
            if (outputStream != null) {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    Logger.e(TAG, "IOException:" + e.getMessage());
                }
            }
        }
        return compressedFileName;
    }

    /**
     * Create joyn folder if not exist
     */
    private static void createRcseFolder() {
        // Create a directory if not already exist
        if (FileFactory.createDirectory(RCSE_FILE_DIR)) {
            Logger.w(TAG, "Create rcse dir success");
        }
        // Create a directory if not already exist
        if (FileFactory.createDirectory(RCSE_COMPRESSED_FILE_DIR)) {
            Logger.w(TAG, "Create rcse compressed dir success");
        }
    }

    /**
     * Get the current available storage size in byte;
     * 
     * @return available storage size in byte; -1 for no external storage
     *         detected
     */
    public static long getFreeStorageSize() {
        boolean isExist = Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
        if (isExist) {
            File path = Environment.getExternalStorageDirectory();
            StatFs stat = new StatFs(path.getPath());
            int availableBlocks = stat.getAvailableBlocks();
            int blockSize = stat.getBlockSize();
            long result = (long) availableBlocks * blockSize;
            Logger.d(TAG, "getFreeStorageSize() blockSize: " + blockSize + " availableBlocks: "
                    + availableBlocks + " result: " + result);
            return result;
        }
        return -1;
    }
    
    /**
     * Whether the specified file exist
     * 
     * @param path The full file path
     * @return True for file exist, otherwise return false;
     */
    public static boolean isFileExist(String path) {
        if (path != null) {
            File file = new File(path);
            return file.exists();
        } else {
            return false;
        }
    }
}
