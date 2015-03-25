package com.mediatek.systemupdate.otatest;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.test.AndroidTestCase;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import junit.framework.Assert;

/**
 * Get Downloaded new version's package information and old version's package information. Include
 * package name, APK source directory, shared user ID. The new version's package information should
 * be pull to PC.
 * 
 * @author mtk80800
 * 
 */
public class WriteSourceTests extends AndroidTestCase {
    private static final String TAG = "SystemUpdate/WriteSourceTests";

    protected void setUp() throws Exception {
        mContext = getContext();
        super.setUp();
    }

    public void testWriteSourcePackageInfo() {
        PackageManager pm = mContext.getPackageManager();
        List<PackageInfo> packageInfoList = pm
                .getInstalledPackages(PackageManager.GET_SIGNATURES);
        Log.e(TAG, "package number is " + packageInfoList.size());
        Util.writePackageInfoToFile(mContext, Util.PACKAGE_INFO_SOURCE_FILE, packageInfoList);
    }

}
