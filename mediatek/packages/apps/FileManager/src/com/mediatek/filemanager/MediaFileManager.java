package com.mediatek.filemanager;

import android.media.MediaFile;

/**
 *wrap for MediaFile 
 *
 */
public class MediaFileManager {
    public static final int FILE_TYPE_MS_POWERPOINT = MediaFile.FILE_TYPE_MS_POWERPOINT;
    public static final int FILE_TYPE_MS_WORD = MediaFile.FILE_TYPE_MS_WORD;
    public static final int FILE_TYPE_MS_EXCEL = MediaFile.FILE_TYPE_MS_EXCEL;
    public static final int FILE_TYPE_PDF  = MediaFile.FILE_TYPE_PDF; 

    public static int getFileTypeForMimeType(String mimeType) {
        return MediaFile.getFileTypeForMimeType(mimeType);
    }
}
