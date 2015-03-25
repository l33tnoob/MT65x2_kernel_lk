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

import android.app.Activity;
import android.appwidget.AppWidgetHost;
import android.appwidget.AppWidgetHostView;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProviderInfo;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.FrameLayout;

import com.mediatek.xlog.Xlog;

import java.util.ArrayList;


/**
 *  <p>
 *  This activity is used for instrument testing your application widget
 *  It provides a simple acivity which you could bind your widget to.
 *  Therefore it is possible to do instrument test the app widget via ActivityTestCases
 *  <p>
 *  Simply override {@link getWidgetLabel} and return your AppWidget Label to add it to this
 *  Activity
 *
 */
public abstract class WidgetHostActivity extends Activity {

    private static final String TAG = "WidgetHostActivity";
    private static final int HOST_ID = 777;        // a random fake id;

    private AppWidgetManager mAppWidgetManager;
    private AppWidgetHost mAppWidgetHost;
    private int mAppWidgetId;


    private AppWidgetProviderInfo getAppWidgetInfoByLabel(
        AppWidgetManager mgr, String label) {
        if (label == null) {
            return null;
        }
        ArrayList<AppWidgetProviderInfo> infos;
        infos = (ArrayList<AppWidgetProviderInfo>) mgr.getInstalledProviders();
        final int size = infos.size();
        AppWidgetProviderInfo info;
        for (int i = 0; i < size; i++) {
            info = infos.get(i);
            if (label.equals(info.label)) {
                return info;
            }
        }

        Xlog.e(TAG, "AppWidget not found: " + label);
        return null;
    }

    /**
    *  Override this function and return your widget Label to bind it to this activity
    */
    protected abstract String getWidgetLabel();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // TODO: considering adding keyguard disable method here.

        setContentView(R.layout.widget_host_main);

        mAppWidgetManager = AppWidgetManager.getInstance(this);
        mAppWidgetHost = new AppWidgetHost(this, HOST_ID);

        mAppWidgetId = mAppWidgetHost.allocateAppWidgetId();
        AppWidgetProviderInfo appWidgetProviderInfo =
            getAppWidgetInfoByLabel(mAppWidgetManager, getWidgetLabel());
        if (appWidgetProviderInfo == null) {
            return;
        }

        // require android.permission.BIND_APPWIDGET permission
        mAppWidgetManager.bindAppWidgetId(mAppWidgetId, appWidgetProviderInfo.provider);

        // init the view.
        FrameLayout rootView = (FrameLayout) findViewById(R.id.root_view);
        // TODO: add correct margin for rootView in xml
        FrameLayout.LayoutParams layoutParam = new FrameLayout.LayoutParams(
            LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT, Gravity.CENTER);
        AppWidgetHostView hv = mAppWidgetHost.createView(this, mAppWidgetId,
                               appWidgetProviderInfo);
        rootView.addView(hv, layoutParam);
    }

    @Override
    protected void onStart() {
        super.onStart();
        mAppWidgetHost.startListening();
    }

    @Override
    protected void onStop() {
        mAppWidgetHost.stopListening();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        if (mAppWidgetHost != null) {
            mAppWidgetHost.deleteAppWidgetId(mAppWidgetId);
            mAppWidgetHost.deleteHost();
        }
        super.onDestroy();
    }

    public AppWidgetManager getWidgetManager() {
        return mAppWidgetManager;
    }

    /**
     *  Because ViewGroup.findViewTraversal() will check v.mPrivateFlags & IS_ROOT_NAMESPACE)
     *  this will fall in widget case.
     *  so we implement another simple api by avoiding checking this flag to find targetView.
     */
    public View findWidgetViewById(View baseView, int id) {

        if (baseView.getId() == id) {
            return baseView;
        }
        if (baseView instanceof ViewGroup) {
            ViewGroup vg = (ViewGroup) baseView;
            final int count = vg.getChildCount();
            View v;
            for (int i = 0; i < count; i++) {
                v = findWidgetViewById(vg.getChildAt(i), id);
                if (v != null) {
                    return v;
                }
            }
        }
        return null;
    }

}
