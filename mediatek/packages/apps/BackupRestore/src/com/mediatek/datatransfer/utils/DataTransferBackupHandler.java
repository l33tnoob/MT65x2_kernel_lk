package com.mediatek.datatransfer.utils;

import java.io.File;
import java.util.List;

import android.content.Context;

public class DataTransferBackupHandler implements BackupsHandler {

    Context mContext;

    public DataTransferBackupHandler(Context context) {
        // TODO Auto-generated constructor stub
        mContext = context;
    }

    @Override
    public boolean init() {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    public void onStart() {
        // TODO Auto-generated method stub

    }

    @Override
    public void reset() {
        // TODO Auto-generated method stub

    }

    @Override
    public void cancel() {
        // TODO Auto-generated method stub

    }

    @Override
    public List<File> onEnd() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public String getBackupType() {
        // TODO Auto-generated method stub
        return Constants.BackupScanType.DATATRANSFER;
    }

}
