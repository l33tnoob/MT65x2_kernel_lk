/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.touchscreen;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.FontMetricsInt;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.ArrayList;

/**
 * Demonstrates wrapping a layout in a ScrollView.
 */

public class TsHandWriting extends Activity {

    public static final int CLEAR_CANVAS_ID = Menu.FIRST;
    MyView mView = null;

    private int mZoom = 1;
    private static final String TAG = "EM/TouchScreen/HW";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // setContentView(new MyView(this));

        Xlog.v(TAG, "onCreate start");
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        mView = new MyView(this);
        setContentView(mView);
        Xlog.v(TAG, "onCreate success");

    }

    @Override
    public void onResume() {
//        Xlog.v(TAG, "-->onResume");
        super.onResume();
        final SharedPreferences preferences = this.getSharedPreferences(
                "touch_screen_settings", android.content.Context.MODE_PRIVATE);
        String file = preferences.getString("filename", "N");
        if (!"N".equals(file)) {
//            if (!file.equals("N")) {
            String[] cmd = { "/system/bin/sh", "-c",
                    "echo [ENTER_HAND_WRITING] >> " + file }; // file

            int ret;
            try {
                ret = TouchScreenShellExe.execCommand(cmd);
                if (0 == ret) {
                    Toast.makeText(this, "Start logging...", Toast.LENGTH_LONG)
                            .show();
                } else {
                    Toast.makeText(this, "Logging failed!", Toast.LENGTH_LONG)
                            .show();
                }
            } catch (IOException e) {
                Xlog.w(TAG, e.toString());
            }

        }

    }

    @Override
    public void onPause() {

        Xlog.v(TAG, "-->onPause");
        final SharedPreferences preferences = this.getSharedPreferences(
                "touch_screen_settings", android.content.Context.MODE_PRIVATE);
        String file = preferences.getString("filename", "N");
        if (!"N".equals(file)) {
//        if (!file.equals("N")) {
            String[] cmd = { "/system/bin/sh", "-c",
                    "echo [LEAVE_HAND_WRITING] >> " + file }; // file

            int ret;
            try {
                ret = TouchScreenShellExe.execCommand(cmd);
                if (0 == ret) {
                    Toast.makeText(this, "Stop logging...", Toast.LENGTH_LONG)
                            .show();
                } else {
                    Toast.makeText(this, "Logging failed!", Toast.LENGTH_LONG)
                            .show();
                }
            } catch (IOException e) {
                Xlog.w(TAG, e.toString());
            }

        }
        super.onPause();

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, CLEAR_CANVAS_ID, 0, "Clean Table.");
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem mi) {
        if (CLEAR_CANVAS_ID == mi.getItemId()) {
            mView.clear();
        }
        
//        switch (mi.getItemId()) {
//        case CLEAR_CANVAS_ID:
//            mView.clear();
//            break;
//        default:
//            break;
//        }
        return super.onOptionsItemSelected(mi);
    }

    public class PT {
        public Float mX;
        public Float mY;

        public PT(Float x, Float y) {
            this.mX = x;
            this.mY = y;
        }
    };

    public class MyView extends View {
        private final Paint mTextPaint;
        private final Paint mTextBackgroundPaint;
        private final Paint mTextLevelPaint;
        private final Paint mPaint;
        private final Paint mTargetPaint;
        private final FontMetricsInt mTextMetrics = new FontMetricsInt();
        public ArrayList<ArrayList<PT>> mLines = new ArrayList<ArrayList<PT>>();
        ArrayList<PT> mCurLine;
        public ArrayList<VelocityTracker> mVelocityList = new ArrayList<VelocityTracker>();
        private int mHeaderBottom;
        private boolean mCurDown;
        private int mCurX;
        private int mCurY;
        private float mCurPressure;
        private int mCurWidth;
        private VelocityTracker mVelocity;

        public MyView(Context c) {
            super(c);

            DisplayMetrics dm = new DisplayMetrics();
            dm = TsHandWriting.this.getApplicationContext().getResources()
                    .getDisplayMetrics();
            int screenWidth = dm.widthPixels;
            int screenHeight = dm.heightPixels;
            if ((480 == screenWidth && 800 == screenHeight)
                    || (800 == screenWidth && 480 == screenHeight)) {
                mZoom = 2;
            }

            mTextPaint = new Paint();
            mTextPaint.setAntiAlias(true);
            mTextPaint.setTextSize(10 * mZoom);
            mTextPaint.setARGB(255, 0, 0, 0);
            mTextBackgroundPaint = new Paint();
            mTextBackgroundPaint.setAntiAlias(false);
            mTextBackgroundPaint.setARGB(128, 255, 255, 255);
            mTextLevelPaint = new Paint();
            mTextLevelPaint.setAntiAlias(false);
            mTextLevelPaint.setARGB(192, 255, 0, 0);
            mPaint = new Paint();
            mPaint.setAntiAlias(true);
            mPaint.setARGB(255, 255, 255, 255);
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeWidth(2);
            mTargetPaint = new Paint();
            mTargetPaint.setAntiAlias(false);
            mTargetPaint.setARGB(192, 0, 255, 0);
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeWidth(1);
        }

        @Override
        protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
            mTextPaint.getFontMetricsInt(mTextMetrics);
            mHeaderBottom = -mTextMetrics.ascent + mTextMetrics.descent + 2;
            Xlog.v(TAG, "Metrics: ascent=" + mTextMetrics.ascent + " descent="
                    + mTextMetrics.descent + " leading=" + mTextMetrics.leading
                    + " top=" + mTextMetrics.top + " bottom="
                    + mTextMetrics.bottom);
        }

        @Override
        protected void onDraw(Canvas canvas) {
            int w = getWidth() / 5;
            int base = -mTextMetrics.ascent + 1;
            int bottom = mHeaderBottom;
            canvas.drawRect(0, 0, w - 1, bottom, mTextBackgroundPaint);
            canvas.drawText("X: " + mCurX, 1, base, mTextPaint);

            canvas.drawRect(w, 0, (w * 2) - 1, bottom, mTextBackgroundPaint);
            canvas.drawText("Y: " + mCurY, 1 + w, base, mTextPaint);

            canvas
                    .drawRect(w * 2, 0, (w * 3) - 1, bottom,
                            mTextBackgroundPaint);
            canvas.drawRect(w * 2, 0, (w * 2) + (mCurPressure * w) - 1, bottom,
                    mTextLevelPaint);
            Xlog.w(TAG, "mCurPressure = " + mCurPressure);
            canvas.drawText("Pres: " + mCurPressure, 1 + w * 2, base,
                    mTextPaint);

            canvas
                    .drawRect(w * 3, 0, (w * 4) - 1, bottom,
                            mTextBackgroundPaint);
            int xVelocity = mVelocity == null ? 0 : (int) (Math.abs(mVelocity
                    .getXVelocity()) * 1000);
            canvas.drawText("XVel: " + xVelocity, 1 + w * 3, base, mTextPaint);

            canvas.drawRect(w * 4, 0, getWidth(), bottom, mTextBackgroundPaint);
            int yVelocity = mVelocity == null ? 0 : (int) (Math.abs(mVelocity
                    .getYVelocity()) * 1000);
            canvas.drawText("YVel: " + yVelocity, 1 + w * 4, base, mTextPaint);

            int lineSz = mLines.size();
            int k = 0;
            for (k = 0; k < lineSz; k++) {
                ArrayList<PT> m = mLines.get(k);

                float lastX = 0;
                float lastY = 0;
                mPaint.setARGB(255, 0, 255, 255);
                int sz = m.size();
                int i = 0;
                for (i = 0; i < sz; i++) {
                    PT n = m.get(i);
                    if (i > 0) {
                        canvas.drawLine(lastX, lastY, n.mX, n.mY, mTargetPaint);
                        canvas.drawPoint(lastX, lastY, mPaint);
                    }

                    lastX = n.mX;
                    lastY = n.mY;
                }

                VelocityTracker velocity = mVelocityList.get(k);
                if (velocity == null) {                    
                    canvas.drawPoint(lastX, lastY, mPaint);
                } else { 
                    mPaint.setARGB(255, 255, 0, 0);
                    float xVel = velocity.getXVelocity() * (1000 / 60);
                    float yVel = velocity.getYVelocity() * (1000 / 60);
                    canvas.drawLine(lastX, lastY, lastX + xVel, lastY + yVel,
                            mPaint);
                }

                if (mCurDown) {
                    canvas.drawLine(0, (int) mCurY, getWidth(), (int) mCurY,
                            mTargetPaint);
                    canvas.drawLine((int) mCurX, 0, (int) mCurX, getHeight(),
                            mTargetPaint);
                    int pressureLevel = (int) (mCurPressure * 255);
                    mPaint
                            .setARGB(255, pressureLevel, 128,
                                    255 - pressureLevel);
                    canvas.drawPoint(mCurX, mCurY, mPaint);
                    canvas.drawCircle(mCurX, mCurY, mCurWidth, mPaint);
                }

            }
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            int action = event.getAction();
            if (action == MotionEvent.ACTION_DOWN) {

                mVelocity = VelocityTracker.obtain();
                mVelocityList.add(mVelocity);

                mCurLine = new ArrayList<PT>();
                mLines.add(mCurLine);
            }
            mVelocity.addMovement(event);
            mVelocity.computeCurrentVelocity(1);
            final int num = event.getHistorySize();
            for (int i = 0; i < num; i++) {
                mCurLine.add(new PT(event.getHistoricalX(i), event
                        .getHistoricalY(i)));
            }
            mCurLine.add(new PT(event.getX(), event.getY()));
            mCurDown = action == MotionEvent.ACTION_DOWN
                    || action == MotionEvent.ACTION_MOVE;
            mCurX = (int) event.getX();
            mCurY = (int) event.getY();
            mCurPressure = event.getPressure();
            Xlog.w(TAG, "event.getPressure()= " + mCurPressure);
            mCurWidth = (int) (event.getSize() * (getWidth() / 3));

            invalidate();
            return true;
        }

        public void clear() {
            for (ArrayList<PT> m : mLines) {
                m.clear();
            }
            mLines.clear();
            mVelocityList.clear();
            invalidate();
        }

    }
}
