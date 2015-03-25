/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.vlw;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.SeekBar;

import com.mediatek.xlog.Xlog;

public class VLWSeekBar extends SeekBar {
    private static final String TAG = "VLWSeekBar";
    
    private static final boolean DEBUG = false;
    
    private int mAlpha = 255;
    private int mLeftThreshold = 0;
    private int mRightThreshold = 1000;

    private OnTouchUpWithoutHandledListener mListener;
    // Indicate whether this progress change caused by key event.
    private boolean mFromKey = false;
    
    public VLWSeekBar(Context context) {
        super(context);
    }

    public VLWSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public VLWSeekBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    /* (non-Javadoc)
     * @see android.widget.AbsSeekBar#drawableStateChanged()
     */
    @Override
    protected void drawableStateChanged() {
        super.drawableStateChanged();
        Drawable progressDrawable = getProgressDrawable();
        if (progressDrawable != null) {
            progressDrawable.setAlpha(mAlpha);
        }
    }

    /**
     * To hide or show the progress bar of this VLWSeekbar.
     * @param alpha the mAlpha to set
     */
    public void setAlpha(int alpha) {
        this.mAlpha = alpha;
    }

    /**
     * (non-javadoc)
     * @return the mLeftThreshold
     */
    public int getLeftThreshold() {
        return mLeftThreshold;
    }

    /**
     * (non-javadoc)
     * @param leftThreshold the mLeftThreshold to set
     */
    public void setLeftThreshold(int leftThreshold) {
        this.mLeftThreshold = leftThreshold;
    }

    /**
     * (non-javadoc)
     * @return the mRightThreshold
     */
    public int getRightThreshold() {
        return mRightThreshold;
    }

    /**
     * (non-javadoc)
     * @param rightThreshold the mRightThreshold to set
     */
    public void setRightThreshold(int rightThreshold) {
        this.mRightThreshold = rightThreshold;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        final int width = getWidth();
        final int available = width - mPaddingLeft - mPaddingRight;
        int x = (int) event.getX();
        // /Support RTL, convert the coordinate of touch point for RTL @{
        int direction = getLayoutDirection();
        if (direction == View.LAYOUT_DIRECTION_RTL) {
            x = getWidth() - x;
        }
        // /@}
        float scale = 0.0f;
        float progress = 0.0f;
        if (x < mPaddingLeft) {
            scale = 0.0f;
        } else if (x > width - mPaddingRight) {
            scale = 1.0f;
        } else {
            scale = (float) (x - mPaddingLeft) / (float) available;
        }
        progress = scale * getMax();
        final int action = event.getAction();
        if (action == MotionEvent.ACTION_CANCEL) {
            Xlog.d(TAG, "MotionEvent.ACTION_CANCEL just return");
            return true;
        }
        if (progress >= mLeftThreshold && progress <= mRightThreshold) {
            return super.onTouchEvent(event);
        } else {
            if (action == MotionEvent.ACTION_UP) {
                setPressed(false);
                invalidate();
                if (mListener != null) {
                    mListener.onTouchUp(progress);
                }
            }
            return true;
        }
    }   
    
    /* (non-Javadoc)
     * @see android.widget.AbsSeekBar#onKeyDown(int, android.view.KeyEvent)
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        boolean handled = false;
        if (isEnabled()) {
            int progress = getProgress();
            int increment = getKeyProgressIncrement();
            if (DEBUG) {
                Xlog.d(TAG, "onKeyDown, progress: " + progress + ", inc: "
                        + increment);
            }

            switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_LEFT:
                progress -= increment;
                if (DEBUG) {
                    Xlog.d(TAG, "KEYCODE_DPAD_LEFT, progress: " + progress
                            + ", lt: " + mLeftThreshold + ", rt: "
                            + mRightThreshold);
                }
                if (progress >= mLeftThreshold && progress <= mRightThreshold) {
                    mFromKey = true;
                    handled = super.onKeyDown(keyCode, event);
                }
                break;

            case KeyEvent.KEYCODE_DPAD_RIGHT:
                progress += increment;
                if (DEBUG) {
                    Xlog.d(TAG, "KEYCODE_DPAD_RIGHT, progress: " + progress
                            + ", lt: " + mLeftThreshold + ", rt: "
                            + mRightThreshold);
                }
                if (progress >= mLeftThreshold && progress <= mRightThreshold) {
                    mFromKey = true;
                    handled = super.onKeyDown(keyCode, event);
                }
                break;

            default:
                handled = super.onKeyDown(keyCode, event);
                break;
            }
        }

        return handled;
    }

    /**
     * Get the value of mFromKey.
     * 
     * @return
     */
    public boolean fromKeyEvent() {
        return mFromKey;
    }
    
    /**
     * Reset the mFromKey variable.
     */
    public void resetFromKeyEvent() {
        mFromKey = false;
    }

    /**
     * Listener called when a touch up is not handled by the seek bar.
     */
    public interface OnTouchUpWithoutHandledListener {
        void onTouchUp(float progress);
    }

    /**
     * Get the listener.
     * 
     * @return
     */
    public OnTouchUpWithoutHandledListener getOnTouchUpWithoutHandledListener() {
        return mListener;
    }

    /**
     * Set the listener.
     * 
     * @param listener listener to be set.
     */
    public void setOnTouchUpWithoutHandledListener(
            OnTouchUpWithoutHandledListener listener) {
        mListener = listener;
    }

}
