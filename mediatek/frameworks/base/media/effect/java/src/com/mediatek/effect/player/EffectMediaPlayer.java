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

import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.view.Surface;

public class EffectMediaPlayer extends EffectPlayer {
    public EffectMediaPlayer(int width, int height) {
        super(width, height);

        setOnVideoSizeChangedListener(null);
        setOnPreparedListener(null);
        setOnCompletionListener(null);

        mTool.saveToStorageInit();
    }

    @Override
    public void setSurface(Surface sf) {
        mTool.log('d', "setSurface() " + sf);

        if (mEffectGraphCore.getGraphEffect() <= 0) {
            super.setSurface(sf);
            return;
        }

        super.setSurface(null);
    }

    private void setSurfaceInternal(Surface sf) {
        super.setSurface(sf);
    }

    private void mediaPlayerStart() {
        mTool.log('d', "start() MediaPlayer Playing withEffect: ["
                + mEffectGraphCore.getGraphEffect() + "]" + mEffectGraphCore.getGraphEffectName() + " .............");
        super.start();
    }

    public class StartGraphThread implements Runnable {
        public void run() {
            Thread.currentThread().setName("[" + mTool.getID() + "] Start Graph Thread - " + " MediaPlayer");
            synchronized (EffectMediaPlayer.this) {
                isStartThreadRunning = true;
                mTool.log('d', "start() Srart-Thread Start ... ");

                if (mEffectGraphCore != null) {
                    boolean result = true;
                    mEffectGraphCore.graphClose();

                    if (mEffectGraphCore.isGraphRunning == false) {
                        mEffectGraphCore.setResourceContext(mCntx, mEffectVideoUri);
                        mEffectGraphCore.setIsFromMediaPlayer(true);
                        if (mEffectGraphCore.graphCreate() == true) {
                            SurfaceTexture st = mEffectGraphCore.graphRun();
                            if (st == null) {
                                result = false;
                            } else {
                                Surface sf = new Surface(st);
                                try {
                                    setSurfaceInternal(sf);
                                } catch (IllegalStateException e) {
                                    e.printStackTrace();
                                    throw new IllegalStateException();
                                } finally {
                                    sf.release();
                                }
                            }
                        } else {
                            result = false;
                        }
                    }

                    if (result) {
                        try {
                            mediaPlayerStart();
                        } catch (IllegalStateException e) {
                            e.printStackTrace();
                        }
                    }
                } else {
                    mTool.log('w', "start() no context to be used ! it may be released before!");
                }

                isStartThreadRunning = false;
                mTool.log('d', "start() Srart-Thread End ... ");
            }
        }
    }

    @Override
    public synchronized void start() {
        if (isStartThreadRunning == false) {
            submit(new StartGraphThread());
        }
    }

    @Override
    public synchronized void stop(){
        mTool.log('d', "stop()");
        super.stop();
    }

    @Override
    public void setOnVideoSizeChangedListener(final MediaPlayer.OnVideoSizeChangedListener listener) {
        mTool.log('d', "setOnVideoSizeChangedListener() " + listener);

        super.setOnVideoSizeChangedListener(new MediaPlayer.OnVideoSizeChangedListener() {
            public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
                mTool.log('d', "onVideoSizeChanged(): " + width + " x " + height + " " + mp);
                mEffectGraphCore.setInputSizeToFitOutputSize(width, height);
                if (listener != null)
                    listener.onVideoSizeChanged(mp, width, height);
            }
        });
    }

    @Override
    public void setOnPreparedListener(final MediaPlayer.OnPreparedListener listener) {
        mTool.log('d', "setOnPreparedListener() " + listener);

        super.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            public void onPrepared(MediaPlayer mp) {
                mTool.log('d', "onPrepared(): " + mp);

                if (listener != null)
                    listener.onPrepared(mp);
            }
        });
    }

    @Override
    public void setOnCompletionListener(final MediaPlayer.OnCompletionListener listener) {
        mTool.log('d', "setOnCompletionListener() " + listener);

        super.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            public void onCompletion(MediaPlayer mp) {
                mTool.log('d', "onCompletion(): " + mp);
                mEffectGraphCore.graphClose();

                if (listener != null)
                    listener.onCompletion(mp);
            }
        });
    }

    @Override
    public void setOnErrorListener(final MediaPlayer.OnErrorListener listener) {
        mTool.log('d', "setOnErrorListener() " + listener);

        super.setOnErrorListener(new MediaPlayer.OnErrorListener() {
            public boolean onError(MediaPlayer mp, int framework_err, int impl_err) {
                mTool.log('d', "onError(): " + mp);

                if (listener != null) {
                    return listener.onError(mp, framework_err, impl_err);
                }
                return true;
            }
        });
    }
}

