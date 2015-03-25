package com.mediatek.hotknotbeam;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Log;

import java.io.File;

import java.util.HashMap;
import java.util.Map;

public final class MimeUtilsEx {
    static protected final String TAG = "MimeUtilsEx";

    private static final Map<String, String> mimeTypeToExtensionMap = new HashMap<String, String>();

    private static final Map<String, String> extensionToMimeTypeMap = new HashMap<String, String>();

    static {
        // Note that this list is _not_ in alphabetical order and must not be sorted.
        // The "most popular" extension must come first, so that it's the one returned
        // by guessExtensionFromMimeType.

        add("image/mpo", "mpo");
    }

    public static boolean isGallerySupport(String filename) {
        boolean ret = false;
        int pos = filename.lastIndexOf('.');
        
        if (pos != -1) {
            String ext = filename.substring(pos+1);
            if (ext.equalsIgnoreCase("png")) {
                ret = true;
            } else if (ext.equalsIgnoreCase("jpg")) {
                ret = true;
            } else if (ext.equalsIgnoreCase("jpe")) {
                ret = true;
            } else if (ext.equalsIgnoreCase("jpeg")) {
                ret = true;
            } else if (ext.equalsIgnoreCase("gif")) {
                ret = true;
            } else if (ext.equalsIgnoreCase("avi")) {
                ret = true;
            } else if (ext.equalsIgnoreCase("mpo")) {
                ret = true;
            } else if (ext.equalsIgnoreCase("3gp")) {
                ret = true;
            }
        }
        
        return ret;
    }

    private static void add(String mimeType, String extension) {
        //
        // if we have an existing x --> y mapping, we do not want to
        // override it with another mapping x --> ?
        // this is mostly because of the way the mime-type map below
        // is constructed (if a mime type maps to several extensions
        // the first extension is considered the most popular and is
        // added first; we do not want to overwrite it later).
        //
        if (!mimeTypeToExtensionMap.containsKey(mimeType)) {
            mimeTypeToExtensionMap.put(mimeType, extension);
        }
        extensionToMimeTypeMap.put(extension, mimeType);
    }

    /**
     * Returns the MIME type for the given extension.
     * @param extension A file extension without the leading '.'
     * @return The MIME type for the given extension or null iff there is none.
     */
    public static String guessMimeTypeFromExtension(String extension) {
        if (extension == null || extension.isEmpty()) {
            return null;
        }
        return extensionToMimeTypeMap.get(extension);
    }

    public static File getFilePathFromUri(Uri uri, Context context) {
        File inputFile = null;
        String filePath = "";

        if(uri == null) {
            Log.e(TAG, "File Uri must not be null");
            throw new IllegalArgumentException("File Uri must not be null");
        }

        String scheme = uri.getScheme();

        if(scheme != null && scheme.equalsIgnoreCase("content")) {
            Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
            if(cursor!= null) {
                int index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
                cursor.moveToFirst();
                filePath = cursor.getString(index);
                cursor.close();
            } else {
                Log.e(TAG, "Cursor is null");
            }
        } else if(scheme != null && scheme.equalsIgnoreCase("file")) {
            filePath = "/" + uri.getHost() + uri.getPath();
        }

        if(filePath.length() == 0) {
            Log.e(TAG, "File path is empty");
            return null;
        }

        Log.d(TAG, "The sending path is " + filePath);
        inputFile = new File(filePath);

        if(!inputFile.exists() || inputFile.isDirectory()) {
            Log.e(TAG, "File is not existed or inputFile is a directory");
            return null;
        }

        return inputFile;
    }

}