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
import android.content.Intent;
import android.net.Uri;
import android.test.AndroidTestCase;
import android.util.LruCache;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.rcse.plugin.contacts.CallListExtensionForRCS;
import com.mediatek.rcse.plugin.contacts.ContactDetailExtensionForRCS;
import com.mediatek.rcse.plugin.contacts.ContactExtention;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;
import com.mediatek.rcse.test.Utils;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Set;

/**
 * Test case for ContactDetailExtensionForRCS.
 */
public class ContactDetailExtensionForRCSTest extends AndroidTestCase {
    private static final String TAG = "CallListExtensionForRCSTest";
    private ContactDetailExtensionForRCS mContactDetailExtensionForRCS = null;

    // private CallLogExtention mCallLogPlugin = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        PluginApiManager.initialize(mContext);
        mContactDetailExtensionForRCS = new ContactDetailExtensionForRCS(mContext);
    }

    /**
     * Test layoutExtentionIcon
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase01_layoutExtentionIcon() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase01_layoutExtentionIcon entry");
        ImageView imageView = new ImageView(mContext);
        imageView.setVisibility(View.VISIBLE);
        Field fieldmRCSIconViewWidth = Utils.getPrivateField(
                mContactDetailExtensionForRCS.getClass(), "mRCSIconViewWidth");
        int RCSIconViewWidth = fieldmRCSIconViewWidth.getInt(mContactDetailExtensionForRCS);
        assertEquals(20 - (1 + RCSIconViewWidth),
                mContactDetailExtensionForRCS.layoutExtentionIcon(1, 1, 20, 20, 1, imageView,
                        CallListExtensionForRCS.COMMD_FOR_RCS));
    }

    /**
     * Test onContactDetailOpen
     * 
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void testCase02_onContactDetailOpen() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase02_onContactDetailOpen entry");
        mContactDetailExtensionForRCS.onContactDetailOpen(Uri.parse(""),
                ContactDetailExtensionForRCS.COMMD_FOR_RCS + "000");
        mContactDetailExtensionForRCS.onContactDetailOpen(Uri.parse(""),
                ContactDetailExtensionForRCS.COMMD_FOR_RCS);
        Field fieldmContactPlugin = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mContactPlugin");
        fieldmContactPlugin.set(mContactDetailExtensionForRCS, null);
        mContactDetailExtensionForRCS.onContactDetailOpen(Uri.parse(""),
                ContactDetailExtensionForRCS.COMMD_FOR_RCS);
    }

    /**
     * Test setExtensionImageView
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase03_setExtensionImageView() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase03_setExtensionImageView entry");
        ImageView imageView = new ImageView(mContext);
        imageView.setImageDrawable(null);
        long contactId = 123456;
        mContactDetailExtensionForRCS.setExtensionImageView(imageView, contactId,
                ContactDetailExtensionForRCS.COMMD_FOR_RCS);
        Field fieldmContactPlugin = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mContactPlugin");
        ContactExtention contactPlugin = (ContactExtention) fieldmContactPlugin
                .get(mContactDetailExtensionForRCS);
        assertEquals(contactPlugin.getContactPresence(contactId), imageView.getDrawable());
    }

    /**
     * Test isVisible
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase04_isVisible() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase04_isVisible entry");
        View view = new TextView(mContext);
        view.setVisibility(View.VISIBLE);
        Method methodIsVisible = Utils.getPrivateMethod(mContactDetailExtensionForRCS.getClass(),
                "isVisible", View.class);
        assertEquals(true, methodIsVisible.invoke(mContactDetailExtensionForRCS, view));
        view = null;
    }

    /**
     * Test measureExtention
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase05_measureExtention() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase05_measureExtention entry");
        ImageView imageView = new ImageView(mContext);
        imageView.setVisibility(View.VISIBLE);
        Field fieldmRCSIconViewWidthAndHeightAreReady = Utils.getPrivateField(
                mContactDetailExtensionForRCS.getClass(), "mRCSIconViewWidthAndHeightAreReady");
        fieldmRCSIconViewWidthAndHeightAreReady.set(mContactDetailExtensionForRCS, false);
        mContactDetailExtensionForRCS.measureExtentionIcon(imageView,
                CallListExtensionForRCS.COMMD_FOR_RCS);
        assertTrue(fieldmRCSIconViewWidthAndHeightAreReady
                .getBoolean(mContactDetailExtensionForRCS));
    }

    /**
     * Test setVisbile
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase06_setVisbile() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Logger.d(TAG, "testCase06_setVisbile entry");
        // Should make mContactPlugin.isEnabled() return true
        // Make mContactPlugin.getMimeType() is equal to mimetype
        Field fieldmContactPlugin = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mContactPlugin");
        ContactExtention contactExtention = (ContactExtention) fieldmContactPlugin
                .get(mContactDetailExtensionForRCS);
        enableContactsPlugin();

        LinearLayout view = new LinearLayout(mContext);
        Activity activity = new Activity();
        String mimetype = contactExtention.getMimeType();
        String data = "";
        String displayName = "";
        String commd = "";
        int vtcall_action_view_container = 10;
        int vertical_divider_vtcall = 11;
        int vtcall_action_button = 12;
        int secondary_action_view_container = 13;
        int secondary_action_button = 14;
        int vertical_divider = 15;
        int plugin_action_view_container = 16;
        int plugin_action_button = 17;
        int messaging_action_view_container = 18;
        int messaging_action_button = 19;
        int vertical_divider_messaging = 20;
        int vertical_divider_secondary_action = 21;
        int text_view = 22;
        // This call will go to else
        mContactDetailExtensionForRCS.setViewVisible(view, activity, mimetype, data, displayName,
                commd, vtcall_action_view_container, vertical_divider_vtcall, vtcall_action_button,
                secondary_action_view_container, secondary_action_button, vertical_divider, plugin_action_view_container, plugin_action_button, 0 , null, messaging_action_view_container, messaging_action_button, vertical_divider_messaging, vertical_divider_secondary_action, text_view);
        // This call will go to inner else
        mContactDetailExtensionForRCS.setViewVisible(view, activity, mimetype, data, displayName,
                ContactDetailExtensionForRCS.COMMD_FOR_RCS, vtcall_action_view_container,
                vertical_divider_vtcall, vtcall_action_button, secondary_action_view_container,
                secondary_action_button, vertical_divider, plugin_action_view_container, plugin_action_button, 0 , null, messaging_action_view_container, messaging_action_button, vertical_divider_messaging, vertical_divider_secondary_action, text_view);
        // Add some sub view to view
        View vtcallActionViewContainer = new View(mContext);
        vtcallActionViewContainer.setId(vtcall_action_view_container);
        view.addView(vtcallActionViewContainer, 0);
        vtcallActionViewContainer.setVisibility(View.INVISIBLE);

        View vewVtCallDivider = new View(mContext);
        vewVtCallDivider.setId(vertical_divider_vtcall);
        view.addView(vewVtCallDivider, 1);
        vewVtCallDivider.setVisibility(View.INVISIBLE);

        ImageView btnVtCallAction = new ImageView(mContext);
        btnVtCallAction.setId(vtcall_action_button);
        view.addView(btnVtCallAction, 2);
        btnVtCallAction.setVisibility(View.INVISIBLE);

        View secondaryActionViewContainer = new View(mContext);
        secondaryActionViewContainer.setId(secondary_action_view_container);
        view.addView(secondaryActionViewContainer, 3);
        secondaryActionViewContainer.setVisibility(View.INVISIBLE);

        ImageView secondaryActionButton = new ImageView(mContext);
        secondaryActionButton.setId(secondary_action_button);
        view.addView(secondaryActionButton, 4);
        secondaryActionButton.setVisibility(View.INVISIBLE);

        View secondaryActionDivider = new View(mContext);
        secondaryActionDivider.setId(vertical_divider);
        view.addView(secondaryActionDivider, 5);
        secondaryActionDivider.setVisibility(View.INVISIBLE);

        Field fieldIMValue = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mIMValue");
        Field fieldFTValue = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mFTValue");
        fieldIMValue.set(mContactDetailExtensionForRCS, 1);
        fieldFTValue.set(mContactDetailExtensionForRCS, 1);
        // Modify mIMValue mFTValue value to call function setViewVisible
        mContactDetailExtensionForRCS.setViewVisible(view, activity, mimetype, data, displayName,
                ContactDetailExtensionForRCS.COMMD_FOR_RCS, vtcall_action_view_container,
                vertical_divider_vtcall, vtcall_action_button, secondary_action_view_container,
                secondary_action_button, vertical_divider, plugin_action_view_container, plugin_action_button, 0 , null, messaging_action_view_container, messaging_action_button, vertical_divider_messaging, vertical_divider_secondary_action, text_view);
        assertEquals(View.GONE, vewVtCallDivider.getVisibility());
        assertEquals(View.VISIBLE, secondaryActionDivider.getVisibility());
        assertEquals(View.VISIBLE, btnVtCallAction.getVisibility());
        assertEquals(View.VISIBLE, secondaryActionButton.getVisibility());
        assertEquals(View.VISIBLE, secondaryActionViewContainer.getVisibility());

        fieldIMValue.set(mContactDetailExtensionForRCS, 1);
        fieldFTValue.set(mContactDetailExtensionForRCS, 0);
        mContactDetailExtensionForRCS.setViewVisible(view, activity, mimetype, data, displayName,
                ContactDetailExtensionForRCS.COMMD_FOR_RCS, vtcall_action_view_container,
                vertical_divider_vtcall, vtcall_action_button, secondary_action_view_container,
                secondary_action_button, vertical_divider, plugin_action_view_container, plugin_action_button, 0 , null, messaging_action_view_container, messaging_action_button, vertical_divider_messaging, vertical_divider_secondary_action, text_view);
        assertEquals(View.GONE, vewVtCallDivider.getVisibility());
        assertEquals(View.GONE, secondaryActionDivider.getVisibility());
        assertEquals(View.GONE, btnVtCallAction.getVisibility());
        assertEquals(View.VISIBLE, secondaryActionButton.getVisibility());
        assertEquals(View.VISIBLE, secondaryActionViewContainer.getVisibility());

        fieldIMValue.set(mContactDetailExtensionForRCS, 0);
        fieldFTValue.set(mContactDetailExtensionForRCS, 1);
        mContactDetailExtensionForRCS.setViewVisible(view, activity, mimetype, data, displayName,
                ContactDetailExtensionForRCS.COMMD_FOR_RCS, vtcall_action_view_container,
                vertical_divider_vtcall, vtcall_action_button, secondary_action_view_container,
                secondary_action_button, vertical_divider, plugin_action_view_container, plugin_action_button, 0 , null, messaging_action_view_container, messaging_action_button, vertical_divider_messaging, vertical_divider_secondary_action, text_view);
        assertEquals(View.GONE, vewVtCallDivider.getVisibility());
        assertEquals(View.GONE, secondaryActionDivider.getVisibility());
        assertEquals(View.GONE, btnVtCallAction.getVisibility());
        assertEquals(View.VISIBLE, secondaryActionButton.getVisibility());
        assertEquals(View.VISIBLE, secondaryActionViewContainer.getVisibility());
    }

    /**
     * Test onClick
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase07_onClick() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Logger.d(TAG, "testCase07_onClick entry");
        Field fieldmsetScondBuottononClickListner = Utils.getPrivateField(
                mContactDetailExtensionForRCS.getClass(), "msetScondBuottononClickListner");
        OnClickListener clickListener = (OnClickListener) fieldmsetScondBuottononClickListner
                .get(mContactDetailExtensionForRCS);
        View view = new View(mContext);
        Intent intent = new Intent();
        view.setTag(null);
        clickListener.onClick(view);

        Field fieldmActivity = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mActivity");
        Activity activity = (Activity) fieldmActivity.get(mContactDetailExtensionForRCS);
        Activity tmpActivity = new Activity();
        if (activity == null) {
            Logger.d(TAG, "activity is null, then set it");
            fieldmActivity.set(mContactDetailExtensionForRCS, tmpActivity);
        }
        view.setTag(intent);
        // Because the activity is simply instanced by new
        try {
            clickListener.onClick(view);
        } catch (NullPointerException e) {
            Logger.e(TAG, e.getMessage() + ":trace:" + e.getStackTrace());
        }
    }

    /**
     * Test canSetExtensionIcon and onPresenceChanged
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     */
    @SuppressWarnings("unchecked")
    public void testCase08_canSetExtensionIcon() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, NoSuchMethodException,
            InvocationTargetException {
        Logger.d(TAG, "testCase08_canSetExtensionIcon entry");
        assertEquals(false, mContactDetailExtensionForRCS.canSetExtensionIcon(0, null));
        Field fieldmContactPlugin = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mContactPlugin");
        ContactExtention contactExtention = (ContactExtention) fieldmContactPlugin
                .get(mContactDetailExtensionForRCS);

        // Make mContactPlugin.isEnabled return true
        enableContactsPlugin();
        long contactId = 123456;
        assertEquals(false, mContactDetailExtensionForRCS.canSetExtensionIcon(contactId,
                CallListExtensionForRCS.COMMD_FOR_RCS + "000"));
        if (contactExtention.getContactPresence(contactId) == null) {
            assertEquals(false, mContactDetailExtensionForRCS.canSetExtensionIcon(contactId,
                    CallListExtensionForRCS.COMMD_FOR_RCS));
        } else {
            assertEquals(true, mContactDetailExtensionForRCS.canSetExtensionIcon(contactId,
                    CallListExtensionForRCS.COMMD_FOR_RCS));
        }

        Field fieldonPresenceChangedListenerList = Utils.getPrivateField(
                contactExtention.getClass(), "mOnPresenceChangedListenerList");
        HashMap<Object, Long> onPresenceChangedListenerList = (HashMap<Object, Long>) fieldonPresenceChangedListenerList
                .get(contactExtention);
        Logger.d(TAG,
                "onPresenceChangedListenerList size = " + onPresenceChangedListenerList.size());
        Set<Object> listenerSet = onPresenceChangedListenerList.keySet();
        for (Object listener : listenerSet) {
            Method methodonPresenceChanged = Utils.getPrivateMethod(listener.getClass(),
                    "onPresenceChanged", long.class, int.class);
            methodonPresenceChanged.invoke(listener, contactId, 1);
        }
    }

    /**
     * Test checkPluginSupport
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase09_checkPluginSupport() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase09_checkPluginSupport entry");
        Field fieldmContactPlugin = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mContactPlugin");
        ContactExtention contactExtention = (ContactExtention) fieldmContactPlugin
                .get(mContactDetailExtensionForRCS);
        assertEquals(contactExtention.isEnabled(),
                mContactDetailExtensionForRCS
                        .checkPluginSupport(CallListExtensionForRCS.COMMD_FOR_RCS));
        assertFalse(mContactDetailExtensionForRCS
                .checkPluginSupport(CallListExtensionForRCS.COMMD_FOR_RCS + "000"));
    }

    /**
     * Test getRCSIcon
     * @throws NoSuchFieldException 
     * @throws IllegalAccessException 
     * @throws IllegalArgumentException 
     */
    @SuppressWarnings("unchecked")
    public void testCase10_getRCSIcon() throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase10_getRCSIcon entry");
        // Add mock data to PluginApiManager.mCache and mContactsCache
        long contactId = 123456;
        String number = "+34200000250";
        Field fieldmContactsCache = Utils.getPrivateField(
                PluginApiManager.getInstance().getClass(), "mContactsCache");
        Field fieldmCache = Utils.getPrivateField(PluginApiManager.getInstance().getClass(),
                "mCache");
        LruCache<Long, List<String>> cache = (LruCache<Long, List<String>>) fieldmCache
                .get(PluginApiManager.getInstance());
        ArrayList<String> numbersArrayLis = new ArrayList<String>();
        numbersArrayLis.add(number);
        cache.put(contactId, numbersArrayLis);
        LruCache<String, ContactInformation> contactsCache = (LruCache<String, ContactInformation>) fieldmContactsCache
                .get(PluginApiManager.getInstance());
        ContactInformation contactInformation = new ContactInformation();
        contactInformation.isRcsContact = 1;
        contactsCache.put(number, contactInformation);
        Field fieldmContactPlugin = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mContactPlugin");
        assertNotNull(mContactDetailExtensionForRCS.getRCSIcon(contactId));
        contactInformation.isRcsContact = 0;
        assertNull(mContactDetailExtensionForRCS.getRCSIcon(contactId));
        //Let mContactPlugin be null
        fieldmContactPlugin.set(mContactDetailExtensionForRCS, null);
        assertNull(mContactDetailExtensionForRCS.getRCSIcon(contactId));
    }

    public void testCase11_getExtentionMimeType() {
        Logger.d(TAG, "testCase11_getExtentionMimeType entry");
        assertNull(mContactDetailExtensionForRCS
                .getExtentionMimeType(ContactDetailExtensionForRCS.COMMD_FOR_RCS + "000"));
        assertEquals("vnd.android.cursor.item/com.orangelabs.rcse.capabilities",
                mContactDetailExtensionForRCS
                        .getExtentionMimeType(ContactDetailExtensionForRCS.COMMD_FOR_RCS));
    }

    public void testCase12_getExtentionKind() {
        Logger.d(TAG, "testCase12_getExtentionKind entry");
        String mimeType = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
        boolean needSetName = true;
        String name = "test";
        assertFalse(mContactDetailExtensionForRCS.getExtentionKind(mimeType, needSetName, name,
                ContactDetailExtensionForRCS.COMMD_FOR_RCS + "000"));
        assertTrue(mContactDetailExtensionForRCS.getExtentionKind(mimeType, needSetName, name,
                ContactDetailExtensionForRCS.COMMD_FOR_RCS));
        assertFalse(mContactDetailExtensionForRCS.getExtentionKind(mimeType + "00", needSetName,
                name, ContactDetailExtensionForRCS.COMMD_FOR_RCS));
    }

    /**
     * Test getExtensionTitles
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase13_getExtensionTitles() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase13_getExtensionTitles entry");
        String data = "key";
        String mimeType = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
        String kind = "kind";
        HashMap<String, String> mPhoneAndSubtitle = new HashMap<String, String>();
        String value = "value";
        mPhoneAndSubtitle.put(data, value);
        String commd = ContactDetailExtensionForRCS.COMMD_FOR_RCS;
        assertEquals(kind, mContactDetailExtensionForRCS.getExtensionTitles(data, mimeType, kind,
                mPhoneAndSubtitle, commd + "000"));
        
        assertEquals(value, mContactDetailExtensionForRCS.getExtensionTitles(data, mimeType, kind,
                mPhoneAndSubtitle, commd));
        //pass a null for mimetype
        assertNull(mContactDetailExtensionForRCS.getExtensionTitles(data, null, kind,
                mPhoneAndSubtitle, commd));
        //Make data and mPhoneAndSubtitle be null
        Field fieldmContactPlugin = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mContactPlugin");
        ContactExtention contactExtention = (ContactExtention) fieldmContactPlugin
                .get(mContactDetailExtensionForRCS);
        assertEquals(contactExtention.getAppTitle(),
                mContactDetailExtensionForRCS.getExtensionTitles(null, mimeType, kind, null, commd));
        assertEquals(kind,
                mContactDetailExtensionForRCS.getExtensionTitles(null, null, kind, null, commd));
    }

    /**
     * Test setViewVisibleWithCharSequence
     * 
     * @throws IllegalAccessException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     */
    public void testCase14_setViewVisibleWithCharSequence() throws IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        Logger.d(TAG, "testCase14_setViewVisibleWithCharSequence entry");
        LinearLayout resultView = new LinearLayout(mContext);
        Activity activity = new Activity();
        String mimeType = "vnd.android.cursor.item/com.orangelabs.rcse.capabilities";
        String data2 = "";
        CharSequence number = "+34200000250";
        String commd = ContactDetailExtensionForRCS.COMMD_FOR_RCS;
        int vertical_divider_vtcall = 1;
        int vtcall_action_button = 2;
        int vertical_divider = 3;
        int secondary_action_button = 4;
        // No meaningful parameters
        int res5 = 0;
        int res6 = 0;
        // Add some sub view to resultView
        View vewFirstDivider = new View(mContext);
        vewFirstDivider.setId(vertical_divider_vtcall);
        resultView.addView(vewFirstDivider, 0);

        ImageView btnFirstAction = new ImageView(mContext);
        btnFirstAction.setId(vtcall_action_button);
        resultView.addView(btnFirstAction, 1);

        View vewSecondDivider = new View(mContext);
        vewSecondDivider.setId(vertical_divider);
        resultView.addView(vewSecondDivider, 2);

        ImageView btnSecondButton = new ImageView(mContext);
        btnSecondButton.setId(secondary_action_button);
        resultView.addView(btnSecondButton, 3);

        enableContactsPlugin();
        //Let mimietype is null
        mContactDetailExtensionForRCS.setViewVisibleWithCharSequence(resultView, activity,
                null, data2, number, commd, vertical_divider_vtcall, vtcall_action_button,
                vertical_divider, secondary_action_button, res5, res6);
        

        Field fieldIMValue = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mIMValue");
        Field fieldFTValue = Utils.getPrivateField(mContactDetailExtensionForRCS.getClass(),
                "mFTValue");
        fieldIMValue.set(mContactDetailExtensionForRCS, 1);
        fieldFTValue.set(mContactDetailExtensionForRCS, 0);
        mContactDetailExtensionForRCS.setViewVisibleWithCharSequence(resultView, activity,
                mimeType, data2, number, commd, vertical_divider_vtcall, vtcall_action_button,
                vertical_divider, secondary_action_button, res5, res6);
        assertEquals(View.VISIBLE, btnFirstAction.getVisibility());
        
        fieldIMValue.set(mContactDetailExtensionForRCS, 0);
        fieldFTValue.set(mContactDetailExtensionForRCS, 1);
        mContactDetailExtensionForRCS.setViewVisibleWithCharSequence(resultView, activity,
                mimeType, data2, number, commd, vertical_divider_vtcall, vtcall_action_button,
                vertical_divider, secondary_action_button, res5, res6);
        assertEquals(View.VISIBLE, btnFirstAction.getVisibility());
        
        fieldIMValue.set(mContactDetailExtensionForRCS, 1);
        fieldFTValue.set(mContactDetailExtensionForRCS, 1);
        mContactDetailExtensionForRCS.setViewVisibleWithCharSequence(resultView, activity,
                mimeType, data2, number, commd, vertical_divider_vtcall, vtcall_action_button,
                vertical_divider, secondary_action_button, res5, res6);
        assertEquals(View.VISIBLE, btnFirstAction.getVisibility());
        assertEquals(View.VISIBLE, btnSecondButton.getVisibility());
    }

    /**
     * Test getExtentionIntent
     */
    public void testCase15_getExtentionIntent(){
        Logger.d(TAG, "testCase15_getExtentionIntent entry");
        int im = 1;
        int ft = 0;
        String cmd = ContactDetailExtensionForRCS.COMMD_FOR_RCS;
        assertNull(mContactDetailExtensionForRCS.getExtentionIntent(im, ft, cmd + "000"));
        Intent intent = mContactDetailExtensionForRCS.getExtentionIntent(im, ft, cmd);
        assertTrue(intent.getBooleanExtra(PluginApiManager.RcseAction.IM_ACTION, false));
        im = 0;
        ft = 1;
        intent = mContactDetailExtensionForRCS.getExtentionIntent(im, ft, cmd);
        assertTrue(intent.getBooleanExtra(PluginApiManager.RcseAction.FT_ACTION, false));
    }

    /**
     * Make sure Contacts plugin enabled
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    private void enableContactsPlugin() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        // Make mContactPlugin.isEnabled return true
        Field fieldmRegistrationApi = Utils.getPrivateField(PluginApiManager.getInstance()
                .getClass(), "mRegistrationApi");
        RegistrationApi registrationApi = new RegistrationApi(mContext);
        if (fieldmRegistrationApi.get(PluginApiManager.getInstance()) == null) {
            Logger.d(TAG, "mRegistrationApi is null , set it");
            fieldmRegistrationApi.set(PluginApiManager.getInstance(), registrationApi);
        }
        PluginApiManager.getInstance().setRegistrationStatus(true);
    }
}
