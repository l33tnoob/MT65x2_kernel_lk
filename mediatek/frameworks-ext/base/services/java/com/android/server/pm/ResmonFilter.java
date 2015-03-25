package com.android.server.pm;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageParser;
import android.os.SystemProperties;

final class ResmonFilter {
    void filt(Settings settings, HashMap<String, PackageParser.Package> packages) {
        /// M: Only enable when the device is running under eng build
        if ("eng".equals(SystemProperties.get("ro.build.type"))){
            /// M: First read the package white list to get package list
            ResmonWhitelistPackage rsp = new ResmonWhitelistPackage();
            rsp.readList();
            if (!rsp.mPackages.isEmpty() && rsp.mPackages.size() > 0) {
                /// M: excludeUidList is used to store which uid should be excluded
                ArrayList<Integer> excludeUidList = new ArrayList<Integer>();
                for (int i = 0; i < rsp.mPackages.size(); i++) {
                    PackageSetting ps = settings.mPackages.get(rsp.mPackages.get(i));
                    /// M: We should check whether the package exist or not
                    if (ps != null && ps.pkg != null) {
                        /// M: Remove redundant uids
                        if (!excludeUidList.contains(ps.pkg.applicationInfo.uid)) {
                            excludeUidList.add(ps.pkg.applicationInfo.uid);
                        }
                    }
                }
                /// M: Search from the already scanned packages
                ArrayList<Integer> uidList = new ArrayList<Integer>();
                Iterator<PackageParser.Package> pkgit = packages.values().iterator();
                while (pkgit.hasNext()) {
                    PackageParser.Package pkg = pkgit.next();
                    PackageSetting ps = settings.mPackages.get(pkg.packageName);
                    /// M: Skip none system apps
                    if (ps != null && (ps.pkgFlags & ApplicationInfo.FLAG_SYSTEM) == 0) {
                        continue;
                    }
                    int curUid = pkg.applicationInfo.uid;
                    if (!excludeUidList.contains(curUid) && 
                            !uidList.contains(curUid)) {
                        uidList.add(curUid);
                    }
                }                

                /// M: Update uid list for resmon
                ResmonUidList rsu = new ResmonUidList();
                rsu.updateList(uidList);
            }
        }
    }
}
