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
import android.content.Intent;
import android.util.AttributeSet;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.widget.ImageView;
import android.widget.RemoteViews.RemoteView;
import android.appwidget.AppWidgetManager;
import android.os.SystemClock;

@RemoteView
public class GestureImageView extends ImageView implements
        GestureDetector.OnGestureListener {

    private static final String TAG = "GestureImageView";
    private GestureDetector gestureDetector;
    private int mAppWidgetId;
    private static final String ACTION_SCROLL = "com.weather.action.SCROLL";
    private static final String ACTION_WEATHER_SETTING = "com.weather.action.SETTING";
    private static final String DIRECTION = "Dircetion";
    private static final String DIRECTION_NEXT = "direction_next";
    private static final String DIRECTION_PREVIOUS = "direction_previous";
    private long sendTime;

    public GestureImageView(Context context) {
        super(context);
        init(context);
    }

    public GestureImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public GestureImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(context);
    }

    @android.view.RemotableViewMethod
    public void setWidgetId(int widgetId) {
        mAppWidgetId = widgetId;
    }

    private void init(Context context) {
        gestureDetector = new GestureDetector(context, this);

    }

    public boolean onTouchEvent(MotionEvent event) {
        return gestureDetector.onTouchEvent(event);

    }

    public boolean onDown(MotionEvent e) {
        return true;
    }

    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
            float velocityY) {
        if (SystemClock.elapsedRealtime() - sendTime < 1000) {
            return true;
        }
        Intent intent = new Intent(ACTION_SCROLL);
        intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetId);
        if (velocityY > 0) {
            intent.putExtra(DIRECTION, DIRECTION_NEXT);
        } else {
            intent.putExtra(DIRECTION, DIRECTION_PREVIOUS);
        }
        getContext().sendBroadcast(intent);
        sendTime = SystemClock.elapsedRealtime();
        return true;
    }

    public void onLongPress(MotionEvent e) {

    }

    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
            float distanceY) {
      
        if (SystemClock.elapsedRealtime() - sendTime < 1000) {
            return true;
        }
        float x = Math.abs(distanceX);
        float y = Math.abs(distanceY);
        if (x > y) {
            return true;
        }
        getParent().requestDisallowInterceptTouchEvent(true);
        if (x > 5 && y > 7) {
            Intent intent = new Intent(ACTION_SCROLL);
            intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetId);
            if (distanceY < 0) {
                intent.putExtra(DIRECTION, DIRECTION_NEXT);
            } else {
                intent.putExtra(DIRECTION, DIRECTION_PREVIOUS);
            }
            getContext().sendBroadcast(intent);
            sendTime = SystemClock.elapsedRealtime();
            return true;
        }
        return false;
    }

    public void onShowPress(MotionEvent e) {

    }

    public boolean onSingleTapUp(MotionEvent e) {
        //Add for WeatherClockWidget
        Intent intent = new Intent(ACTION_WEATHER_SETTING);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getContext().startActivity(intent);

        return false;
    }

}

