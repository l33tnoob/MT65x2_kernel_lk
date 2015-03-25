package com.mediatek.ppl.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.RelativeLayout;

/**
 * A subclass of RelativeLayout which applies different layout policy according to current height.
 */
public class SlidingPanelLayout extends RelativeLayout {
    private static final String TAG = "PPL/SlidingPanelLayout";

    private View mUpperPanel;
    private View mLowerPanel;
    private ILayoutPolicyChanged mListener;

    public SlidingPanelLayout(Context context) {
        super(context);
    }

    public SlidingPanelLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public SlidingPanelLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        Log.d(TAG,
                "Height Measure: " + getMeasureSpecMode(MeasureSpec.getMode(heightMeasureSpec)) + ", "
                        + MeasureSpec.getSize(heightMeasureSpec));
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        final int height = getMeasuredHeight();
        if (mUpperPanel != null && mLowerPanel != null) {
            final int upperHeight = mUpperPanel.getMeasuredHeight();
            final int lowerHeight = mLowerPanel.getMeasuredHeight();
            if (upperHeight + lowerHeight > height) {
                Log.d(TAG, "upper(" + upperHeight + ") + lower(" + lowerHeight + ") > previousTotal(" + height + ")");
                RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) mLowerPanel.getLayoutParams();
                lp.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM, RelativeLayout.TRUE);
                lp.removeRule(RelativeLayout.BELOW);
                mLowerPanel.setLayoutParams(lp);
                mLowerPanel.bringToFront();
                if (mListener != null) {
                    mListener.onLayoutPolicyChanged(
                            ILayoutPolicyChanged.LAYOUT_POLICY_FLOATING,
                            lowerHeight > height);
                }
            } else {
                Log.d(TAG, "upper(" + upperHeight + ") + lower(" + lowerHeight + ") <= previousTotal(" + height + ")");
                RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) mLowerPanel.getLayoutParams();
                lp.removeRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                lp.addRule(RelativeLayout.BELOW, mUpperPanel.getId());
                mLowerPanel.setLayoutParams(lp);
                if (mListener != null) {
                    mListener.onLayoutPolicyChanged(
                            ILayoutPolicyChanged.LAYOUT_POLICY_FOLLOW,
                            lowerHeight > height);
                }
            }
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        }
        Log.d(TAG, "Result Height: " + height);
    }

    /**
     * Register panels and listener to be notified if the layout policy is changed.
     * 
     * @param upper     The upper panel. It should be the child of this layout.
     * @param lower     The lower panel. It should be the child of this layout.
     * @param listener  The listener.
     */
    public void registerPanels(View upper, View lower, ILayoutPolicyChanged listener) {
        if (hasChild(upper) && hasChild(lower)) {
            mUpperPanel = upper;
            mLowerPanel = lower;
            mListener = listener;
        } else {
            throw new Error("upper and lower must both be the direct child of this layout");
        }
    }

    public static interface ILayoutPolicyChanged {
        /**
         * The lower panel will slide over the upper panel if there is no enough height for both of them.
         */
        int LAYOUT_POLICY_FLOATING = 0;
        /**
         * The lower panel will follow the upper panel immediately if there is enough height for both of them.
         */
        int LAYOUT_POLICY_FOLLOW = 1;

        /**
         * Invoked when the layout policy is changed.
         * 
         * @param policy    The new policy.
         */
        void onLayoutPolicyChanged(int policy, boolean lowerClipped);
    }

    private boolean hasChild(View view) {
        final int childCount = getChildCount();
        for (int i = 0; i < childCount; ++i) {
            if (getChildAt(i) == view) {
                return true;
            }
        }
        return false;
    }

    /*
     * Used for debugging.
     * @param mode
     * @return
     */
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
