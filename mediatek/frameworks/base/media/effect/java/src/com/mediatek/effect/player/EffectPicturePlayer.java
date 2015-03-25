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

package com.mediatek.effect.player;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.Surface.OutOfResourcesException;

public class EffectPicturePlayer extends EffectPlayer {
    private Bitmap mBitmap = null;

    public EffectPicturePlayer(int width, int height) {
        super(width, height);
    }

    public class StartGraphThread implements Runnable {
        public void run() {
            Thread.currentThread().setName("[" + mTool.getID() + "] Start Graph Thread - " + " PicturePlayer");
            synchronized (EffectPicturePlayer.this) {
                mTool.log('d', "showPicture() Srart-Thread Start ... ");

                isStartThreadRunning = true;

                if (mEffectGraphCore != null && mEffectGraphCore.isGraphRunning == false) {

                    mEffectGraphCore.setResourceContext(mCntx, mEffectVideoUri);
                    if (mEffectGraphCore.graphCreate() == true) {
                        SurfaceTexture st = mEffectGraphCore.graphRun();
                        mEffectGraphCore.setProcessMaxFrameCount(1);

                        if (st != null) {
                            st.setDefaultBufferSize(mWidth, mHeight);

                            Surface sf = new Surface(st);
                            Rect rect = new Rect(0, 0, mWidth, mHeight);

                            float scaleX = (float) mWidth / (float) mBitmap.getWidth();
                            float scaleY = (float) mHeight / (float) mBitmap.getHeight();
                            float scale = scaleX > scaleY ? scaleX : scaleY;

                            try {
                                Canvas cc = sf.lockCanvas(rect);

                                Matrix matrix = new Matrix();
                                matrix.postScale(scale, scale);

                                Bitmap dstbmp = Bitmap.createBitmap(mBitmap, 0, 0, mBitmap.getWidth(),
                                    mBitmap.getHeight(), matrix, true);
                                cc.drawBitmap(dstbmp, 0, 0, null);

                                sf.unlockCanvasAndPost(cc);
                            } catch (IllegalArgumentException e) {
                                e.printStackTrace();
                            } catch (OutOfResourcesException e) {
                                e.printStackTrace();
                            } finally {
                                sf.release();
                            }
                        }

                        if (null != mBitmap) {
                            mBitmap.recycle();
                        }
                        mBitmap = null;
                    }
                }

                isStartThreadRunning = false;
                mTool.log('d', "showPicture() Srart-Thread End ... ");
            }
        }
    }

    public synchronized void showPicture(Bitmap sbmp) {
        mTool.log('d', "showPicture() ");
        mBitmap = sbmp;
        if (isStartThreadRunning == false) {
            submit(new StartGraphThread());
        }
    }

    @Override
    public synchronized void release() {
        super.release();
        if (null != mBitmap) {
            mBitmap.recycle();
        }
        mBitmap = null;
    }
}
