package com.mediatek.systemupdate.otatest;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.test.AndroidTestCase;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import junit.framework.Assert;

/**
 * Compare package information between versions.
 * 
 * @author mtk80800
 * 
 */
public class PackageInfoTests extends AndroidTestCase {
    private static final String TAG = "SystemUpdate/PackageInfoTests";

    private String mPkgInfoFolder;
    private String mResultFolder;

    private class MOTAPackageInfo {
        String sourceDir;
        String packageName;
        String sharedUserId;
    }

    protected void setUp() throws Exception {
        mContext = getContext();
        mPkgInfoFolder = mContext.getApplicationInfo().dataDir + File.separator
                + Util.PACKAGE_INFO_FOLDER;
        Log.v(TAG, "[setUp]mPkgInfoFolder = " + mPkgInfoFolder);
        mResultFolder = mContext.getApplicationInfo().dataDir + File.separator + Util.RESULT_FOLDER;
        super.setUp();
    }

    /**
     * Compare downloaded version and upgraded version. The two versions package information should
     * be all the same.
     */
    public void testInstall() {
        String file = mPkgInfoFolder + File.separator + Util.PACKAGE_INFO_TARGET_FILE;
        List<MOTAPackageInfo> newPkgInfoList = readPackageInfoFromFile(file);
        List<MOTAPackageInfo> pkgInfoList = getMotaPackageInfoFromPM();
        List<MOTAPackageInfo> newPkgInfoListBk = new ArrayList<MOTAPackageInfo>(newPkgInfoList);

        StringBuilder installedBuilder = new StringBuilder();

        int newlength = newPkgInfoList.size();
        for (MOTAPackageInfo newPkgInfo :newPkgInfoListBk){
            String newDir = newPkgInfo.sourceDir;
            String newPkgName = newPkgInfo.packageName;
            String newSharedUid = newPkgInfo.sharedUserId;

            int length = pkgInfoList.size();
            int j = 0;
            for (; j < length; ++j) {
                String dir = pkgInfoList.get(j).sourceDir;
                String pkgName = pkgInfoList.get(j).packageName;
                String sharedUid = pkgInfoList.get(j).sharedUserId;

                boolean sameDir = dir.equals(newDir);
                boolean samepkgName = pkgName.equals(newPkgName);
                boolean sameUid = sharedUid.equals(newSharedUid);
                if (sameDir && samepkgName && sameUid) {
                    newPkgInfoList.remove(newPkgInfo);
                    pkgInfoList.remove(j);
                    break;
                }
            }
        }

        newlength = newPkgInfoList.size();
        if (newlength > 0) {
            installedBuilder.append("Applications in flash_download load but not in firmware_upgrade load:\n");
            for (int i = 0; i < newlength; ++i) {
                String dir = newPkgInfoList.get(i).sourceDir;
                String pkgName = newPkgInfoList.get(i).packageName;
                String sharedUid = newPkgInfoList.get(i).sharedUserId;
                installedBuilder.append(dir).append(":").append(pkgName).append(":sharedUid=")
                        .append(sharedUid).append("\n");
            }
        }

        int length = pkgInfoList.size();
        if (length > 0) {
            installedBuilder.append("Applications in firmware_upgrade load but not in flash_download load:\n");
            for (int i = 0; i < length; ++i) {
                String dir = pkgInfoList.get(i).sourceDir;
                String pkgName = pkgInfoList.get(i).packageName;
                String sharedUid = pkgInfoList.get(i).sharedUserId;
                installedBuilder.append(dir).append(":").append(pkgName).append(":sharedUid=")
                        .append(sharedUid).append("\n");
            }
        }
        writeResult(Util.INSTALLED_PACKAGE_FILE, installedBuilder.toString());
    }

