package com.hissage.util.data;

import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.Context;
import android.provider.Settings;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.text.format.DateUtils;
import android.text.format.Time;

import com.hissage.R;

public class NmsDateUtils {

    public String Tag = "NmsDateUtils";

    public static String getCurrentFormatTime(Context context, long lateMillis) {

        Date curDate = new Date();

        long curMillis = System.currentTimeMillis();
        Date today = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate(), 0, 0, 0);
        long curDateMillis = today.getTime();
        long elapsedTime = curDateMillis - lateMillis;

        String timeString = "";
        long oneDay = 24 * 60 * 60 * 1000;
        if (elapsedTime - oneDay > 0) {
            // show time at date time without today, yesterday.
            SimpleDateFormat formater = new SimpleDateFormat("HH:mm");
            String time = formater.format(lateMillis);
            timeString = NmsDateTool.getDispTimeStr(lateMillis) + " " + time;
        } else if (curMillis - lateMillis >= 0 && curMillis - lateMillis < 60 * 1000) {
            // show time at now
            timeString = context.getResources().getString(R.string.STR_NMS_NOW);
        } else if (lateMillis - curDateMillis > 0) {
            // show time at today
            SimpleDateFormat formater = new SimpleDateFormat("HH:mm");
            timeString = formater.format(lateMillis);
        } else {
            // show time at yesterday
            SimpleDateFormat formater = new SimpleDateFormat("HH:mm");
            String time = formater.format(lateMillis);
            timeString = context.getResources().getString(R.string.STR_NMS_YESTERDAY) + " " + time;
        }

        return timeString;
    }

    public static String formatCurrentTime(Context context, long lateMillis) {
        String timeString = "";
        Date curDate = new Date();
        Date today = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate(), 0, 0, 0);
        long curDateMillis = today.getTime();
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
                | DateUtils.FORMAT_CAP_AMPM;
        format_flags |= DateUtils.FORMAT_SHOW_TIME;
        String curTime = DateUtils.formatDateTime(context, lateMillis, format_flags);

        if (lateMillis - curDateMillis > 0) {
            timeString = curTime + " " + context.getResources().getString(R.string.STR_NMS_TODAY);
        } else {
            timeString = curTime + " " + NmsDateTool.getDispTimeStr(lateMillis);
        }

        return timeString;
    }

    public static String getAllLocationsFormatTime(Context context, long when) {
        String ret = "";
        long curr = System.currentTimeMillis();

        Time now = new Time();
        now.set(curr);
        int nowYear = now.year;
        int nowMonth = now.month + 1;
        int nowMonthDay = now.monthDay;

        Time then = new Time();
        then.set(when);
        int thenYear = then.year;
        int thenMonth = then.month + 1;
        int thenMonthDay = then.monthDay;

        if (nowYear == thenYear && nowMonth == thenMonth && nowMonthDay == thenMonthDay) {
            ret = DateFormat.getTimeFormat(context).format(new Date(when));
        } else {
            String fm = Settings.System.getString(context.getContentResolver(),
                    Settings.System.DATE_FORMAT);
            if (TextUtils.isEmpty(fm)) {
                fm = "yyyy/MM/dd";
            }

            SimpleDateFormat sdf = new SimpleDateFormat(fm);
            ret = sdf.format(when);
        }

        return ret;
    }
}
