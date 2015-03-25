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

package com.mediatek.ngin3d.tests;

import android.net.Uri;
import com.mediatek.ngin3d.Point;

import com.mediatek.ngin3d.Video;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.animation.Timeline;

public class VideoTest extends Ngin3dInstrumentationTestCase {
    private Video mVideo;
    private void waitForTime(PresentationStubActivity activity, int millisecond) throws InterruptedException {
        final int delta = 50;
        // Change to continuously render mode to trigger timeline
        activity.getStageView().setRenderMode(StageView.RENDERMODE_CONTINUOUSLY);
        Timeline time = new Timeline(millisecond + delta);
        time.start();
        while (time.isStarted()) {
            try {
                Thread.sleep(30);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        activity.getStageView().setRenderMode(StageView.RENDERMODE_WHEN_DIRTY);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mVideo = Video.createFromVideo(getActivity(), Uri.parse("android.resource://com.mediatek.ngin3d.tests/" + R.raw.gg_taeyeon), 240, 160);
        mStage.add(mVideo);
        mVideo.setPosition(new Point(0.2f, 0.2f, true));
        mStageView.waitSurfaceReady();
    }

    public void testVideoLooping() throws InterruptedException {
        PresentationStubActivity activity = getActivity();

        assertEquals(false, mVideo.isPlaying());
        waitForTime(activity, 2000);
        mVideo.setLooping(true).play();
        waitForTime(activity, 2000);
        assertEquals(true, mVideo.isPlaying());
        waitForTime(activity, 10000);
        assertEquals(true, mVideo.isPlaying());
        mVideo.pause();
        waitForTime(activity, 500);
        assertEquals(false, mVideo.isPlaying());
    }

    public void testVideoPlaying() throws InterruptedException {
        PresentationStubActivity activity = getActivity();

        assertEquals(false, mVideo.isPlaying());
        waitForTime(activity, 2000);
        mVideo.play();
        waitForTime(activity, 2000);
        assertEquals(true, mVideo.isPlaying());
        mVideo.play();
        waitForTime(activity, 500);
        assertEquals(true, mVideo.isPlaying());
        mVideo.pause();
        waitForTime(activity, 500);
        assertEquals(false, mVideo.isPlaying());
    }
}
