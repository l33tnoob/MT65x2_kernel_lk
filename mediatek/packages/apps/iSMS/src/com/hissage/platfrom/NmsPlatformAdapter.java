package com.hissage.platfrom;

import java.util.ArrayList;

import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;

import com.hissage.message.ip.NmsIpMessageConsts.NmsCancelNotification;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsPlatformAdapter {
    private static final String TAG = "NmsPlatformAdapter";
    private static NmsPlatformAdapter mInstance = null;
    private static Context mContext = null;
    private static NmsMtkTelephonyManager telephonyMan = null;
    private static NmsMtkSmsManager smsMan = null;
    private static NmsMtkSimInfo sysSimInfoMan = null;
    private static NmsMtkSettings sysSettingsMan = null;
    private static NmsMtkVCard sysVcardComposer = null;
    private static NmsMtkStatusBarManager mStatusBarManager = null;

    private NmsPlatformAdapter() {

    }

    public synchronized static NmsPlatformAdapter getInstance(Context context) {
        if (null == mInstance) {
            mInstance = new NmsPlatformAdapter();
            if (context != null)
                mContext = context.getApplicationContext();
            telephonyMan = new NmsMtkTelephonyManager(context);
            smsMan = new NmsMtkSmsManager(context);
            sysSimInfoMan = new NmsMtkSimInfo(context);
            sysSettingsMan = new NmsMtkSettings(context);
            sysVcardComposer = new NmsMtkVCard(context);
            mStatusBarManager = new NmsMtkStatusBarManager(context);
        }
        return mInstance;
    }

    public String getImsi(int slotId) {
        NmsMtkTelephonyManager telephonyManager = new NmsMtkTelephonyManager(mContext);
        return telephonyManager.getSubscriberId(slotId);
    }

    public String[] getAllImsi() {

        String[] imsiList = new String[NmsConsts.SIM_CARD_COUNT];

        imsiList[NmsConsts.SIM_CARD_SLOT_1] = telephonyMan
                .getSubscriberId(NmsConsts.SIM_CARD_SLOT_1);
        if (sysSimInfoMan.isMtkGeminiSupport()) {
            if (telephonyMan.mPlatfromMode == NmsPlatformBase.NMS_INTEGRATION_MODE) {
                imsiList[NmsConsts.SIM_CARD_SLOT_2] = telephonyMan
                        .getSubscriberId(NmsConsts.SIM_CARD_SLOT_2);
            }
        }
        NmsLog.trace(TAG, "get all imsi: " + imsiList[NmsConsts.SIM_CARD_SLOT_1] + ", "
                + imsiList[NmsConsts.SIM_CARD_SLOT_2]);
        return imsiList;
    }

    public long getSimIdByImsi(String imsi, int simId) {

        if (telephonyMan.mPlatfromMode == NmsPlatformBase.NMS_INTEGRATION_MODE) {
            String[] allImsi = getAllImsi();
            if (imsi.equals(allImsi[NmsConsts.SIM_CARD_SLOT_1])) {
                return sysSimInfoMan.getSimIdBySlot(mContext, NmsConsts.SIM_CARD_SLOT_1);
            } else if (imsi.equals(allImsi[NmsConsts.SIM_CARD_SLOT_2])) {
                return sysSimInfoMan.getSimIdBySlot(mContext, NmsConsts.SIM_CARD_SLOT_2);
            } else {
                NmsLog.error(TAG, "getSimIdByImsi error, both of slots not include this imsi.");
                return simId;
            }
        } else {
            NmsLog.error(TAG, "platform mode is: " + sysSimInfoMan.getModeString());
            return simId;
        }
    }

    public ArrayList<String> divideMessage(String text) {
        return smsMan.divideMessage(text);
    }

    public void sendMultipartTextMessage(String destinationAddress, String scAddress,
            ArrayList<String> parts, long simId, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        smsMan.sendMultipartTextMessage(smsMan, destinationAddress, scAddress, parts, simId,
                sentIntents, deliveryIntents);
    }

    public int getSlotIdBySimId(long simId) {
        return sysSimInfoMan.getSlotIdBySimId(mContext, simId);
    }

    public long getSimIdBySlotId(int slotId) {
        return sysSimInfoMan.getSimIdBySlot(mContext, slotId);
    }

    public String getSimName(long simId) {
        return sysSimInfoMan.getName(mContext, simId);
    }

    public int getSimColor(long simId) {
        return sysSimInfoMan.getColor(mContext, simId);
    }

    public CharSequence getSimIndicator(long simId){
        return sysSimInfoMan.getSimIndicator((int)simId);
    }
    
    public long getCurrentSimId() {
        return sysSettingsMan.getCurrentSimId(mContext);
    }
    
    public long getNmsCurrentSimId(){
        return sysSettingsMan.getNmsCurrentSimId(mContext);
    }

    public boolean setCurrentSimId(long simId) {
        if (getSimIdBySlotId(NmsConsts.SIM_CARD_SLOT_1) != simId
                && getSimIdBySlotId(NmsConsts.SIM_CARD_SLOT_2) != simId) {
            return false;
        }
        return sysSettingsMan.setCurrentSimId(mContext, simId);
    }

    public String getLine1Number(int slotId) {
        return telephonyMan.getLine1Number(slotId);
    }

    public void hideSIMIndicator(ComponentName componentName) {
        mStatusBarManager.hideSIMIndicator(componentName);
    }

    public void showSIMIndicator(ComponentName componentName) {
        mStatusBarManager.showSIMIndicator(componentName);
    }

    public String getVcfViaSysContactId(Context context, long[] contactsId) {
        return sysVcardComposer.nmsGetVcfViaSysContactId(context, contactsId);
    }

    public void CancelNotification(long id) {
        Intent intent = new Intent();
        intent.setAction(NmsCancelNotification.NMS_CANCEL_NOTIFICATION);
        intent.putExtra(NmsCancelNotification.NMS_CANCEL_NONTFICATION_ID, id);
        mContext.sendBroadcast(intent);
        NmsLog.trace(TAG, "cancel isms new msg notification");
    }

    public boolean isMtkGeminiSupport(){
        return sysSimInfoMan.isMtkGeminiSupport();
    }
}
