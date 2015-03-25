/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.activities.widgets;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;

/**
 * The time bar view, which includes the current and total time, the progress
 * bar, and the scrubber.
 */
public class TimeBar extends View {

    public interface Listener {
        void onScrubbingStart();

        void onScrubbingMove(int time);

        void onScrubbingEnd(int time);
    }

    // Padding around the scrubber to increase its touch target
    private static final int SCRUBBER_PADDING_IN_DP = 10;

    // The total padding, top plus bottom
    private static final int V_PADDING_IN_DP = 30;

    private static final int TEXT_SIZE_IN_DP = 14;

    private final Listener mListener;

    // the bars we use for displaying the progress
    private final Rect mProgressBar;
    private final Rect mPlayedBar;

    private final Paint mProgressPaint;
    private final Paint mPlayedPaint;
    private final Paint mTimeTextPaint;

    private final Bitmap mScrubber;
    private final int mScrubberPadding; // adds some touch tolerance around the
                                       // scrubber

    private int mScrubberLeft;
    private int mScrubberTop;
    private int mScrubberCorrection;
    private boolean mScrubbing;
    private boolean mShowTimes;
    private boolean mShowScrubber;

    private int mTotalTime;
    private int mCurrentTime;

    private final Rect mTimeBounds;

    private int mPaddingInPx;

    public TimeBar(Context context, Listener listener) {
        super(context);
        this.mListener = checkNotNull(listener);

        mShowTimes = true;
        mShowScrubber = true;

        mProgressBar = new Rect();
        mPlayedBar = new Rect();
        mSecondaryBar = new Rect();

        mProgressPaint = new Paint();
        mProgressPaint.setColor(0xFF808080);
        mPlayedPaint = new Paint();
        mPlayedPaint.setColor(0xFFFFFFFF);
        mSecondaryPaint = new Paint();
        mSecondaryPaint.setColor(0xFF5CA0C5);

        DisplayMetrics metrics = context.getResources().getDisplayMetrics();
        float textSizeInPx = metrics.density * TEXT_SIZE_IN_DP;
        mTimeTextPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mTimeTextPaint.setColor(0xFFCECECE);
        mTimeTextPaint.setTextSize(textSizeInPx);
        mTimeTextPaint.setTextAlign(Paint.Align.CENTER);

        mTimeBounds = new Rect();
        // timeTextPaint.getTextBounds("0:00:00", 0, 7, timeBounds);

        mInfoPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mInfoPaint.setColor(0xFFCECECE);
        mInfoPaint.setTextSize(textSizeInPx);
        mInfoPaint.setTextAlign(Paint.Align.CENTER);

        mEllipseLength = (int) Math.ceil(mInfoPaint.measureText(ELLIPSE));

        mScrubber = BitmapFactory.decodeResource(getResources(), R.drawable.scrubber_knob);
        mScrubberPadding = (int) (metrics.density * SCRUBBER_PADDING_IN_DP);

        mPaddingInPx = (int) (metrics.density * V_PADDING_IN_DP);
        mTextPadding = mScrubberPadding / 2;
    }

    private void update() {
        Logger.d(TAG, "update(), totalTime = " + mTotalTime + " currentTime = " + mCurrentTime);
        mPlayedBar.set(mProgressBar);

        if (mTotalTime > 0) {
            mPlayedBar.right =
                    mPlayedBar.left + (int) ((mProgressBar.width() * (long) mCurrentTime) / mTotalTime);
            // if duration is not accurate, here just adjust playedBar
            // we also show the accurate position text to final user.
            if (mPlayedBar.right > mProgressBar.right) {
                mPlayedBar.right = mProgressBar.right;
            }
        } else {
            mPlayedBar.right = mProgressBar.left;
        }

        if (!mScrubbing) {
            mScrubberLeft = mPlayedBar.right - mScrubber.getWidth() / 2;
        }
        // update text bounds when layout changed or time changed
        updateBounds();
        updateVisibleText();
        invalidate();
    }

    /**
     * @return the preferred height of this view, including invisible padding
     */
    public int getPreferredHeight() {
        // double height for time bar
        return mTimeBounds.height() * 2 + mPaddingInPx + mScrubberPadding + mTextPadding;
    }

    /**
     * @return the height of the time bar, excluding invisible padding
     */
    public int getBarHeight() {
        // double height for time bar
        return mTimeBounds.height() * 2 + mPaddingInPx + mTextPadding;
    }

