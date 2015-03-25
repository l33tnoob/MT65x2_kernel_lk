package com.hissage.util.data;

import com.hissage.util.log.NmsLog;

import android.text.TextUtils;

public final class NmsConverter {
    /*
     * This function convert 4 bytes to an integer.
     */
    public static int bytes2Int(byte[] data) {
        int x1 = (s2u((int) data[0]) << 24);
        int x2 = (s2u((int) data[1]) << 16);
        int x3 = (s2u((int) data[2]) << 8);
        int x4 = s2u(data[3]);
        return (x1 | x2 | x3 | x4);
    }

    /*
     * This function convert an integer to 4 bytes.
     */
    public static byte[] int2Bytes(int data) {
        byte[] ret = new byte[4];
        ret[0] = (byte) (data >>> 24);
        ret[1] = (byte) ((data << 8) >>> 24);
        ret[2] = (byte) ((data << 16) >>> 24);
        ret[3] = (byte) ((data << 24) >>> 24);
        return ret;
    }

    /*
     * this function convert a short value to 2 bytes
     */
    public static byte[] short2Bytes(short val) {
        byte[] ret = new byte[2];
        ret[0] = (byte) (val >>> 8);
        ret[1] = (byte) (val << 8 >>> 8);
        return ret;
    }

    /*
     * this function convert 2 bytes to a short.
     */
    public static short bytes2Short(byte[] data) {
        int val = data[0];
        return (short) (((short) val) | ((short) data[1] >>> 8));
    }

    /*
     * This function convert an integer to a string.
     */
    public static String int2String(int data) {
        StringBuffer ret = new StringBuffer();
        ret.append(data);
        return ret.toString();
    }

    /*
     * this function convert a string to an integer.
     */
    public static int string2Int(String data) {
        String strTemp = data.replaceAll("[^0-9]", "");
        return Integer.parseInt(strTemp);
    }

    /*
     * this function convert a IP32 value to a string
     */
    public static String intIp2StringIp(int ip) {
        StringBuffer sb = new StringBuffer();
        sb.append((int) (ip >>> 24));
        sb.append('.');
        sb.append((int) ((ip << 8) >>> 24));
        sb.append('.');
        sb.append((int) ((ip << 16) >>> 24));
        sb.append('.');
        sb.append((int) ((ip << 24) >>> 24));
        return sb.toString();
    }

    /*
     * this function convert a string IP to a IP32 value.
     */
    public static int stringIp2IntIp(String ip) {
        if (!TextUtils.isEmpty(ip)) {
            StringBuffer[] bug = new StringBuffer[4];
            int idx = 0;
            for (int i = 0; i < ip.length(); i++) {
                if (ip.charAt(i) == '.') {
                    idx++;
                } else {
                    if (null == bug[idx])
                        bug[idx] = new StringBuffer();
                    bug[idx].append(ip.charAt(i));
                }
            }
            int a = Integer.parseInt(bug[0].toString());
            int b = Integer.parseInt(bug[1].toString());
            int c = Integer.parseInt(bug[2].toString());
            int d = Integer.parseInt(bug[3].toString());
            return (a << 24) | (b << 16) | (c << 8) | d;
        } else {
            return 0;
        }
    }

    /*
     * this function convert a byte array to a HEX string.
     */
    public static String bytes2HexString(final byte[] bytes) {
        StringBuffer sb = new StringBuffer();

        final char[] map = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
                'E', 'F' };
        sb.append("\r\n\t\t\t");
        for (int i = 0; i < bytes.length;) {
            int b = bytes[i];
            b = b < 0 ? b & 0x7F + 128 : b;
            sb.append(map[b >>> 4]);
            sb.append(map[b & 0x0F]);
            sb.append(' ');
            i++;
            if (i % 16 == 0) {
                sb.append("\n\t\t\t");
            }
        }
        return sb.toString();
    }

    /*
     * this function convert BE to LE or LE to BE for INT.
     */
    public static int swapFlowBytesInt(int val) {
        return (val << 24) | ((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >>> 8) | (val >>> 24);
    }

    /*
     * this function convert BE to LE or LE to BE for short.
     */
    public static int swapFlowBytesShort(short val) {
        return (val << 8) | (val >>> 8);
    }

    /*
     * if some JVM don't support this feature. re-implement these 2 methods
     * below. it seems that all platform support this converter since MIDP-2.0
     */
    public static String utf8Bytes2String(byte[] dat) {
        if (dat == null)
            return null;
        String cs = null;
        try {
            cs = new String(dat, "UTF-8");
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return cs;
    }

    public static byte[] string2Utf8Bytes(String dat) {
        if (TextUtils.isEmpty(dat))
            return null;

        byte[] re = null;
        try {
            re = dat.getBytes("UTF-8");
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return re;
    }

    // singed to unsigned.
    public static int s2u(int s) {
        if (s < 0) {
            s += 256;
        }
        return s;
    }
}
