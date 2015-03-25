package com.mediatek.filemanager.utils;

import android.text.TextUtils;
import android.view.Gravity;
import android.widget.TextView;

public class LongStringUtils {
    private static final String TAG = LongStringUtils.class.getSimpleName();

    /**
     * 
     * @param textView
     *            The view to be set adjust with the long string.
     */
    public static void fadeOutLongString(TextView textView) {
        if (textView == null) {
            LogUtils.w(TAG, "#adjustWithLongString(),the view is to be set is null");
            return;
        }
        if (!(textView instanceof TextView)) {
            LogUtils.w(TAG, "#adjustWithLongString(),the view instance is not right,execute failed!");
            return;
        }

        textView.setHorizontalFadingEdgeEnabled(true);
        textView.setSingleLine(true);
        textView.setGravity(Gravity.LEFT);
        textView.setGravity(Gravity.CENTER_VERTICAL);
    }
}
