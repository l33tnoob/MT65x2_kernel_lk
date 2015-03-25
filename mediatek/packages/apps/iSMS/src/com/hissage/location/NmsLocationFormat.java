package com.hissage.location;

import android.text.TextUtils;

import com.hissage.util.log.NmsLog;

public class NmsLocationFormat {

    private final static String TAG = "NmsLocationFormat";

    /*
     * .[Location] Addr: XXX, XXX Lng: XX.XXXXXX Lat: XXX.XXXXXX ...
     */

    public final static String FLAG_FIRST = "\n";
    public final static String FLAG_SECOND = ": ";
    public final static String FLAG_THIRD = "-";

    public final static String BEGINE = ".[Location]";
    public final static String BEGINE2 = ".<Location>";
    public final static String END = "...";

    public final static String ADDRESS = "Addr";
    public final static String LONGITUDE = "Lng";
    public final static String LATITUDE = "Lat";

    public final static int ADDR_TYPE_NAME = 1;
    public final static int ADDR_TYPE_VICINITY = 2;

    public static int checkIsLocationData(String data) {
        if (TextUtils.isEmpty(data)) {
            return -1;
        }

        int ret = -1;

        data = data.replace("\r", "");
        String[] cutLocFormat = data.split(NmsLocationFormat.FLAG_FIRST);
        if (cutLocFormat.length > 0
                && (cutLocFormat[0].equals(NmsLocationFormat.BEGINE) || cutLocFormat[0]
                        .equals(NmsLocationFormat.BEGINE2))
                && cutLocFormat[cutLocFormat.length - 1].equals(NmsLocationFormat.END)) {
            ret = 0;
        }

        return ret;
    }

    public static String formatLocationStr(String str) {
        if (TextUtils.isEmpty(str)) {
            NmsLog.error(TAG, "formatLocationStr: str is empty!");
            return "";
        }

        str = str.replace(FLAG_FIRST, " ");
        while (str.contains(FLAG_SECOND)) {
            str = str.replace(FLAG_SECOND, ":");
        }

        str = str.trim();

        return str;
    }

    public static String getDetailAddr(String addr, int addrType) {
        if (TextUtils.isEmpty(addr)) {
            NmsLog.error(TAG, "getDetailAddr: addr is empty!");
            return "";
        }

        String ret = addr;
        if (addr.contains(FLAG_THIRD)) {
            String[] cutAddr = addr.split(FLAG_THIRD);
            if (cutAddr.length == 1) {
                ret = cutAddr[0];
            } else if (cutAddr.length == 2) {
                if (addrType == ADDR_TYPE_NAME) {
                    ret = cutAddr[0];
                } else if (addrType == ADDR_TYPE_VICINITY) {
                    ret = cutAddr[1];
                } else {
                    NmsLog.error(TAG, "getDetailAddr: addrType is UNKNOWN");
                }
            } else {
                NmsLog.error(TAG, "getDetailAddr: cutAddr.length != 2");
            }
        } else {
            NmsLog.error(TAG, "getDetailAddr: addr does not contains 'FLAG_THIRD'");
        }

        ret = ret.trim();

        return ret;
    }

}
