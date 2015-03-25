package com.mediatek.hotknotbeam;

import android.content.Context;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

public class ZipFileUtils {
    private static String TAG = "ZipFileUtils";

    public static final String zipUris(Uri[] uris, File zip, File base, Context context)  throws IOException {
        String zipName = "";
        ZipOutputStream zos = new ZipOutputStream(new FileOutputStream(zip));
        Log.i(TAG, "uris:" + uris.length);
        for(int i = 0; i < uris.length; i++) {
            Log.i(TAG, "zip uri:" + uris[i]);
            if(i == 0) {
                zipName = zipUri(uris[i], base, zos, context);
            } else {
                zipUri(uris[i], base, zos, context);
            }
        }
        zos.close();
        return zipName;
    }

    private static final String zipUri(Uri uri, File base, ZipOutputStream zos, Context context) throws IOException {
        byte[] buffer = new byte[8192];
        int read = 0;
        String zipName = "";

        Log.d(TAG, "zip uri:" + uri);

        File f = MimeUtilsEx.getFilePathFromUri(uri, context);
        if(f == null) {
            Log.e(TAG, "File is null");
            return zipName;
        }
        if (f.isDirectory()) {
            Log.e(TAG, "File is directory");
            return zipName;
        }

        zipName = f.getPath().substring(base.getPath().length() + 1);
        FileInputStream in = new FileInputStream(f);
        ZipEntry entry = new ZipEntry(zipName);

        zos.putNextEntry(entry);
        while (-1 != (read = in.read(buffer))) {
            zos.write(buffer, 0, read);
        }
        in.close();

        return zipName;
    }

    public static final void zipDirectory( File directory, File zip ) throws IOException {
        ZipOutputStream zos = new ZipOutputStream( new FileOutputStream( zip ) );
        zip( directory, directory, zos );
        zos.close();
    }

    private static final void zip(File directory, File base,
                                  ZipOutputStream zos) throws IOException {
        File[] files = directory.listFiles();
        byte[] buffer = new byte[8192];
        int read = 0;
        for (int i = 0, n = files.length; i < n; i++) {
            if (files[i].isDirectory()) {
                zip(files[i], base, zos);
            } else {
                FileInputStream in = new FileInputStream(files[i]);
                ZipEntry entry = new ZipEntry(files[i].getPath().substring(
                                                  base.getPath().length() + 1));
                zos.putNextEntry(entry);
                while (-1 != (read = in.read(buffer))) {
                    zos.write(buffer, 0, read);
                }
                in.close();
            }
        }
    }

    public static final void unzip(File zip, File extractTo, Context context) throws IOException {
        ZipFile archive = new ZipFile(zip);
        Enumeration e = archive.entries();



        while (e.hasMoreElements()) {
            ZipEntry entry = (ZipEntry) e.nextElement();
            File file = new File(extractTo, entry.getName());

            Log.d(TAG, "[unzip]ZipEntry:" + entry);

            if (entry.isDirectory() && !file.exists()) {
                file.mkdirs();
            } else {
                if (!file.getParentFile().exists()) {
                    file.getParentFile().mkdirs();
                }

                InputStream in = archive.getInputStream(entry);
                BufferedOutputStream out = new BufferedOutputStream(
                    new FileOutputStream(file));

                byte[] buffer = new byte[8192];
                int read;

                while (-1 != (read = in.read(buffer))) {
                    out.write(buffer, 0, read);
                }

                in.close();
                out.close();

                String[] paths = new String[1];
                paths[0] = file.getCanonicalPath();
                MediaScannerConnection.scanFile(context, paths, null, null);
            }
        }
        archive.close();
    }
}