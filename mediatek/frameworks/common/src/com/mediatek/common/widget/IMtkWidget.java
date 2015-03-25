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

package com.mediatek.common.widget;

import android.os.Bundle;

/**
 * Widget interface for the specified AppWidget. Some special AppWidgets need
 * Launcher send more action details. Here defines some functions will be called
 * in Launcher App. You can implement your view(viewgroup) to handle these
 * details. <br/>
 * Note: One AppWidget can have only one view(viewgroup) implementing
 * IMTKWidget, beacuse Launcher App may only search the first IMTKWidget's
 * implementation.
 *
 * @hide
 */
public interface IMtkWidget {
    /**
     * The count should be installed in launcher.
     *
     * @return
     */
    int getPermittedCount();

    /**
     * The screen index of current AppWidget.
     *
     * @return
     */
    int getScreen();

    /**
     * The AppWidgetId of current AppWidget.
     *
     * @return
     */
    int getWidgetId();

    /**
     * Set the AppWidgetId of current AppWidget.
     *
     * @param widgetId
     */
    void setWidgetId(int widgetId);

    /**
     * Set the screen index of current AppWidget.
     *
     * @param screen
     */
    void setScreen(int screen);

    /**
     * Will be called when user start to drag current AppWidget.
     */
    void startDrag();

    /**
     * Will be called when user stop to drag current AppWidget.
     */
    void stopDrag();

    /**
     * Will be called when user leave the screen which current AppWidget locates
     * in.
     *
     * @param curScreen
     *            which side's screen user will be seen. -1 means move to left,
     *            +1 means move to right.
     * @return if IMTKWidget's implemention is ready for moving out, it will
     *         return true. otherwise, return false. <br/>
     *         Note: while return true, the Launcher will
     */
    boolean moveOut(int curScreen);

    /**
     * Will be called when the screen which AppWidget locates in will be seen by
     * user.
     *
     * @param curScreen
     *            the screen AppWidget locates in.
     */
    void moveIn(int curScreen);

    /**
     * Will be called when the current AppWidget will be not seen before
     * launcher makes other views cover the current AppWidget.
     *
     * @param curScreen
     */
    void startCovered(int curScreen);

    /**
     * Will be called when the current AppWidget will be seen after launcher
     * moves away other views on the top of current AppWidget.
     *
     * @param curScreen
     */
    void stopCovered(int curScreen);

    /**
     * Will be called after launcher's onPause is called.
     *
     * @param curScreen
     */
    void onPauseWhenShown(int curScreen);

    /**
     * Will be called after launcher's onResume is called.
     *
     * @param curScreen
     */
    void onResumeWhenShown(int curScreen);

    /**
     * Will be called after launcher's onSaveInstanceState is called.
     *
     * @param outSate
     */
    void onSaveInstanceState(Bundle outSate);

    /**
     * Will be called after launcher's onRestoreInstanceState is called.
     *
     * @param state
     */
    void onRestoreInstanceState(Bundle state);

    /**
     * Will be called when user leave the current screen which AppWidget is in
     * for Video favorite widget. just in ics
     */
    void leaveAppwidgetScreen();

    /**
     * Will be called when user enter the current screen which AppWidget is
     * in.for Videofavorite widget. just in ics
     */
    void enterAppwidgetScreen();
}
