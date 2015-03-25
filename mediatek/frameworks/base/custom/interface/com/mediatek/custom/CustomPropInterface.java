package com.mediatek.custom;

import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.Date;
import java.util.Calendar;
import java.util.Locale;
import java.text.SimpleDateFormat;
import java.text.ParsePosition;
import android.util.Log;
import android.content.res.Resources;


public class CustomPropInterface {
    private static String getBrowserVersion() {
        int resID = com.android.internal.R.string.web_user_agent;
        String result = null;

        try {
            String defaultAgent = Resources.getSystem().getText(resID).toString();
            Pattern pattern = Pattern.compile("\\s+AppleWebKit\\/(\\d+\\.?\\d*)\\s+");
            Matcher matcher = pattern.matcher(defaultAgent);

            if (matcher.find()) {
                Log.i("CustomProp", "getBrowserVersion->matcher.find:true" + " matcher.group(0):" + matcher.group(0) + " matcher.group(1):" + matcher.group(1)); 
                result = matcher.group(1);
            }
            else {
                Log.i("CustomProp", "getBrowserVersion->matcher.find:false"); 
            }
        } catch (RuntimeException e) {
        }

        Log.i("CustomProp", "getBrowserVersion->result:" + result);
        
        return result;
    }

    private static String getReleaseDate(String buildDate) {
        Log.i("CustomProp", "getReleaseDate->buildDate[" + buildDate + "]");
        SimpleDateFormat format = new SimpleDateFormat("EEE MMM dd HH:mm:ss zzz yyyy", new Locale("en","CN"));
        Date date;
        String result;

        try {
            date = format.parse(buildDate);
        } catch (Exception e) {
            date = null;
        }

        if (date != null) {
            Log.i("CustomProp", "date: " + date);
            Calendar calendar = format.getCalendar();
            result = String.format("%02d.%02d.%d", calendar.get(Calendar.MONTH)+1, calendar.get(Calendar.DATE), calendar.get(Calendar.YEAR));
        }
        else
            result = null;

        return result;
    }
}

