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
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.mediatek.xlog.Xlog;

import java.util.Vector;

public class TsVerifyLine extends Activity implements View.OnTouchListener {
    public DiversityCanvas mDiversityCanvas;
    public boolean mRun = false;
    public double mDiversity = 0;
    public Vector<Point> mPts1 = null;
    public Vector<Point> mInput = new Vector<Point>();
    public int mLineIndex = 0;

    public static final int CALCULATE_ID = Menu.FIRST;
    public static final int NEXTLINE_ID = Menu.FIRST + 1;

    private static final String TAG = "EM/TouchScreen/VL";

    private int mZoom = 1;
    private int mRectWidth;
    private int mRectHeight;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        DisplayMetrics dm = new DisplayMetrics();
        dm = this.getApplicationContext().getResources().getDisplayMetrics();
        mRectWidth = dm.widthPixels;
        mRectHeight = dm.heightPixels;
        if ((480 == mRectWidth && 800 == mRectHeight)
                || (800 == mRectWidth && 480 == mRectHeight)) {
            mZoom = 2;
        }

        // mRun=true;
        mPts1 = readPoints(0);
        mLineIndex++;
        mDiversityCanvas = new DiversityCanvas((Context) this);
        setContentView(mDiversityCanvas);
        mDiversityCanvas.setOnTouchListener(this);
        Xlog.i(TAG, "Oncreate");

    }



    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, CALCULATE_ID, 0, "Calculate");
        menu.add(0, NEXTLINE_ID, 1, "NextLine");
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
        case CALCULATE_ID:
            calculateDiversity();
            break;
        case NEXTLINE_ID:
            mInput.clear();

            mPts1 = readPoints(mLineIndex);
            mLineIndex++;

            if (4 == mLineIndex) {
                mLineIndex = 0;
            }
            mDiversity = 0.0;
            mDiversityCanvas.invalidate();
            break;
        default:
            break;
        }
        return super.onOptionsItemSelected(aMenuItem);
    }

    public void calculateDiversity() {
        int i;
        Point cp = new Point(0, 0);
        if (mInput.isEmpty()) {
//            if (mInput.size() == 0) {
            return;
        }
        double error = 0.0;
        float ratio = (float) mRectHeight / (float) mRectWidth;

        switch (mLineIndex - 1) {
        case 0:
            for (i = 0; i < mInput.size(); i++) {
                cp = mInput.get(i);
                error += Math.abs(cp.x - mRectWidth / 2);
            }
            break;
        case 1:
            for (i = 0; i < mInput.size(); i++) {
                cp = mInput.get(i);
                error += Math.abs(cp.y - mRectHeight / 2);
            }
            break;
        case 2:
            for (i = 0; i < mInput.size(); i++) {
                cp = mInput.get(i);
                error += Math.abs(ratio * cp.x - cp.y)
                        / Math.sqrt(1 + ratio * ratio);
            }
            break;
        case -1:
            for (i = 0; i < mInput.size(); i++) {
                cp = mInput.get(i);
                error += Math.abs(ratio * cp.x + cp.y - mRectHeight)
                        / Math.sqrt(1 + ratio * ratio);
            }
            break;
        default:
            break;
        }

        mDiversity = error / mInput.size();
    }

    public boolean onTouch(View v, MotionEvent e) {

        if (MotionEvent.ACTION_DOWN == e.getAction()
                || MotionEvent.ACTION_MOVE == e.getAction()) {
            if (v.equals(mDiversityCanvas)) {
//                if (v == mDiversityCanvas) {
                mInput.add(new Point((int) e.getX(), (int) e.getY()));
            }
        }

        return true;
    }

    public Vector<Point> readPoints(int lineIndex) {
        int x;
        int y;
        int i;
        Vector<Point> v = new Vector<Point>();
        Point p;
        float ratio = (float) mRectHeight / (float) mRectWidth;

        switch (mLineIndex) {
        case 0:
            for (i = 0; i < mRectHeight; i++) {
                x = mRectWidth / 2;
                y = i;
                p = new Point(x, y);
                v.add(p);
            }
            break;
        case 1:
            for (i = 0; i < mRectWidth; i++) {
                x = i;
                y = mRectHeight / 2;
                p = new Point(x, y);
                v.add(p);
            }
            break;
        case 2:
            for (i = 0; i < mRectWidth; i++) {
                x = i;
                y = (int) (i * ratio);
                p = new Point(x, y);
                v.add(p);
            }
            break;
        case 3:
            for (i = 0; i < mRectWidth; i++) {
                x = mRectWidth - i;
                y = (int) (i * ratio);
                p = new Point(x, y);
                v.add(p);
            }
            break;
        default:
            break;
        }
        return v;
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
            Xlog.v(TAG, "surfaceCreated");
            mRun = true;

            mThread = new DiversityThread(holder, null);
            mThread.start();
        }

        public void surfaceDestroyed(SurfaceHolder holder) {
            mRun = false;
            Xlog.v(TAG, "surfaceDestroyed");
        }

        class DiversityThread extends Thread {
            private SurfaceHolder mSurfaceHolder = null;
            private Paint mLinePaint = null;
            private Paint mTextPaint = null;
            private Paint mRectPaint = null;
            private Rect mRect = null;

            public DiversityThread(SurfaceHolder s, Context c) {
                mSurfaceHolder = s;
                mLinePaint = new Paint();
                mLinePaint.setAntiAlias(true);
                mTextPaint = new Paint();
                mTextPaint.setAntiAlias(true);
                mTextPaint.setTextSize(9.0f * mZoom);
                mTextPaint.setARGB(255, 0, 0, 0);
                mRect = new Rect(0, 0, mRectWidth, mRectHeight);
                mRectPaint = new Paint();
                mRectPaint.setARGB(255, 255, 255, 255);
            }

            @Override
            public void run() {
                while (mRun) {
                    Canvas c = null;
//                    try {
                    c = mSurfaceHolder.lockCanvas(null);
                    synchronized (mSurfaceHolder) {
                        if (c != null) {
                            doDraw(c);
                        }
                    }
//                    } catch (Exception e) {
//                        Xlog.v(TAG, "do draw fail!");
//                        Xlog.v(TAG, e.toString());
//                    } finally {
                    if (c != null) {
                        mSurfaceHolder.unlockCanvasAndPost(c);
                    }
//                    }
                }
            }

            private void doDraw(Canvas canvas) {
                int i;
                Point p1;
                Point p2;
                canvas.drawRect(mRect, mRectPaint);
                mLinePaint.setARGB(255, 0, 0, 255);
                try {
                    for (i = 0; i < mPts1.size() - 1; i++) {
                        p1 = mPts1.get(i);
                        p2 = mPts1.get(i + 1);
                        canvas.drawLine(p1.x, p1.y, p2.x, p2.y, mLinePaint);
                    }
                } catch (ArrayIndexOutOfBoundsException e) {
                    Xlog.v(TAG, "mPts1 ArrayIndexOutOfBoundsException: " + e.getMessage());
                    return;
                }

                mLinePaint.setARGB(255, 255, 0, 0);
                try {
                    for (i = 0; i < mInput.size() - 1; i++) {
                        p1 = mInput.get(i);
                        p2 = mInput.get(i + 1);
                        canvas.drawLine(p1.x, p1.y, p2.x, p2.y, mLinePaint);
                    }
                } catch (ArrayIndexOutOfBoundsException e) {
                    Xlog.v(TAG, "mInput ArrayIndexOutOfBoundsException: "
                            + e.getMessage());
                    return;
                }
                canvas.drawText("Diversity : " + Double.toString(mDiversity),
                        20 * mZoom, mRectHeight - 10 * mZoom, mTextPaint);
            }
        }
    }
}
