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

package com.mediatek.systemupdate;

import android.content.Context;

import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

final class UpgradePkgManager {
    private static final String TAG = "SystemUpdate/UnzipChecksum";

    private static final String MD5_TAG = "MD5";
    private static final int PKG_BUFF_SIZE = 1024 * 1024;
    private static final int MD5_BUFF_SIZE = 64;
    static final int UNZIP_FAILED = 0;
    static final int CKSUM_ERROR = 1;
    static final int UNZIP_SUCCESS = 2;

    static final int MD5_MASK = 0xff;

    static final int OTA_PACKAGE = 0;
    static final int SD_PACKAGE = 1;

    private UpgradePkgManager() {
    }

    static int unzipUpgradePkg(String zipFile, String deltaPath) {

        File resFile = new File(zipFile);
        boolean result = false;
        if (deltaPath == null) {
            deltaPath = resFile.getParent();
        }

        if (deltaPath == null) {
            return UNZIP_FAILED;
        }
        result = unZipFile(resFile, deltaPath);

        Xlog.i(TAG, "unzipDelta:result = " + result);

        if (!result) {

            return UNZIP_FAILED;
        }

        if (!checkPackage(deltaPath)) {
            Xlog.i(TAG, "unzipDelta: cksum error");
            return CKSUM_ERROR;
        }
        return UNZIP_SUCCESS;
    }

    static void deleteUnusedOtaFile(String filePath) {

        File pkg = new File(filePath + Util.PathName.TEMP_PKG_NAME);
        if (pkg != null && pkg.exists()) {
            pkg.delete();
        }
        File chf = new File(filePath + Util.PathName.MD5_FILE_NAME);
        if (chf != null && chf.exists()) {
            chf.delete();
        }
    }

    static void resetPkg(Context context) {

        String folderPath = Util.getPackagePathName(context);

        File pkg = new File(folderPath + Util.PathName.PACKAGE_NAME);
        if (pkg != null && pkg.exists()) {
            pkg.delete();
        }
        File chf = new File(folderPath + Util.PathName.MD5_FILE_NAME);
        if (chf != null && chf.exists()) {
            chf.delete();
        }

        File pkgTemp = new File(folderPath + Util.PathName.TEMP_PKG_NAME);

        if (pkgTemp != null && pkgTemp.exists()) {
            pkgTemp.renameTo(new File(folderPath + Util.PathName.PACKAGE_NAME));
        }

    }

    static long getSpaceForUnzipOtaPkg(Context context) {

        String strTempPkgPath = Util.getPackagePathName(context) + Util.PathName.TEMP_PKG_NAME;
        return (long) (Util.getFileSize(strTempPkgPath) * Util.DECOMPRESS_RATIO);

    }

    static String getTempOtaPackage(Context context) {
        return Util.getPackagePathName(context) + Util.PathName.TEMP_PKG_NAME;
    }

    static void deleteCrashPkgFile(String filePath) {

        File pkg = new File(filePath + Util.PathName.PACKAGE_NAME);
        if (pkg != null && pkg.exists()) {
            pkg.delete();
        }
        File chf = new File(filePath + Util.PathName.MD5_FILE_NAME);
        if (chf != null && chf.exists()) {
            chf.delete();
        }
    }

    static boolean renameOtaPkg(Context context) {

        String strOriPkg = Util.getPackageFileName(context);

        if (strOriPkg != null) {
            File pkg = new File(strOriPkg);

            if (pkg != null && pkg.exists()) {
                return pkg.renameTo(new File(Util.getPackagePathName(context)
                        + Util.PathName.TEMP_PKG_NAME));

            }

        }

        return false;
    }

