package com.android.server.pm;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import android.util.Log;

final class ResmonWhitelistPackage {
    private final File mSystemDir;
    private final File mWhitelistFile;
    
    final ArrayList<String> mPackages = new ArrayList<String>();
    
    ResmonWhitelistPackage() {
        mSystemDir = new File("/system/", "etc");
        mWhitelistFile = new File(mSystemDir, "resmonwhitelist.txt");
    }
    
    void readList() {
        if (!mWhitelistFile.exists()) {
            return;
        }
        try {
            /// M: Clear white list record before update it
            mPackages.clear();
            BufferedReader br = new BufferedReader(new FileReader(mWhitelistFile));
            String line = br.readLine();
            while (line != null) {
                mPackages.add(line);
                line = br.readLine();
            }
            br.close();
        } catch (IOException e) {
            //Log.e(PackageManagerService.TAG, "IO Exception happened while reading resmon whitelist");
            e.printStackTrace();
        }
    }   
}
