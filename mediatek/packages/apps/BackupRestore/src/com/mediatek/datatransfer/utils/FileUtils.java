package com.mediatek.datatransfer.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.math.BigDecimal;
import java.nio.channels.FileChannel;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.List;

import android.content.ContentResolver;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageParser;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;

import com.mediatek.datatransfer.AppSnippet;
import com.mediatek.datatransfer.R;
import com.mediatek.common.featureoption.FeatureOption;

public class FileUtils {
    
    public static final String CLASS_TAG = MyLogger.LOG_TAG + "/FileUtils";
    private static final int BUFF_SIZE = 1024 * 1024; // 1M Byte
    static final int MD5_MASK = 0xff;
    public static String getDisplaySize(long bytes, Context context) {
        String displaySize = context.getString(R.string.unknown);
        long iKb = bytes2KB(bytes);
        if (iKb == 0 && bytes >= 0) {
            // display "less than 1KB"
            displaySize = context.getString(R.string.less_1K);
        } else if (iKb >= 1024) {
            // diplay MB
            double iMb = ((double) iKb) / 1024;
            iMb = round(iMb, 2, BigDecimal.ROUND_UP);
            StringBuilder builder = new StringBuilder(new Double(iMb).toString());
            builder.append("MB");
            displaySize = builder.toString();
        } else {
            // display KB
            StringBuilder builder = new StringBuilder(new Long(iKb).toString());
            builder.append("KB");
            displaySize = builder.toString();
        }
        return displaySize;
    }

    /**
     * create files
     * 
     * @param filePath
     * @return
     */
     public static File createFile(String filePath) {
     File file = null;
     File tmpFile = new File(filePath);
     if (createFileorFolder(tmpFile)) {
     file = tmpFile;
     }
     return file;
     }

    /**
     * create the file
     * 
     * @param file
     * @return
     */
     public static boolean createFileorFolder(File file) {
         boolean success = true;
         if (file != null) {
             File dir = file.getParentFile();
             if (dir != null && !dir.exists()) {
             dir.mkdirs();
             }
             
             try {
                 if(file.isFile()){
                     success = file.createNewFile();
                 }else{
                     success = file.mkdirs();
                 }
             } catch (IOException e) {
                 success = false;
                 Log.d(CLASS_TAG, "createFile() failed !cause:" + e.getMessage());
                 e.printStackTrace();
             }
         }
         return success;
     }

    /**
     * see if the file exsit
     * 
     * @param filePath
     * @return
     */
    // public static boolean isFileExist(String filePath) {
    // File file = new File(filePath);
    // return file.exists();
    // }

    public static String getNameWithoutExt(String fileName) {
        String nameWithoutExt = fileName;
        int iExtPoint = fileName.lastIndexOf(".");
        if (iExtPoint != -1) {
            nameWithoutExt = fileName.substring(0, iExtPoint);
        }
        return nameWithoutExt;
    }

    public static long bytes2MB(long bytes) {
        return bytes2KB(bytes) / 1024;
    }

    public static long bytes2KB(long bytes) {
        return bytes / 1024;
    }

    /**
     * return the filename's ext
     * 
     * @param file
     * @return
     */
    public static String getExt(File file) {
        if (file == null) {
            return null;
        }
        return getExt(file.getName());
    }

    /**
     * return the filename's ext
     * 
     * @param fileName
     * @return
     */
    public static String getExt(String fileName) {
        if (fileName == null) {
            return null;
        }
        String ext = null;

        int iLastOfPoint = fileName.lastIndexOf(".");
        if (iLastOfPoint != -1) {
            ext = fileName.substring(iLastOfPoint + 1, fileName.length());
        }
        return ext;
    }

    public static double round(double value, int scale, int roundingMode) {
        BigDecimal bd = new BigDecimal(value);
        bd = bd.setScale(scale, roundingMode);
        double d = bd.doubleValue();
        bd = null;
        return d;
    }
    
    public static long computeAllFileSizeInFolder(File folderFile){
        long size = 0;
        if (folderFile != null) {
            try {
                if (folderFile.isFile()) {
                    size = folderFile.length();
                } else if (folderFile.isDirectory()) {
                    File[] files = folderFile.listFiles();
                    for (File file : files) {
                        if (file.isDirectory()) {
                            size += computeAllFileSizeInFolder(file);
                        } else if (file.isFile()) {
                            size += file.length();
                        }
                    }
                }
            } catch (NullPointerException e) {
                size = 0;
                Log.e(CLASS_TAG, "computeAllFileSizeInFolder: sd card is out when ");
                e.printStackTrace();
            }
        }
        return size;
    }
    
