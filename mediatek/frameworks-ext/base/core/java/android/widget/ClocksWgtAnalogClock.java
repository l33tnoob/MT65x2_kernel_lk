/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package android.widget;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.text.format.DateFormat;
import android.text.format.Time;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.view.ViewGroup.LayoutParams;
import android.widget.RemoteViews.RemoteView;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

@RemoteView
public class ClocksWgtAnalogClock extends View implements ScreenStateChangeListener {

    private Drawable mHourHand;

    private Drawable mMinuteHand;

    private Drawable mDial;

    private Drawable mHat;

    private int mDialWidth;

    private int mDialHeight;

    private boolean mAttached;

    private final Handler mHandler = new Handler();

    private float mMinutes;

    private float mHour;

    private boolean mChanged;

    private String mTimeZone;

    private Drawable mDayDial;

    private Drawable mNightDial;

    private Drawable mDayHourHand;

    private Drawable mNightHourHand;

    private Drawable mDayMinuteHand;

    private Drawable mNightMinuteHand;

    private Drawable mDayHat;

    private Drawable mNightHat;

    private static final String WEEK_FORMAT = "E";

    private static final String TAG = "ClocksWgtAnalogClock";

    private float mDateFontSize;

    private String mDateFormatString;

    private boolean mIsScreenEnter = true;

    private Runnable mTicker;

    public ClocksWgtAnalogClock(Context context) {
        this(context, null);
    }

    public ClocksWgtAnalogClock(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ClocksWgtAnalogClock(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

       // Util.check();

        Resources r = mContext.getResources();
        TypedArray a = context.obtainStyledAttributes(attrs,
                com.android.internal.R.styleable.AnalogClock, defStyle, 0);

        mDial = a.getDrawable(com.android.internal.R.styleable.AnalogClock_dial);
        if (mDial == null) {
            mDial = r.getDrawable(com.android.internal.R.drawable.clock_dial);
        }

        mHourHand = a.getDrawable(com.android.internal.R.styleable.AnalogClock_hand_hour);
        if (mHourHand == null) {
            mHourHand = r.getDrawable(com.android.internal.R.drawable.clock_hand_hour);
        }

        mMinuteHand = a.getDrawable(com.android.internal.R.styleable.AnalogClock_hand_minute);
        if (mMinuteHand == null) {
            mMinuteHand = r.getDrawable(com.android.internal.R.drawable.clock_hand_minute);
        }
        mDialWidth = mDial.getIntrinsicWidth();
        mDialHeight = mDial.getIntrinsicHeight();
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        if (!mAttached) {
            mAttached = true;
            IntentFilter filter = new IntentFilter();

            filter.addAction(Intent.ACTION_TIME_TICK);
            filter.addAction(Intent.ACTION_TIME_CHANGED);
            filter.addAction(Intent.ACTION_TIMEZONE_CHANGED);
            filter.addAction(Intent.ACTION_SCREEN_ON);

            getContext().registerReceiver(mIntentReceiver, filter, null, mHandler);
        }
        onTimeChanged();
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        if (mAttached) {
            getContext().unregisterReceiver(mIntentReceiver);
            mAttached = false;
        }
        Utils.onDetachedFromWindowClearUp(this);
    }

    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {

        int widthMode = MeasureSpec.getMode(widthMeasureSpec);
        int widthSize = MeasureSpec.getSize(widthMeasureSpec);
        int heightMode = MeasureSpec.getMode(heightMeasureSpec);
        int heightSize = MeasureSpec.getSize(heightMeasureSpec);

        float hScale = 1.0f;
        float vScale = 1.0f;

        if (widthMode != MeasureSpec.UNSPECIFIED && widthSize < mDialWidth) {
            hScale = (float) widthSize / (float) mDialWidth;
        }

        if (heightMode != MeasureSpec.UNSPECIFIED && heightSize < mDialHeight) {
            vScale = (float) heightSize / (float) mDialHeight;
        }

        float scale = Math.min(hScale, vScale);

        setMeasuredDimension(resolveSize((int) (mDialWidth * scale), widthMeasureSpec),
                resolveSize((int) (mDialHeight * scale), heightMeasureSpec));
    }

    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        mChanged = true;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        boolean changed = mChanged;
        if (changed) {
            mChanged = false;
        }

        int availableWidth = mRight - mLeft;
        int availableHeight = mBottom - mTop;

        int x = availableWidth / 2;
        int y = availableHeight / 2;

        final Drawable dial = mDial;
        final Drawable hat = mHat;
        int w = dial.getIntrinsicWidth();
        int h = dial.getIntrinsicHeight();
        int wHat = 0;
        int hHat = 0;
        if (null != hat) {
            wHat = hat.getIntrinsicWidth();
            hHat = hat.getIntrinsicHeight();
        }

        boolean scaled = false;

        if (availableWidth < w || availableHeight < h) {
            scaled = true;
            float scale = Math.min((float) availableWidth / (float) w, (float) availableHeight
                    / (float) h);
            canvas.save();
            canvas.scale(scale, scale, x, y);
        }

        if (changed) {
            dial.setBounds(x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2));
        }
        dial.draw(canvas);