    /**
     * Compare old version and upgraded version to check the package name and shared uid.
     * Attention: We can't find out the application which package name and dir both changed
     */
    public void testPackageName() {
        String file = mPkgInfoFolder + File.separator + Util.PACKAGE_INFO_SOURCE_FILE;
        List<MOTAPackageInfo> oldPkgInfoList = readPackageInfoFromFile(file);
        List<MOTAPackageInfo> pkgInfoList = getMotaPackageInfoFromPM();
        StringBuilder packageNameBuilder = new StringBuilder();
        StringBuilder sharedUidBuilder = new StringBuilder();

        int oldlength = oldPkgInfoList.size();
        for (int i = 0; i < oldlength; ++i) {
            boolean isfound = false;
            String oldDir = oldPkgInfoList.get(i).sourceDir;
            String oldPkgName = oldPkgInfoList.get(i).packageName;
            String oldSharedUid = oldPkgInfoList.get(i).sharedUserId;
            int length = pkgInfoList.size();
            int j = 0;
            for (; j < length; ++j) {
                String dir = pkgInfoList.get(j).sourceDir;
                String pkgName = pkgInfoList.get(j).packageName;

                if (!pkgName.equals(oldPkgName) && !dir.equals(oldDir)) {
                    continue;
                } else if (dir.equals(oldDir) && pkgName.equals(oldPkgName)) {
                    // package name matches
                    isfound = true;
                    break;
                } else if (dir.equals(oldDir) && !pkgName.equals(oldPkgName)) {
                    // package name changed
                    String message = new StringBuilder(dir).append(", old package = ")
                            .append(oldPkgName).append(", new package = ").append(pkgName)
                            .toString();
                    Log.i(TAG, message);
                    packageNameBuilder.append(message).append("\n");
                    isfound = true;
                    break;
                } else if (!dir.equals(oldDir) && pkgName.equals(oldPkgName)) {
                    // dir changed
                    String message = new StringBuilder(pkgName).append(", old dir = ")
                            .append(oldDir).append(", new dir = ").append(dir).toString();
                    Log.d(TAG, message);
                    isfound = true;
                    break;
                }
            }
            if (isfound) {
                String sharedUid = pkgInfoList.get(j).sharedUserId;
                if (!sharedUid.equals(oldSharedUid)) {
                    // shared user id changed
                    String message = new StringBuilder(oldPkgName).append(", old sharedUserId = ")
                            .append(oldSharedUid).append(", new sharedUserId = ").append(sharedUid)
                            .toString();
                    Log.i(TAG, message);
                    sharedUidBuilder.append(message).append("\n");
                }
            } else {
                Log.d(TAG, oldPkgName + " has been deleted");
            }
        }
        writeResult(Util.PACKAGE_NAME_FILE, packageNameBuilder.toString());
        writeResult(Util.SHARED_UID_FILE, sharedUidBuilder.toString());
    }

    private List<MOTAPackageInfo> readPackageInfoFromFile(String fileName) {
        List<MOTAPackageInfo> motaPkgInfoList = new ArrayList<MOTAPackageInfo>();

        FileReader freader = null;
        BufferedReader breader = null;
        try {
            File file = new File(fileName);
            if (!file.exists()) {
                Assert.fail(fileName + " is not exist");
            }

            freader = new FileReader(file);
            breader = new BufferedReader(freader);

            while (true) {
                String line = breader.readLine();
                if (line == null) {
                    Log.v(TAG, fileName + " no more data");
                    break;
                } else {
                    String[] temp = line.split(":");
                    if (temp.length != 3) {
                        continue;
                    }
                    MOTAPackageInfo info = new MOTAPackageInfo();
                    info.sourceDir = temp[0];
                    info.packageName = temp[1];
                    info.sharedUserId = temp[2];
                    motaPkgInfoList.add(info);
                }
            }

        } catch (IOException e) {
            Assert.fail("read line exception" + e.getMessage());
        } finally {
            Log.d(TAG, "close all reader & fileStream");
            try {
                if (freader != null) {
                    freader.close();
                    freader = null;
                }

                if (breader != null) {
                    breader.close();
                    breader = null;
                }
            } catch (IOException e) {
                Assert.fail("read line exception" + e.getMessage());
            }
        }
        return motaPkgInfoList;
    }

    private List<MOTAPackageInfo> getMotaPackageInfoFromPM() {
//        List<MOTAPackageInfo> motaPkgInfoList = new ArrayList<MOTAPackageInfo>();

        PackageManager pm = mContext.getPackageManager();
        List<PackageInfo> packageInfoList = pm
                .getInstalledPackages(PackageManager.GET_SIGNATURES);
        Log.e(TAG, "package number is " + packageInfoList.size());
        Util.writePackageInfoToFile(mContext, Util.PACKAGE_INFO_OTA_FILE, packageInfoList);
        
        return readPackageInfoFromFile(mPkgInfoFolder + File.separator + Util.PACKAGE_INFO_OTA_FILE);

//        for (PackageInfo info : packageInfoList) {
//
//            if(info.applicationInfo.sourceDir.startsWith("/data")){
//                Log.d(TAG, info.applicationInfo.sourceDir + " doesn't count");
//                continue;
//            }
//            
//            MOTAPackageInfo motaInfo = new MOTAPackageInfo();
//            motaInfo.sourceDir = info.applicationInfo.sourceDir;
//            motaInfo.packageName = info.packageName;
//            motaInfo.sharedUserId = String.valueOf(info.sharedUserId);
//            motaPkgInfoList.add(motaInfo);
//        }
//        return motaPkgInfoList;
    }

    private boolean writeResult(String fileName, String result) {
        Boolean ret = false;
        FileWriter fwriter = null;
        File dir = new File(mResultFolder);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        String filepath = mResultFolder + File.separator + fileName;
        File file = new File(filepath);
        if (file.exists()) {
            file.delete();
            Log.d(TAG, filepath + " exists, delete it");
        }

        if (TextUtils.isEmpty(result)) {
            Log.i(TAG, "result is null, return");
            return true;
        }

        try {
            fwriter = new FileWriter(file);
            fwriter.write(result);
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
