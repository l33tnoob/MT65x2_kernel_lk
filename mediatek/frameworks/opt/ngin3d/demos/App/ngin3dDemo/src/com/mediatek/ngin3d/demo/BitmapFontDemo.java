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
import com.mediatek.ngin3d.*;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.demo.R;


public class BitmapFontDemo extends StageActivity {
    private static final String TAG = "BmFontDemo";

    public void onCreate(Bundle savedInstanceState) {
        // setting the bitmap font for FPS indicator
        BitmapFont font = new BitmapFont(getResources(), R.raw.bmfont, R.drawable.bmfont);
        BitmapText.setDefaultFont(font);
        super.onCreate(savedInstanceState);

        // Use orthographic projection
        mStage.setProjection(Stage.ORTHOGRAPHIC, -1.0f, 1.0f, 0.0f);
        mStage.setCameraWidth(300);
        mStage.setCamera(new Point(240, 400, 0), new Point(240, 400, -1));

        // Rotate screen so that origin is in top-left corner.
        // Text and images are upside-down and facing away from the camera by
        // default, so rotating them puts them the right way up, and causes
        // them to face towards the camera.
        Container screen = new Container();
        screen.setPosition(new Point(0, 800, 0));
        screen.setRotation(new Rotation(180, 0, 0));
        mStage.add(screen);

        // Setting a string using BitmapFont
        BitmapFont font3 = new BitmapFont(getResources(), R.raw.bmfont1, R.drawable.bmfont1);
        BitmapText text = new BitmapText("MediaTek", font3);
        text.setPosition(new Point(240, 200));
        screen.add(text);

        // setting a string using anther Bitmap Font
        BitmapFont font2 = new BitmapFont(getResources(), R.raw.bmfont2, R.drawable.bmfont2);
        BitmapText text2 = new BitmapText("Android", font2);
        text2.setPosition(new Point(240, 300));
        screen.add(text2);

        // Setting a string using system text
        Text text3 = new Text("MediaTek");
        text3.setTextColor(Color.WHITE);
        text3.setTextSize(40f);
        text3.setPosition(new Point(240, 400));
        screen.add(text3);

    }
}