        drawDate(canvas, x, y);
        if (null != mDayHat) {
            drawWeekDay(canvas, x, y);
        }

        canvas.save();
        canvas.rotate(mHour / 12.0f * 360.0f, x, y);

        final Drawable hourHand = mHourHand;
        if (changed) {
            w = hourHand.getIntrinsicWidth();
            h = hourHand.getIntrinsicHeight();
            hourHand.setBounds(x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2));
        }
        hourHand.draw(canvas);
        hourHand.setAlpha(225);
        canvas.restore();

        canvas.save();
        canvas.rotate(mMinutes / 60.0f * 360.0f, x, y);

        final Drawable minuteHand = mMinuteHand;
        if (changed) {
            w = minuteHand.getIntrinsicWidth();
            h = minuteHand.getIntrinsicHeight();
            minuteHand.setBounds(x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2));
        }
        minuteHand.draw(canvas);
        minuteHand.setAlpha(225);
        canvas.restore();

        if (scaled) {
            canvas.restore();
        }

        if (availableWidth < wHat || availableHeight < hHat) {
            scaled = true;
            float scale = Math.min((float) availableWidth / (float) wHat, (float) availableHeight
                    / (float) hHat);
            canvas.save();
            canvas.scale(scale, scale, x, y);
        }

        if (changed) {
            if (null != hat) {
                hat.setBounds(x - (wHat / 2), y - (hHat / 2), x + (wHat / 2), y + (hHat / 2));
            }

        }
        if (null != hat) {
            hat.draw(canvas);
        }

    }

    /**
     * Draw date string on dial
     * 
     * @param canvas the canvas.
     * @param halfWidth half of available width of the view.
     * @param HalfHeight half of available height of the view.
     */
    private void drawDate(Canvas canvas, int halfWidth, int HalfHeight) {
        Paint paint = new Paint();
        paint.setTextSize(mDateFontSize);
        paint.setAntiAlias(true);
        int hour = Integer.valueOf(getFormatStr("k"));
        if (hour > 5 && hour < 18) {
            paint.setColor(Color.BLACK);
        } else {
            paint.setColor(Color.WHITE);
        }
        Rect rect = new Rect();
        String string = null;
        string = this.getFormatStr(mDateFormatString);
        if (string != null) {
            paint.getTextBounds(string, 0, string.length(), rect);
            canvas.drawText(string, (float) (halfWidth - rect.width() / 2.0),
                    (float) (HalfHeight - rect.height() / 2.0) + (HalfHeight * 2.0f / 4.0f), paint);
        }
    }

    /**
     * Draw weekday string on dial
     * 
     * @param canvas the canvas.
     * @param halfWidth half of available width of the view.
     * @param HalfHeight half of available height of the view.
     */
    private void drawWeekDay(Canvas canvas, int halfWidth, int HalfHeight) {
        Paint paint = new Paint();
        paint.setTextSize(mDateFontSize);
        paint.setAntiAlias(true);
        int hour = Integer.valueOf(getFormatStr("k"));
        if (hour > 5 && hour < 18) {
            paint.setColor(Color.BLACK);
        } else {
            paint.setColor(Color.WHITE);
        }
        Rect rect = new Rect();
        String string = null;
        string = this.getFormatStr(WEEK_FORMAT);
        if (string != null) {
            paint.getTextBounds(string, 0, string.length(), rect);
            canvas.drawText(string,
                    (float) (halfWidth - rect.width() / 2.0f - halfWidth * 1.6f / 4.0f),
                    (float) (HalfHeight) + rect.height() / 2.0f, paint);
        }
    }

    /**
     * Get a time string according to the formatter and timezone.
     * 
     * @param format format string.
     * @return a timezone's time string when the mTimeZone is not null, else
     *         return current timezone's time string.
     */
    private String getFormatStr(String format) {
        if (mTimeZone != null) {
            if (format != null) {
                java.text.SimpleDateFormat sdf = new SimpleDateFormat(format);
                sdf.setTimeZone(TimeZone.getTimeZone(mTimeZone));
                return sdf.format(new Date());
            }
        } else {
            if (format != null) {
                java.text.SimpleDateFormat sdf = new SimpleDateFormat(format);
                return sdf.format(new Date());
            }
        }
        return null;
    }

    /**
     * When time is changed,we can call this method to refresh data.
     */
    private void onTimeChanged() {
        int hour = Integer.valueOf(getFormatStr("h"));
        int minute = Integer.valueOf(getFormatStr("m"));
        int second = Integer.valueOf(getFormatStr("s"));
        mMinutes = minute + second / 60.0f;
        mHour = hour + mMinutes / 60.0f;

        if (null == mDayHat) {
            mChanged = true;
            return;
        }

        int hour24 = Integer.valueOf(getFormatStr("k"));
        if (hour24 > 5 && hour24 < 18) {
            if (null != mDayDial) {
                mDial = mDayDial;
            }

            if (null != mDayHourHand) {
                mHourHand = mDayHourHand;
            }

            if (null != mDayMinuteHand) {
                mMinuteHand = mDayMinuteHand;
            }
            if (null != mDayHat) {
                mHat = mDayHat;
            }

        } else {
            if (null != mNightDial) {
                mDial = mNightDial;
            }

            if (null != mNightHourHand) {
                mHourHand = mNightHourHand;
            }

            if (null != mNightMinuteHand) {
                mMinuteHand = mNightMinuteHand;
            }

            if (null != mNightHat) {
                mHat = mNightHat;
            }

        }

        mChanged = true;
    }

    /**
     * A receiver to listening to the change of time and screen on,then
     * invalidate the view.
     */
    private final BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            if (!mIsScreenEnter) {
                return;
            }
            onTimeChanged();
            invalidate();
        }
    };

    @android.view.RemotableViewMethod
    public void setTimeZone(String zoneStr) {
        mTimeZone = zoneStr;
        onTimeChanged();
    }

    @android.view.RemotableViewMethod
    public void setDayDialResource(int resid) {
        Resources r = getResources();
        mDayDial = r.getDrawable(resid);
    }

    @android.view.RemotableViewMethod
    public void setNightDialResource(int resid) {
        Resources r = getResources();
        mNightDial = r.getDrawable(resid);
    }

    @android.view.RemotableViewMethod
    public void setDayHourResource(int resid) {
        Resources r = getResources();
        mDayHourHand = r.getDrawable(resid);

    }

    @android.view.RemotableViewMethod
    public void setNightHourResource(int resid) {
        Resources r = getResources();
        mNightHourHand = r.getDrawable(resid);

    }

    @android.view.RemotableViewMethod
    public void setDayMinuteResource(int resid) {
        Resources r = getResources();
        mDayMinuteHand = r.getDrawable(resid);
    }

    @android.view.RemotableViewMethod
    public void setNightMinuteResource(int resid) {
        Resources r = getResources();
        mNightMinuteHand = r.getDrawable(resid);
    }

    @android.view.RemotableViewMethod
    public void setDayHatResource(int resid) {
        Resources r = getResources();
        mDayHat = r.getDrawable(resid);
    }

    @android.view.RemotableViewMethod
    public void setNightHatResource(int resid) {
        Resources r = getResources();
        mNightHat = r.getDrawable(resid);
    }

    @android.view.RemotableViewMethod
    public void setDateFontSize(int i) {
        mDateFontSize = this.getResources().getDimension(i);
    }

    @android.view.RemotableViewMethod
    public void setDateHeight(int i) {
        // mDateHeight = this.getResources().getDimension(i);
    }

    @android.view.RemotableViewMethod
    public void setDateFormatString(int i) {
        mDateFormatString = getContext().getString(i);
    }

    @android.view.RemotableViewMethod
    public void setOnTimeChanged(int i) {
        onTimeChanged();
    }

    public void onScreenStateChanged(int screen, int state) {
        if (ScreenStateChangeListener.SCREEN_ENTER == state) {
            mIsScreenEnter = true;
            mTicker = new Runnable() {
                public void run() {
                    onTimeChanged();
                    invalidate();
                }
            };
            mHandler.post(mTicker);
        } else {
            mIsScreenEnter = false;
        }
    }
}
