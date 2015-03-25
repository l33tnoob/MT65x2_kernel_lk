package com.hissage.util.data;

import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.Context;
import android.provider.Settings;
import android.text.TextUtils;
import android.text.format.DateFormat;

import com.hissage.R;
import com.hissage.service.NmsService;

public class NmsDateTool {

    public static SimpleDateFormat SDF = getDateFormat(NmsService.getInstance());
    public static SimpleDateFormat SDF1 = new SimpleDateFormat("MM-dd");
    public static final SimpleDateFormat SDF2 = new SimpleDateFormat("HH:mm");

    public static final SimpleDateFormat getDateFormat(Context context) {
        String value = Settings.System.getString(context.getContentResolver(),
                Settings.System.DATE_FORMAT);
        if (TextUtils.isEmpty(value)) {
            value = "yyyy-MM-dd";
        }

        return new SimpleDateFormat(value);
    }

    private static String getDefaultFM(boolean bThisYear) {
        String fm;
        char[] order = DateFormat.getDateFormatOrder(NmsService.getInstance());
        if (order != null) {
            if (bThisYear) {
                if (order[0] == 'y' || order[0] == 'Y') {
                    fm = "" + order[1] + order[1] + "/" + order[2] + order[2];
                } else {
                    fm = "" + order[0] + order[0] + "/" + order[1] + order[1];
                }
            } else {
                fm = "" + order[0] + order[0] + "/" + order[1] + order[1] + "/" + order[2]
                        + order[2];
            }
        } else {
            fm = "MM/DD";
        }
        return fm;
    }

    public static String getCurrentFormatTime(long lateMillis) {
        Date curDate = new Date();

        Date today = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate(), 0, 0, 0);
        long curDateMillis = today.getTime();
        long elapsedTime = curDateMillis - lateMillis;

        String timeString = "";
        long oneDay = 24 * 60 * 60 * 1000;
        if (lateMillis - curDateMillis > 0) {
            SimpleDateFormat formater = new SimpleDateFormat("HH:mm");
            timeString = formater.format(lateMillis);
        }

        else if (elapsedTime - oneDay > 0) {

            SimpleDateFormat formater = new SimpleDateFormat("HH:mm");
            String time = formater.format(lateMillis);

            timeString = getDispTimeStr(lateMillis) + " " + time;

        } else {
            SimpleDateFormat formater = new SimpleDateFormat("HH:mm");
            String time = formater.format(lateMillis);

            timeString = NmsService.getInstance().getString(R.string.STR_NMS_YESTERDAY) + " "
                    + time;
        }

        return timeString;
    }

    public static String getDispTimeStr(long time) {

        boolean bThisYear = false;
        Date inDate = new Date(time);
        Date now = new Date();
        if (now.getYear() == inDate.getYear()) {
            if (now.getMonth() == inDate.getMonth() && now.getDate() == inDate.getDate()) {
                return SDF2.format(time);
            }
            bThisYear = true;
        }

        String fm = Settings.System.getString(NmsService.getInstance().getContentResolver(),
                Settings.System.DATE_FORMAT);
        if (TextUtils.isEmpty(fm)) {
            fm = getDefaultFM(bThisYear);
        } else {
            if (bThisYear) {
                if (fm.startsWith("Y") || fm.startsWith("y")) {
                    fm = fm.replace("Y", "");
                    fm = fm.replace("y", "");
                    int pos = 0;
                    while (pos < fm.length()) {
                        if (Character.isLetter(fm.charAt(pos))) {
                            break;
                        }
                        pos++;
                    }
                    fm = fm.substring(pos);

                } else if (fm.endsWith("Y") || fm.endsWith("y")) {
                    fm = fm.replace("Y", "");
                    fm = fm.replace("y", "");
                    int pos = fm.length() - 1;
                    while (pos >= 0) {
                        if (Character.isLetter(fm.charAt(pos))) {
                            pos += 1;
                            break;
                        }
                        pos--;
                    }
                    fm = fm.substring(0, pos);
                }
            }
        }

        if (TextUtils.isEmpty(fm))
            fm = getDefaultFM(bThisYear);

        SDF1 = new SimpleDateFormat(fm);
        return SDF1.format(time);

    }

    public static String getDispLoneTimeStr(long time) {
        SDF = getDateFormat(NmsService.getInstance());
        return SDF.format(time);
    }
}
