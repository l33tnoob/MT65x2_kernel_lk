package com.mediatek.datatransfer.modules;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.BrowserContract;
import android.provider.BrowserContract.Bookmarks;

import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.net.URI;

public class BookmarkBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BookmarkBackupComposer";
    private Cursor mCursor;
    private File mFolderFile;

    private static final String TAG_DEFAULT = "[DEFAULT]";
    private static final String TAG_BASEURL = "BASEURL=";
    private static final String TAG_SHORTCUT = "[InternetShortcut]";
    private static final String TAG_URL = "URL=";
    private static final Uri BOOKMARKS_URI=Uri.parse("content://com.android.browser/bookmarks");
    private static final String FILE_EXT = ".url";

    public BookmarkBackupComposer(Context context) {
        super(context);
    }

    @Override
    public int getModuleType() {
        return ModuleType.TYPE_BOOKMARK;
    }

    @Override
    public int getCount() {
        int count = 0;
        if (mCursor != null&& !mCursor.isClosed()) {
            count = mCursor.getCount();
            MyLogger.logD(CLASS_TAG, "getCount() count = " +count);
        }
        return count;
    }

    @Override
    public boolean isAfterLast() {
        boolean result = true;
        if (mCursor != null) {
            result = mCursor.isAfterLast();
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    @Override
    public boolean init() {
        boolean result = true;

        mCursor = mContext.getContentResolver()
                .query(BrowserContract.Bookmarks.CONTENT_URI, null, BrowserContract.Bookmarks.IS_FOLDER+"=0", null, null);
        
        if (mCursor != null) {
            mCursor.moveToFirst();
        } else {
            result = false;
        }
        MyLogger.logD(CLASS_TAG, "init():" + result);

        return result;
    }

    @Override
    protected boolean implementComposeOneEntity() {
        boolean result = false;

        if (mCursor != null && !mCursor.isAfterLast()) {
            int id = mCursor.getInt(mCursor.getColumnIndex(BrowserContract.Bookmarks._ID));
            String title = mCursor.getString(mCursor.getColumnIndex(BrowserContract.Bookmarks.TITLE));
            String url = mCursor.getString(mCursor.getColumnIndex(BrowserContract.Bookmarks.URL));
            MyLogger.logD(CLASS_TAG, "implementComposeOneEntity:" + "title-" + title + ", url-" + url);
            mCursor.moveToNext();

            if (url == null || url.isEmpty()) {
                MyLogger.logD(CLASS_TAG, "implementComposeOneEntity(): url is null or empty");
                return result;
            }

            if (title == null || title.isEmpty() || !title.matches("[^/\\\\<>*?:\"|]+")) {
                MyLogger.logD(CLASS_TAG,
                        "implementComposeOneEntity(): title is null or empty, use id:" + id);
                title = String.valueOf(id);
            }

            StringBuffer content = new StringBuffer("");
            content.append(TAG_DEFAULT + "\n").append(TAG_BASEURL + url + "\n")
                    .append(TAG_SHORTCUT + "\n").append(TAG_URL + url + "\n");
            MyLogger.logD(CLASS_TAG, "The URL content is " + content);

            File file = new File(mFolderFile.getAbsolutePath() + File.separator + title + FILE_EXT);
            if (!file.exists()) {
                try {
                    file.createNewFile();
                } catch (Exception e) {
                    MyLogger.logE(CLASS_TAG, "implementComposeOneEntity():create file failed");
                }
            }

            try {
                FileWriter fstream = new FileWriter(file);
                fstream.write(content.toString());
                // BufferedWriter outBufferWriter = new BufferedWriter(fstream);
                // outBufferWriter.write();
                fstream.flush();
                fstream.close();
                result = true;
            } catch (Exception e) {
                MyLogger.logD(CLASS_TAG, "VCAL: onStart() write file failed");
            }
        }

        return result;
    }

    /**
     * Describe <code>onStart</code> method here.
     * 
     */
    public final void onStart() {
        super.onStart();
        if (mCursor != null) {
            if (getCount() > 0) {
                mFolderFile = new File(mParentFolderPath + File.separator
                        + ModulePath.FOLDER_BOOKMARK);
                if (!mFolderFile.exists()) {
                    mFolderFile.mkdirs();
                }
            }
        }
    }

    /**
     * Describe <code>onEnd</code> method here.
     * 
     */
    public void onEnd() {
        super.onEnd();
        if (mCursor != null) {
            mCursor.close();
            mCursor = null;
        }

        MyLogger.logD(CLASS_TAG, "onEnd");
    }

}
