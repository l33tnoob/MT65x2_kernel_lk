package com.mediatek.systemupdate.otatest;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.List;

import junit.framework.Assert;

public final class Util {
    private static final String TAG = "SystemUpdateTest/Util";

    static final String PACKAGE_INFO_FOLDER = "pkginfo";
    static final String MD5_FOLDER = "md5";
    static final String RESULT_FOLDER = "result";
    static final String PACKAGE_INFO_OTA_FILE = "package_info_ota";
    static final String PACKAGE_INFO_TARGET_FILE = "package_info_target";
    static final String PACKAGE_INFO_SOURCE_FILE = "package_info_source";
    static final String MD5_OTA_FILE = "md5_ota";
    static final String MD5_TARGET_FILE = "md5_target";
    static final String MD5_RESULT_FILE = "result_md5.txt";
    static final String INSTALLED_PACKAGE_FILE = "result_installed_package.txt";
    static final String PACKAGE_NAME_FILE = "result_package_name.txt";
    static final String SHARED_UID_FILE = "result_shared_uid.txt";

    public static boolean writePackageInfoToFile(Context context, String fileName,
            List<PackageInfo> infoList) {
        Boolean ret = false;
        FileWriter fwriter = null;
        String folder = context.getApplicationInfo().dataDir + File.separator + PACKAGE_INFO_FOLDER;
        File dir = new File(folder);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        File file = new File(folder + File.separator + fileName);
        if (file.exists()) {
            Log.d(TAG, fileName + " exists, delete it");
            file.delete();
        }

        try {
            fwriter = new FileWriter(file);

            StringBuilder outputBuilder = new StringBuilder();
            for (PackageInfo info : infoList) {
                if(info.applicationInfo.sourceDir.startsWith("/data")){
                    Log.d(TAG, info.applicationInfo.sourceDir + " doesn't count");
                    continue;
                }
                outputBuilder.append(info.applicationInfo.sourceDir).append(":")
                        .append(info.packageName).append(":")
                        .append(String.valueOf(info.sharedUserId)).append("\n");
            }

            fwriter.write(outputBuilder.toString());
            ret = true;
        } catch (IOException e) {
            Assert.fail("writeToFile exception" + e.getMessage());
        } finally {
            Log.d(TAG, "close writer");
            try {
                if (fwriter != null) {
                    fwriter.close();
                    fwriter = null;
                }
            } catch (IOException e) {
                Log.e(TAG, e.getMessage());
            }
        }
        return ret;
    }

}
