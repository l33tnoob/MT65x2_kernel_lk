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
import com.mediatek.ngin3d.Color;
import junit.framework.TestCase;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.lessThan;

/**
 * Add description here.
 */
public class ColorTest extends TestCase {

    @SmallTest
    public void testDefaultValue() {
        Color c = new Color(0, 1, 2);
        assertEquals(255, c.alpha);
        assertEquals(0, c.red);
        assertEquals(1, c.green);
        assertEquals(2, c.blue);

        Color c2 = c.copy();
        assertEquals(c, c2);
    }

    public void testColorSetting() {
        Color c = new Color(0, 1, 2, 0);
        Color tmpColor = c.red(1);
        assertThat(tmpColor.red, is(1));
        tmpColor = c.green(0);
        assertThat(tmpColor.green, is(0));
        tmpColor = c.blue(1);
        assertThat(tmpColor.blue, is(1));
        tmpColor = c.alpha(1);
        assertThat(tmpColor.alpha, is(1));
    }

    public void testLight() {
        Color c = new Color(1, 1, 1);
        Color tmpColor = c.brighter();
        assertThat(tmpColor.red, greaterThan(1));
        assertThat(tmpColor.green, greaterThan(1));
        assertThat(tmpColor.blue, greaterThan(1));

        Color c1 = new Color(100, 100, 100);
        tmpColor = c1.darker();
        assertThat(tmpColor.red, lessThan(100));
        assertThat(tmpColor.green, lessThan(100));
        assertThat(tmpColor.blue, lessThan(100));
    }

    public void testHLS() {
        Color c = new Color(1, 1, 1);
        c.setHls(0f, 0.1f, 0f);
        assertThat(c.red, is(26));

        c.setHls(1.2f, 0.5f, 0.8f);
        float h = (1.2f - (float) Math.floor(1.2f)) * 6.0f;
        float f = h - (float) Math.floor(h);
        float p = 0.5f * (1.0f - 0.8f);
        float q = 0.5f * (1.0f - 0.8f * f);
        float r = (int) (q * 255.0f + 0.5f);
        float g = (int) (0.5f * 255.0f + 0.5f);
        float b = (int) (p * 255.0f + 0.5f);

        assertThat(c.red, is((int)r));
        assertThat(c.green, is((int)g));
        assertThat(c.blue, is((int)b));
    }

}
