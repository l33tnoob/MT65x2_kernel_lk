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

package com.mediatek.atci.service;

import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.util.Log;

import java.io.IOException;

public class Mp3Player implements OnCompletionListener,OnErrorListener {
    static final String TAG = "ATCIJ_MP3PLAYER";
    private static final String NO_SIGNAL_FILE_NAME = "NoSignal_LR_128k.mp3";
    private static final String LR_FILE_NAME = "1kHz_0dB_LR_128k.mp3";
    private static final String L_FILE_NAME = "1kHz_0dB_L128k.mp3";
    private static final String R_FILE_NAME = "1kHz_0dB_R_128k.mp3";
    private static final String MULTI_LR_FILE_NAME = "MultiSine_20-20kHz-0dBp_128k.mp3";
    private static final int STATE_IDLE  = 0;
    private static final int STATE_PLAYING  = 1;
    private static final int STATE_ERROR  = 2;
    private static final boolean DBG = true;
    private int mState  = STATE_IDLE;
    private MediaPlayer mPlayer = null;
    private static Mp3Player sMp3Player = null;

    public static Mp3Player getInstanceOfMp3Player() {
        if (sMp3Player == null) {
            sMp3Player =  new Mp3Player();
        }
        return sMp3Player;
    }

    public boolean isPlaying() {
        return mState == STATE_PLAYING;
    }

    public boolean isIdle() {
        return mState == STATE_IDLE;
    }

    public boolean isError() {
        return mState == STATE_ERROR;
    }

    private String getfilePath(char mode) {
        String fileName = null;        
        String fireDir = "/system/media/audio/lgeSounds/factorytest/";
        
        Log.i(TAG,"file  directory is:" + fireDir);

        switch(mode) {
        case '1':
            fileName = NO_SIGNAL_FILE_NAME;
            break;
        case '2':
            fileName = LR_FILE_NAME;
            break;
        case '3':
            fileName = L_FILE_NAME;
            break;
        case '4':
            fileName = R_FILE_NAME;
            break;
        case '5':
            fileName = MULTI_LR_FILE_NAME;
            break;
        default:
            break;            
        }   
        Log.i(TAG,"file Name is:" + fileName);
        Log.i(TAG,"file Path is:" + fireDir + fileName);
        return fireDir + fileName;
    }
    
    public void startPlayer(char mode) {
        Log.i(TAG,"startPlayer:" + mode);
        stopPlayer();
        
        mPlayer = new MediaPlayer();
        mPlayer.setOnErrorListener(this);
        
        getfilePath(mode);
        try {
            mPlayer.setDataSource(getfilePath(mode));
            mPlayer.setOnCompletionListener(this);
            mPlayer.prepare();
            mPlayer.start();
        } catch (IllegalArgumentException e) {
            mState  = STATE_ERROR;
            return;
        } catch (IOException e) {
            mState  = STATE_ERROR;
            return;
        }
        mState  = STATE_PLAYING;
    }

    public void stopPlayer() {
        if (mPlayer == null) {
            // we were not in Player
            return;
        }

        Log.i(TAG,"stopPlayer");
        mPlayer.stop();
        mPlayer.release();
        mPlayer = null;
        mState  = STATE_IDLE;
    }
    
    /** {@inheritDoc} */
    @Override
    public void onCompletion(MediaPlayer mp) {
        Log.i(TAG,"onCompletion");
        stopPlayer();
    }

    /** {@inheritDoc} */
    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        Log.i(TAG,"onError");
        stopPlayer();
        mState  = STATE_ERROR;
        return true;
    }
}
