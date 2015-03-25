package com.mediatek.hotknotbeam;


import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.MimeTypeMap;


import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import com.mediatek.hotknotbeam.HotKnotBeamConstants.State;
import com.mediatek.hotknotbeam.HotKnotFileServer.CommunicationThread;

/**
 * Stores information about an individual download.
 */
public class DownloadInfo {
    private final static String TAG = HotKnotBeamService.TAG;
    private final static int MAX_ID_VALUE = 100000;

    public int mId;
    public String mRootPath;
    public String mFileName;
    public State mState;
    public boolean mIsSucceed;
    public boolean mIsCompress;

    public int mTotalBytes;
    public int mCurrentBytes;

    private CommunicationThread mClientThread;
    private String mExtInfo;
    private Map<String, List<String>> mFileParams;

    public DownloadInfo(String rootPath, String fileName, int fileSize, CommunicationThread clientThread) {
        Random rn = new Random();
        mId = rn.nextInt(MAX_ID_VALUE);
        mRootPath = rootPath;
        mFileName = fileName;
        mCurrentBytes = 0;
        mTotalBytes = fileSize;
        mState = State.CONNECTING;
        mClientThread = clientThread;
        mIsSucceed = false;

        mIsCompress = false;
        mFileParams = null;
    }

    public void setExtInfo(String extInfo) {
        if (extInfo == null) {
            throw new NullPointerException("extInfo");
        }
        try {
            mExtInfo = Uri.decode(extInfo);
        } catch(Exception e) {
            Log.e(TAG, "extInfo:" + extInfo);
            e.printStackTrace();
            return;
        }
        mFileParams = new HashMap<String, List<String>>();
        mFileParams = getFileParameters(mExtInfo);
    }

    public CommunicationThread getClientThread() {
        return mClientThread;
    }

    public String getTitle() {
        return mFileName;
    }

    public String getTag() {
        return mId + ":" + mTotalBytes;
    }

    public void setState(State state) {
        mState = state;
    }

    public boolean getResult() {
        return mIsSucceed;
    }

    public void setResult(boolean succeed) {
        mIsSucceed = succeed;
    }

    public void setCurrentBytes(int remainingBytes) {
        mCurrentBytes = mTotalBytes - remainingBytes;
    }

    public int getCurrentBytes() {
        return mCurrentBytes;
    }

    public int getTotalBytes() {
        return mTotalBytes;
    }

    public Uri getUri() {
        Uri uri = null;

        Log.d(TAG, "getUri:" + mFileName);
        try {
            uri = Uri.parse("file://" + mRootPath + "/" + mFileName);
        } catch(Exception e) {
            Log.e(TAG, "[Failed]uri parse:" + e.getMessage());
        }
        Log.d(TAG, "getUri:" + uri);
        return uri;
    }

    public String getMimeType() {
        String mimeType = "";

        MimeTypeMap mimeTypeMap = MimeTypeMap.getSingleton();
        String extension = MimeTypeMap.getFileExtensionFromUrl(mFileName).toLowerCase();
        if (TextUtils.isEmpty(extension)) {
            int dotPos = mFileName.lastIndexOf('.');
            if (0 <= dotPos) {
                extension = mFileName.substring(dotPos + 1);
                extension = extension.toLowerCase();
            }
        }
        mimeType = mimeTypeMap.getMimeTypeFromExtension(extension);

        if(mimeType == null) {
            //Check self-defined
            mimeType = MimeUtilsEx.guessMimeTypeFromExtension(extension);
            if(mimeType == null) {
                Log.e(TAG, "No corresponding mime type");
            }
        }

        return mimeType;
    }

    public String toString() {
        StringBuilder str = new StringBuilder("TAG:");

        str.append(getTag() + " " + mState + " " + mCurrentBytes + "/" + mTotalBytes);

        return str.toString();
    }

    public boolean equals(DownloadInfo info) {
        return mId == info.mId;
    }

    public String getSaveFolder() {
        String folderPath = getFileParameter(HotKnotBeamConstants.QUERY_FOLDER);
        if(folderPath != null && folderPath.length() > 0) {
            if(folderPath.charAt(0) != '/') {
                mRootPath = mRootPath + "/" + folderPath;
            } else {
                mRootPath = mRootPath + folderPath;
            }
        }
        return mRootPath;
    }

    public boolean isCompressed() {
       return checkParameterValue(HotKnotBeamConstants.QUERY_ZIP, HotKnotBeamConstants.QUERY_VALUE_YES, true);
    }

    public boolean isShowNotification() {
        return checkParameterValue(HotKnotBeamConstants.QUERY_SHOW, HotKnotBeamConstants.QUERY_VALUE_NO, false);
    }

    public boolean isRenameFile() {
        return checkParameterValue(HotKnotBeamConstants.QUERY_FORMAT, "raw", false);
    }

    private boolean checkParameterValue(String keyObject, String keyValue, boolean expectedValue) {
        boolean isMatch = !expectedValue;

        String obj = getFileParameter(keyObject);
        if(obj != null) {
            obj = obj.toLowerCase ();
            if(obj.equalsIgnoreCase(keyValue)) {
                isMatch = expectedValue;
            }
        }
        Log.d(TAG, "key:" + keyObject + ":" + isMatch + ":" + obj);
        return isMatch;
    }

    public String getFileParameter(String keyObject) {
        if (keyObject == null) {
            throw new NullPointerException("key");
        }

        if (mFileParams == null) {
            return null;
        }

        List<String> values = mFileParams.get(keyObject);
        if(values == null) {
            return null;
        }
        return (String) values.get(0);
    }

    private static Map<String, List<String>> getFileParameters(String line) {
        Map<String, List<String>> params = new HashMap<String, List<String>>();

        for (String param : line.split("&")) {
            String pair[] = param.split("=");
            String key = pair[0];
            String value = "";

            if (pair.length > 1) {
                value = pair[1];
            }

            List<String> values = params.get(key);
            if (values == null) {
                values = new ArrayList<String>();
                params.put(key, values);
            }
            values.add(value);
        }

        return params;
    }
}