    private static boolean unZipFile(File zipFile, String folderPath) {

        InputStream in = null;
        OutputStream out = null;
        ZipFile zf = null;
        try {
            zf = new ZipFile(zipFile);
            for (Enumeration<?> entries = zf.entries(); entries.hasMoreElements();) {
                ZipEntry entry = ((ZipEntry) entries.nextElement());
                in = zf.getInputStream(entry);
                String str = folderPath + File.separator + entry.getName();
                Xlog.i(TAG, "unZipFile:file = " + str);
                if (entry == null || in == null || str == null) {
                    return false;
                }
                File desFile = new File(str);
                if (desFile.exists()) {
                    desFile.delete();
                }
                desFile.createNewFile();
                out = new FileOutputStream(desFile);
                byte[] buffer = new byte[PKG_BUFF_SIZE];
                int realLength = 0;
                while ((realLength = in.read(buffer)) > 0) {
                    out.write(buffer, 0, realLength);
                }
                out.close();
                out = null;
                in.close();
                in = null;
            }
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    private static String getFileMD5(String file) {

        try {
            MessageDigest md = MessageDigest.getInstance(MD5_TAG);
            FileInputStream fis = new FileInputStream(file);
            byte[] buffer = new byte[PKG_BUFF_SIZE];
            int length = -1;

            if (fis == null || md == null) {
                return null;
            }
            while ((length = fis.read(buffer)) != -1) {
                md.update(buffer, 0, length);
            }
            if (fis != null) {
                fis.close();
            }
            byte[] bytes = md.digest();
            if (bytes == null) {

                return null;
            }
            StringBuffer buf = new StringBuffer();
            for (int i = 0; i < bytes.length; i++) {
                String md5s = Integer.toHexString(bytes[i] & MD5_MASK);
                if (md5s == null || buf == null) {
                    return null;
                }
                if (md5s.length() == 1) {
                    buf.append("0");
                }
                buf.append(md5s);
            }

            return buf.toString();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return null;
        } catch (NoSuchAlgorithmException ex) {
            ex.printStackTrace();
            return null;
        } catch (IOException ex) {
            ex.printStackTrace();
            return null;
        }
    }

    private static String getMD5sum(String file) {

        try {
            FileInputStream fis = new FileInputStream(file);
            if (fis == null) {
                return null;
            }
            byte[] buffer = new byte[MD5_BUFF_SIZE];

            int length = fis.read(buffer);
            Xlog.i(TAG, "getMD5sum:length = " + length);
            if (fis != null) {
                fis.close();
            }
            return new String(buffer, 0, length);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    private static HashMap<String, String> parseChecksumFile(String fileName, String[] fileList) {
        HashMap<String, String> table = new HashMap<String, String>();
        FileReader fReader = null;
        BufferedReader reader = null;

        try {
            File file = new File(fileName);
            fReader = new FileReader(file);
            reader = new BufferedReader(fReader);
            String line = null;
            int lineNum = 0;

            int fileCount = fileList.length;
            while ((line = reader.readLine()) != null) {
                if (lineNum < fileCount) {
                    table.put(fileList[lineNum], line.trim());
                    Xlog.v(TAG, line + " md5sum put in table: " + line.trim());
                }
                lineNum++;
            }

            fReader.close();
            reader.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return table;
    }

    private static boolean checkFilesEntities(String path) {

        File updf = new File(path + Util.PathName.PACKAGE_NAME);
        if (updf == null || !updf.exists()) {
            return false;
        }
        File chf = new File(path + Util.PathName.MD5_FILE_NAME);
        if (chf == null || !chf.exists()) {
            return false;
        }
        return true;
    }

    public static boolean checkPackage(String path) {
        if (!checkFilesEntities(path)) {
            Xlog.i(TAG, "checkPackage:lost file from net");
            return false;
        }
        String filemd5 = getFileMD5(path + Util.PathName.PACKAGE_NAME).substring(0, 31);
        String md5sum = getMD5sum(path + Util.PathName.MD5_FILE_NAME).substring(0, 31);
        Xlog.i(TAG, "checkPackage:filemd5=" + filemd5 + "+++++");
        Xlog.i(TAG, "checkPackage:md5sum =" + md5sum + "+++++");
        if (filemd5 == null) {
            return false;
        }
        return filemd5.equals(md5sum);

    }

    public static boolean checkSdPackage(String path, String[] fileList) {
        HashMap<String, String> table = parseChecksumFile(path + Util.PathName.MD5_FILE_NAME,
                fileList);

        for (String file : fileList) {
            Xlog.i(TAG, "checkSdPackage, file:" + file);
            String filemd5 = getFileMD5(path + File.separator + file);
            if (filemd5 == null || filemd5.length() < 31) {
                return false;
            }
            filemd5 = filemd5.substring(0, 31);

            String md5sum = table.get(file);
            if (md5sum == null || md5sum.length() < 31) {
                return false;
            }
            md5sum = md5sum.substring(0, 31);

            Xlog.i(TAG, "checkSdPackage:filemd5=" + filemd5 + "+++++");
            Xlog.i(TAG, "checkSdPackage:md5sum5=" + md5sum + "+++++");

            if (!filemd5.equals(md5sum)) {
                return false;
            }
        }

        return true;
    }
}
