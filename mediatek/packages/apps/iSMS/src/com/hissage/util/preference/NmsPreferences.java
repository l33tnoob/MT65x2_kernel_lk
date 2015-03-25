package com.hissage.util.preference;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

import com.hissage.util.log.NmsLog;

public final class NmsPreferences {

    private static final String TAG = "NmsPreference";
    private static SharedPreferences mSp;
    private static final String preferences_name = "hesinePreferences";

    public static void initPreferences(Context context) {
        try {
            if (mSp != null) {
                return;
            }
            mSp = context.getSharedPreferences(preferences_name, Activity.MODE_WORLD_WRITEABLE);
        } catch (Exception e) {
            NmsLog.trace(TAG,
                    "init preferences xml file exception:" + NmsLog.nmsGetStactTrace(e));
        }
    }

    public static void delete(String name) {
        try {
            Editor editor = mSp.edit();
            editor.remove(name);
            editor.commit();
        } catch (Exception e) {
            NmsLog.trace(TAG,
                    "delete key-value from preferences xml and key:" + NmsLog.nmsGetStactTrace(e));
        }
    }

    public static synchronized String getStringValue(String key) {
        try {
            if (null == mSp)
                return null;
            else
                return mSp.getString(key, null);
        } catch (Exception e) {
            NmsLog.trace(TAG, "get string key-value from preferences xml and key:" + key);
            NmsLog.nmsPrintStackTrace(e);
            return "";
        }
    }

    public static synchronized Integer getIntegerValue(String key) {
        try {
            if (null == mSp)
                return 0;
            else
                return mSp.getInt(key, 0);
        } catch (Exception e) {
            NmsLog.trace(TAG, "get Integer key-value from preferences xml and key:" + key);
            NmsLog.nmsPrintStackTrace(e);
            return -1;
        }
    }

    public static synchronized long getLongValue(String key) {// added by
                                                              // luozheng in
                                                              // 12.2.29;
        try {
            if (null == mSp)
                return 0;
            else
                return mSp.getLong(key, 0);
        } catch (Exception e) {
            NmsLog.trace(TAG, "get Long key-value from preferences xml and key:" + key);
            NmsLog.nmsPrintStackTrace(e);
            return -1;
        }
    }

    public static synchronized Integer getIntegerValueDefault(String key) {
        try {
            if (null == mSp)
                return -1;
            else
                return mSp.getInt(key, -1);
        } catch (Exception e) {
            NmsLog.trace(TAG, "get default Integer key-value from preferences xml and key:" + key);
            return -1;
        }
    }

    public static synchronized void setStringValue(String key, String value) {
        try {
            if (null != mSp) {
                Editor editor = mSp.edit();
                editor.putString(key, value);
                editor.commit();
            }
        } catch (Exception e) {
            NmsLog.trace(TAG, "set String key-(String)value from preferences xml and key:" + key
                    + ",value:" + value);
        }
    }

    public static synchronized void setIntegerValue(String key, Integer value) {
        try {
            if (null != mSp) {
                Editor editor = mSp.edit();
                editor.putInt(key, value);
                editor.commit();
            }
        } catch (Exception e) {
            NmsLog.trace(TAG, "set String key-(int)value from preferences xml and key:" + key
                    + ",value:" + value);
        }
    }

    public static synchronized void setLongValue(String key, long value) {// added
                                                                          // by
                                                                          // luozheng
                                                                          // in
                                                                          // 12.2.29
        try {
            if (null != mSp) {
                Editor editor = mSp.edit();
                editor.putLong(key, value);
                editor.commit();
            }
        } catch (Exception e) {
            NmsLog.trace(TAG, "set String key-(long)value from preferences xml and key:" + key
                    + ",value:" + value);
        }
    }

    public static synchronized void clearAllConfig() {
        try {
            if (null != mSp) {
                Editor editor = mSp.edit();
                editor.clear();
                editor.commit();
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public static void destory() {
        mSp = null;
        NmsLog.trace(TAG, "hesine preferences xml destory.");
    }
}
