package com.mediatek.datatransfer.utils;


import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.mediatek.datatransfer.R;


public class Utils {

    public static boolean isRestoring;
    public static boolean isBackingUp;

    public static int getWorkingInfo() {
        int stringId = -1;
        if (isRestoring || isBackingUp) {
            return R.string.state_running;
        }
        return stringId;

    }

    public static String getPhoneSearialNumber() {
        String serial = null;
        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method get = c.getMethod("get", String.class);
            serial = (String) get.invoke(c, "ro.boot.serialno");
        } catch (Exception e) {
            e.printStackTrace();
        }
        return serial;
    }

    public static void writeToFile(String content, String filePath) {
        try {
            FileOutputStream outStream = new FileOutputStream(filePath);
            byte[] buf = content.getBytes();
            outStream.write(buf, 0, buf.length);
            outStream.flush();
            outStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static String readFromFile(String fileName) {
        try {
            InputStream is = new FileInputStream(fileName);
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int len = -1;
            byte[] buffer = new byte[512];
            while ((len = is.read(buffer, 0, 512)) != -1) {
                baos.write(buffer, 0, len);
            }
            is.close();
            return baos.toString();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    /**
     * @author mtk81346
     * @param timestamp the byte[] which Feature Phone saved in file PDU.o
     * @return Feature Phone sendbox's SMS send time.If failed, return
     *         System.currentTimeMillis.
     */
    public static long getFPsendSMSTime(byte[] timestamp) {
        if (timestamp.length != 4) {
            return System.currentTimeMillis();
        }
        long time = unsigned4BytesToInt(timestamp);
        return getFPsendSMSTime(time);
    }

    private static long getFPsendSMSTime(long time) {
        long sec = time % (3600 * 24);
        long hour = sec / 3600;
        sec %= 3600;
        long min = sec / 60;
        long secd = sec % 60;
        int d = 0;
        long day = time / (3600 * 24);
        int y = 1970;
        for (; day > 0; y++) {
            d = 365 + isLeakYeay(y);
            if (day >= d) {
                day -= d;
            } else {
                break;
            }
        }
        int m = 1;
        for (; m < 13; m++) {
            d = getLaseDayPerMonth(m, y);
            if (day >= d) {
                day -= d;
            } else {
                break;
            }
        }
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
        try {
            String sendtime = y + "-" + m + "-" + (day + 1) + " " + hour + ":" + min + ":" + secd;
            Date date = format.parse(sendtime);
            return date.getTime();
        } catch (ParseException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return System.currentTimeMillis();
        }

    }

    private static int getLaseDayPerMonth(int m, int y) {
        int days = 0;
        if (m == 2) {
            if (isLeakYeay(y) == 1) {
                days = 29;
            } else {
                days = 28;
            }
        } else {
            switch (m) {
                case 1:
                case 3:
                case 5:
                case 7:
                case 8:
                case 10:
                case 12:
                    days = 31;
                    break;
                case 4:
                case 6:
                case 9:
                case 11:
                    days = 30;
                    break;
            }
        }
        return days;
    }

    private static int isLeakYeay(int y) {
        if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) {
            return 1;
        }
        return 0;
    }

    private static long unsigned4BytesToInt(byte[] buf) {
        int firstByte = 0;
        int secondByte = 0;
        int thirdByte = 0;
        int fourthByte = 0;
        int index = 0;
        firstByte = (0x000000FF & ((int) buf[index]));
        secondByte = (0x000000FF & ((int) buf[index + 1]));
        thirdByte = (0x000000FF & ((int) buf[index + 2]));
        fourthByte = (0x000000FF & ((int) buf[index + 3]));
        index = index + 4;
        return ((long) (firstByte << 24 | secondByte << 16 | thirdByte << 8 | fourthByte)) & 0xFFFFFFFFL;
    }
}
