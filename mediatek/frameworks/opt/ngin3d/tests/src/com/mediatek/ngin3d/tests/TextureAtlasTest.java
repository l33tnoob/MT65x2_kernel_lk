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

import android.content.res.Resources;
import android.test.ActivityInstrumentationTestCase2;
import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Property;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.TextureAtlas;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.ImageSource;
import com.mediatek.ngin3d.presentation.PresentationEngine;

/**
 * Add description here.
 */
public class TextureAtlasTest extends ActivityInstrumentationTestCase2<PresentationStubActivity> {
    protected Stage mStage;
    private StageView mStageView;
    protected PresentationEngine mPresentationEngine;

    public TextureAtlasTest() {
        super(PresentationStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mStageView = getActivity().getStageView();
        mStageView.waitSurfaceReady();
        mPresentationEngine = mStageView.getPresentationEngine();
        mStage = mStageView.getStage();
    }

    @SmallTest
    public void testLoading() {
        final PresentationStubActivity activity = getActivity();
        activity.runOnUiThread(new Runnable() {
            public void run() {
                try {
                    TextureAtlas.getDefault().add(activity.getResources(), 0, 0);
                    fail("Should throw exception.");
                } catch (Resources.NotFoundException e) {
                    // expected
                }
            }
        });
        getInstrumentation().waitForIdleSync();
    }

    public void testTextureAtlasMapping() throws InterruptedException {
        Resources resources = getActivity().getResources();
        mStage.addTextureAtlas(resources, R.raw.media3d_altas, R.raw.media3d_atlas_res);
        mStage.addTextureAtlas(resources, "media3d_atlas.pvr", R.raw.media3d_atlas_asset);
        Image image1 = Image.createFromResource(resources, R.drawable.icon_video_frame);
        Image image2 = Image.createFromResource(resources, R.drawable.video_background);
        Image image3 = Image.createFromResource(resources, R.drawable.android);

        Thread.sleep(5000);

        // The image from TextureAtlas which is stored in resource
        Property property1 = image1.getProperty("image_source");
        ImageSource src1 = (ImageSource)image1.getValue(property1);
        assertEquals(src1.srcType, ImageSource.RES_ID);
        assertEquals(((ImageDisplay.Resource)src1.srcInfo).resId, R.raw.media3d_altas);

        // The image from TextureAtlas which is stored in asset
        Property property2 = image2.getProperty("image_source");
        ImageSource src2 = (ImageSource)image2.getValue(property2);
        assertEquals(src2.srcType, ImageSource.ASSET);
        assertEquals((String)src2.srcInfo, "media3d_atlas.pvr");

        // The image from normal resource
        Property property3 = image3.getProperty("image_source");
        ImageSource src3 = (ImageSource)image3.getValue(property3);
        assertEquals(src3.srcType, ImageSource.RES_ID);
        assertEquals(((ImageDisplay.Resource)src3.srcInfo).resId, R.drawable.android);

        TextureAtlas.getDefault().cleanup();
    }
}
