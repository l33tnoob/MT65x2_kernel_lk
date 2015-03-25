/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.effect.filterpacks;

import java.io.BufferedInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;

import android.content.ContentResolver;
import android.content.res.Resources;
import android.filterfw.core.FilterContext;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GLFrame;
import android.filterfw.format.ImageFormat;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Bitmap.CompressFormat;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;

public class MyUtility {
    public String TAG;
    public int id = 0;

    public MyUtility(String name, int [] i) {
        setTag(name);
        setID(i);
    }

    public void setTag(String name) {
        TAG = name;
    }

    public void setID(int [] i) {
        if (i != null) {
            id = i[0];
        }
    }

    public int getID() {
        return id;
    }

    public void setIDandIncrease(int [] i) {
        if (i != null) {
            id = i[0];
            i[0]++;
        }
    }

    public void log(char level, String logline) {
        switch (level) {
            case 'd':
                Log.d(TAG, "[" + id + "] " + logline);
                break;

            case 'i':
                Log.i(TAG, "[" + id + "] " + logline);
                break;

            case 'w':
                Log.w(TAG, "[" + id + "] " + logline);
                break;

            case 'e':
                Log.e(TAG, "[" + id + "] " + logline);
                break;

            case 'v':
            default:
                Log.v(TAG, "[" + id + "] " + logline);
                break;
        }
    }

    public void saveToStorageInit() {
        /*String root = Environment.getExternalStorageDirectory().toString();
        File myDir = new File(root + "/EffectMediaPlayer");
        myDir.mkdirs();

        fname = "log_" + id + ".txt";

        File file = new File(myDir, fname);
        if (file.exists())
            file.delete();

        Date dNow = new Date( );
        SimpleDateFormat ft = new SimpleDateFormat ("HH:mm:ss.SSS ");

        try {
            String filename = file.getPath();
            FileWriter fw = new FileWriter(filename, true);
            fw.write(ft.format(dNow) + "EffectMediaPlayer() " + mWidth + "x" + mHeight + "\r\n");
            fw.close();
        } catch (IOException e) {
            System.err.println("IOException: " + e.getMessage());
            e.printStackTrace();
        }*/
    }

    public void logLog(char level, String logline) {
        /*String root = Environment.getExternalStorageDirectory().toString();
        File myDir = new File(root + "/EffectMediaPlayer");
        myDir.mkdirs();

        File file = new File(myDir, fname);

        Date dNow = new Date( );
        SimpleDateFormat ft = new SimpleDateFormat ("HH:mm:ss.SSS ");

        try {
            String filename = file.getPath();
            FileWriter fw = new FileWriter(filename, true);
            fw.write(ft.format(dNow) + logline + "\r\n");
            fw.close();
        } catch (IOException e) {
            System.err.println("IOException: " + e.getMessage());
        }*/
    }

