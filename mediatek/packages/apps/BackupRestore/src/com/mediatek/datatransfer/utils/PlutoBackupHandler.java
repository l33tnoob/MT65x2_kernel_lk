package com.mediatek.datatransfer.utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.datatransfer.R;

import android.content.Context;
import android.content.SharedPreferences;

public class PlutoBackupHandler implements BackupsHandler {
    private String mPath = "";
    private static final String CLASS_TAG = "PlutoBackupHandler";

    public PlutoBackupHandler() {
        super();
    }

    Context mContext;

    public PlutoBackupHandler(Context context) {
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
        mPath = externalStoragePath + File.separator + Constants.BackupScanType.PLUTO_PATH;
        MyLogger.logD(CLASS_TAG, "init()=====>>mPath is " + mPath);
        return true;
    }

    @Override
    public void onStart() {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "onStart()");
        File file = new File(mPath);
        if (file.exists() && file.isFile() && file.canRead()) {
            result.add(file);
        } else {
            MyLogger.logE(CLASS_TAG, "file is not exist====>>>mPath is " + mPath);
        }
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
