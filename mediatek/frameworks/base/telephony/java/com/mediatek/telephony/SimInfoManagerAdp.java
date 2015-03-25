package com.mediatek.telephony;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Environment;
import android.os.Parcelable;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;
import android.util.Patterns;

import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

//MTK-START [mtk04070][111121][ALPS00093395]MTK added
import android.content.ContentUris;
import android.database.DatabaseUtils;
import com.android.internal.telephony.PhoneConstants;
import java.util.ArrayList;
import java.util.List;

import android.provider.BaseColumns;
import android.provider.Telephony.SIMInfo;

import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

/**
 *@hide
 */
public final class SimInfoManagerAdp {
    private static final String LOG_TAG = "PHONE";

    public static SIMInfo copyFromSimInfoRecord(SimInfoRecord simInfoRecord) {
        if (simInfoRecord == null) {
            return null;
        } else {
            SIMInfo info = SIMInfo.getSIMInfoInstance();
            info.mSimId = simInfoRecord.mSimInfoId;
            info.mICCId = simInfoRecord.mIccId;
            info.mDisplayName = simInfoRecord.mDisplayName;
            info.mNameSource = simInfoRecord.mNameSource;
            info.mNumber = simInfoRecord.mNumber;
            info.mDispalyNumberFormat = simInfoRecord.mDispalyNumberFormat;
            info.mColor = simInfoRecord.mColor;
            info.mDataRoaming = simInfoRecord.mDataRoaming;
            info.mSlot = simInfoRecord.mSimSlotId;
            info.mOperator = simInfoRecord.mOperator;
            return info;
        }
    }

    public static List<SIMInfo> getInsertedSimInfoListAdp(Context ctx) {
        logd("[getInsertedSimInfoListAdp]");
        List<SimInfoRecord> simInfoRecordList = SimInfoManager.getInsertedSimInfoList(ctx);
        if (simInfoRecordList == null) {
            return null;
        } else {
            List<SIMInfo> simList = new ArrayList<SIMInfo>();
            for (int i=0; i < simInfoRecordList.size(); i++) {
                simList.add(copyFromSimInfoRecord(simInfoRecordList.get(i)));
            }
            return simList;
        }
    }

    public static List<SIMInfo> getAllSimInfoListAdp(Context ctx) {
        logd("[getAllSimInfoListAdp]");
        List<SimInfoRecord> simInfoRecordList = SimInfoManager.getAllSimInfoList(ctx);
        if (simInfoRecordList == null) {
            return null;
        } else {
            List<SIMInfo> simList = new ArrayList<SIMInfo>();
            for (int i=0; i < simInfoRecordList.size(); i++) {
                simList.add(copyFromSimInfoRecord(simInfoRecordList.get(i)));
            }
            return simList;
        }
    }

    public static SIMInfo getSimInfoByIdAdp(Context ctx, long simId) {
        logd("[getSimInfoByIdAdp]");
        SimInfoRecord simInfoRecord = SimInfoManager.getSimInfoById(ctx, simId);
        return copyFromSimInfoRecord(simInfoRecord);
    }

    public static SIMInfo getSimInfoByNameAdp(Context ctx, String simName) {
        logd("[getSimInfoByNameAdp]");
        SimInfoRecord simInfoRecord = SimInfoManager.getSimInfoByName(ctx, simName);
        return copyFromSimInfoRecord(simInfoRecord);
    }
    
    public static SIMInfo getSimInfoBySlotAdp(Context ctx, int slotId) {
        logd("[getSimInfoBySlotAdp]");
        SimInfoRecord simInfoRecord = SimInfoManager.getSimInfoBySlot(ctx, slotId);
        return copyFromSimInfoRecord(simInfoRecord);
    }
    
    public static SIMInfo getSimInfoByIccIdAdp(Context ctx, String iccId) {
        logd("[getSimInfoByIccIdAdp]");
        SimInfoRecord simInfoRecord = SimInfoManager.getSimInfoByIccId(ctx, iccId);
        return copyFromSimInfoRecord(simInfoRecord);
    }

    public static int getInsertedSimCountAdp(Context ctx) {
        logd("[getInsertedSimCountAdp]");
        return SimInfoManager.getInsertedSimCount(ctx);
    }
    
    public static int getAllSimCountAdp(Context ctx) {
        logd("[getAllSimCountAdp]");
        return SimInfoManager.getAllSimCount(ctx);
    }

    public static int setOperatorByIdAdp(Context ctx, String operator, long simId) {
        logd("[setOperatorByIdAdp]");
        return SimInfoManager.setOperatorById(ctx, operator, simId);
    }

    public static int setDisplayNameAdp(Context ctx, String displayName, long simId) {
        logd("[setDisplayNameAdp]");
        return SimInfoManager.setDisplayName(ctx, displayName, simId);
    }

    public static int setDisplayNameExAdp(Context ctx, String displayName, long simId, long source) {
        logd("[setDisplayNameExAdp]");
        return SimInfoManager.setDisplayNameEx(ctx, displayName, simId, source);
    }

    public static int setNumberAdp(Context ctx, String number, long simId) {
        logd("[setNumberAdp]");
        return SimInfoManager.setNumber(ctx, number, simId);
    }

    public static int setColorAdp(Context ctx, int color, long simId) {
        logd("[setColorAdp]");
        return SimInfoManager.setColor(ctx, color, simId);
    }
   
    public static int setDispalyNumberFormatAdp(Context ctx, int format, long simId) {
        logd("[setDispalyNumberFormatAdp]");
        return SimInfoManager.setDispalyNumberFormat(ctx, format, simId);
    }

    public static int setDataRoamingAdp(Context ctx, int roaming, long simId) {
        logd("[setDataRoamingAdp]");
        return SimInfoManager.setDataRoaming(ctx, roaming, simId);
    }

    public static Uri addSimInfoRecordAdp(Context ctx, String iccId, int slot) {
        logd("[addSimInfoRecordAdp]");
        return SimInfoManager.addSimInfoRecord(ctx, iccId, slot);
    }
    
    public static int setDefaultNameAdp(Context ctx, long simId, String name) {
        logd("[setDefaultNameAdp]");
        return SimInfoManager.setDefaultName(ctx, simId, name);
    }

    public static int setDefaultNameExAdp(Context ctx, long simId, String name, long nameSource) {
        logd("[setDefaultNameExAdp]");
        return SimInfoManager.setDefaultNameEx(ctx, simId, name, nameSource);
    }
    
    private static void logd(String msg) {
        Log.d(LOG_TAG, "[SimInfoManagerAdp]" + msg);
    }
}