    public static boolean isEmptyFolder(File folderName){
        boolean ret = true;
        
        if(folderName != null && folderName.exists()){
            if(folderName.isFile()){
                ret = false;
            }else{
                File[] files = folderName.listFiles();
                if(files != null){
                    for(File file : files){
                        if(!isEmptyFolder(file)){
                            ret = false;
                            break;
                        }
                    }
                }
            }
        }
        return ret;
    }
    
    public static boolean deleteFileOrFolder(File file){
        boolean result = true;
        if(!file.exists()){
            return result;
        }
        if(file.isFile()){
            return file.delete();
        }else if(file.isDirectory()){
            File[] files = file.listFiles();
            for(File f : files){
                if(!deleteFileOrFolder(f)){
                    result = false;
                }
            }
            if(!file.delete()){
                result = false;
            }
        }
        return result;
    }

    public static boolean deleteFileOrFolder(File file, Context context) {
        boolean result = true;
        if (!file.exists()) {
            return result;
        }
        if (file.isFile()) {
            return file.delete();
        } else if (file.isDirectory()) {
            File[] files = file.listFiles();
            if (file.getName().equals(Constants.ModulePath.FOLDER_MUSIC)
                    || file.getName().equals(Constants.ModulePath.FOLDER_PICTURE)) {
                // delete database!
                List<String> mediaFiles = new ArrayList<String>();
                for (File tFile : files) {
                    mediaFiles.add(tFile.getAbsolutePath());
                }
                deleteFileInMediaStore(mediaFiles, context);
            }
            for (File f : files) {
                if (!deleteFileOrFolder(f, context)) {
                    result = false;
                }
            }
            if (!file.delete()) {
                result = false;
            }
        }
        return result;
    }

    /**
     * scan Path for new file or folder in MediaStore
     * 
     * @param path the scan path
     */
    public static void scanPathforMediaStore(String path, Context mContext) {
        MyLogger.logD(CLASS_TAG, "scanPathforMediaStore.path =" + path);
        if (mContext != null && !TextUtils.isEmpty(path)) {
            String[] paths = {
                path
            };
            MyLogger.logD(CLASS_TAG, "scanPathforMediaStore,scan file .");
            MediaScannerConnection.scanFile(mContext, paths, null, null);
        }
    }

    public static void scanPathforMediaStore(List<String> scanPaths, Context mContext) {
        MyLogger.logD(CLASS_TAG, "scanPathforMediaStore,scanPaths.");
        if (mContext != null && !scanPaths.isEmpty()) {
            String[] paths = new String[scanPaths.size()];
            scanPaths.toArray(paths);
            MyLogger.logD(CLASS_TAG, "scanPathforMediaStore,scan file.");
            MediaScannerConnection.scanFile(mContext, paths, null, null);
        }
    }

    /**
     * delete the record in MediaStore
     * 
     * @param paths
     *            the delete file or folder in MediaStore
     */
    public static void deleteFileInMediaStore(List<String> paths,Context mContext) {
        MyLogger.logD(CLASS_TAG, "deleteFileInMediaStore.");
        Uri uri = MediaStore.Files.getContentUri("external");
        StringBuilder whereClause = new StringBuilder();
        whereClause.append("?");
        for (int i = 0; i < paths.size() - 1; i++) {
            whereClause.append(",?");
        }
        String where = MediaStore.Files.FileColumns.DATA + " IN(" + whereClause.toString() + ")";
        // notice that there is a blank before "IN(".
        if (mContext != null && !paths.isEmpty()) {
            ContentResolver cr = mContext.getContentResolver();
            String[] whereArgs = new String[paths.size()];
            paths.toArray(whereArgs);
            MyLogger.logD(CLASS_TAG, "deleteFileInMediaStore,delete.");
            if (!FeatureOption.MTK_2SDCARD_SWAP) {
                cr.delete(uri, where, whereArgs);
            } else {
                try {
                    cr.delete(uri, where, whereArgs);
                } catch (UnsupportedOperationException e) {
                    MyLogger.logD(CLASS_TAG, "Error, database is closed!!!");
                }
            }
        }
    }
    
    
    public static ArrayList<File> getAllApkFileInFolder(File file){
        if(file == null){
            return null;
        }
        if(!file.exists() || file.isFile()){
            return null;
        }
        ArrayList<File> list = new ArrayList<File>();
        File[] files = file.listFiles();
        for(File f : files){
            String ext = getExt(f);
            if(ext != null && ext.equalsIgnoreCase("apk")){
                list.add(f);
            }
        }
        return list;
    }

