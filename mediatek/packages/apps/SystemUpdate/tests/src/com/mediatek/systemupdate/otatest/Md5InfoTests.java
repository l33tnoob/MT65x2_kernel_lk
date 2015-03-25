package com.mediatek.systemupdate.otatest;

import android.test.AndroidTestCase;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import junit.framework.Assert;

/**
 * Check MD5 sum between downloaded new version and upgraded new version.
 * 
 * @author mtk80800
 * 
 */
public class Md5InfoTests extends AndroidTestCase {
    private static final String TAG = "SystemUpdate/Md5InfoTests";
    private String[] mFileNames;
    private String mMd5Folder;
    private String mResultFolder;

    protected void setUp() throws Exception {
        mContext = getContext();
        mMd5Folder = mContext.getApplicationInfo().dataDir + File.separator
                + Util.MD5_FOLDER;
        Log.v(TAG, "[setUp]folder = " + mMd5Folder);
        mFileNames = (new File(mMd5Folder)).list();
        Assert.assertEquals("[testMD5]md5 files size error!!", 2, mFileNames.length);
        mResultFolder = mContext.getApplicationInfo().dataDir + File.separator
                + Util.RESULT_FOLDER;
        super.setUp();
    }

    public void testMD5() {
        StringBuilder md5Builder = new StringBuilder();

        HashMap<String, String> downloadMd5Map = new HashMap<String, String>();
        HashMap<String, String> otaMd5Map = new HashMap<String, String>();
        List<String> dlKeyList = new ArrayList<String>();
        List<String> otaKeyList = new ArrayList<String>();
        List<String> changedKeyList = new ArrayList<String>();

        readFromFile(mMd5Folder + File.separator + Util.MD5_TARGET_FILE, downloadMd5Map, dlKeyList);
        readFromFile(mMd5Folder + File.separator + Util.MD5_OTA_FILE, otaMd5Map, otaKeyList);

        List<String> keyList = new ArrayList<String>(dlKeyList);
        for (String key : keyList) {
            if (otaKeyList.contains(key)) {
                String downloadValue = downloadMd5Map.get(key);
                String otaValue = otaMd5Map.get(key);
                if (!downloadValue.equals(otaValue)) {
                    Log.i(TAG, key + "md5 changed");
                    changedKeyList.add(key);
                }
                otaKeyList.remove(key);
                dlKeyList.remove(key);
            }
        }

        if (changedKeyList.size() > 0){
            md5Builder.append("Files changed:\n");
            for (String key : changedKeyList) {
                md5Builder.append(key).append("\n");
            }
        }
        if (dlKeyList.size() > 0) {
            md5Builder.append("Files in flash_download load but not in firmware_upgrade load:\n");
            for (String key : dlKeyList) {
                md5Builder.append(key).append("\n");
            }
        }

        if (otaKeyList.size() > 0) {
            md5Builder.append("Files in firmware_upgrade load but not in flash_download load:\n");
            for (String key : otaKeyList) {
                md5Builder.append(key).append("\n");
            }
        }

        writeResult(Util.MD5_RESULT_FILE, md5Builder.toString());
    }

    private boolean readFromFile(String file, HashMap<String, String> map, List<String> keyList) {
        Log.d(TAG, "[readFromFile]" + file);
        Boolean ret = false;
        FileReader freader = null;
        BufferedReader breader = null;
        try {
            freader = new FileReader(file);
            breader = new BufferedReader(freader);

            while (true) {
                String line = breader.readLine();
                if (line == null) {
                    Log.v(TAG, file + " no more data");
                    break;
                } else {
                    if (line.length() < 34 || line.contains("could not read")
                            || line.contains("recovery-from-boot.p")
                            || line.contains("install-recovery.sh")) {
                        Log.w(TAG, line);
                        continue;
                    }

                    String key = line.substring(34);
                    if (keyList != null) {
                        keyList.add(key);
                    }
                    map.put(key, line.substring(0, 32));
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
        return ret;
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
            Log.e(TAG, filepath + " exists, delete it");
        }

        if (TextUtils.isEmpty(result)) {
            Log.i(TAG, "[writeResult]result is null, return");
            ret = true;
            return ret;
        }

        try {
            fwriter = new FileWriter(file);
            fwriter.write(result);
            ret = true;
        } catch (IOException e) {
            Assert.fail("writeToFile exception" + e.getMessage());
        } finally {
            Log.d(TAG, "[writeResult]close writer");
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
