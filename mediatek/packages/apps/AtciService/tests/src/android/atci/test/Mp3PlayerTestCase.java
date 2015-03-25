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

package android.atci.test;

import android.test.AndroidTestCase;
import android.content.Context;
import android.util.Log;
import com.mediatek.atci.service.Mp3Player;

public class Mp3PlayerTestCase extends
        AndroidTestCase {
        	
    public static final String LOG_TAG = "Mp3Player";
    private char mMp3PlayerMode = '0';

/*
    public Mp3PlayerTestCase() {
        super(Mp3Player.class);
    }
*/
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @Override
    public void tearDown() throws Exception {
        super.tearDown();        
    }

    // Test mainUIState right
    public void testcase01_Mp3Player() {
    	  Log.d(LOG_TAG, "start test & get instance.");
        Mp3Player mp3player = Mp3Player.getInstanceOfMp3Player();
        
        boolean play = mp3player.isPlaying();
        Log.d(LOG_TAG, "play == " + play);
        
        Log.d(LOG_TAG, "stopPlayer 1");
        mp3player.stopPlayer(); 	
        
        boolean idle = mp3player.isIdle();
        Log.d(LOG_TAG, "idle = " + idle);
        //mMp3PlayerMode = '1';
        
        boolean err = mp3player.isError();
        Log.d(LOG_TAG, "err = " + err);
        
        mMp3PlayerMode = '1';
        Log.d(LOG_TAG, "startPlayer --1");
        mp3player.startPlayer(mMp3PlayerMode);
        
        mMp3PlayerMode = '2';
        Log.d(LOG_TAG, "startPlayer --2");
        mp3player.startPlayer(mMp3PlayerMode);
        
        mMp3PlayerMode = '3';
        Log.d(LOG_TAG, "startPlayer --3");
        mp3player.startPlayer(mMp3PlayerMode);
        
        mMp3PlayerMode = '4';
        Log.d(LOG_TAG, "startPlayer --4");
        mp3player.startPlayer(mMp3PlayerMode);
        
        mMp3PlayerMode = '5';
        Log.d(LOG_TAG, "startPlayer --5");
        mp3player.startPlayer(mMp3PlayerMode);
        
        
        Log.d(LOG_TAG, "stopPlayer");
        mp3player.stopPlayer();
        
        Log.d(LOG_TAG, "onCompletion ");
        mp3player.onCompletion(null);
        
        Log.d(LOG_TAG, "onError ");
        mp3player.onError(null, 0, 0);
    }
}
