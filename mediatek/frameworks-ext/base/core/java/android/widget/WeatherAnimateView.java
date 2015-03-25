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

package android.widget;

import android.animation.Animator;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.animation.Animator.AnimatorListener;
import android.content.Context;
import android.graphics.Bitmap;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.RemoteViews.RemoteView;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

@RemoteView
public class WeatherAnimateView extends RelativeLayout {

    private static final String TAG = "WeatherAnimationView";

    /**
     * The property for setting the increment/decrement button alpha.
     */
    private static final String PROPERTY_IMAGEVIEW_ALPHA = "alpha";

    /**
     * The alpha for the increment/decrement ImageView when it is transparent.
     */
    private static final int IMAGEVIEW_ALPHA_TRANSPARENT = 0;

    /**
     * The alpha for the increment/decrement ImageView when it is opaque.
     */
    private static final int IMAGEVIEW_ALPHA_OPAQUE = 1;

    private static final int ANIM_DURATION = 800;

    private ImageView mOutView;

    private ImageView mInView;

    private Bitmap mLastRes = null;

    private Animator mFadeInAnimator;

    private Animator mFadeOutAnimator;

    public WeatherAnimateView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initAnimation();
    }

    public WeatherAnimateView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initAnimation();
    }

    public WeatherAnimateView(Context context) {
        super(context);
    }

    private void initAnimation() {
        mFadeInAnimator = ObjectAnimator.ofFloat(mInView, PROPERTY_IMAGEVIEW_ALPHA,
                IMAGEVIEW_ALPHA_TRANSPARENT, IMAGEVIEW_ALPHA_OPAQUE);
        mFadeInAnimator.setDuration(ANIM_DURATION);
        mFadeOutAnimator = ObjectAnimator.ofFloat(mOutView, PROPERTY_IMAGEVIEW_ALPHA,
                IMAGEVIEW_ALPHA_OPAQUE, IMAGEVIEW_ALPHA_TRANSPARENT);
        mFadeOutAnimator.setDuration(ANIM_DURATION);
    }

    protected void onFinishInflate() {
        super.onFinishInflate();
        Xlog.d(TAG, "onFinishInflate");
        mOutView = (ImageView) findViewWithTag("tag_weather_out_view");
        mInView = (ImageView) findViewWithTag("tag_weather_in_view");
        initAnimation();
    }

    @android.view.RemotableViewMethod
    public void setImageViewBitmap(final Bitmap res) {
        Xlog.d(TAG, "setImageViewBitmap mLastResId = " + mLastRes + ", new res=" + res);
        if (mLastRes == null) {
            mLastRes = res;
            mOutView.setVisibility(View.INVISIBLE);
            mInView.setImageBitmap(res);
            mInView.setVisibility(View.VISIBLE);
            mFadeInAnimator.setTarget(mInView);
            mFadeInAnimator.start();
        } else {
            ImageView tmpImageView = mOutView;
            mOutView = mInView;
            mInView = tmpImageView;
            tmpImageView = null;
            mFadeOutAnimator.addListener(new AnimatorListener() {

                @Override
                public void onAnimationCancel(Animator animation) {
                    Xlog.d(TAG, "onAnimationCancel");
                }

                @Override
                public void onAnimationEnd(Animator animation) {
                    Xlog.d(TAG, "onAnimationEnd");
                    mOutView.setVisibility(View.INVISIBLE);
                    mLastRes = res;
                    mInView.setImageBitmap(res);
                    mInView.setVisibility(View.VISIBLE);
                    mFadeInAnimator.setTarget(mInView);
                    mFadeInAnimator.start();
                }

                @Override
                public void onAnimationRepeat(Animator animation) {
                }

                @Override
                public void onAnimationStart(Animator animation) {
                }
            });
            mOutView.setImageBitmap(mLastRes);
            mOutView.setVisibility(View.VISIBLE);
            mFadeOutAnimator.setTarget(mOutView);
            mFadeOutAnimator.start();
        }
    }

    @android.view.RemotableViewMethod
    public void setLastRes(Bitmap res) {
        mLastRes = res;
    }
}
