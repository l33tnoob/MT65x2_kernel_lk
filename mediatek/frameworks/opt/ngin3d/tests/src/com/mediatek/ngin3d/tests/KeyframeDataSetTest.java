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

import com.mediatek.ngin3d.animation.KeyframeData;
import com.mediatek.ngin3d.animation.KeyframeDataSet;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;


public class KeyframeDataSetTest extends Ngin3dTest{

    public void testKeyFrame() {
        KeyframeDataSet keyframeDataSet = new KeyframeDataSet();
        keyframeDataSet.setAnchor(1f, 1f);
        assertThat(keyframeDataSet.getAnchor().x, is(1f));
        assertThat(keyframeDataSet.getAnchor().y, is(1f));

        keyframeDataSet.setOpacity(10);
        assertThat(keyframeDataSet.getOpacity(), is(10));

        keyframeDataSet.setPosition(1f, 1f, 1f);
        assertThat(keyframeDataSet.getPosition().x, is(1f));
        assertThat(keyframeDataSet.getPosition().y, is(1f));
        assertThat(keyframeDataSet.getPosition().z, is(1f));

        keyframeDataSet.setRotation(1f, 1f, 1f);
        assertThat(keyframeDataSet.getRotation().x, is(1f));
        assertThat(keyframeDataSet.getRotation().y, is(1f));
        assertThat(keyframeDataSet.getRotation().z, is(1f));

        keyframeDataSet.setScale(1f, 1f);
        assertThat(keyframeDataSet.getScale().x, is(1f));
        assertThat(keyframeDataSet.getScale().y, is(1f));
    }

    public void testKeyFrameData() {
        KeyframeData keyframeData = new KeyframeData(10, 1, null);
        assertThat(keyframeData.getDelay(), is(1));
        assertThat(keyframeData.getDuration(), is(10));
    }
}
