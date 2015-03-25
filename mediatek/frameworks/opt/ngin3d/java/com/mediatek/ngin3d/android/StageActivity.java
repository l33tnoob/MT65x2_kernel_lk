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

package com.mediatek.ngin3d.android;

import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.animation.AnimationLoader;

/**
 * Represents an activity that shows a Stage as its content.
 */
public class StageActivity extends Activity {

    private static final String TAG = "StageActivity";

    /**
     * The stage of this activity.
     */
    protected Stage mStage;

    /**
     * A new stage view object for this activity.
     */
    protected StageView mStageView;

    /**
     * Gets the stage object of this object.
     *
     * @return the stage object.
     */
    public Stage getStage() {
        return mStage;
    }

    /**
     * Gets the stage view object of this object.
     *
     * @return the stage view object.
     */
    public StageView getStageView() {
        return mStageView;
    }

    /**
     * Initializes the Activity with a default StageView filling the screen.
     * This is the default behaviour of a StageActivity, and is convenient for
     * simple applications, where you do not need to extend the default
     * StageView, and have no need for addition views in your layout.
     *
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        onCreate(savedInstanceState, new StageView(this));
        setContentView(mStageView);
    }

    /**
     * Initializes the activity using a custom StageView.
     * In all but the most basic applications, you will likely want to extend
     * StageView and/or use an Android layout XML file.  This function allows
     * you to provide the layout ID of your StageView.  Make sure to call
     * setContentView() beforehand.
     *
     * @param savedInstanceState
     * @param stageViewId ID of StageView to control using this Activity
     */
    protected void onCreate(Bundle savedInstanceState, int stageViewId) {
        onCreate(savedInstanceState, (StageView) findViewById(stageViewId));
    }

    /**
     * Initializes the activity using a custom StageView.
     * In all but the most basic applications, you will likely want to extend
     * StageView and/or use an Android layout XML file.  This function allows
     * you to directly provide your own StageView.  This function will not call
     * setContentView() with the StageView.
     *
     * @param savedInstanceState
     * @param stageView StageView to control using this Activity
     */
    protected void onCreate(Bundle savedInstanceState, StageView stageView) {
        super.onCreate(savedInstanceState);
        Log.v(TAG, "onCreate");
        mStageView = stageView;
        mStage = mStageView.getStage();
        AnimationLoader.setCacheDir(getCacheDir());
    }

    /**
     * When pause this activity.
     */
    @Override
    protected void onPause() {
        Log.v(TAG, "onPause");
        mStageView.onPause();
        super.onPause();
    }

    /**
     * When resume this activity.
     */
    @Override
    protected void onResume() {
        super.onResume();
        Log.v(TAG, "onResume");
        mStageView.onResume();
    }

    /**
     * When the setting of this activity changed.
     *
     * @param newConfig
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mStageView.requestRender();
        Log.v(TAG, "onConfigurationChanged");
    }

}
