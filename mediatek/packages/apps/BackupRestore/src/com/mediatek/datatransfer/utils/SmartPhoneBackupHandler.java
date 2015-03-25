package com.mediatek.datatransfer.utils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.mediatek.datatransfer.R;

import android.content.Context;
import android.content.SharedPreferences;

public class SmartPhoneBackupHandler implements BackupsHandler {
    private String mPath = "";
    private static final String CLASS_TAG = "SmartPhoneBackupHandler";

    public SmartPhoneBackupHandler() {
        super();
    }

    Context mContext;

    public SmartPhoneBackupHandler(Context context) {
        // TODO Auto-generated constructor stub
        mContext = context;
    }

    @Override
    public boolean init() {
        // TODO Auto-generated method stub
        String externalStoragePath = SDCardUtils.getExternalStoragePath(mContext);
        if (externalStoragePath == null) {
            return false;
        }
        mPath = externalStoragePath + File.separator;
        MyLogger.logD(CLASS_TAG, "init()=====>>mPath is " + mPath);
        return true;
    }

    @Override
    public void onStart() {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "onStart()");
        File file = new File(mPath);
        if (file.exists() && file.isDirectory() && file.canRead()) {
            File[] files = file.listFiles();
            result.addAll(filterFile(files));
        } else {
            MyLogger.logE(CLASS_TAG, "file is not exist====>>>mPath is " + mPath);
        }
    }

    private Collection<? extends File> filterFile(File[] files) {
        // TODO Auto-generated method stub
        ArrayList<File> resultList = new ArrayList<File>();
        if (files == null || files.length == 0) {
            return resultList;
        }

        for (File file : files) {
            String nameString = file.getName();
            if (nameString != null && !nameString.isEmpty()) {
                if (nameString.endsWith(".vcf") && nameString.length() == 9) {
                    String fileNameString = nameString.substring(0, nameString.lastIndexOf("."));
                    try {
                        Integer.parseInt(fileNameString);
                        resultList.add(file);
                    } catch (NumberFormatException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                }
            }
        }
        return resultList;
    }

    @Override
    public void cancel() {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "cancel()");

    }

    @Override
    public List<File> onEnd() {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "onEnd()");
        return result;
    }

    @Override
    public String getBackupType() {
        // TODO Auto-generated method stub
        return Constants.BackupScanType.PLUTO;
    }

    @Override
    public void reset() {
        // TODO Auto-generated method stub
        result.clear();
    }
}
