package com.mediatek.security.datamanager;

import android.content.Context;
import android.content.pm.PackageInfo;

import com.mediatek.common.mom.IMobileManager;
import com.mediatek.common.mom.Permission;
import com.mediatek.common.mom.PermissionRecord;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;


public class DatabaseManager {
    private static final String TAG = "DatabaseManager";
    
    private static DatabaseHelper sDataBaseHelper;
    private static HashMap<String, List<PermissionRecord>> sPkgKeyCache = new HashMap<String, List<PermissionRecord>>();
    private static HashMap<String, List<PermissionRecord>> sPermKeyCache = new HashMap<String, List<PermissionRecord>>();
    private static HashMap<String, List<PermissionRecord>> sRemovedPkgCache = new HashMap<String, List<PermissionRecord>>();
    
    /**
     * Init database and cache must be called at first and in async process
     * @param context Context of process
     */
    public static void initDataBase(Context context) {
        Xlog.d(TAG,"initDataBase");
        createDataBase(context);
        updateDataBase(context);
        setDataIntoCache();
    }
    
    private static void createDataBase(Context context) {
        if (sDataBaseHelper == null) {
            Xlog.d(TAG,"new DatabaseHelper");
            sDataBaseHelper = new DatabaseHelper(context);
        }
    }
    
    private static HashMap<String, List<PermissionRecord>> getInstalledPkg(Context context) {
        IMobileManager mms = (IMobileManager)context.getSystemService(Context.MOBILE_SERVICE);
        List<PackageInfo> packageInfoList = mms.getInstalledPackages();
        if (packageInfoList == null) {
            //in some case no app install /data/app
            return null;
        }
        //Get current install package info and set in permission record list into a map
        HashMap<String, List<PermissionRecord>> currentInstallMap = null;
        for (PackageInfo info : packageInfoList) {
            List<Permission> permissionList = mms.getPackageGrantedPermissions(info.packageName);
            List<PermissionRecord> permRecordList = 
                    PermControlUtils.getPermRecordListByPkg(info.packageName, permissionList);
            if (permRecordList != null) {
                if (currentInstallMap == null) {
                    currentInstallMap = new HashMap<String, List<PermissionRecord>>();
                }
                currentInstallMap.put(info.packageName, permRecordList);
            }
        }
        return currentInstallMap;
    }
    
    private static void updateDataBase(Context context) {
        HashMap<String, List<PermissionRecord>> installedMap = getInstalledPkg(context);
        testPrint("installedMap",installedMap);
        //Get data from database
        HashMap<String, List<PermissionRecord>> databaseMap = sDataBaseHelper.getPkgFromDB();
        testPrint("databaseMap",databaseMap);
        //find new installed app but db not record
        HashMap<String, List<PermissionRecord>> newInstallAppMap = getNewInstalledPkg(installedMap,databaseMap);
        testPrint("newInstallAppMap",newInstallAppMap);
        
        //find removed package but db not removed
        List<String> removedPkgList = getRemovePkg(installedMap,databaseMap);
        Xlog.e(TAG,"removedPkgList = " + removedPkgList + " check whether it installed under sdcard");
        
        //First delete the removed pkg from db
        //sDataBaseHelper.delete(removedPkgList);
        
        //Then add new installed into db
        sDataBaseHelper.add(newInstallAppMap);
    }
    
    private static void testPrint(String type, Map<String, List<PermissionRecord>> map) {
        if (map != null){
            Set<String> key = map.keySet();
            for (String pkgName : key) {
                Xlog.d(TAG, type + " = " + pkgName);
            }
        }
    }
    
    public static void setDataIntoCache() {
        sDataBaseHelper.initCacheData();
        sPkgKeyCache = sDataBaseHelper.getmPkgKeyCache();
        sPermKeyCache = sDataBaseHelper.getmPermKeyCache();
        updateCacheForRmCase();
    }
    
    private static void updateCacheForRmCase() {
        Set<String> keys = sRemovedPkgCache.keySet();
        for (String pkgName : keys) {
            deletePkgFromCache(pkgName);
        }
    }
    
    public static void deletePkgFromCache(String pkgName) {
        if (pkgName != null) {
            sPkgKeyCache.remove(pkgName);
            deletePermKeyCache(pkgName);
        }  
    }
    
    

    public static HashMap<String, List<PermissionRecord>> getRemovedCacheMap() {
        return sRemovedPkgCache;
    }
    