    /**
     * Set total time and current time
     * 
     * @param int current time
     * @param int total time
     */
    public void setTime(int currentTime, int totalTime) {
        Logger.v(TAG, "setTime(" + currentTime + ", " + totalTime + ")");
        mOriginalTotalTime = totalTime;
        if (this.mCurrentTime == currentTime && this.mTotalTime == totalTime) {
            return;
        }
        this.mCurrentTime = currentTime;
        this.mTotalTime = Math.abs(totalTime);
        update();
    }

    /**
     * Set show time
     * 
     * @param int show time
     */
    public void setShowTimes(boolean showTimes) {
        this.mShowTimes = showTimes;
        requestLayout();
    }

    /**
     * Reset time
     */
    public void resetTime() {
        setTime(0, 0);
    }

    /**
     * Set show Scrubber
     * 
     * @param boolean show
     */
    public void setShowScrubber(boolean showScrubber) {
        Logger.v(TAG, "setShowScrubber(" + showScrubber + ") showScrubber=" + showScrubber);
        this.mShowScrubber = showScrubber;
        if (!showScrubber && mScrubbing) {
            mListener.onScrubbingEnd(getScrubberTime());
            mScrubbing = false;
        }
        requestLayout();
    }

    private boolean inScrubber(float x, float y) {
        int scrubberRight = mScrubberLeft + mScrubber.getWidth();
        int scrubberBottom = mScrubberTop + mScrubber.getHeight();
        return mScrubberLeft - mScrubberPadding < x && x < scrubberRight + mScrubberPadding
                && mScrubberTop - mScrubberPadding < y && y < scrubberBottom + mScrubberPadding;
    }

    private void clampScrubber() {
        int half = mScrubber.getWidth() / 2;
        int max = mProgressBar.right - half;
        int min = mProgressBar.left - half;
        mScrubberLeft = Math.min(max, Math.max(min, mScrubberLeft));
    }

