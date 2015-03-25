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

import android.animation.AnimatorSet;
import android.test.InstrumentationTestCase;
import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.animation.AnimationLoader;
import com.mediatek.ngin3d.animation.KeyframeAnimator;

import static android.test.MoreAsserts.assertNotEqual;

public class KeyframeAnimatorTest extends InstrumentationTestCase {

    public void testKeyFrameAnimator() {
        Actor actor = new Empty();
        KeyframeAnimator animator = AnimationLoader.loadAnimator(getInstrumentation().getContext()
            , R.raw.photo_swap_enter_right_photo1_ani, actor);
        animator.setCurrentPlayTime(0);

        Point startPos = new Point(actor.getPosition());
        Quaternion q = actor.getRotation().getQuaternion();
        Rotation startRot = new Rotation(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3());

        animator.setCurrentPlayTime(animator.getDuration());
        Point endPos = actor.getPosition();
        Rotation endRot = actor.getRotation();
        assertNotEqual(startPos, endPos);
        assertNotEqual(startRot, endRot);

    }

    public void testAnimatorSet() {
        Actor actor1 = new Empty();
        Actor actor2 = new Empty();
        KeyframeAnimator animator1 = AnimationLoader.loadAnimator(getInstrumentation().getContext()
            , R.raw.photo_swap_enter_right_photo1_ani, actor1);
        KeyframeAnimator animator2 = AnimationLoader.loadAnimator(getInstrumentation().getContext()
            , R.raw.photo_swap_enter_right_photo2_ani, actor2);


        Point startPos1 = new Point(actor1.getPosition());
        Quaternion q = actor1.getRotation().getQuaternion();
        Rotation startRot1 = new Rotation(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3());

        Point startPos2 = new Point(actor1.getPosition());
        q = actor1.getRotation().getQuaternion();
        Rotation startRot2 = new Rotation(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3());

        AnimatorSet animatorSet = new AnimatorSet();
        animatorSet.playTogether(animator1, animator2);
        animatorSet.start();
        animatorSet.end();

        Point endPos1 = actor1.getPosition();
        Rotation endRot1 = actor1.getRotation();
        Point endPos2 = actor2.getPosition();
        Rotation endRot2 = actor2.getRotation();
        assertNotEqual(startPos1, endPos1);
        assertNotEqual(startRot1, endRot1);
        assertNotEqual(startPos2, endPos2);
        assertNotEqual(startRot2, endRot2);
    }
}
