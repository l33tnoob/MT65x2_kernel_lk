/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.test.plugin;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.test.InstrumentationTestCase;
import android.util.LruCache;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.rcse.plugin.contacts.CallDetailExtensionForRCS;
import com.mediatek.rcse.plugin.contacts.CallLogExtention;
import com.mediatek.rcse.plugin.contacts.CallLogExtention.Action;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;
import com.mediatek.rcse.test.Utils;
import com.mediatek.rcse.activities.PluginProxyActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * Test case for CallDetailExtensionForRCS.
 */
public class CallDetailExtensionForRCSTest extends InstrumentationTestCase {
    private static final int TIME_OUT = 5000;
    private static final String TAG = "CallDetailExtensionForRCSTest";
    private CallDetailExtensionForRCS mCallDetailExtensionForRCS = null;
    private Context mContext = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        PluginApiManager.initialize(mContext);
        mCallDetailExtensionForRCS = new CallDetailExtensionForRCS(mContext);
    }

    /**
     * Test setViewVisibleByActivity
     * @throws Throwable 
     */
    public void testCase01_setViewVisibleByActivity()
            throws Throwable {
        Logger.d(TAG, "testCase01_setViewVisibleByActivity entry");
        //Activity activity = new Activity();
        String displayName = "test";
        String number = "+34200000258";
        int RCS_container = 1;
        int separator = 2;
        int RCS = 3;
        int RCS_action = 4;
        int RCS_text = 5;
        int RCS_icon = 6;
        int RCS_divider = 7;
        String commd = CallDetailExtensionForRCS.COMMD_FOR_RCS;

        View RCSContainer = new View(mContext);
        RCSContainer.setId(RCS_container);

        View separator03 = new View(mContext);
        separator03.setId(separator);

        LinearLayout convertView3 = new LinearLayout(mContext);
        convertView3.setId(RCS);

        View RCSACtion = new View(mContext);
        RCSACtion.setId(RCS_action);
        convertView3.addView(RCSACtion,0);
        TextView RCSText = new TextView(mContext);
        RCSText.setId(RCS_text);
        convertView3.addView(RCSText,1);
        ImageView icon = new ImageView(mContext);
        icon.setId(RCS_icon);
        convertView3.addView(icon,2);
        
        View divider = new View(mContext);
        divider.setId(RCS_divider);
        convertView3.addView(divider,3);

        final LinearLayout activityLayout = new LinearLayout(mContext);
        activityLayout.addView(RCSContainer, 0);
        activityLayout.addView(separator03, 1);
        activityLayout.addView(convertView3, 2);
        final LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(
                100, 100);
        Intent intent = new Intent("com.mediatek.rcse.action.PROXY");
        final Activity activity = launchActivityWithIntent(getInstrumentation()
                .getTargetContext().getPackageName(),
                PluginProxyActivity.class, intent);
        final CountDownLatch countDownLatch = new CountDownLatch(1);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "addContentView ");
                activity.addContentView(activityLayout, layoutParams);
                countDownLatch.countDown();
            }
        });
        countDownLatch.await(TIME_OUT, TimeUnit.MILLISECONDS);
        enableContactsPlugin();
        enableIMAndFTCapability(number);
        // Let go to first if
        mCallDetailExtensionForRCS.setViewVisibleByActivity(activity,
                displayName, number, RCS_container, separator, RCS, RCS_action,
                RCS_text, RCS_icon, RCS_divider, commd + "000");

        enableIMAndFTCapability(number);
        mCallDetailExtensionForRCS.setViewVisibleByActivity(activity,
                displayName, number, RCS_container, separator, RCS, RCS_action,
                RCS_text, RCS_icon, RCS_divider, commd);
        
        enableOnlyFT(number);
        mCallDetailExtensionForRCS.setViewVisibleByActivity(activity,
                displayName, number, RCS_container, separator, RCS, RCS_action,
                RCS_text, RCS_icon, RCS_divider, commd);
        assertFalse(RCSACtion.isClickable());
        
        enableOnlyIM(number);
        mCallDetailExtensionForRCS.setViewVisibleByActivity(activity,
                displayName, number, RCS_container, separator, RCS, RCS_action,
                RCS_text, RCS_icon, RCS_divider, commd);
        assertEquals(View.GONE, icon.getVisibility());
        assertEquals(View.GONE, divider.getVisibility());        
        
        disableIMAndFT(number);
        mCallDetailExtensionForRCS.setViewVisibleByActivity(activity,
                displayName, number, RCS_container, separator, RCS, RCS_action,
                RCS_text, RCS_icon, RCS_divider, commd);
        assertEquals(View.GONE, RCSContainer.getVisibility());
        assertEquals(View.GONE, icon.getVisibility());
        assertEquals(View.GONE, divider.getVisibility());
        assertEquals(View.GONE, separator03.getVisibility());
        assertEquals(View.GONE, RCSACtion.getVisibility());
        
        activity.finish();
    }

    /**
     * Test isEnabled
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void testCase02_isEnabled() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase02_isEnabled entry");
        assertFalse(mCallDetailExtensionForRCS.isEnabled(null));
        enableContactsPlugin();
        String number = "+34200000246";
        Field fieldmCallLogPlugin = Utils.getPrivateField(
                mCallDetailExtensionForRCS.getClass(), "mCallLogPlugin");
        CallLogExtention callLogExtension = (CallLogExtention) fieldmCallLogPlugin
                .get(mCallDetailExtensionForRCS);
        Drawable drawable = callLogExtension.getContactPresence(number);
        boolean expectedValue = drawable == null ? false : true;
        assertEquals(expectedValue,
                mCallDetailExtensionForRCS.isEnabled(number));
    }

    /**
     * Test isVisible
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase03_isVisible() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Logger.d(TAG, "testCase03_isVisible entry");
        View view = new TextView(mContext);
        view.setVisibility(View.VISIBLE);
        Method methodIsVisible = Utils.getPrivateMethod(
                mCallDetailExtensionForRCS.getClass(), "isVisible", View.class);
        assertEquals(true,
                methodIsVisible.invoke(mCallDetailExtensionForRCS, view));
        view = null;
    }

    /**
     * Test setViewVisible
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase04_setViewVisible() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase04_setViewVisible entry");
        LinearLayout view = new LinearLayout(mContext);
        String number = "+34200000246";
        String commd2 = CallDetailExtensionForRCS.COMMD_FOR_RCS;
        int header_RCS_container = 1;
        // No usefull parameters
        int res2 = 0;
        int res3 = 0;
        int res4 = 0;
        int res5 = 0;
        int res6 = 0;
        int res7 = 0;
        View subView = new View(mContext);
        subView.setId(header_RCS_container);
        view.addView(subView);
        mCallDetailExtensionForRCS.setViewVisible(subView, number, commd2,
                header_RCS_container, res2, res3, res4, res5, res6, res7);
        boolean isEnabled = mCallDetailExtensionForRCS.isEnabled(number);
        int visbile = isEnabled ? View.VISIBLE : View.GONE;
        assertEquals(visbile, subView.getVisibility());
        mCallDetailExtensionForRCS.setViewVisible(subView, number, commd2
                + "000", header_RCS_container, res2, res3, res4, res5, res6,
                res7);
    }

    /**
     * Test onClick
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase05_onClick() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase05_onClick entry");
        Field fieldmRCSTransforActionListener = Utils.getPrivateField(
                mCallDetailExtensionForRCS.getClass(),
                "mRcsTransforActionListener");
        Field fieldmRCSTextActionListener = Utils
                .getPrivateField(mCallDetailExtensionForRCS.getClass(),
                        "mRcsTextActionListener");
        View.OnClickListener RCSTransforActionListener = (OnClickListener) fieldmRCSTransforActionListener
                .get(mCallDetailExtensionForRCS);
        View.OnClickListener RCSTextActionListener = (OnClickListener) fieldmRCSTextActionListener
                .get(mCallDetailExtensionForRCS);
        View view = new View(mContext);
        String number = "+34200000255";
        String[] tags = new String[] {
                number, null
        };
        view.setTag(tags);
        Action[] actions = getRCSActions();
        if (actions == null) {
            Logger.d(TAG, "RCSActions is null, so set it");
            setRCSActions();
        }
        actions = getRCSActions();
        // Let actions[1] be null and not null to test

        // Since the activity is instanced by new and is not a real Activity.
        // As a result startActivity will throw NullpointerException, catch it
        try {
            RCSTransforActionListener.onClick(view);
        } catch (NullPointerException e) {

        }
        actions[1] = null;
        try {
            RCSTransforActionListener.onClick(view);
        } catch (NullPointerException e) {

        }

        try {
            RCSTextActionListener.onClick(view);
        } catch (NullPointerException e) {

        }
        actions[0] = null;
        try {
            RCSTextActionListener.onClick(view);
        } catch (NullPointerException e) {

        }

    }

    /**
     * Make sure Contacts plugin enabled
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    private void enableContactsPlugin() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        // Make mContactPlugin.isEnabled return true
        Field fieldmRegistrationApi = Utils.getPrivateField(PluginApiManager
                .getInstance().getClass(), "mRegistrationApi");
        RegistrationApi registrationApi = new RegistrationApi(mContext);
        if (fieldmRegistrationApi.get(PluginApiManager.getInstance()) == null) {
            Logger.d(TAG, "mRegistrationApi is null , set it");
            fieldmRegistrationApi.set(PluginApiManager.getInstance(),
                    registrationApi);
        }
        PluginApiManager.getInstance().setRegistrationStatus(true);
    }

    private Action[] getRCSActions() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Field fieldRCSActions = Utils.getPrivateField(
                mCallDetailExtensionForRCS.getClass(), "mRcsActions");
        return (Action[]) fieldRCSActions.get(mCallDetailExtensionForRCS);
    }

    private void setRCSActions() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Action[] actions = new Action[2];
        Action imAction = new Action();
        Intent imIntent = null;
        imIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
        imIntent.putExtra(PluginApiManager.RcseAction.IM_ACTION, true);

        Drawable ftDrawable = null;
        Intent ftIntent = null;
        ftIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
        ftIntent.putExtra(PluginApiManager.RcseAction.FT_ACTION, true);
        imAction.intentAction = imIntent;
        actions[0] = imAction;
        Action ftAction = new Action();
        ftAction.intentAction = ftIntent;
        ftAction.icon = ftDrawable;
        actions[1] = ftAction;
        Field fieldRCSActions = Utils.getPrivateField(
                mCallDetailExtensionForRCS.getClass(), "mRcsActions");
        fieldRCSActions.set(mCallDetailExtensionForRCS, actions);
    }

    @SuppressWarnings("unchecked")
    private void enableIMAndFTCapability(String number)
            throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        LruCache<String, ContactInformation> contactCache = getContactCache();
        ContactInformation contactInformation = new ContactInformation();
        contactInformation.isFtSupported = true;
        contactInformation.isImSupported = true;
        contactCache.evictAll();
        contactCache.put(number, contactInformation);
    }

    private void enableOnlyIM(String number) throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        LruCache<String, ContactInformation> contactCache = getContactCache();
        ContactInformation contactInformation = new ContactInformation();
        contactInformation.isFtSupported = false;
        contactInformation.isImSupported = true;
        contactCache.evictAll();
        contactCache.put(number, contactInformation);
    }

    private void enableOnlyFT(String number) throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        LruCache<String, ContactInformation> contactCache = getContactCache();
        ContactInformation contactInformation = new ContactInformation();
        contactInformation.isFtSupported = true;
        contactInformation.isImSupported = false;
        contactCache.evictAll();
        contactCache.put(number, contactInformation);
    }

    private void disableIMAndFT(String number) throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        LruCache<String, ContactInformation> contactCache = getContactCache();
        ContactInformation contactInformation = new ContactInformation();
        contactInformation.isFtSupported = false;
        contactInformation.isImSupported = false;
        contactCache.evictAll();
        contactCache.put(number, contactInformation);
    }

    @SuppressWarnings("unchecked")
    private LruCache<String, ContactInformation> getContactCache()
            throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        PluginApiManager.initialize(mContext);
        Field fieldmContactsCache = Utils.getPrivateField(PluginApiManager
                .getInstance().getClass(), "mContactsCache");
        LruCache<String, ContactInformation> contactCache = (LruCache<String, ContactInformation>) fieldmContactsCache
                .get(PluginApiManager.getInstance());
        return contactCache;
    }

}
