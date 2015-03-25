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

import com.mediatek.ngin3d.BitmapFont;
import com.mediatek.ngin3d.BitmapText;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Text;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;

public class BitmapTextTest extends Ngin3dInstrumentationTestCase {
    private PresentationStubActivity mActivity;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
    }

    public void testBmText() {
        Point p1 = new Point(1f, 0f);

        BitmapFont font = new BitmapFont(mActivity.getResources(), R.raw.bmfont, R.drawable.bmfont);
        BitmapText.setDefaultFont(font);
        assertEquals(font, BitmapText.getDefaultFont());
        Text text = new Text();
        text.setAnchorPoint(p1);
        assertEquals(p1, text.getAnchorPoint());
        text.setText("Test");
        assertThat(text.getText(), is("Test"));

        BitmapText text2 = new BitmapText("Test2");
        assertThat(text2.getText(), is("Test2"));
        BitmapFont font2 = new BitmapFont(mActivity.getResources(), R.raw.bmfont, R.drawable.bmfont);
        text2.setFont(font2);
        assertEquals(font2, text2.getFont());
        assertThat(text2.getText(), is("Test2"));
        text2.setAnchorPoint(p1);
        assertEquals(p1, text2.getAnchorPoint());

        BitmapFont font3 = new BitmapFont(mActivity.getResources(), R.raw.bmfont, R.drawable.bmfont);
        BitmapText text3 = new BitmapText("Test3", font3);
        assertEquals(font3, text3.getFont());
        assertThat(text3.getText(), is("Test3"));

        Text text4 = new Text("string");
        text4.setText("string2");
        assertThat(text4.getText(), is("string2"));

        Color c = new Color(0, 0, 0);
        text4.setBackgroundColor(c);
        assertEquals(c, text4.getBackgroundColor());
        text4.setTextColor(c);
        assertEquals(c, text4.getTextColor());
        text4.setTextSize(32f);
        assertThat(text4.getTextSize(), is(32f));
        text4.setTypeface(null);
        assertNull(text4.getTypeface());
        text4.setShadowLayer(2f, 1f, 1f, 100);
        assertThat(text4.getShadowLayer().radius, is(2f));
        assertThat(text4.getShadowLayer().color, is(100));
        assertThat(text4.getShadowLayer().dx, is(1f));
        assertThat(text4.getShadowLayer().dy, is(1f));
    }
}
