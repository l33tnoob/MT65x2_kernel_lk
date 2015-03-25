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

// class models the music player state machine
public class StatePattern {
    // transition table
    private String[][] context = {
            // Space added in trackText is workaround for Redmine bug #525.
            // Please do not remove unless the blurry issue is fixed in ngin3d.
            {
                    "Moneybrother", "We Die Only Once ", "monybrother.jpg"
            },
            {
                    "Coldplay", "Clocks", "coldplay.jpg"
            },
            {
                    "Daft Punk", " Da Funk ", "daft_punk.jpg"
            },
            {
                    "Deportees", " When They Come ", "deportees.jpg"
            },
            {
                    "James Blake", "Limit To Your Love", "james_blake.jpg"
            },
            {
                    "Radiohead", "Fake Plastic Trees ", "radio_head.jpg"
            },
            {
                    "Britney Spears", "I Wanna Go", "britney_spears.jpg"
            },
            {
                    "Robyn", "    Hang With Me    ", "robyn.jpg"
            },
            {
                    "Pink Floyd", "  Eclipse   ", "pink_floyd.jpg"
            }
    };

    private int mCurrent;
    private int mPrevious;
    private boolean mSwipeUp;
    private boolean mSwipeDirChange;

    public StatePattern() {
        mCurrent = 1;
        mPrevious = 0;
        mSwipeUp = true;
        mSwipeDirChange = false;
    }

    public void updateState() {
        mPrevious = mCurrent;
        if (mSwipeUp)
            mCurrent = (Integer) ((mCurrent < (context.length) - 1) ? mCurrent + 1 : 0);
        else
            mCurrent = (Integer) ((mCurrent > 0) ? mCurrent - 1 : (context.length) - 1);
    }

    public boolean getSwipeState() {
        return mSwipeUp;
    }

    public String[] getContext(int index) {
        return context[index];
    }

    public int getContextLength() {
        return context.length;
    }

    public int getCurrent() {
        if (ismSwipeDirChange()) {
            if (mSwipeUp) {
                // swipe direction change from down to up. Move mCurrent + 2
                mCurrent = (Integer) ((mCurrent < (context.length) - 1) ? mCurrent + 1 : 0);
                mCurrent = (Integer) ((mCurrent < (context.length) - 1) ? mCurrent + 1 : 0);
            } else {
                // swipe direction change from up to down. Move mCurrent - 2
                mCurrent = (Integer) ((mCurrent > 0) ? mCurrent - 1 : (context.length) - 1);
                mCurrent = (Integer) ((mCurrent > 0) ? mCurrent - 1 : (context.length) - 1);
            }
        }
        return mCurrent;
    }

    public int getPrevious() {
        return mPrevious;
    }

    public void setSwipeState(boolean state) {
        if (mSwipeUp == state) {
            setmSwipeDirChange(false);
        } else {
            setmSwipeDirChange(true);
        }
        mSwipeUp = state;
    }

    public void setmSwipeDirChange(boolean mSwipeDirChange) {
        this.mSwipeDirChange = mSwipeDirChange;
    }

    public boolean ismSwipeDirChange() {
        return mSwipeDirChange;
    }
}