    private int getScrubberTime() {
        return (int) ((long) (mScrubberLeft + mScrubber.getWidth() / 2 - mProgressBar.left)
                * mTotalTime / mProgressBar.width());
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        int textH = mTimeBounds.height() + mTextPadding;
        int w = r - l;
        int h = b - t;

        if (!mShowTimes && !mShowScrubber) {
            mProgressBar.set(0, 0, w, h);
        } else {
            int margin = mScrubber.getWidth() / 3;
            /**
             * mark for show time upon the progress bar if (showTimes) { margin
             * += timeBounds.width(); }
             */
            // int progressY = textH + (h + scrubberPadding - textH) / 2;
            int progressY = textH + 50;
            mScrubberTop = progressY - mScrubber.getHeight() / 2 + 1;
            mProgressBar.set(getPaddingLeft() + margin, progressY, w - getPaddingRight() - margin,
                    progressY + 4);
        }
        update();
    }

    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);

        // draw progress bars
        canvas.drawRect(mProgressBar, mProgressPaint);
        canvas.drawRect(mPlayedBar, mPlayedPaint);
        if (mBufferPercent >= 0) {
            canvas.drawRect(mSecondaryBar, mSecondaryPaint);
            Logger.v(TAG, "draw() bufferPercent=" + mBufferPercent + ", secondaryBar="
                    + mSecondaryBar);
        }

        // draw scrubber and timers
        if (mShowScrubber) {
            canvas.drawBitmap(mScrubber, mScrubberLeft, mScrubberTop, null);
        }
        if (mShowTimes) {
            canvas.drawText(stringForTime(mCurrentTime), mTimeBounds.width() / 2 + getPaddingLeft(),
                    mTimeBounds.height() + mScrubberPadding + 1 + mTextPadding, mTimeTextPaint);
            canvas.drawText(stringForTime(mTotalTime), getWidth() - getPaddingRight()
                    - mTimeBounds.width() / 2, mTimeBounds.height() + mScrubberPadding + 1
                    + mTextPadding, mTimeTextPaint);
        }
        if (mInfoText != null && mVisibleText != null) {
            canvas.drawText(mVisibleText, getPaddingLeft()
                    + (getWidth() - getPaddingLeft() - getPaddingRight()) / 2, mTimeBounds.height()
                    + mScrubberPadding + 1 + mTextPadding, mInfoPaint);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        Logger.v(TAG, "onTouchEvent() showScrubber=" + mShowScrubber + ", enableScrubbing="
                + mEnableScrubbing + ", totalTime=" + mTotalTime);
        if (mShowScrubber && mEnableScrubbing) {
            int x = (int) event.getX();
            int y = (int) event.getY();

            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    if (inScrubber(x, y)) {
                        if (mOriginalTotalTime > 0) {
                            mScrubbing = true;
                            mScrubberCorrection = x - mScrubberLeft;
                            mListener.onScrubbingStart();
                        }
                        return true;
                    }
                    break;
                case MotionEvent.ACTION_MOVE:
                    if (mScrubbing) {
                        mScrubberLeft = x - mScrubberCorrection;
                        clampScrubber();
                        mCurrentTime = getScrubberTime();
                        mListener.onScrubbingMove(mCurrentTime);
                        invalidate();
                        return true;
                    }
                    break;
                case MotionEvent.ACTION_UP:
                    if (mScrubbing) {
                        mListener.onScrubbingEnd(getScrubberTime());
                        mScrubbing = false;
                        return true;
                    }
                    break;
                 default:
                    break;
            }
        }
        return false;
    }

    private String stringForTime(long millis) {
        int totalSeconds = (int) millis / 1000;
        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours = totalSeconds / 3600;
        if (hours > 0) {
            return String.format("%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return String.format("%02d:%02d", minutes, seconds).toString();
        }
    }

    private static final String TAG = "Gallery3D/TimeBar";

    public static final int UNKNOWN = -1;
    private static final String ELLIPSE = "...";

    private final Rect mSecondaryBar;
    private final Paint mSecondaryPaint;
    private final Paint mInfoPaint;

    private int mBufferPercent = UNKNOWN;
    private int mLastShowTime = UNKNOWN;
    private String mInfoText;
    private String mVisibleText;
    private boolean mEnableScrubbing;
    private final int mEllipseLength;
    private final int mTextPadding;

    /**
     * Set show setScrubbing enable
     * 
     * @param boolean enable
     */
    public void setScrubbing(boolean enable) {
        Logger.v(TAG, "setScrubbing(" + enable + ")");
        mEnableScrubbing = enable;
    }

    /**
     * Set show video info
     * 
     * @param String video info
     */
    public void setInfo(String info) {
        Logger.v(TAG, "setInfo(" + info + ")");
        mInfoText = info;
        invalidate();
    }

    /**
     * Set Buffer Percent
     * 
     * @param Buffer Percent
     */
    public void setBufferPercent(int percent) {
        Logger.v(TAG, "setBufferPercent(" + percent + ")");
        // enable buffer progress bar
        mBufferPercent = percent;
        if (mBufferPercent >= 0) {
            mSecondaryBar.set(mProgressBar);
            mSecondaryBar.right =
                mSecondaryBar.left + (int) (mBufferPercent * mProgressBar.width() / 100);
        } else {
            mSecondaryBar.right = mSecondaryBar.left;
        }
        invalidate();
    }

    private void updateBounds() {
        int showTime = mTotalTime > mCurrentTime ? mTotalTime : mCurrentTime;
        if (mLastShowTime == showTime) {
            // do not need to recompute the bounds.
            return;
        }
        String durationText = stringForTime(showTime);
        int length = durationText.length();
        mTimeTextPaint.getTextBounds(durationText, 0, length, mTimeBounds);
        mLastShowTime = showTime;
        Logger.v(TAG, "updateBounds() durationText=" + durationText + ", timeBounds=" + mTimeBounds);
    }

    private void updateVisibleText() {
        if (mInfoText == null) {
            mVisibleText = null;
            return;
        }
        float tw = mInfoPaint.measureText(mInfoText);
        float space =
                mProgressBar.width() - mTimeBounds.width() * 2 - getPaddingLeft() - getPaddingRight();
        if (tw > 0 && space > 0 && tw > space) {
            // we need to cut the info text for visible
            float originalNum = mInfoText.length();
            int realNum = (int) ((space - mEllipseLength) * originalNum / tw);
            Logger.v(TAG, "updateVisibleText() infoText=" + mInfoText + " text width=" + tw
                    + ", space=" + space + ", originalNum=" + originalNum + ", realNum=" + realNum
                    + ", getPaddingLeft()=" + getPaddingLeft() + ", getPaddingRight()="
                    + getPaddingRight() + ", progressBar=" + mProgressBar + ", timeBounds="
                    + mTimeBounds);
            mVisibleText = mInfoText.substring(0, realNum) + ELLIPSE;
        } else {
            mVisibleText = mInfoText;
        }
        Logger.v(TAG, "updateVisibleText() infoText=" + mInfoText + ", visibleText=" + mVisibleText
                + ", text width=" + tw + ", space=" + space);
    }

    // for duration displayed
    private int mOriginalTotalTime;

    /**
     * Check null
     * 
     * @param Object need checked object
     */
    public static <T> T checkNotNull(T object) {
        if (object == null) {
            throw new NullPointerException();
        }
        return object;
    }
}