    public static File getNewestFile(List<File> files) {
        // TODO Auto-generated method stub
        if (files == null || files.isEmpty()) {
            return null;
        }
        File newestFile = files.get(0);
        long newest = newestFile.lastModified();
        for (File file : files) {
            MyLogger.logD(CLASS_TAG, "onStart() ---->" + file.getName() + "  lastModified = "
                    + file.lastModified());
            newestFile = (file.lastModified() > newest ? file : newestFile);
        }
        return newestFile;
    }

    public static String getFileMD5(String file) {
        if (file == null || file.isEmpty()) {
            return null;
        }
        FileInputStream fis = null;
        StringBuffer buf = new StringBuffer();
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            fis = new FileInputStream(file);
            byte[] buffer = new byte[BUFF_SIZE];
            int length = -1;

            long s = System.currentTimeMillis();
            if (fis == null || md == null) {
                return null;
            }
            while ((length = fis.read(buffer)) != -1) {
                md.update(buffer, 0, length);
            }
            byte[] bytes = md.digest();
            if (bytes == null) {
                return null;
            }
            for (int i = 0; i < bytes.length; i++) {
                String md5s = Integer.toHexString(bytes[i] & MD5_MASK);
                if (md5s == null || buf == null) {
                    return null;
                }
                if (md5s.length() == 1) {
                    buf.append("0");
                }
                buf.append(md5s);
            }
            MyLogger.logD(CLASS_TAG, "getFileMD5:GenMd5 success! spend the time: "
                    + (System.currentTimeMillis() - s) + "ms");
            return buf.toString();
        } catch (Exception ex) {

            ex.printStackTrace();
            return null;
        } finally {
            try {
                if (fis != null) {
                    fis.close();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
    }

    /**
     * Describe <code>writeToFile</code> method here.
     * 
     * @param fileName a <code>String</code> value
     * @param buf a <code>byte</code> value
     * @exception IOException if an error occurs
     */
    public static void writeToFile(String fileName, byte[] buf) throws IOException {
        try {
            FileOutputStream outStream = new FileOutputStream(fileName);
            // byte[] buf = inBuf.getBytes();
            outStream.write(buf, 0, buf.length);
            outStream.flush();
            outStream.close();
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void combineFiles(List<File> files, String saveFileName) throws IOException {
        if (files == null || files.isEmpty()) {
            MyLogger.logD(CLASS_TAG, "no file need to be combined, return and do nothing");
            return;
        }
        File mFile = new File(saveFileName);

        if (!mFile.exists()) {
            mFile.createNewFile();
        }
        FileChannel mFileChannel = new FileOutputStream(mFile).getChannel();
        FileChannel inFileChannel;
        for (File file : files) {

            inFileChannel = new FileInputStream(file).getChannel();
            inFileChannel.transferTo(0, inFileChannel.size(), mFileChannel);

            inFileChannel.close();
        }

        mFileChannel.close();

    }

    /**
     * 
     * @param context
     * @param archiveFilePath  apk absolute path name
     * @return AppSnippet
     */
    public static AppSnippet getAppSnippet(Context context, String archiveFilePath){
        
        PackageParser packageParser = new PackageParser(archiveFilePath);
        
        File sourceFile = new File(archiveFilePath);
        DisplayMetrics metrics = new DisplayMetrics();
        metrics.setToDefaults();
        PackageParser.Package pkg = packageParser.parsePackage(sourceFile, archiveFilePath, metrics, 0);
        if (pkg == null) {
            return null;
        }

        ApplicationInfo appInfo = pkg.applicationInfo;
        Resources pRes = context.getResources();
      
        AssetManager assmgr = new AssetManager();
        assmgr.addAssetPath(archiveFilePath);
        Resources res = new Resources(assmgr, pRes.getDisplayMetrics(), pRes.getConfiguration());
        CharSequence label = null;
      // Try to load the label from the package's resources. If an app has not explicitly
      // specified any label, just use the package name.
        if (appInfo.labelRes != 0) {
            try {
                label = res.getText(appInfo.labelRes);
            } catch (Resources.NotFoundException e) {
            
            }
       }
       if (label == null) {
           label = (appInfo.nonLocalizedLabel != null) ?
                   appInfo.nonLocalizedLabel : appInfo.packageName;
       }
       Drawable icon = null;
       // Try to load the icon from the package's resources. If an app has not explicitly
       // specified any resource, just use the default icon for now.
       if (appInfo.icon != 0) {
           try {
               icon = res.getDrawable(appInfo.icon);
           } catch (Resources.NotFoundException e) {
           }
       }
       if (icon == null) {
           icon = context.getPackageManager().getDefaultActivityIcon();
       }
       AppSnippet snippet = new AppSnippet(icon, label, appInfo.packageName);
       snippet.SetFileName(archiveFilePath);
       return snippet;
    }

    public static void deleteEmptyFolder(File file) {
        // TODO Auto-generated method stub
        if (file == null || !file.isDirectory()) {
            return;
        }
        for (File subFolderFile : file.listFiles()) {
            if (subFolderFile.isDirectory() && isEmptyFolder(subFolderFile)) {
                subFolderFile.deleteOnExit();
            }
        }
    }

//    public static AppSnippet getAppSnippet(Context context, String apkPath) {  
//        String PATH_PackageParser = "android.content.pm.PackageParser";  
//        String PATH_AssetManager = "android.content.res.AssetManager";  
//        try {
//
//            Class pkgParserCls = Class.forName(PATH_PackageParser);  
//            Class[] typeArgs = new Class[1];  
//            typeArgs[0] = String.class;  
//            Constructor pkgParserCt = pkgParserCls.getConstructor(typeArgs);
//            Object[] valueArgs = new Object[1];  
//            valueArgs[0] = apkPath;  
//            Object pkgParser = pkgParserCt.newInstance(valueArgs);  
//            Log.d("ANDROID_LAB", "pkgParser:" + pkgParser.toString());  
//            DisplayMetrics metrics = new DisplayMetrics();  
//            metrics.setToDefaults();  
//            // PackageParser.Package mPkgInfo = packageParser.parsePackage(new   
//            // File(apkPath), apkPath,   
//            // metrics, 0);   
//            typeArgs = new Class[4];  
//            typeArgs[0] = File.class;  
//            typeArgs[1] = String.class;  
//            typeArgs[2] = DisplayMetrics.class;  
//            typeArgs[3] = Integer.TYPE;  
//            Method pkgParser_parsePackageMtd = pkgParserCls.getDeclaredMethod("parsePackage",  
//                    typeArgs);  
//            valueArgs = new Object[4];  
//            valueArgs[0] = new File(apkPath);  
//            valueArgs[1] = apkPath;  
//            valueArgs[2] = metrics;  
//            valueArgs[3] = 0;  
//            Object pkgParserPkg = pkgParser_parsePackageMtd.invoke(pkgParser, valueArgs);  
//
//            Field appInfoFld = pkgParserPkg.getClass().getDeclaredField("applicationInfo");  
//            ApplicationInfo appInfo = (ApplicationInfo) appInfoFld.get(pkgParserPkg);  
//            Log.d("ANDROID_LAB", "pkg:" + appInfo.packageName + " uid=" + appInfo.uid);  
//            Class assetMagCls = Class.forName(PATH_AssetManager);  
//            Constructor assetMagCt = assetMagCls.getConstructor((Class[]) null);  
//            Object assetMag = assetMagCt.newInstance((Object[]) null);  
//            typeArgs = new Class[1];  
//            typeArgs[0] = String.class;  
//            Method assetMag_addAssetPathMtd = assetMagCls.getDeclaredMethod("addAssetPath",  
//                    typeArgs);  
//            valueArgs = new Object[1];  
//            valueArgs[0] = apkPath;  
//            assetMag_addAssetPathMtd.invoke(assetMag, valueArgs);  
//            Resources res = context.getResources();  
//            typeArgs = new Class[3];  
//            typeArgs[0] = assetMag.getClass();  
//            typeArgs[1] = res.getDisplayMetrics().getClass();  
//            typeArgs[2] = res.getConfiguration().getClass();  
//            Constructor resCt = Resources.class.getConstructor(typeArgs);  
//            valueArgs = new Object[3];  
//            valueArgs[0] = assetMag;  
//            valueArgs[1] = res.getDisplayMetrics();  
//            valueArgs[2] = res.getConfiguration();  
//            res = (Resources) resCt.newInstance(valueArgs);  
//            CharSequence label = null;  
//         // Try to load the label from the package's resources. If an app has not explicitly
//            // specified any label, just use the package name.
//              if (appInfo.labelRes != 0) {
//                  try {
//                      label = res.getText(appInfo.labelRes);
//                  } catch (Resources.NotFoundException e) {
//                  
//                  }
//             }
//             if (label == null) {
//                 label = (appInfo.nonLocalizedLabel != null) ?
//                         appInfo.nonLocalizedLabel : appInfo.packageName;
//             }
//             Drawable icon = null;
//             // Try to load the icon from the package's resources. If an app has not explicitly
//             // specified any resource, just use the default icon for now.
//             if (appInfo.icon != 0) {
//                 try {
//                     icon = res.getDrawable(appInfo.icon);
//                 } catch (Resources.NotFoundException e) {
//                 }
//             }
//             if (icon == null) {
//                 icon = context.getPackageManager().getDefaultActivityIcon();
//             }
//             return new AppSnippet(icon, label, appInfo.packageName);
//        } catch (Exception e) {  
//            e.printStackTrace();  
//        }
//        return null;
//    }
}
