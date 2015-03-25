
package com.mediatek.DataUsageLockScreenClient;

import com.mediatek.ngin3d.Color;

public class Constants {
    private Constants() {
        // do nothing
    }

    // SIM Card Number
    public static final int ONE_CARD = 1; /* Gemini support, but only insert 1 sim */
    public static final int TWO_CARD = 2; /* Gemini support, & insert 2 sim */

    // SIM Card type
    public static final int SINGLE_CARD = 0;
    public static final int GEMINI_CARD_ONE = 1;
    public static final int GEMINI_CARD_TWO = 2;
    public static final int NO_CARD = 3;

    // SIM Card color
    public static final int CIRCLE_BULE = 0;
    public static final int CIRCLE_ORANGE = 1;
    public static final int CIRCLE_GREEN = 2;
    public static final int CIRCLE_PURPLE = 3;

    // Show Data Usage Text
    public static final int TEXT_LEFT = 0;
    public static final int TEXT_RIGHT = 1;

    // circle color
    public static final int OUT_CIRCLE_COLOR_ALPHA_MORE = android.graphics.Color.argb(200, 0, 0, 0);
    public static final int OUT_CIRCLE_COLOR_ALPHA_NORMAL = android.graphics.Color.argb(100, 0, 0, 0);

    public static final int IN_CIRCLE_ORANGE = android.graphics.Color.argb(200, 255, 136, 0);
    public static final int IN_CIRCLE_BULE = android.graphics.Color.argb(200, 0, 191, 255);
    public static final int IN_CIRCLE_GREEN = android.graphics.Color.argb(200, 0, 255, 0);
    public static final int IN_CIRCLE_PURPLE = android.graphics.Color.argb(200, 160, 32, 240);

    public static final int KB_UNIT = 1024;
    public static final long MB_UNIT = KB_UNIT * KB_UNIT;
    public static final long GB_UNIT = KB_UNIT * KB_UNIT * KB_UNIT;

    public static final int IN_CIRCLE_RADIUS = 2;
    public static final int OUT_CIRCLE_RADIUS = 3;

    // show data usage word's size
    public static final float TEXT_SIZE_LANDSCAPE_SMALL = 10f; // for landscape
    public static final float TEXT_SIZE_SMALL = 17f; // Display Width < 480
    public static final float TEXT_SIZE_MIDDLE = 20f; //  480 < Display Width < 1080
    public static final float TEXT_SIZE_LARGE = 32f; // Display Width >= 1080

    public static final int HVGA_WIDTH = 480;
    public static final int XXHDPI_WIDTH = 1080;

}
