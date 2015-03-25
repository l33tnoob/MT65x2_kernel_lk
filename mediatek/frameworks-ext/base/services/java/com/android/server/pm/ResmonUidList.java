package com.android.server.pm;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import android.os.Environment;
import android.os.FileUtils;

final class ResmonUidList {
    private final File mResmonUidListFile;
    private final File mDataDir;
    
    ResmonUidList() {
        mDataDir = new File(Environment.getDataDirectory(), "system");
        mResmonUidListFile = new File(mDataDir, "resmon-uid.txt");
    }
    
    void updateList(ArrayList<Integer> uidList) {
        try {
            FileWriter fw = new FileWriter(mResmonUidListFile);
            StringBuilder sb = new StringBuilder();
            if (!uidList.isEmpty() && uidList.size() > 0) {
                for(int i = 0; i < uidList.size(); i++) {
                    sb.append(String.valueOf(uidList.get(i)));
                    sb.append("\r\n");
                }
            }
            fw.write(sb.toString());
            fw.flush();
            fw.close();
            FileUtils.setPermissions(mResmonUidListFile.toString(),
                    FileUtils.S_IRUSR|FileUtils.S_IWUSR
                    |FileUtils.S_IRGRP|FileUtils.S_IWGRP
                    |FileUtils.S_IROTH,
                    -1, -1);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }   
}
