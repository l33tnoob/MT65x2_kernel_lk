package com.hissage.contact;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;

import com.hissage.api.NmsContactApi;
import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.db.NmsContentResolver;
import com.hissage.jni.engineadapter;
import com.hissage.util.log.NmsLog;

public class NmsUIContact {

    private static final String TAG = "NmsUIContact";

    private long systemContactId;
    private short engineContactId;
    private int type; // NmsContactType
    // private Bitmap avatar;
    private String name;
    private String numberOrEmail;
    private int    typeStrId = -1 ;
    private String typtStrLabel = null ;
    private String sortKey; // for search

    public void setSystemContactId(long systemContactId) {
        this.systemContactId = systemContactId;
    }

    public long getSystemContactId() {
        return systemContactId;
    }

    public void setEngineContactId(short engineContactId) {
        this.engineContactId = engineContactId;
    }

    public short getEngineContactId() {
        return engineContactId;
    }

    public void setType(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }

    // public void setAvatar(Bitmap avatar) {
    // this.avatar = avatar;
    // }
    //
    // public Bitmap getAvatar() {
    // return avatar;
    // }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setNumberOrEmail(String numberOrEmail) {
        this.numberOrEmail = numberOrEmail;
    }

    public String getNumberOrEmail() {
        return numberOrEmail;
    }

    public void setNumberOrEmailType(int typeId, String typeLabel) {
        typeStrId = typeId ;
        typtStrLabel = typeLabel ;
    }

    public String getNumberOrEmailType(Context context) {
        if (typeStrId == -1 && typtStrLabel == null) 
            return "" ;
        
        return Phone.getTypeLabel(context.getResources(), typeStrId, typtStrLabel).toString();
    }

    public void setSortKey(String sortKey) {
        this.sortKey = sortKey;
    }

    public String getSortKey() {
        return sortKey;
    }

    public Bitmap getAvatar(Context context) {
        if (context == null) {
            NmsLog.error(TAG, "getAvatar. conext is null");
            return null;
        }

        Bitmap result = NmsContactApi.getInstance(context).getSystemAvatarViaSystemContactId(
                systemContactId);
        if (result == null && type == NmsContactType.HISSAGE_USER) {
            result = NmsContactApi.getInstance(context).getEngineAvatarViaEngineContactId(
                    engineContactId);
        }

        return result;
    }

    public static NmsUIContact getUiContactInfoViaId(Context context, long _id) {
        if (context == null) {
            NmsLog.error(TAG, "conext is null");
            return null;
        }
        if (_id <= 0) {
            NmsLog.error(TAG, "_id <= 0");
            return null;
        }

        NmsUIContact result = null;
        Cursor cursor = null;
        try {
            cursor = NmsContentResolver.query(context.getContentResolver(), Data.CONTENT_URI,
                    new String[] { Data.CONTACT_ID, Data.DISPLAY_NAME, Data.MIMETYPE, Data.DATA1,
                            Data.DATA2, Data.DATA3, Data.SORT_KEY_PRIMARY }, Data._ID + "=" + _id,
                    null, null);
            if (cursor != null && cursor.moveToFirst()) {
                result = getUiContactViaCursor(context, cursor);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        return result;
    }

    public static NmsUIContact getUiContactViaCursor(Context context, Cursor cursor) {
        if (cursor == null) {
            NmsLog.error(TAG, "cursor is null");
            return null;
        }
        if (cursor.isClosed()) {
            NmsLog.error(TAG, "cursor is closed.");
            return null;
        }

        try {
            NmsUIContact result = new NmsUIContact();
            // setSystemContactId
            result.setSystemContactId(cursor.getLong(cursor.getColumnIndex(Data.CONTACT_ID)));
            // setName
            result.setName(cursor.getString(cursor.getColumnIndex(Data.DISPLAY_NAME)));

            if (cursor.getString(cursor.getColumnIndex(Data.MIMETYPE)).equals(
                    Phone.CONTENT_ITEM_TYPE)) {
                // setNumber
                result.setNumberOrEmail(NmsCommonUtils.nmsGetStandardPhoneNum(cursor
                        .getString(cursor.getColumnIndex(Data.DATA1))));
                // setType (NmsContactType)
                boolean isHissageNumber = NmsContactApi.getInstance(context).isHissageNumber(
                        result.getNumberOrEmail());
                result.setType(isHissageNumber ? NmsContactType.HISSAGE_USER
                        : NmsContactType.NOT_HISSAGE_USER);
                // setEngineContactId
                if (isHissageNumber) {
                    result.setEngineContactId((short) engineadapter.get().nmsUIGetContactId(
                            result.getNumberOrEmail()));
                }
                // setNumberType
                result.setNumberOrEmailType(cursor.getInt(cursor.getColumnIndex(Data.DATA2)), 
                        cursor.getString(cursor.getColumnIndex(Data.DATA3)));
                
            } else if (cursor.getString(cursor.getColumnIndex(Data.MIMETYPE)).equals(
                    Email.CONTENT_ITEM_TYPE)) {
                // setEmail
                result.setNumberOrEmail(cursor.getString(cursor.getColumnIndex(Data.DATA1)));
                // setType (NmsContactType)
                result.setType(NmsContactType.NOT_HISSAGE_USER);
                // setEmailType
                result.setNumberOrEmailType(cursor.getInt(cursor.getColumnIndex(Data.DATA2)), 
                        cursor.getString(cursor.getColumnIndex(Data.DATA3)));
            } else {
                NmsLog.error(TAG, "mimeType is UNKNOWN!");
            }
            // setSortKey
            result.setSortKey(cursor.getString(cursor.getColumnIndex(Data.SORT_KEY_PRIMARY)));

            return result;
        } catch (Exception e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
            return null;
        }
    }

}
