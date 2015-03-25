package com.mediatek.datatransfer.modules;

import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.provider.Browser;

import org.apache.http.util.EncodingUtils;

import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class BookmarkRestoreComposer extends Composer {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BookmarkRestoreComposer";
    private static final String TAG_DEFAULT = "[DEFAULT]";
    private static final String TAG_BASEURL = "BASEURL=";
    private static final String TAG_SHORTCUT = "[InternetShortcut]";
    private static final String TAG_URL = "URL=";
    private static final String FILE_EXT = ".url";

    // sub domain
    final String SUB_DOMAIN = "(?i:[a-z0-9]|[a-z0-9][-a-z0-9]*[a-z0-9])";

    // top domain
    final String TOP_DOMAINS = "(?x-i:com\\b|edu\\b|biz\\b|in(?:t|fo)\\b|mil\\b|net\\b|org\\b|[a-z][a-z]\\b)";
    // host
    final String HOSTNAME = "(?:" + SUB_DOMAIN + "\\.)+" + TOP_DOMAINS;

    // URL not permitted code
    final String NOT_IN = ";:\"'<>()\\[\\]{}\\s\\x7F-\\xFF";
    final String NOT_END = "!.,?";
    final String ANYWHERE = "[^" + NOT_IN + NOT_END + "]";
    final String EMBEDDED = "[" + NOT_END + "]";
    final String URL_PATH = "/" + ANYWHERE + "*(" + EMBEDDED + "+" + ANYWHERE + "+)*";

    // port 0~65535
    final String PORT = "(?:[0-5]?[0-9]{1,4}|6(?:[0-4][0-9]{3}|5(?:[0-4][0-9]{2}|5(?:[0-2][0-9]|3[0-5]))))";

    // URL Matcher
    final String URL = "(?x:\n\\b\n(?:\n(?:ftp|https?)://[-\\w]+(\\.\\w[-\\w]*)+\n|\n" + HOSTNAME
            + "\n)\n(?::" + PORT + ")?\n(?:" + URL_PATH + ")?\n)";

    private int mIndex;
    private File[] mFileList;

    public BookmarkRestoreComposer(Context context) {
        super(context);
    }

    @Override
    public int getModuleType() {
        return ModuleType.TYPE_BOOKMARK;
    }

    @Override
    public int getCount() {
        if (mFileList == null) {
            return 0;
        }

        return mFileList.length;
    }

    @Override
    public boolean isAfterLast() {
        if (mFileList == null) {
            return true;
        }

        if (mIndex == mFileList.length) {
            return true;
        }

        return false;
    }

    @Override
    public boolean init() {
        mFileList = null;
        File file = new File(mParentFolderPath + File.separator + ModulePath.FOLDER_BOOKMARK);
        if (file == null || !file.exists() || !file.isDirectory()) {
            MyLogger.logD(CLASS_TAG, "Bookmark backup file is not correct");
            return false;
        }

        mFileList = file.listFiles();
        if (mFileList == null || mFileList.length == 0) {
            MyLogger.logD(CLASS_TAG, "init(): Bookmark backup folder is empty");
            return false;
        }

        MyLogger.logD(CLASS_TAG, "Init(): " + mFileList.length);
        return true;
    }

    @Override
    protected boolean implementComposeOneEntity() {
        boolean result = false;

        if (mFileList != null && !isAfterLast()) {
            File file = mFileList[mIndex];
            String fileName = file.getName();
            String title = fileName.substring(0, fileName.lastIndexOf(FILE_EXT));
            String url = parseBookmarkFileUrl(file);
            MyLogger.logD(CLASS_TAG, "implementComposeOneEntity():url  = " + url+"  and file is = "+ file);

            ContentValues inputValue = new ContentValues();
            inputValue.put(android.provider.Browser.BookmarkColumns.BOOKMARK, mIndex + 1);
            inputValue.put(android.provider.Browser.BookmarkColumns.TITLE, title);
            inputValue.put(android.provider.Browser.BookmarkColumns.URL, url);
            mContext.getContentResolver()
                    .insert(android.provider.Browser.BOOKMARKS_URI, inputValue);
            result = true;
            mIndex++;
        }

        MyLogger.logD(CLASS_TAG, "implementComposeOneEntity():" + result);
        return result;
    }

    @Override
    public void onStart() {
        super.onStart();
        deleteAllBookmarks();
        mIndex = 0;
        MyLogger.logD(CLASS_TAG, "onStart()");
    }

    @Override
    public void onEnd() {
        super.onEnd();
        MyLogger.logD(CLASS_TAG, "onEnd()");
    }

    private void deleteAllBookmarks() {
        mContext.getContentResolver().delete(Browser.BOOKMARKS_URI, null, null);
    }

    private String parseBookmarkFileUrl(File file) {
        String content = null;
        try {
            FileInputStream inStream = new FileInputStream(file);
            int length = inStream.available();
            byte[] buffer = new byte[length];
            inStream.read(buffer);
            content = EncodingUtils.getString(buffer, "UTF-8");
            MyLogger.logD(CLASS_TAG, "content = " + content);
            inStream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return null;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }

        if (content == null || content.isEmpty()) {
            return null;
        }

        int index = content.lastIndexOf(TAG_URL) + TAG_URL.length();
        MyLogger.logD(CLASS_TAG, "content = " + content + "  index = " + index);
        String url = content.substring(index);
        MyLogger.logD(CLASS_TAG, "url = " + url);
        if (url != null) {
            return url;
        }
        // Pattern pattern = Pattern.compile(URL);
        // Matcher matcher = pattern.matcher(url);
        // if (matcher.matches()) {
        // return url;
        // }

        return null;
    }
}
