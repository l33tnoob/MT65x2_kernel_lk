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

package com.mediatek.rcse.test.activity;

import android.app.Activity;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.widget.ListView;

import com.mediatek.rcse.activities.ContactsListFragment;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;



public class SelectContactsActivityTest extends
		ActivityInstrumentationTestCase2<SelectContactsActivity> {

	private static final String TAG = "SelectContactsActivityTest";
	private static final String ACTION_MAIN = "android.intent.action.MAIN";
	static final String CATEGORY_DEFAULT = "android.intent.category.DEFAULT";
	public static final int LAUNCH_TIME_OUT = 1000;
	static final double SELECT_CONTACTS_PATIAL = 0.5;
	static final int MIN_NUMBER = 1;

	private ActivityMonitor mSelectContactsActivityMonitor = new ActivityMonitor(
			SelectContactsActivity.class.getName(), null, false);

	public SelectContactsActivityTest() {
		super(SelectContactsActivity.class);
	}

	@Override
	protected void setUp() throws Exception {
		super.setUp();
		getInstrumentation().addMonitor(mSelectContactsActivityMonitor);
	}

	/**
	 * Use to test the contacts' result selected from selectedContactsActivity
	 * is equal with the number in intent used for the activity result.
	 */
	public void testCase1_SelectedNumEqualsWithNuminIntent() throws Throwable {
		Logger.v(TAG, "testCase1_SelectedNumEqualsWithNuminIntent()");
		final SelectContactsActivity activity = (SelectContactsActivity) launchActivity(ACTION_MAIN);
		assertTrue(activity != null);
		Class<? extends SelectContactsActivity> clazz = activity.getClass();
		assertTrue(clazz != null);
		Field field = getPrivateField(clazz, "mContactsListFragment");
		assertTrue(field != null);
		ContactsListFragment contactsListFragment = (ContactsListFragment) field
				.get(activity);
		assertTrue(contactsListFragment != null);
		final ListView listView = contactsListFragment.getListView();
		assertNotNull(listView);
		int totalNum = 0;
		runTestOnUiThread(new Runnable() {
			@Override
			public void run() {
				listView.setFocusable(true);
				listView.setFocusableInTouchMode(true);
				boolean focus = listView.requestFocus();
				listView.setSelection(0);
				Logger.v(TAG, "testCase1_SelectedNumEqualsWithNuminIntent() focurs = " + focus);
			}
		});
		getInstrumentation().waitForIdleSync();
		runTestOnUiThread(new Runnable() {
			@Override
			public void run() {
				listView.setSelection(0);
			}
		});
		getInstrumentation().waitForIdleSync();
		final int num = listView.getCount();
		Logger.d(TAG, "testCase1_SelectedNumEqualsWithNuminIntent() listview number is " + num);
		int selectNum = (int) (num * SELECT_CONTACTS_PATIAL);
		if(num <= MIN_NUMBER) {
			selectNum = num;
		} else {
			Logger.d(TAG, "testCase1_SelectedNumEqualsWithNuminIntent() the num > 1");
		}
		Logger.d(TAG, "testCase1_SelectedNumEqualsWithNuminIntent() halfnumber is " + selectNum);
		sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
		sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
		for (int i = MIN_NUMBER; i <= selectNum; i++) {
			sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
			Object item = listView.getItemAtPosition(i);
			assertNotNull(item);
			Field fieldNumber = getPrivateField(item.getClass(), "mNumber");
			String contactNumber = (String) fieldNumber.get(item);
			if (contactNumber != null) {
				Logger.d(TAG,
						"testCase1_SelectedNumEqualsWithNuminIntent() contact.mNumber is not null");
				totalNum++;
			} else {
				Logger.d(TAG,
						"testCase1_SelectedNumEqualsWithNuminIntent() contact.mNumber is null");
			}
			sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
		}
		getInstrumentation().waitForIdleSync();
		Logger.d(TAG,
				"testCase1_SelectedNumEqualsWithNuminIntent() total number is  "
						+ totalNum);
		final Method method = getPrivateMethod(contactsListFragment.getClass()
				.getSuperclass(), "startChat");
		assertNotNull(method);
		method.invoke(contactsListFragment);
		Method methodResult = getPrivateMethod(contactsListFragment.getClass()
				.getSuperclass(), "getResult");
		assertNotNull(methodResult);
		Intent result = (Intent) methodResult.invoke(contactsListFragment);
		int parListNum = 0;
		if (result != null) {
			ArrayList<Participant> parArrayList = result
					.getParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST);
			parListNum = parArrayList.size();
		} else {
			totalNum = 0;
			Logger.d(TAG, "no contacts to select");
		}

		Logger.d(TAG, "testCase1_SelectedNumEqualsWithNuminIntent() participantnumber in intent is " + parListNum + " total number is " + totalNum);
		assertEquals(totalNum, parListNum);
	}

	private Activity launchActivity(String action) throws InterruptedException {
		Intent intent = new Intent();
		intent.setAction(action);
		intent.addCategory(CATEGORY_DEFAULT);
		setActivityIntent(intent);
		// Test launch
		Activity activity = getActivity();
		assertTrue(activity != null);
		Activity ret = mSelectContactsActivityMonitor
				.waitForActivityWithTimeout(LAUNCH_TIME_OUT);
		assertTrue(ret != null);
		return activity;
	}

	private Field getPrivateField(Class clazz, String filedName)
			throws NoSuchFieldException {

		Field field = clazz.getDeclaredField(filedName);
		assertTrue(field != null);
		field.setAccessible(true);
		return field;

	}

	private Method getPrivateMethod(Class clazz, String methodName,
			Class<?>... parameterTypes) throws NoSuchMethodException {

		Method method = clazz.getDeclaredMethod(methodName, parameterTypes);
		assertTrue(method != null);
		method.setAccessible(true);
		return method;

	}

}
