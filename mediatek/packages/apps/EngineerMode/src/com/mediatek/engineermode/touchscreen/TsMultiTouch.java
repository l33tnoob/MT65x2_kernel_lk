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
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.InputType;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * Demonstrates wrapping a layout in a ScrollView.
 * 
 * 
 */

public class TsMultiTouch extends Activity {
    public static final int CLEAR_CANVAS_ID = 1;
    public static final int SET_PT_SIZE_ID = 2;
    public static final int DIS_HISTORY_ID = 3;
    public static final int[][] RGB = { { 255, 0, 0 }, { 0, 255, 0 },
            { 0, 0, 255 }, { 255, 255, 0 }, { 0, 255, 255 }, { 255, 0, 255 },
            { 100, 0, 0 }, { 0, 100, 0 }, { 0, 0, 100 }, { 100, 100, 0 },
            { 0, 100, 100 }, { 100, 0, 100 }, { 255, 255, 255 } };
    MyView mView = null;
    volatile boolean mDisplayHistory = true;
    DisplayMetrics mMetrics = new DisplayMetrics();

    public int mPointSize = 1;
    private static final String TAG = "EM/TouchScreen/MT";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        mView = new MyView(this);
        setContentView(mView);
    }

    @Override
    public void onResume() {
        super.onResume();
        final SharedPreferences preferences = this.getSharedPreferences(
                "touch_screen_settings", android.content.Context.MODE_PRIVATE);
        String fileString = preferences.getString("filename", "N");
        if (!"N".equals(fileString)) {
            final String commPath = fileString;
            new Thread() {
                public void run() {
                    String[] cmd = { "/system/bin/sh", "-c",
                            "echo [ENTER_MULTI_TOUCH] >> " + commPath }; // file
                    int ret;
                    try {
                        ret = TouchScreenShellExe.execCommand(cmd);
                        if (0 == ret) {
                            Xlog.v(TAG, "-->onResume Start logging...");
                        } else {
                            Xlog.v(TAG, "-->onResume Logging failed!");
                        }
                    } catch (IOException e) {
                        Xlog.e(TAG, e.toString());
                    }
                }
            }.start();

        }
        mPointSize = preferences.getInt("size", 10);
        getWindowManager().getDefaultDisplay().getMetrics(mMetrics);
    }

    @Override
    public void onPause() {
        Xlog.v(TAG, "-->onPause");
        final SharedPreferences preferences = this.getSharedPreferences(
                "touch_screen_settings", android.content.Context.MODE_PRIVATE);
        String fileString = preferences.getString("filename", "N");
        if (!"N".equals(fileString)) {
            String[] cmd = { "/system/bin/sh", "-c",
                    "echo [LEAVE_MULTI_TOUCH] >> " + fileString }; // file
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
                Xlog.e(TAG, e.toString());
            }
        }
        super.onPause();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.add(0, CLEAR_CANVAS_ID, 0, "Clean Table");
        menu.add(0, SET_PT_SIZE_ID, 0, "Set Point Size");
        menu.add(0, DIS_HISTORY_ID, 0, "Hide History");
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mDisplayHistory) {
            menu.getItem(2).setTitle("Hide History");
        } else {
            menu.getItem(2).setTitle("Show History");
        }
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem mi) {
        switch (mi.getItemId()) {
        case CLEAR_CANVAS_ID:
            mView.clear();
            break;
        case DIS_HISTORY_ID:
            if (mDisplayHistory) {
                mDisplayHistory = false;
            } else {
                mDisplayHistory = true;
            }
            mView.invalidate();
            break;
        case SET_PT_SIZE_ID:
            final EditText input = new EditText(this);
            input.setInputType(InputType.TYPE_CLASS_NUMBER);
            new AlertDialog.Builder(this).setTitle(
                    "Insert pixel size of point [1-10]").setView(input)
                    .setPositiveButton("OK", new OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            if (input.getText() != null
                                    && (!input.getText().toString().equals(""))) {
                                int sz;
                                try {
                                    sz = Integer.valueOf(input.getText()
                                            .toString());
                                } catch (NumberFormatException e) {
                                    return;
                                }
                                if (sz < 1) {
                                    TsMultiTouch.this.mPointSize = 1;
                                } else if (sz > 10) {
                                    TsMultiTouch.this.mPointSize = 10;
                                } else {
                                    TsMultiTouch.this.mPointSize = sz;
                                }
                                final SharedPreferences preferences = TsMultiTouch.this
                                        .getSharedPreferences(
                                                "touch_screen_settings",
                                                android.content.Context.MODE_PRIVATE);
                                preferences.edit().putInt("size",
                                        TsMultiTouch.this.mPointSize).commit();

                                mView.invalidate();
                            }
                        }
                    }).setNegativeButton("Cancel", null).show();
            break;
        default:
            break;
        }
        return super.onOptionsItemSelected(mi);
    }

    public class MyView extends View {
        private HashMap<Integer, TsPointDataStruct> mCurrPoints = new HashMap<Integer, TsPointDataStruct>();
        private ArrayList<TsPointDataStruct> mHistory = new ArrayList<TsPointDataStruct>();

        public MyView(Context c) {
            super(c);
        }

        @Override
        protected void onDraw(Canvas canvas) {
            if (mDisplayHistory) {
                for (int i = 0; i < mHistory.size(); i++) {
                    TsPointDataStruct point = mHistory.get(i);
                    int x = point.getmCoordinateX();
                    int y = point.getmCoordinateY();
                    canvas.drawCircle(x, y, mPointSize, getPaint(point.getmPid()));
                }
            }

            for (TsPointDataStruct point : mCurrPoints.values()) {
                Paint targetPaint = getPaint(point.getmPid());
                String s = "pid " + String.valueOf(point.getmPid())
                        + " x=" + String.valueOf(point.getmCoordinateX())
                        + ", y=" + String.valueOf(point.getmCoordinateY());
                Xlog.i(TAG, "Touch pos: " + point.getmCoordinateX() + "," + point.getmCoordinateY());

                Rect rect = new Rect();
                targetPaint.getTextBounds(s, 0, s.length(), rect);

                int x = point.getmCoordinateX() - rect.width() / 2;
                int y = point.getmCoordinateY() - rect.height() * 3;

                if (x < 0) {
                    x = 0;
                } else if (x > mMetrics.widthPixels - rect.width()) {
                    x = mMetrics.widthPixels - rect.width();
                }

                if (y < rect.height()) {
                    y = rect.height();
                } else if (y > mMetrics.heightPixels) {
                    y = mMetrics.heightPixels;
                }

                canvas.drawText(s, x, y, targetPaint);
                canvas.drawCircle(point.getmCoordinateX(),
                        point.getmCoordinateY(), mPointSize * 3,
                        targetPaint);
            }
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            Xlog.i(TAG, "onTouchEvent: Pointer counts: " + event.getPointerCount() +
                    " Action: " + event.getAction());

            for (int i = 0; i < event.getPointerCount(); i++) {
                Xlog.i(TAG, "onTouchEvent: idx: " + i + " pid: " + event.getPointerId(i) +
                        " (" + event.getX(i) + ", " + event.getY(i) + ")");
                int pid = event.getPointerId(i);
                TsPointDataStruct n = new TsPointDataStruct();
                n.setmCoordinateX((int) event.getX(i));
                n.setmCoordinateY((int) event.getY(i));
                n.setmPid(pid);
                mCurrPoints.put(pid, n);
                mHistory.add(n);
            }

            invalidate();
            return true;
        }

        public void clear() {
            mCurrPoints.clear();
            mHistory.clear();
            invalidate();
        }

        Paint getPaint(int idx) {
            Paint paint = new Paint();
            paint.setAntiAlias(false);
            if (idx < RGB.length) {
                paint.setARGB(255, RGB[idx][0], RGB[idx][1], RGB[idx][2]);
            } else {
                paint.setARGB(255, 255, 255, 255);
            }
            int textsize = (int) (mPointSize * 3.63 + 7.37);
            paint.setTextSize(textsize);
            return paint;
        }
    }
}
