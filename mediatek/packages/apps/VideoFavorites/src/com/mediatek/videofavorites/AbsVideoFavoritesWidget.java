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
 */

package com.mediatek.videofavorites;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.widget.RemoteViews;

import com.mediatek.xlog.Xlog;

/**
 *  The abstraction class of applicaiton widget provider of Videofavorites widget.
 *
 */
public abstract class AbsVideoFavoritesWidget extends AppWidgetProvider {

    private static final String TAG = "AbsVideoFavoritesWidget";

    public static final String ACTION_REFRESH = "com.mediatek.videofavorites.REFRESH";

    static HandlerThread sWorkerThread;
    static Handler sWorkerQueue;

    public AbsVideoFavoritesWidget() {
        // do nothing here, we init worker queue and handler thread when widget is enabled.
    }


    /**
     *  For childs to implement their remoteviews.
     *
     *  @param c Context
     *  @return a remote view for updating widget
     */
    protected abstract RemoteViews update(Context c);

    /**
     *  For childs to return the widget Collection View id
     *
     *  @return a remote view widget ID
     */
    protected abstract int getCollectionViewId();

    /**
     *  Get the pending intent for launching video recorder.
     *
     *  @param c  Context for getting the intent
     *  @return The PendingIntent which will launch the recorder
     */
    protected PendingIntent getRecordPendingIntent(Context c) {
        Intent i = new Intent(c, WidgetActionActivity.class);
        i.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        i.setAction(WidgetActionActivity.ACTION_LAUNCH_RECORDER);
        return PendingIntent.getActivity(c, 0, i, PendingIntent.FLAG_UPDATE_CURRENT);
    }

    ComponentName mComponentName;
    protected ComponentName getComponentName(Context c) {
        if (mComponentName == null) {
            if (c != null) {
                mComponentName = new ComponentName(c, this.getClass().getName());
            }
        }
        return mComponentName;
    }

    private void prepareWorkerQueue() {
        if (sWorkerQueue != null) {
            return;
        }

        if (sWorkerThread == null) {
            sWorkerThread = new HandlerThread("VideoFavoritesWidget-worker");
            sWorkerThread.start();
        }
        sWorkerQueue = new Handler(sWorkerThread.getLooper());

    }

    @Override
    public void onUpdate(Context c, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        super.onUpdate(c, appWidgetManager, appWidgetIds);

        final int n = appWidgetIds.length;

        for (int i = 0; i < n; i++) {
            int appWidgetId = appWidgetIds[i];
            RemoteViews views = update(c);
            appWidgetManager.updateAppWidget(appWidgetId, views);
            Xlog.v(TAG, "appwidgetId: " + appWidgetId);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();
        Xlog.w(TAG, "onReceive(): " + action);
        if (action.equals(ACTION_REFRESH)/* ||
                (action.equals(Intent.ACTION_SETTINGS_PACKAGE_DATA_CLEARED) &&
                 context.getPackageName().equals(intent.getStringExtra("packageName")))*/) {
            // According to Intent.ACTION_PACKAGE_DATA_CLEARED, the application being cleared
            // won't receive this broadcast.
            // Therefore a new intent Intent.ACTION_SETTINGS_PACKAGE_DATA_CLEARED is created
            // for cases where the user selected "clear data" in settings to clear widget data.
            // The status of Widget can be updated by handling this broadcast.
            final AppWidgetManager apm = AppWidgetManager.getInstance(context);
            apm.notifyAppWidgetViewDataChanged(apm.getAppWidgetIds(getComponentName(context)),
                                               getCollectionViewId());
        }
        super.onReceive(context, intent);
    }

}
