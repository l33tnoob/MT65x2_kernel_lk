/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package android.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.widget.LinearLayout;
import android.widget.RemoteViews.RemoteView;

@RemoteView
public class AlphaLinearLayout extends LinearLayout {

    // private static final Animation ANIMATION0 = new AlphaAnimation(0, 0.2f);
    // private static final Animation ANIMATION1 = new AlphaAnimation(0.2f,
    // 0.4f);
    // private static final Animation ANIMATION2 = new AlphaAnimation(0.4f,
    // 0.6f);
    // private static final Animation ANIMATION3 = new AlphaAnimation(0.6f,
    // 0.8f);
    // private static final Animation ANIMATION4 = new AlphaAnimation(0.8f, 1f);

    private static final Animation ANIMATION = new AlphaAnimation(0.2f, 1f);

    public AlphaLinearLayout(Context context) {
        super(context);
        init();
    }

    public AlphaLinearLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    @android.view.RemotableViewMethod
    public void startAlpha(int alph) {
        // switch(alph){
        // case 0: startAnimation(ANIMATION0);break;
        // case 1: startAnimation(ANIMATION1);break;
        // case 2: startAnimation(ANIMATION2);break;
        // case 3: startAnimation(ANIMATION3);break;
        // case 4: startAnimation(ANIMATION4);break;
        // default:
        // break;
        // }
        startAnimation(ANIMATION);
    }

    private void init() {
        // Util.check();
        // ANIMATION0.setDuration(200);
        // ANIMATION1.setDuration(200);
        // ANIMATION2.setDuration(200);
        // ANIMATION3.setDuration(200);
        // ANIMATION4.setDuration(200);
        // ANIMATION0.setFillAfter(true);
        // ANIMATION1.setFillAfter(true);
        // ANIMATION2.setFillAfter(true);
        // ANIMATION3.setFillAfter(true);
        // ANIMATION4.setFillAfter(true);
        // ANIMATION0.setFillBefore(true);
        // ANIMATION1.setFillBefore(true);
        // ANIMATION2.setFillBefore(true);
        // ANIMATION3.setFillBefore(true);
        // ANIMATION4.setFillBefore(true);
        ANIMATION.setDuration(800);
    }

}
