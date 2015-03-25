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

import android.app.Activity;
import android.os.Bundle;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.android.StageView;

/**
 * Long String ellipsize demo
 */
public class LongStringDemo extends Activity {

    private Stage mStage = new Stage();
    private StageView mStageView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mStageView = new StageView(this, mStage);
        setContentView(mStageView);

        // Normal text
        Text normal = new Text("Normal");
        normal.setPosition(new Point(0.42f, 0.06f, true));
        normal.setTextSize(12);

        Text hello1 = new Text("Hello World, Hello MAGE");
        // adding a background to check bounding box
        hello1.setBackgroundColor(new Color(255, 0, 0, 128));
        hello1.setPosition(new Point(0.42f, 0.1f, true));

        // The text of limited width
        Text limitedWidth = new Text("Text with limited width");
        limitedWidth.setPosition(new Point(0.42f, 0.18f, true));
        limitedWidth.setTextSize(12);

        Text hello2 = new Text("Hello World, Hello Ngin3d, Hello MAGE");
        // adding a background to check bounding box
        hello2.setBackgroundColor(new Color(255, 0, 0, 128));
        hello2.setPosition(new Point(0.42f, 0.28f, true));
        hello2.setMaxWidth(250);

        // The text of limited width / lines
        Text limitedWidthLines = new Text("Text with limited width and lines");
        limitedWidthLines.setPosition(new Point(0.42f, 0.42f, true));
        limitedWidthLines.setTextSize(12);

        Text hello3 = new Text("Hello World, Hello Ngin3d, Hello MAGE");
        // adding a background to check bounding box
        hello3.setBackgroundColor(new Color(255, 0, 0, 128));
        hello3.setPosition(new Point(0.42f, 0.48f, true));
        hello3.setMaxWidth(250);
        hello3.setMaxLines(2);

        // The text of limited width / lines and ellipsize by fadeout
        Text ellipsizeByFadeout = new Text("Single Text ellipsize by fadeout");
        ellipsizeByFadeout.setPosition(new Point(0.42f, 0.61f, true));
        ellipsizeByFadeout.setTextSize(12);

        Text hello4 = new Text("Hello World, Hello Ngin3d, Hello MAGE");
        // adding a background to check bounding box
        hello4.setBackgroundColor(new Color(255, 0, 0, 128));
        hello4.setPosition(new Point(0.42f, 0.66f, true));
        hello4.setMaxWidth(250);
        hello4.setMaxLines(1);


        // The text of limited width / lines and ellipsize by 3 dot
        Text ellipsizeBy3Dot = new Text("Single Text ellipsize by 3 dot (...)");
        ellipsizeBy3Dot.setPosition(new Point(0.42f, 0.79f, true));
        ellipsizeBy3Dot.setTextSize(12);
        Text hello5 = new Text("Hello World, Hello Ngin3d, Hello MAGE");
        // adding a background to check bounding box
        hello5.setBackgroundColor(new Color(255, 0, 0, 128));
        hello5.setPosition(new Point(0.42f, 0.84f, true));
        hello5.setMaxWidth(250);
        hello5.setMaxLines(1);
        hello5.setEllipsizeStyle(Text.ELLIPSIZE_BY_3DOT);

        mStage.add(hello1);
        mStage.add(hello2);
        mStage.add(hello3);
        mStage.add(hello4);
        mStage.add(hello5);

        mStage.add(normal);
        mStage.add(limitedWidth);
        mStage.add(limitedWidthLines);
        mStage.add(ellipsizeByFadeout);
        mStage.add(ellipsizeBy3Dot);

    }

    @Override
    protected void onPause() {
        mStageView.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mStageView.onResume();
    }

}