    public static Bitmap loadBitmap(String url) {
        Bitmap bm = null;
        InputStream is = null;
        BufferedInputStream bis = null;
        try {
            URLConnection conn = new URL(url).openConnection();
            conn.connect();
            is = conn.getInputStream();
            bis = new BufferedInputStream(is, 8192);
            bm = BitmapFactory.decodeStream(bis);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (bis != null) {
                try {
                    bis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return bm;
    }

    public void saveBitmap(Bitmap bmp) {
        saveBitmap(bmp, Environment.getExternalStorageDirectory().getPath() + "/bitmap_out.png");
    }

    public void saveBitmap(Bitmap bmp, String filename) {
        if (bmp != null) {
            try {
                OutputStream stream;
                stream = new FileOutputStream(filename);
                bmp.compress(CompressFormat.PNG, 100, stream);
                try {
                    stream.flush();
                    stream.close();
                } catch (IOException e) {
                    Log.e(TAG, "Could not close image: " + e.getMessage());
                    e.printStackTrace();
                }
                Log.d(TAG, "Bitmap Saved: " + filename);
            } catch (FileNotFoundException e) {
                Log.e(TAG, "Could not save image: " + e.getMessage());
                e.printStackTrace();
            }
        }
    }

    public static void saveBitmapS(Bitmap bmp, String filename) {
        if (bmp != null) {
            try {
                OutputStream stream;
                stream = new FileOutputStream(filename);
                bmp.compress(CompressFormat.PNG, 100, stream);
                try {
                    stream.flush();
                    stream.close();
                } catch (IOException e) {
                    Log.e("MyUtility", "Could not close image: " + e.getMessage());
                    e.printStackTrace();
                }
                Log.d("MyUtility", "Bitmap Saved: " + filename);
            } catch (FileNotFoundException e) {
                Log.e("MyUtility", "Could not save image: " + e.getMessage());
                e.printStackTrace();
            }
        }
    }

    /**
     * Resizes the large image to the specified size
     *
     * @param oldBitmap The original bitmap
     * @return The resized bitmap
     */
    public static Bitmap generateSmallImage(Bitmap oldBitmap, float scale) {
        int width = oldBitmap.getWidth();
        int height = oldBitmap.getHeight();
        Bitmap newBitmap = oldBitmap;

        if (scale < 1.0f) {
            Matrix matrix = new Matrix();
            matrix.postScale(scale, scale);
            newBitmap = Bitmap.createBitmap(oldBitmap, 0, 0, width, height, matrix, true);
        } else {
            newBitmap = oldBitmap;
        }

        return newBitmap;
    }

    /**
     * Rotate the image to the specified 0, 90, 180, 270
     *
     * @param oldBitmap The original bitmap
     * @return The rotate bitmap
     */
    public static Bitmap generateRotateImage(Bitmap oldBitmap, int rotation, boolean recycle) {
        if (rotation == 0)
            return oldBitmap;

        int w = oldBitmap.getWidth();
        int h = oldBitmap.getHeight();
        Matrix m = new Matrix();
        m.postRotate(rotation);
        Bitmap newBitmap = Bitmap.createBitmap(oldBitmap, 0, 0, w, h, m, true);

        if (recycle)
            oldBitmap.recycle();

        return newBitmap;
    }

    public static Bitmap getCutBitmap(Bitmap in, int outw, int outh, boolean recycle) {
        float localInputWidth = in.getWidth();
        float localInputHeight = in.getHeight();

        float outputAspectRatio = (float) outw / outh;
        float inputAspectRatio = (float) localInputWidth / localInputHeight;

        int xOffset = 0;
        int yOffset = 0;

        Matrix matrix = new Matrix();
        Bitmap scaleBitmap;
        float scale;

        if (outputAspectRatio <= inputAspectRatio) {
            xOffset = (int) Math.abs((localInputWidth - localInputHeight * outputAspectRatio) / 2.0f);
            scale = outh / localInputHeight;
            xOffset *= scale;
        } else {
            yOffset = (int) Math.abs((localInputHeight - localInputWidth / outputAspectRatio) / 2.0f);
            scale = outw / localInputWidth;
            yOffset *= scale;
        }

        matrix.setScale(scale, scale);
        scaleBitmap = Bitmap.createBitmap(in, 0, 0, in.getWidth(), in.getHeight(), matrix, true);

        Log.w("MyUtility", "scaleBitmap: " + scaleBitmap.getWidth() + "x" + scaleBitmap.getHeight());

        Bitmap cutbitmap = getCutBitmap(scaleBitmap, xOffset, yOffset, outw, outh, true);

        if (recycle)
            in.recycle();
        return cutbitmap;
    }

    public static Bitmap getCutBitmap(Bitmap in, int x, int y, int outw, int outh, boolean recycle) {
        Bitmap result = Bitmap.createBitmap(outw, outh, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(result);
        canvas.drawBitmap(in, new Rect(x, y, x + outw, y + outh), new Rect(0, 0, outw, outh), new Paint());
        if (recycle)
            in.recycle();
        return result;
    }

    public static Bitmap getBitmapFromUri(ContentResolver con, Uri uri) {
        try {
            Bitmap bitmap = MediaStore.Images.Media.getBitmap(con, uri);
            return bitmap;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public static Bitmap getBitmapFromResource(Resources res, int r) {
        BitmapFactory.Options bfoOptions = new BitmapFactory.Options();
        bfoOptions.inScaled = false;
        return BitmapFactory.decodeResource(res, r, bfoOptions);
    }

    public static GLFrame createBitmapFrame(FilterContext context, Bitmap bitmap) {
        FrameFormat format = ImageFormat.create(bitmap.getWidth(),
                                                bitmap.getHeight(),
                                                ImageFormat.COLORSPACE_RGBA,
                                                FrameFormat.TARGET_GPU);

        GLFrame frame = (GLFrame)context.getFrameManager().newFrame(format);
        frame.setBitmap(bitmap);

        return frame;
    }

    public void showCallStack() {
        StackTraceElement[] stackTraceElements = Thread.currentThread().getStackTrace();
        for (int i = 2; i < stackTraceElements.length; i++) {
            StackTraceElement ste = stackTraceElements[i];
            String classname = ste.getClassName();
            String methodName = ste.getMethodName();
            int lineNumber = ste.getLineNumber();
            log('d', "\t" + classname + "." + methodName + ":" + lineNumber);
        }
    }

    /*
     * return one dimension as copy
     */

    public static float[] MatrixToOneWay(float[][] A) {
        int m = A.length;
        int n = A[0].length;
        float[] C = new float[n * m];
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                C[i * n + j] = A[i][j];
        return C;
    }

    /*
     * return n-by-n identity matrix I
     */
    public static float[][] MatrixIdentity(int n) {
        float[][] I = new float[n][n];
        for (int i = 0; i < n; i++)
            I[i][i] = 1;
        return I;
    }

    /*
     * return x^T y
     */
    public static float MatrixDot(float[] x, float[] y) {
        if (x.length != y.length)
            throw new RuntimeException("Illegal vector dimensions.");
        float sum = 0.0f;
        for (int i = 0; i < x.length; i++)
            sum += x[i] * y[i];
        return sum;
    }

    /*
     * return C = A^T
     */
    public static float[][] MatrixTranspose(float[][] A) {
        int m = A.length;
        int n = A[0].length;
        float[][] C = new float[n][m];
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                C[j][i] = A[i][j];
        return C;
    }

    /*
     * return C = A + B
     */
    public static float[][] MatrixAdd(float[][] A, float[][] B) {
        int m = A.length;
        int n = A[0].length;
        float[][] C = new float[m][n];
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                C[i][j] = A[i][j] + B[i][j];
        return C;
    }

    /*
     * return C = A - B
     */
    public static float[][] MatrixSubtract(float[][] A, float[][] B) {
        int m = A.length;
        int n = A[0].length;
        float[][] C = new float[m][n];
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                C[i][j] = A[i][j] - B[i][j];
        return C;
    }

    /*
     * return C = A * B
     */
    public static float[][] MatrixMultiply(float[][] A, float[][] B) {
        int mA = A.length;
        int nA = A[0].length;
        int mB = B.length;
        int nB = A[0].length;
        if (nA != mB)
            throw new RuntimeException("Illegal matrix dimensions.");
        float[][] C = new float[mA][nB];
        for (int i = 0; i < mA; i++)
            for (int j = 0; j < nB; j++)
                for (int k = 0; k < nA; k++)
                    C[i][j] += (A[i][k] * B[k][j]);
        return C;
    }

    /*
     * matrix-vector multiplication (y = A * x)
     */
    public static float[] MatrixMultiply(float[][] A, float[] x) {
        int m = A.length;
        int n = A[0].length;
        if (x.length != n)
            throw new RuntimeException("Illegal matrix dimensions.");
        float[] y = new float[m];
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                y[i] += (A[i][j] * x[j]);
        return y;
    }

    /*
     * vector-matrix multiplication (y = x^T A)
     */
    public static float[] MatrixMultiply(float[] x, float[][] A) {
        int m = A.length;
        int n = A[0].length;
        if (x.length != m)
            throw new RuntimeException("Illegal matrix dimensions.");
        float[] y = new float[n];
        for (int j = 0; j < n; j++)
            for (int i = 0; i < m; i++)
                y[j] += (A[i][j] * x[i]);
        return y;
    }
}
