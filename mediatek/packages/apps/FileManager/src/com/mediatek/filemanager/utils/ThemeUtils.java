package com.mediatek.filemanager.utils;

import android.content.Context;


public class ThemeUtils {

    private static final int THEME_COLOR_DEFAULT = 0x7F33b5e5;
    //private static final int COLOR_ALPHA = 0x7FFFFFFF;

    public static final int getThemeColor(Context context) {
        int themeColor = 0;
        /*
        if (OptionsUtils.isMtkThemeSupported()) {
            themeColor = context.getResources().getThemeMainColor();
            themeColor = themeColor & COLOR_ALPHA;
        }
        */

        if (themeColor == 0) {
            themeColor = THEME_COLOR_DEFAULT;
        }

        return themeColor;
    }
}
