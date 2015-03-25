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

package com.mediatek.ngin3d.demo;

import android.net.Uri;
import android.os.Bundle;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Video;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.animation.PropertyAnimation;
import com.mediatek.ngin3d.demo.R;

public class VideoTextureDemo extends StageActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Video video1 = Video.createFromVideo(this, Uri.parse("android.resource://com.mediatek.ngin3d.demo/" + R.raw.gg_taeyeon), 240, 160);
        Video video2 = Video.createFromVideo(this, Uri.parse("android.resource://com.mediatek.ngin3d.demo/" + R.raw.gg_hyoyeon), 240, 160);
        Video video3 = Video.createFromVideo(this, Uri.parse("android.resource://com.mediatek.ngin3d.demo/" + R.raw.gg_jessica), 240, 160);
        Video video4 = Video.createFromVideo(this, Uri.parse("android.resource://com.mediatek.ngin3d.demo/" + R.raw.weather_video), 240, 160);

        mStage.add(video1);
        video1.setPosition(new Point(0.2f, 0.2f, true));
        new PropertyAnimation(video1, "rotation", new Rotation(0, 0, 0), new Rotation(0, 0, 360)).setDuration(7000).setLoop(true).start();

        mStage.add(video2);
        video2.setPosition(new Point(0.4f, 0.4f, true));
        new PropertyAnimation(video2, "rotation", new Rotation(0, 0, 30), new Rotation(0, 0, 390)).setDuration(7000).setLoop(true).start();

        mStage.add(video3);
        video3.setPosition(new Point(0.6f, 0.6f, true));
        new PropertyAnimation(video3, "rotation", new Rotation(0, 0, 60), new Rotation(0, 0, 420)).setDuration(7000).setLoop(true).start();

        mStage.add(video4);
        video4.setPosition(new Point(0.8f, 0.8f, true));
        new PropertyAnimation(video4, "rotation", new Rotation(0, 0, 90), new Rotation(0, 0, 450)).setDuration(7000).setLoop(true).start();

        video1.setLooping(true).play();
        video2.setLooping(true).play();
        video3.setLooping(true).play();
        video4.setLooping(true).play();
    }

}