    public static void setRemovedCacheMap(String pkgName) {
        if (sRemovedPkgCache != null) {
            sRemovedPkgCache.clear();
            if (sPkgKeyCache.containsKey(pkgName)) {
                sRemovedPkgCache.put(pkgName, sPkgKeyCache.get(pkgName));
            }
        }
    }
    
    
    private static List<String> getRemovePkg(
            HashMap<String, List<PermissionRecord>> currentInstall,
            HashMap<String, List<PermissionRecord>> currentDB) {
        List<String> removePkgList = null;
        sRemovedPkgCache.clear();
        // If currentDB is null means db not yet accessed, so no need to remove any from db
        if (currentDB == null) {
            Xlog.d(TAG,"First time boot up or database no data stored in");
            return null;
        }
        // If currentInstall is null means all pkg should be removed from db
        if (currentInstall == null) {
            Xlog.d(TAG,
                    "no pkg installed in phone but db store info so all db info should removed");
            for (String pkgName : currentDB.keySet()) {
                if (removePkgList == null) {
                    removePkgList = new ArrayList<String>();
                }
                removePkgList.add(pkgName);
            }
            sRemovedPkgCache.putAll(currentDB);
            return removePkgList;
        }
        // If current installed not contains pkg from db, then this pkg should
        // be removed
        for (String pkgName : currentDB.keySet()) {
            if (!currentInstall.containsKey(pkgName)) {
                if (removePkgList == null) {
                    removePkgList = new ArrayList<String>();
                }
                sRemovedPkgCache.put(pkgName, currentDB.get(pkgName));
                removePkgList.add(pkgName);
            }
        }
        return removePkgList;
    }

    private static HashMap<String, List<PermissionRecord>> 
        getNewInstalledPkg(HashMap<String, List<PermissionRecord>> currentInstall, 
                           HashMap<String, List<PermissionRecord>> currentDB) {
        
        HashMap<String, List<PermissionRecord>> newInstallMap = null;
        // If current phone no installed any pkg then no new installed pkg
        if (currentInstall == null) {
            Xlog.d(TAG,"No app installed in phone");
            return null;
        }
        
        // If database no any record means first time boot up, currentInstall
        // will be new installed
        if (currentDB == null) {
            Xlog.d(TAG, "First time boot up or database no data stored in");
            return currentInstall;
        }

        // Find any pkg that existed in currentInstall map but not in currentDB
        // map
        for (String pkgName : currentInstall.keySet()) {
            if (!currentDB.containsKey(pkgName)) {
                if (newInstallMap == null) {
                    newInstallMap = new HashMap<String, List<PermissionRecord>>();
                }
                newInstallMap.put(pkgName, currentInstall.get(pkgName));
            }
        }
        return newInstallMap;
    }
    
    /**
     * Get a list permissionRecord store in database. Load data from cache
     * @return List<PermissionRecord> from database 
     */
    public static List<PermissionRecord> getAllPermRecordList() {
        List<PermissionRecord> permRecordList = null;
        Set<Entry<String, List<PermissionRecord>>> entry = sPkgKeyCache.entrySet();
        for (Entry<String, List<PermissionRecord>> e : entry) {
            if (permRecordList == null) {
                permRecordList = new ArrayList<PermissionRecord>();
            }
            permRecordList.addAll(e.getValue());
        }
        return permRecordList;
    }
    
    /**
     * Get all package names store in database
     * @return a String list includes all packages in database
     */
    public static List<String> getPackageNames() {
        List<String> nameList = new ArrayList<String>();
        Set<String> keys = sPkgKeyCache.keySet();
        for (String pkgName : keys) {
            nameList.add(pkgName);
        }
        return nameList;
    }
    
    /**
     * Get the specific permission record of the package
     * @param pkgName the package name of app
     * @return the list of permission record
     */
    public static List<PermissionRecord> getPermRecordListByPkgName(String pkgName) {
        List<PermissionRecord> permissionRecordList = sPkgKeyCache.get(pkgName);
        if (permissionRecordList != null) {
            return new ArrayList<PermissionRecord>(permissionRecordList);
        } else {
            return null;
        }
    }
    
    /**
     * Get a list permissin record contain the specific permission name
     * @param pkgName the package name of app
     * @return the list of permission record
     */
    public static List<PermissionRecord> getPermRecordListByPermName(String permName) {
        List<PermissionRecord> permissionRecordList = sPermKeyCache.get(permName);
        if (permissionRecordList != null) {
            return new ArrayList<PermissionRecord>(permissionRecordList);
        } else {
            return null;
        }
    }
    
    /**
     * Add a new install pkg permission into database. Since it will write database,call it in Asynchronous.
     * @param pkgName package name
     * @param permList package related Permission List
     * @return the package corresponding permission record list, if null the package no permission under monitor
     * or no permission registered
     */
    public static List<PermissionRecord> add(String pkgName, List<Permission> permList) {
        List<PermissionRecord> newPkgPermRecordList = getPermRecordListForNewPkg(pkgName,permList);
        if (newPkgPermRecordList != null) {
            // update cache and database
            delete(pkgName);
            for (PermissionRecord permRecord : newPkgPermRecordList) {
                addIntoCache(permRecord,sPkgKeyCache,KEY_TYPE.Pkg_Key);
                addIntoCache(permRecord,sPermKeyCache,KEY_TYPE.Perm_Key);
                sDataBaseHelper.add(permRecord);
            }
        }
        return newPkgPermRecordList;
    }
    
