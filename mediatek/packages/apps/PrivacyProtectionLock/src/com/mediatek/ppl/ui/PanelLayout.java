package com.mediatek.ppl.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.LinearLayout;

/**
 * A specialized LinearLayout which always ask for the measures under UNSPECIFIED restrictions.
 */
public class PanelLayout extends LinearLayout {
    private static final String TAG = "PPL/PanelLayout";

    public PanelLayout(Context context) {
        super(context);
    }

    public PanelLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public PanelLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        Log.d(TAG,
                "Height Measure: " + getMeasureSpecMode(MeasureSpec.getMode(heightMeasureSpec)) + ", "
                        + MeasureSpec.getSize(heightMeasureSpec));
        int newHeightSpec = MeasureSpec.makeMeasureSpec(MeasureSpec.getSize(heightMeasureSpec), MeasureSpec.UNSPECIFIED);
        super.onMeasure(widthMeasureSpec, newHeightSpec);
        Log.d(TAG, "Result Height: " + getMeasuredHeight());
    }

    private String getMeasureSpecMode(int mode) {
        switch (mode) {
        case MeasureSpec.AT_MOST:
            return "AT_MOST";
        case MeasureSpec.EXACTLY:
            return "EXACTLY";
        case MeasureSpec.UNSPECIFIED:
            return "UNSPECIFIED";
        default:
            return "<error>";
        }
    }
}
