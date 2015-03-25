package com.mediatek.datatransfer;

import com.mediatek.datatransfer.utils.ModuleType;

public class PersonalItemData {

    private int mType;
    private boolean mIsEnable;
    
    public PersonalItemData(int type, boolean isEnable) {
        mType = type;
        mIsEnable = isEnable;
    }

    public int getType() {
        return mType;
    }

    public int getIconId() {
        int ret = ModuleType.TYPE_INVALID;
        switch (mType) {
        case ModuleType.TYPE_CONTACT:
            ret = R.drawable.ic_contact;
            break;

        case ModuleType.TYPE_MESSAGE:
            ret = R.drawable.ic_message;
            break;

        case ModuleType.TYPE_PICTURE:
            ret = R.drawable.ic_picture;
            break;
        case ModuleType.TYPE_CALENDAR:
            ret = R.drawable.ic_canlendar;
            break;

        case ModuleType.TYPE_MUSIC:
            ret = R.drawable.ic_music;
            break;

        case ModuleType.TYPE_NOTEBOOK:
            ret = R.drawable.ic_notebook;
            break;

        case ModuleType.TYPE_BOOKMARK:
            ret = R.drawable.ic_bookmark;
            break;

        default:
            break;
        }
        return ret;
    }

    public int getTextId() {
        int ret = ModuleType.TYPE_INVALID;
        switch (mType) {
//        case ModuleType.TYPE_SELECT:
//            ret = R.string.selectall;
//            break;
            
        case ModuleType.TYPE_CONTACT:
            ret = R.string.contact_module;
            break;

        case ModuleType.TYPE_MESSAGE:
            ret = R.string.message_module;
            break;

        case ModuleType.TYPE_PICTURE:
            ret = R.string.picture_module;
            break;
        case ModuleType.TYPE_CALENDAR:
            ret = R.string.calendar_module;
            break;

        case ModuleType.TYPE_MUSIC:
            ret = R.string.music_module;
            break;

        case ModuleType.TYPE_NOTEBOOK:
            ret = R.string.notebook_module;
            break;

        case ModuleType.TYPE_BOOKMARK:
            ret = R.string.bookmark_module;
            break;

        default:
            break;
        }
        return ret;
    }

    public boolean isEnable() {
        return mIsEnable;
    }

    public void setEnable(boolean enable) {
        mIsEnable = enable;
    }
}
