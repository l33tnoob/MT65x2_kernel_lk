package com.mediatek.datatransfer.utils;

import android.content.Context;

import com.mediatek.datatransfer.R;

public class ModuleType {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/ModuleType";
    public static final int TYPE_INVALID = 0x0;
    public static final int TYPE_CONTACT = 0x1;
    public static final int TYPE_SMS = 0x2;
    public static final int TYPE_MMS = 0x4;
    public static final int TYPE_CALENDAR = 0x8;
    public static final int TYPE_APP = 0x10;
    public static final int TYPE_PICTURE = 0x20;
    public static final int TYPE_MESSAGE = 0x40;
    public static final int TYPE_MUSIC = 0x80;
    public static final int TYPE_NOTEBOOK = 0x100;
    public static final int TYPE_BOOKMARK = 0x200;
    //public static final int TYPE_SELECT = 0x400;

    public static String getModuleStringFromType(Context context, int type) {
        int resId = 0;
        switch (type) {
//        case ModuleType.TYPE_SELECT:
//            resId = R.string.contact_module;
//            break;
            
        case ModuleType.TYPE_CONTACT:
            resId = R.string.contact_module;
            break;

        case ModuleType.TYPE_MESSAGE:
            resId = R.string.message_module;
            break;

        case ModuleType.TYPE_CALENDAR:
            resId = R.string.calendar_module;
            break;

        case ModuleType.TYPE_PICTURE:
            resId = R.string.picture_module;
            break;

        case ModuleType.TYPE_APP:
            resId = R.string.app_module;
            break;

        case ModuleType.TYPE_MUSIC:
            resId = R.string.music_module;
            break;

        case ModuleType.TYPE_NOTEBOOK:
            resId = R.string.notebook_module;
            break;

        case ModuleType.TYPE_BOOKMARK:
            resId = R.string.bookmark_module;
            break;

        default:
            break;
        }
        MyLogger.logD(CLASS_TAG, "getModuleStringFromType: resId = " + resId);
        return context.getResources().getString(resId);
    }

    private ModuleType() {
    }
}