    public static void add(List<PermissionRecord> permRecordList,String pkgName) {
        if (permRecordList != null) {
            for (PermissionRecord permRecord : permRecordList) {
                addIntoCache(permRecord,sPkgKeyCache,KEY_TYPE.Pkg_Key);
                addIntoCache(permRecord,sPermKeyCache,KEY_TYPE.Perm_Key);
            }
        }
    }
    
    
    /**
     * Get a PermissionRecord List for a new package
     * @param pkgName new package name
     * @param permList corresponding Permission in List
     * @return the new package's List in PermissionRecord type 
     */
    private static List<PermissionRecord> getPermRecordListForNewPkg(String pkgName, List<Permission> permList) {
        List<PermissionRecord> permRecordList = PermControlUtils.getPermRecordListByPkg(pkgName, permList);
        if (permRecordList == null) {
            Xlog.e(TAG,"permRecordList = null");
            return null;
        }
        if (sPkgKeyCache.containsKey(pkgName)) {
            List<PermissionRecord> origList = sPkgKeyCache.get(pkgName);
            Map<String,Integer> map = new HashMap<String,Integer>();
            for (PermissionRecord record : origList) {
                // Record the permission status that is not default for this pkg
                if (record.getStatus() != IMobileManager.PERMISSION_STATUS_CHECK) {
                    map.put(record.mPermissionName, record.getStatus());
                }
            }
            for (PermissionRecord record : permRecordList) {
                if (map.get(record.mPermissionName) != null) {
                    // Change the status as is already been modified
                    record.setStatus(map.get(record.mPermissionName));
                }
            }
        }
        return permRecordList;
    }
    
    private static void addIntoCache(PermissionRecord permRecord, Map<String,List<PermissionRecord>> map,
                                    KEY_TYPE type) {
        String key = getKeyValue(permRecord,type);
        if (map.containsKey(key)) {
            map.get(key).add(permRecord);
        } else {
            List<PermissionRecord> newPermRecordList = new ArrayList<PermissionRecord>();
            newPermRecordList.add(permRecord);
            map.put(key, newPermRecordList);
        }
    }
    
    /**
     * Delete the specific package from database. Since it will write database, 
     * call it in Asynchronous.
     * @param pkgName package name of apk
     */
    public static void delete(String pkgName) {
        if (pkgName != null) {
            sPkgKeyCache.remove(pkgName);
            deletePermKeyCache(pkgName);
            sDataBaseHelper.delete(pkgName);
        }  
    }
    
    private static void deletePermKeyCache(String pkgName) {
        Set<String> keys = sPermKeyCache.keySet();
        for (String permName : keys) {
            List<PermissionRecord> recordList = sPermKeyCache.get(permName);
            int pos = 0;
            for (PermissionRecord record : recordList) {
                if (record.mPackageName.equals(pkgName)) {
                    recordList.remove(pos);
                    break;
                }
                pos ++;
            }
        }
    }
    
    /**
     * Modify a permission's status of package. Since it write db, call it in Asynchronous
     * @param permRecord the target permission
     */
    public static void modify(PermissionRecord permRecord) {
        modifyCache(permRecord);
        sDataBaseHelper.updatePermStatus(permRecord.mPackageName, permRecord.mPermissionName, permRecord.getStatus());
    }
    
    private static String getKeyValue(PermissionRecord permRecord, KEY_TYPE type) {
        String key;
        if (type == KEY_TYPE.Pkg_Key) {
            key = permRecord.mPackageName;
        } else {
            key = permRecord.mPermissionName;
        }
        return key;
    }
    
    private static void modifyCache(PermissionRecord permRecord) {
        List<PermissionRecord> permRecordList = sPkgKeyCache.get(permRecord.mPackageName);
        if (permRecordList != null) {
            for (PermissionRecord record : permRecordList) {
                if (record.mPermissionName.equals(permRecord.mPermissionName)) {
                    record.setStatus(permRecord.getStatus());
                }
            }
        } else {
            Xlog.e(TAG,"Something not right need to check mPackageName = " + permRecord.mPackageName);
        }
        permRecordList = sPermKeyCache.get(permRecord.mPermissionName);
        if (permRecordList != null) {
            for (PermissionRecord record : permRecordList) {
                if (record.mPackageName.equals(permRecord.mPackageName)) {
                    record.setStatus(permRecord.getStatus());
                }
            }
        } else {
            Xlog.e(TAG,"Something not right need to check mPermissionName = " + permRecord.mPermissionName);
        }
    }
    enum KEY_TYPE {
        Pkg_Key,
        Perm_Key
    }
}
