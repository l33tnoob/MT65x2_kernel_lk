package com.hissage.message.ip;

import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.location.NmsLocationFormat;
import com.hissage.util.log.NmsLog;

public class NmsIpLocationMessage extends NmsIpAttachMessage {

    /**
     * 
     */
    private static final long serialVersionUID = 104885267870420072L;
    public double longitude;
    public double latitude;
    public String address; /* max length is 100 */
    public String thumbPath; /* for send msg, it's "" */
    private NmsIpLocationMessage mInstance;

    public NmsIpLocationMessage() {
        super();
        setType(NmsIpMessageConsts.NmsIpMessageType.LOCATION);
        mInstance = this;
    }

    public NmsIpLocationMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom,
            String jpath, String jurl, int jsize, double jlongitude, double jlatitude,
            String jaddress, String jthumbPath) {
        super(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom,
                jpath, jurl, jsize);
        longitude = jlongitude;
        latitude = jlatitude;
        address = jaddress;
        thumbPath = jthumbPath;
    }

    public static NmsIpLocationMessage formatLocationMsg(NmsIpLocationMessage msg) {
        if (msg == null) {
            NmsLog.error("formatLocationMsg", "msg is null!");
            return null;
        }

        NmsIpLocationMessage locMsg = str2LocMsg(msg.address);

        if (locMsg != null) {
            msg.address = locMsg.address;
            msg.latitude = locMsg.latitude;
            msg.longitude = locMsg.longitude;
        }

        return msg;
    }

    public static String locMsg2Str(NmsIpLocationMessage msg) {
        if (msg == null) {
            NmsLog.error("locMsg2Str", "msg is null!");
            return "";
        }

        String formatLoc = NmsLocationFormat.BEGINE + NmsLocationFormat.FLAG_FIRST;

        formatLoc += NmsLocationFormat.ADDRESS + NmsLocationFormat.FLAG_SECOND
                + NmsLocationFormat.formatLocationStr(msg.address) + NmsLocationFormat.FLAG_FIRST;

        formatLoc += NmsLocationFormat.LONGITUDE + NmsLocationFormat.FLAG_SECOND + msg.longitude
                + NmsLocationFormat.FLAG_FIRST;

        formatLoc += NmsLocationFormat.LATITUDE + NmsLocationFormat.FLAG_SECOND + msg.latitude
                + NmsLocationFormat.FLAG_FIRST;

        formatLoc += NmsLocationFormat.END;

        return formatLoc;
    }

    public static NmsIpLocationMessage str2LocMsg(String formatLoc) {
        if (NmsLocationFormat.checkIsLocationData(formatLoc) == -1) {
            NmsLog.error("str2LocMsg", "This str is not location!");
            return null;
        }

        NmsIpLocationMessage locMsg = new NmsIpLocationMessage();

        String[] cutLocFormat = formatLoc.split(NmsLocationFormat.FLAG_FIRST);

        for (int i = 1; i < cutLocFormat.length - 1; ++i) {
            String keyValue = cutLocFormat[i];
            if (!keyValue.contains(NmsLocationFormat.FLAG_SECOND)) {
                NmsLog.error("str2LocMsg", "This record is error.");
                continue;
            }
            String[] cutKeyValue = keyValue.split(NmsLocationFormat.FLAG_SECOND);
            if (cutKeyValue.length != 2) {
                NmsLog.error("str2LocMsg", "This record is error..");
                continue;
            }
            String key = cutKeyValue[0];
            String value = cutKeyValue[1];
            if (key.equals(NmsLocationFormat.ADDRESS)) {
                locMsg.address = value;
            } else if (key.equals(NmsLocationFormat.LONGITUDE)) {
                try {
                    locMsg.longitude = Double.parseDouble(value);
                } catch (Exception e) {
                    NmsLog.error("str2LocMsg", value + " cannot be parsed as a double value.");
                    continue;
                }
            } else if (key.equals(NmsLocationFormat.LATITUDE)) {
                try {
                    locMsg.latitude = Double.parseDouble(value);
                } catch (Exception e) {
                    NmsLog.error("str2LocMsg", value + " cannot be parsed as a double value.");
                    continue;
                }
            } else {
                NmsLog.error("str2LocMsg", "This record is error...");
                continue;
            }
        }

        return locMsg;
    }

    public void send(String number, int simId, final int sendMode, final boolean delDraft) {
        if (TextUtils.isEmpty(number)) {
            return;
        }
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX);
        setSimId(simId);
        setReceiver(number);
        mExecutorService.submit(new Runnable() {
            @Override
            public void run() {
                NmsIpMessageApiNative.nmsSaveIpMsg(mInstance, sendMode, delDraft, true);
            }
        });
    }

    public int saveDraft(String number, int simId) {
        if (TextUtils.isEmpty(number)) {
            return -1;
        }
        setSimId(simId);
        setReceiver(number);
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.DRAFT);
        return NmsIpMessageApiNative.nmsSaveIpMsg(mInstance,
                NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, true, true);
    }

    // @Override
    // public void readFromParcel(Parcel in) {
    // super.readFromParcel(in);
    // longitude = in.readDouble();
    // latitude = in.readDouble();
    // address = in.readString();
    // thumbPath = in.readString();
    // }
    //
    // @Override
    // public void writeToParcel(Parcel out, int flags) {
    // super.writeToParcel(out, flags);
    // out.writeDouble(longitude);
    // out.writeDouble(latitude);
    // out.writeString(address);
    // out.writeString(thumbPath);
    // }

}
