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
import com.mediatek.ngin3d.animation.MasterClock;
import com.mediatek.ngin3d.animation.Timeline;
import junit.framework.TestCase;

public class MasterClockTest extends TestCase {

    @SmallTest
    public void testDefaultValues() {
        Double timeScale = MasterClock.getDefault().getTimeScale();
        assertEquals(1.0, timeScale);
    }

    @SmallTest
    public void testTimeScale() {
        try {
            MasterClock.getDefault().setTimeScale(-1.0);
            fail("Should throw exception when timescale is negative");
        } catch (IllegalArgumentException e) {
            // expected
        }

        MasterClock.getDefault().setTimeScale(2.0);
        assertEquals(2.0, MasterClock.getDefault().getTimeScale());

        // Set time scale back to 1 or it will affect other tests
        MasterClock.getDefault().setTimeScale(1.0);
    }

    @SmallTest
    public void testRegister() {
        Timeline timeline = new Timeline(1000);
        MasterClock.register(timeline);
        MasterClock.unregister(timeline);
        MasterClock.register(timeline);
        MasterClock.getDefault().tick();
        assertTrue("Timeline should be registered", MasterClock.getDefault().isTimelineRegistered(timeline));

        MasterClock.unregister(timeline);
        MasterClock.register(timeline);
        MasterClock.unregister(timeline);
        MasterClock.getDefault().tick();
        assertFalse("Timeline should already be unregistered", MasterClock.getDefault().isTimelineRegistered(timeline));

        MasterClock.register(timeline);
        MasterClock.unregister(timeline);
        MasterClock.register(timeline);
        MasterClock.getDefault().tick();
        MasterClock.cleanup();
        assertFalse("Timeline should be cleaned", MasterClock.getDefault().isTimelineRegistered(timeline));
    }
}
