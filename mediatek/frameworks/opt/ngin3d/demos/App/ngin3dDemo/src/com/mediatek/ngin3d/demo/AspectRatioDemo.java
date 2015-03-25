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

import android.os.Bundle;
import com.mediatek.ngin3d.Dimension;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationLoader;
import com.mediatek.ngin3d.demo.R;

/**
 * Add description here.
 */
public class AspectRatioDemo extends StageActivity {
    private static final String TAG = "AspectRatioDemo";

    private static final int[] PHOTO_FRAMES = new int[] {
        R.drawable.earth,
        R.drawable.danger,
        R.drawable.icon_nightcloud,
        R.drawable.photo_04,
    };

    private static final int[] ANIMATIONS = new int[] {
        R.raw.photo_next_enter_photo1_ani, R.raw.photo_next_enter_photo2_ani,
        R.raw.photo_next_enter_photo3_ani, R.raw.photo_next_enter_photo4_ani
    };

    private static final int PHOTO_PER_PAGE = 4;
    Image[] photo = new Image[PHOTO_PER_PAGE];
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Animation.Listener l = new Animation.Listener() {
            public void onStarted(Animation animation) {
                android.util.Log.v(TAG, "onStarted" + animation);
            }

            public void onCompleted(Animation animation) {
                android.util.Log.v(TAG, "onCompleted" + animation);
                for(int i = 0 ; i < PHOTO_PER_PAGE ; i ++)
                    photo[i].setKeepAspectRatio(true);
            }
        };

        for (int i = 0; i < PHOTO_PER_PAGE; i++) {
            photo[i] = Image.createFromResource(getResources(), PHOTO_FRAMES[i]);
            photo[i].setSize(new Dimension(600,200));
            photo[i].setKeepAspectRatio(false);
            mStage.add(photo[i]);
            Animation animation = AnimationLoader.loadAnimation(this, ANIMATIONS[i]);
            animation.setTarget(photo[i]);
            animation.addListener(l);
            animation.start();
        }
    }
}
