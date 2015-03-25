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

import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.ngin3d.animation.Alpha;
import com.mediatek.ngin3d.animation.Mode;
import com.mediatek.ngin3d.animation.Timeline;
import junit.framework.TestCase;

public class AlphaTest extends TestCase {

    @SmallTest
    public void testDefaultValue() {
        Alpha alpha = new Alpha();
        assertEquals(0.0f, alpha.getAlpha());
        assertEquals(Mode.LINEAR, alpha.getMode());

        Timeline timeline1 = new Timeline(2000);
        alpha.setTimeline(timeline1);
        assertEquals(timeline1, alpha.getTimeline());

        Timeline timeline2 = new Timeline(2000);
        alpha.setTimeline(timeline2);
        assertEquals(timeline2, alpha.getTimeline());
    }

    public void testAlphaMode() {
        Timeline timeline = new Timeline(2000);
        Alpha alpha = new Alpha(timeline, Mode.LINEAR);
        float delta = 0.001f;
        float p = 0;
        timeline.setProgress(p);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_QUAD);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_QUAD);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_QUAD);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_CUBIC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_CUBIC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_QUART);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_QUART);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_QUINT);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_QUINT);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_QUINT);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_SINE);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_SINE);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_OUT_SINE);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_EXPO);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_OUT_EXPO);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_OUT_EXPO);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_CIRC);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_OUT_CIRC);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_OUT_CIRC);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_ELASTIC);
        assertEquals(alpha.getAlpha(), p, delta);

        alpha.setMode(Mode.EASE_OUT_ELASTIC);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_OUT_ELASTIC);
        assertEquals(alpha.getAlpha(), p, delta);

        alpha.setMode(Mode.EASE_IN_BACK);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_OUT_BACK);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_OUT_BACK);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_BOUNCE);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_OUT_BOUNCE);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        alpha.setMode(Mode.EASE_IN_OUT_BOUNCE);
        assertEquals(Math.abs(alpha.getAlpha()), p);

        p = 1.0f;
        timeline.setProgress(p);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_QUAD);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_QUAD);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_QUAD);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_CUBIC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_CUBIC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_QUART);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_QUART);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_QUINT);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_QUINT);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_QUINT);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_SINE);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_SINE);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_SINE);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_EXPO);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_EXPO);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_EXPO);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_CIRC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_CIRC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_CIRC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_ELASTIC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_ELASTIC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_ELASTIC);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_BACK);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_BACK);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_BACK);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_BOUNCE);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_OUT_BOUNCE);
        assertEquals(alpha.getAlpha(), p);

        alpha.setMode(Mode.EASE_IN_OUT_BOUNCE);
        assertEquals(alpha.getAlpha(), p);

    }


}
