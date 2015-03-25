package com.mediatek.view.animation;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.animation.Interpolator;


/*
 * @hide
 */
public class DecelerateInterpolatorEx implements Interpolator {
    public DecelerateInterpolatorEx() {
    }

    public DecelerateInterpolatorEx(float factor) {
        mFactor = factor;
    }

    public DecelerateInterpolatorEx(float factor, float scale) {
        mFactor = factor;
        mScale = scale;
    }

    public DecelerateInterpolatorEx(Context context, AttributeSet attrs) {
        TypedArray a = context.obtainStyledAttributes(
                attrs,
                com.mediatek.internal.R.styleable.DecelerateInterpolatorEx);

        mFactor = a.getFloat(com.mediatek.internal.R.styleable.DecelerateInterpolatorEx_factor, 1.0f);
        mScale = a.getFloat(com.mediatek.internal.R.styleable.DecelerateInterpolatorEx_scale, 1.0f);

        a.recycle();
    }

    public float getInterpolation(float input) {
        final boolean needScale = (mScale != 1.0f && mScale != 0.0f);
        if (needScale) {
            input = input * mScale;
        }

        float result;
        if (mFactor == 1.0f) {
            result = (float)(1.0f - (1.0f - input) * (1.0f - input));
        } else {
            result = (float)(1.0f - Math.pow((1.0f - input), 2 * mFactor));
        }

        if (needScale && input >= 1.0f) {
            result = 1.0f;
        }

        return result;
    }

    private float mFactor = 1.0f;
    private float mScale = 1.0f;
}
