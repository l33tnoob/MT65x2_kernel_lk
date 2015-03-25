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
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.Random;
import java.util.Vector;

public class TsVerifyShakingPoint extends Activity implements View.OnTouchListener {
    private static final String TAG = "EM/TouchScreen/TsVerifyShakingPoint";
    private boolean mIsVerifyPoint = true;
    private boolean mRun = false;
    private Random mRand;
    private Vector<Point> mInputPoint = new Vector<Point>();
    private Point mPrePoint;
    private double mAverageShakingError = 0.0;
    private double mPointError = 0.0;
    private Bitmap mBitmap;
    private int mBitmapPad = 0;
    private int mZoom = 1;
    private int mRectWidth;
    private int mRectHeight;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle extra = getIntent().getExtras();
        if (null != extra) {
            mIsVerifyPoint = extra.getBoolean(TsVerifyList.TS_POINT);
        }
        Xlog.v(TAG, "is verify point ? " + mIsVerifyPoint);
        if (mIsVerifyPoint) {
            setTitle(R.string.TouchScreen_VerificationPoint);
        }
        DisplayMetrics dm = new DisplayMetrics();
        dm = this.getApplicationContext().getResources().getDisplayMetrics();
        mRectWidth = dm.widthPixels;
        mRectHeight = dm.heightPixels;
        if ((480 == mRectWidth && 800 == mRectHeight)
                || (800 == mRectWidth && 480 == mRectHeight)) {
            mZoom = 2;
        }
        mRand = new Random();
        mPrePoint = new Point(mRectWidth / 2, mRectHeight / 2);
        DiversityCanvas diversityCanvas = new DiversityCanvas((Context) this);
        setContentView(diversityCanvas);
        diversityCanvas.setOnTouchListener(this);
        Resources resource = this.getResources();
        mBitmap = BitmapFactory.decodeResource(resource, R.drawable.cross);
        if (mBitmap != null) {
            mBitmapPad = mBitmap.getHeight() / 2;
        }
    }

    public boolean onTouch(View arg0, MotionEvent event) {
        if (mIsVerifyPoint) {
            if (MotionEvent.ACTION_DOWN == event.getAction()) {
                Xlog.i(TAG, "The PrePoint.x value is : "
                        + String.valueOf(mPrePoint.x));
                Xlog.i(TAG, "The PrePoint.y value is : "
                        + String.valueOf(mPrePoint.y));
                int xTouch = (int) event.getX();
                int yTouch = (int) event.getY();
                Xlog.i(TAG, "The xTouch value is : " + String.valueOf(xTouch));
                Xlog.i(TAG, "The yTouch value is : " + String.valueOf(yTouch));
                int dx2 = (xTouch - mPrePoint.x) * (xTouch - mPrePoint.x);
                int dy2 = (yTouch - mPrePoint.y) * (yTouch - mPrePoint.y);
                mPointError = Math.sqrt((double) (dx2 + dy2));
                int xNextRand = mRand.nextInt(mRectWidth);
                int yNextRand = mRand.nextInt(mRectHeight);
                mPrePoint = new Point(xNextRand, yNextRand);
            }
        } else {
            if (MotionEvent.ACTION_DOWN == event.getAction()
                    || MotionEvent.ACTION_MOVE == event.getAction()) {
                mInputPoint.add(new Point((int) event.getX(), (int) event
                        .getY()));
            } else if (MotionEvent.ACTION_UP == event.getAction()) {
                mAverageShakingError = 0.0;
                for (int i = 0; i < mInputPoint.size(); i++) {
                    int dx2 = (mInputPoint.get(i).x - mPrePoint.x)
                            * (mInputPoint.get(i).x - mPrePoint.x);
                    int dy2 = (mInputPoint.get(i).y - mPrePoint.y)
                            * (mInputPoint.get(i).y - mPrePoint.y);
                    mAverageShakingError += Math.sqrt((double) (dx2 + dy2));
                }
                mAverageShakingError = mAverageShakingError
                        / mInputPoint.size();
                mInputPoint.clear();
                int xNextRand = mRand.nextInt(mRectWidth);
                int yNextRand = mRand.nextInt(mRectHeight);
                mPrePoint = new Point(xNextRand, yNextRand);
            }
        }
        return true;
    }

    class DiversityCanvas extends SurfaceView implements SurfaceHolder.Callback {
        DiversityThread mThread = null;

        public DiversityCanvas(Context context) {
            super(context);
            SurfaceHolder holder = getHolder();
            holder.addCallback(this);
        }

        public void surfaceChanged(SurfaceHolder holder, int format, int width,
                int height) {
            Xlog.v(TAG, "surfaceChanged");
        }

        public void surfaceCreated(SurfaceHolder holder) {
            mRun = true;
            mThread = new DiversityThread(holder, null);
            mThread.start();
        }

        public void surfaceDestroyed(SurfaceHolder holder) {
            mRun = false;
        }

        class DiversityThread extends Thread {
            private SurfaceHolder mSurfaceHolder = null;
            private Paint mTextPaint = null;
            private Paint mRectPaint = null;
            private Rect mRect = null;
            private Paint mCrossPaint = null;

            public DiversityThread(SurfaceHolder s, Context c) {
                mSurfaceHolder = s;
                mTextPaint = new Paint();
                mTextPaint.setAntiAlias(true);
                mTextPaint.setTextSize(9.0f * mZoom);
                mTextPaint.setARGB(255, 0, 0, 0);
                mRect = new Rect(0, 0, mRectWidth, mRectHeight);
                mRectPaint = new Paint();
                mRectPaint.setARGB(255, 255, 255, 255);
                mCrossPaint = new Paint();
                mCrossPaint.setARGB(255, 255, 0, 0);
            }

            @Override
            public void run() {
                while (mRun) {
                    Canvas c = null;
                    try {
                        c = mSurfaceHolder.lockCanvas(null);
                        synchronized (mSurfaceHolder) {
                            if (c != null) {
                                doDraw(c);
                            }
                        }
                    } finally {
                        if (c != null) {
                            mSurfaceHolder.unlockCanvasAndPost(c);
                        }
                    }
                }
            }

            private void doDraw(Canvas canvas) {
                canvas.drawRect(mRect, mRectPaint);
                if (mBitmap == null) {
                    canvas.drawLine(mPrePoint.x - 15, mPrePoint.y - 15,
                            mPrePoint.x + 15, mPrePoint.y + 15, mCrossPaint);
                    canvas.drawLine(mPrePoint.x - 15, mPrePoint.y + 15,
                            mPrePoint.x + 15, mPrePoint.y - 15, mCrossPaint);
                } else {
                    canvas.drawBitmap(mBitmap, mPrePoint.x - mBitmapPad,
                            mPrePoint.y - mBitmapPad, null);
                }
                if (mIsVerifyPoint) {
                    canvas.drawText("point error : "
                            + Double.toString(mPointError), 20,
                            mRectHeight / 2, mTextPaint);
                } else {
                    canvas.drawText("Average shaking error : "
                            + Double.toString(mAverageShakingError), 20,
                            mRectHeight / 2, mTextPaint);
                }
            }
        }
    }
}