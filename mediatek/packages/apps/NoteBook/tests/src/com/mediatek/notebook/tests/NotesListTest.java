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

package com.mediatek.notebook.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ListView;


import com.jayway.android.robotium.solo.Solo;

import com.mediatek.notebook.NoteAdapter.NoteItem;
import com.mediatek.notebook.NoteReading;
import com.mediatek.notebook.NotesList;
import com.mediatek.notebook.R;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Locale;
import java.util.Random;

//build apk:mk -t -o=MTK_AUTO_TEST=yes mm mediatek/packages/apps/NoteBook
//install apk:adb install -r out/target/product/mt6589tdv1_phone/data/app/NotebookTests.apk
//run apk:adb shell am instrument -w com.mediatek.notebook.tests/.NotebookTestRunner
public class NotesListTest extends ActivityInstrumentationTestCase2<NotesList> {

    private static final String TAG = "NotesListTest";
    private Solo mSolo;
    private Instrumentation mIns;
    private Activity mActivity;
    private Random mRandom = new Random();
    private ActivityMonitor mActivityMonitor = null;
    private int mSleeptime = 1000;

    public NotesListTest() {
        super("com.mediatek.notebook", NotesList.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        if (mActivity != null) {
            mActivity.finish();
        }
    }
    
    public int getCurrentNoteNumber() {
          ArrayList<ListView> listview = mSolo.getCurrentViews(ListView.class); 
          return listview.get(0).getAdapter().getCount();
    }

    public void itemSync(String noteItem) {
        mSolo.clickOnText(noteItem);
        assertTrue(mSolo.searchText(noteItem));
        mSolo.clickOnView(findView(R.id.menu_edit_current));

        /*if ("en".equals(Locale.getDefault().getLanguage())) {
            if (noteItem.equals(mSolo.getString(R.string.menu_study))) {
                assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_study_ui)));  
            } else if (noteItem.equals(mSolo.getString(R.string.menu_work))) {
                assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_work_ui)));  
            } else if (noteItem.equals(mSolo.getString(R.string.menu_personal))) {
                assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_personal_ui)));  
            } else if (noteItem.equals(mSolo.getString(R.string.menu_family))) {
                assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_family_ui)));  
            }
        } else {*/
            assertTrue(mSolo.searchText(noteItem));  
        //}

        mSolo.clickOnImage(0);
    }

    public void addNote(int noteCount) {
        int number = 0;
        View view = mActivity.findViewById(R.id.menu_add);
        for (int i = 0; i < noteCount; i ++) {
        mSolo.clickOnView(view);
        number = mRandom.nextInt(5);
        switch (number) {
            case 0:
                mSolo.enterText(0, "New note of none");
                break;
            case 1:
                mSolo.enterText(0, "New note of work");
                mSolo.clickOnText(mSolo.getString(R.string.menu_none));
                mSolo.clickOnText(mSolo.getString(R.string.menu_work));
                break;
            case 2:
                mSolo.enterText(0, "New note of personal");
                mSolo.clickOnText(mSolo.getString(R.string.menu_none));
                mSolo.clickOnText(mSolo.getString(R.string.menu_personal));
                break;
            case 3:
                mSolo.enterText(0, "New note of family");
                mSolo.clickOnText(mSolo.getString(R.string.menu_none));
                mSolo.clickOnText(mSolo.getString(R.string.menu_family));
                break;
            case 4:
                mSolo.enterText(0, "New note of study");
                mSolo.clickOnText(mSolo.getString(R.string.menu_none));
                mSolo.clickOnText(mSolo.getString(R.string.menu_study));
                break;
            default:
                break;
            }
            isCurrentCorrect();
        }
    }
    
    public void isCurrentCorrect() {
        mActivityMonitor = mIns.addMonitor(NotesList.class.getName(), null, false);
        mSolo.clickOnText(mSolo.getString(R.string.title_notes_edit));
        mSolo.sleep(mSleeptime);
        assertNotNull(mActivityMonitor.waitForActivityWithTimeout(200));
        assertTrue(mSolo.searchText(mSolo.getString(R.string.note_saved))); 
    }

    public void editDoneToReadingScreen() {
        mActivityMonitor = mIns.addMonitor(NoteReading.class.getName(), null, false);
        mSolo.clickOnText(mSolo.getString(R.string.title_notes_edit));
        mSolo.sleep(mSleeptime);
        assertNotNull(mActivityMonitor.waitForActivityWithTimeout(200));
        assertTrue(mSolo.searchText(mSolo.getString(R.string.note_saved))); 
    }
    
    protected View findView(int id) {
        ArrayList<View> views = mSolo.getViews();
        for (int i = views.size() - 1; i >= 0; i --) {
            Log.i(TAG, "id = " + views.get(i).getId() + "; text = " + views.get(i).getContentDescription());
            if (views.get(i).getId() == id) {
                return views.get(i);
            }
        }
        return null;
    }
    
    public void test01AddNewNote() {
        Log.i(TAG, "test01_AddNewNote: begining");
        addNote(3);
        Log.i(TAG, "test01_AddNewNote: ending");
    }
    
    public void test02AddLongNote() {
        Log.i(TAG, "test02_AddLongNote: begining");
        View view = mActivity.findViewById(R.id.menu_add);
        mSolo.clickOnView(view);

        mSolo.enterText(0, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
             
        assertTrue(mSolo.searchText(mSolo.getString(R.string.editor_full)));
        isCurrentCorrect();
        Log.i(TAG, "test02_AddLongNote: ending");
    }
    
    public void test03ScrollBarInView() {
        Log.i(TAG, "test07_ScrollBarInView: begining");
        int height = mActivity.getWindowManager().getDefaultDisplay().getHeight();
        int width = mActivity.getWindowManager().getDefaultDisplay().getWidth();
        int count = 0;
        if (getCurrentNoteNumber() == 0) {
        	test02AddLongNote();
        }
        mSolo.clickInList(1);
        mSolo.sleep(mSleeptime);

        for (; count <= 10; count ++) {
            if (mSolo.scrollDown()) {
                mSolo.drag(width / 2, width / 2, height - 100, 150, 1);
                Log.i(TAG, "count = " + count);
                mSolo.sleep(mSleeptime);
            } else {
                break;
            }
        }
        mSolo.sleep(mSleeptime);
        assertTrue(!mSolo.scrollDown());
        mSolo.clickOnImage(0);
        Log.i(TAG, "test07_ScrollBarInView: ending");
    }
    
    public void test04MenuSort1() {
        Log.i(TAG, "test05_MenuSort1: begining");
        if (getCurrentNoteNumber() == 0) {
            Log.e(TAG, "There is no note in the list");
            addNote(5);
        }
        mSolo.sendKey(KeyEvent.KEYCODE_MENU);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_DOWN);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_DOWN); 
        mSolo.sleep(mSleeptime);
        mSolo.sendKey(KeyEvent.KEYCODE_ENTER);
        mSolo.sleep(mSleeptime);
        mSolo.assertCurrentActivity("Can not stay NotesList activity", NotesList.class);
        Log.i(TAG, "test05_MenuSort1: ending");
    }
    
    public void test05MenuSort2() {
        Log.i(TAG, "test06_MenuSort2: begining");
        NoteItem itemPre;
        NoteItem itemNext;
        if (getCurrentNoteNumber() == 0) {
            Log.e(TAG, "There is no note in the list");
            addNote(5);
        }
        mSolo.sendKey(KeyEvent.KEYCODE_MENU);
        mSolo.sleep(mSleeptime);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_DOWN);
        mSolo.sleep(mSleeptime);
        mSolo.sendKey(KeyEvent.KEYCODE_ENTER);
        mSolo.sleep(mSleeptime);

        ArrayList<ListView> listview = mSolo.getCurrentViews(ListView.class);
        mSolo.sleep(mSleeptime);
        int currentNumber = getCurrentNoteNumber();
        mSolo.sleep(mSleeptime);
        for (int i = 1; i < currentNumber; i ++) {
            itemPre = (NoteItem)(listview.get(0).getAdapter().getItem(i - 1));
            Log.i(TAG, "item_pre = " + itemPre.modify_time);
            itemNext = (NoteItem)(listview.get(0).getAdapter().getItem(i));
            Log.i(TAG, "item_next = " + itemNext.modify_time);
            assertTrue((itemNext.modify_time.compareTo(itemPre.modify_time)) <= 0); 
        }

        mSolo.assertCurrentActivity("Can not stay NotesList activity", NotesList.class);
        Log.i(TAG, "test06_MenuSort2: ending");
    }

    public void test06CancelDeleteAllFromMenu() {
    	if (getCurrentNoteNumber() == 0) {
            Log.e(TAG, "There is no note in the list");
            addNote(2);
    	} 
    	mSolo.sendKey(KeyEvent.KEYCODE_MENU);
    	mSolo.sleep(mSleeptime);
        mSolo.sendKey(KeyEvent.KEYCODE_ENTER);
        mSolo.sleep(mSleeptime);
        mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));
        mSolo.sleep(mSleeptime);
    }
    
    public void test07DeleteAllFromMenu() {
    	if (getCurrentNoteNumber() == 0) {
            Log.e(TAG, "There is no note in the list");
            addNote(2);
    	} 
    	mSolo.sendKey(KeyEvent.KEYCODE_MENU);
    	mSolo.sleep(mSleeptime);
        mSolo.sendKey(KeyEvent.KEYCODE_ENTER);
        mSolo.sleep(mSleeptime);
        mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
        mSolo.sleep(mSleeptime);
    }
    
    public void test08EditFirstNote() {
        Log.i(TAG, "test08_EditFirstNote: begining");
        if (getCurrentNoteNumber() == 0) {
            Log.e(TAG, "There is no note in the list");
            addNote(1);
        }
        mSolo.clickInList(1);
        mSolo.sleep(mSleeptime);
        mSolo.assertCurrentActivity("Can not enter NoteReading activity", NoteReading.class);

        mSolo.clickOnView(findView(R.id.menu_edit_current));
        mSolo.sleep(mSleeptime);

        mSolo.clearEditText(0);
        mSolo.enterText(0, "Edit current note for test08 long long long long long long");
        //isCurrentCorrect();
        editDoneToReadingScreen();

        assertTrue(mSolo.searchText("Edit current note for test08 long long long long long long"));
        mSolo.sendKey(KeyEvent.KEYCODE_BACK);
            
        Log.i(TAG, "test08_EditFirstNote: ending");
     }

    public void test09DeleteCurrentNote() {
        Log.i(TAG, "test09_DeleteCurrentNote: begining");
        int countBefore = getCurrentNoteNumber();
        if (countBefore == 0) {
            Log.e(TAG, "There is no note in the list");
            addNote(1);
            countBefore = 1;
        }
        mSolo.clickInList(1);
        mSolo.sleep(mSleeptime);
        mSolo.assertCurrentActivity("Can not enter NoteReading activity", NoteReading.class);

        mSolo.clickOnView(findView(R.id.menu_delete_current));
        mSolo.sleep(mSleeptime);
        assertTrue(mSolo.searchText(mSolo.getString(android.R.string.ok)));

        mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
        mSolo.sleep(mSleeptime);
        int countAfter = getCurrentNoteNumber();
        assertTrue(countBefore - 1 == countAfter);
        Log.i(TAG, "test09_DeleteCurrentNote: ending");
     }
   
     public void test10CancelDeleteOne() {
         Log.i(TAG, "test10_CancelDeleteOne: begining");
         int count = getCurrentNoteNumber();
         if (count == 0) {
             Log.e(TAG, "There is no note in the list");
             addNote(1);
         }
         mSolo.clickInList(0);
         mSolo.sleep(mSleeptime);
         mSolo.assertCurrentActivity("Can not enter NoteReading activity", NoteReading.class);

         mSolo.clickOnView(findView(R.id.menu_delete_current));
         assertTrue(mSolo.searchText(mSolo.getString(android.R.string.cancel)));

         mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));
         mSolo.sleep(mSleeptime);
         mSolo.assertCurrentActivity("Can not enter NoteReading activity", NoteReading.class);
         mSolo.clickOnImage(0);
         mSolo.sleep(mSleeptime);
         Log.i(TAG, "test10_CancelDeleteOne: ending");
     }
    
     public void test11GroupSync() {
         Log.i(TAG, "test11_GroupSync: begining");
         int count = getCurrentNoteNumber();
         if (count == 0) {
             Log.e(TAG, "There is no note in the list");
             addNote(1);
         }
         if (mSolo.searchText(mSolo.getString(R.string.menu_work))) {
             itemSync(mSolo.getString(R.string.menu_work));
         } else if (mSolo.searchText(mSolo.getString(R.string.menu_personal))) {
             itemSync(mSolo.getString(R.string.menu_personal));
         } else if (mSolo.searchText(mSolo.getString(R.string.menu_study))) {
             itemSync(mSolo.getString(R.string.menu_study));
         } else if (mSolo.searchText(mSolo.getString(R.string.menu_family))) {
             itemSync(mSolo.getString(R.string.menu_family));
         } else {
             mSolo.clickInList(1);
             mSolo.sleep(mSleeptime);
             mSolo.clickOnView(findView(R.id.menu_edit_current));
             mSolo.sleep(mSleeptime);
             mSolo.sendKey(KeyEvent.KEYCODE_BACK);
             assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_none)));
             mSolo.clickOnImage(0);
         }
         mSolo.clickOnImage(0);
         Log.i(TAG, "test11_GroupSync: ending");
     }
      
     public void test12SaveNoNote() {
         Log.i(TAG, "test12_SaveNoNote: begining");
         View view = mActivity.findViewById(R.id.menu_add);
         mSolo.sleep(mSleeptime);
         mSolo.clickOnView(view);
         mSolo.sleep(mSleeptime);
        
         mActivityMonitor = mIns.addMonitor(NotesList.class.getName(), null, false);
         mSolo.clickOnText(mSolo.getString(R.string.title_notes_edit));
         mSolo.sleep(mSleeptime);
         assertNotNull(mActivityMonitor.waitForActivityWithTimeout(200));
        
         assertTrue(mSolo.searchText(mSolo.getString(R.string.save_none)));
         Log.i(TAG, "test12_SaveNoNote: ending");
     }
     
     public void test13ClearNoteAndSave() {
         Log.i(TAG, "test13_ClearNoteAndSave: begining");
         int count = getCurrentNoteNumber();
         if (count == 0) {
             Log.e(TAG, "There is no note in the list");
             addNote(1);
         }
         mSolo.clickInList(1);
         mSolo.assertCurrentActivity("Can not enter NoteReading activity", NoteReading.class);
         mSolo.sleep(mSleeptime);
         mSolo.clickOnView(findView(R.id.menu_edit_current));
         mSolo.sleep(mSleeptime);
        
         mSolo.clearEditText(0);
         mSolo.sleep(mSleeptime);
         mSolo.clickOnText(mSolo.getString(R.string.title_notes_edit));
         
         assertTrue(mSolo.searchText(mSolo.getString(R.string.empty_note)));
         
         mSolo.enterText(0, "Success!");
         mActivityMonitor = mIns.addMonitor(NoteReading.class.getName(), null, false);
         mSolo.clickOnText(mSolo.getString(R.string.title_notes_edit));
         mSolo.sleep(mSleeptime);
         assertNotNull(mActivityMonitor.waitForActivityWithTimeout(200));
         mSolo.sendKey(KeyEvent.KEYCODE_BACK);
         Log.i(TAG, "test13_ClearNoteAndSave: ending");    
     }
     
     public void test14UpdateModifyTime() {
         Log.i(TAG, "test14_UpdateModifyTime: begining");
         if (getCurrentNoteNumber() == 0) {
             Log.e(TAG, "There is no note in the list");
             addNote(1);
         }

         mSolo.clickInList(0);
         mSolo.sleep(mSleeptime);
         mSolo.assertCurrentActivity("Can not enter NoteReading activity", NoteReading.class);
         mSolo.clickOnView(findView(R.id.menu_edit_current));
         mSolo.sleep(mSleeptime);

         mSolo.clearEditText(0);
         mSolo.enterText(0, "Edit current note for test15");
         mSolo.sleep(mSleeptime);
                
         Calendar c = Calendar.getInstance();
         String hour = String.valueOf(c.get(Calendar.HOUR_OF_DAY));
         String min = String.valueOf(c.get(Calendar.MINUTE));
      
         mSolo.clickOnText(mSolo.getString(R.string.title_notes_edit));
         mSolo.sleep(mSleeptime);
         mSolo.sendKey(KeyEvent.KEYCODE_BACK);
         assertTrue(mSolo.searchText(hour));
         assertTrue(mSolo.searchText(min));
     
         Log.i(TAG, "test14_UpdateModifyTime: ending");
     }
    
     public void test15LongClickNote() {
        Log.i(TAG, "test04_LongClickNote: begining");
        if (getCurrentNoteNumber() == 0) {
            Log.e(TAG, "test04_LongClickNote: there is no note in the list");
            addNote(1);
        }
        mSolo.clickLongInList(0);
        mSolo.sleep(mSleeptime);
        assertTrue(mSolo.searchText("1"));
        mSolo.clickOnImage(0);
        Log.i(TAG, "test04_LongClickNote: ending");
    }
      
    public void test16DeleteFirstNote() {
         Log.i(TAG, "test15_DeleteFirstNote: begining");
         int count = getCurrentNoteNumber();
         Log.i(TAG, "count = " + count);
         if (count == 0) {
             Log.e(TAG, "There is no note in the list");
             addNote(1);
         }
         mSolo.clickLongInList(0);
         mSolo.sleep(mSleeptime);

         mSolo.clickOnView(findView(R.id.menu_delete));
         mSolo.sleep(mSleeptime);

         mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
         mSolo.sleep(mSleeptime);

         Log.i(TAG, "test15_DeleteFirstNote: ending");
    }
      
    public void test17DeleteSecondNote() {
    	int countBefore = getCurrentNoteNumber();
        if (countBefore <= 1) {
            Log.e(TAG, "There is no note in the list");
            addNote(2);
        }
        mSolo.clickLongInList(0);
        mSolo.sleep(mSleeptime);
        mSolo.clickLongInList(2);
        mSolo.sleep(mSleeptime);
        mSolo.clickLongInList(1);
        mSolo.sleep(mSleeptime);
        assertTrue(mSolo.searchText("1"));

    }
    
    public void test18CancelDeleteMuti() {
        Log.i(TAG, "test17_CancelDeleteMuti: begining");
        int countBefore = getCurrentNoteNumber();
        if (countBefore <= 1) {
            Log.e(TAG, "There is no note in the list");
            addNote(2);
        }
        mSolo.clickLongInList(0);
        mSolo.sleep(mSleeptime);
        
        mSolo.clickOnView(findView(R.id.select_items)); 
        mSolo.clickOnText(mSolo.getString(R.string.menu_select_all_button));
        mSolo.sleep(mSleeptime);
        assertTrue(mSolo.searchText(String.valueOf(getCurrentNoteNumber()))); 
        mSolo.clickOnView(findView(R.id.menu_delete));
         
        assertTrue(mSolo.searchText(mSolo.getString(android.R.string.cancel))); 
        mSolo.clickOnButton(mSolo.getString(android.R.string.cancel));
        mSolo.sleep(mSleeptime);

        mSolo.clickOnImage(0);
        Log.i(TAG, "test17_CancelDeleteMuti: ending"); 
    }
    
    public void test19MenuOfNoteDelete() {
        Log.i(TAG, "test19_MenuOfNoteDelete: begining");
        if (getCurrentNoteNumber() <= 1) {
            Log.e(TAG, "There is no note in the list");
            addNote(2);
        }
        mSolo.clickLongInList(0);
        mSolo.sleep(mSleeptime);

        mSolo.clickOnView(findView(R.id.select_items)); 
        mSolo.clickOnText(mSolo.getString(R.string.menu_select_all_button));
        mSolo.sleep(mSleeptime);
        assertTrue(mSolo.searchText(String.valueOf(getCurrentNoteNumber())));

        mSolo.clickOnView(findView(R.id.select_items)); 
        mSolo.clickOnText(mSolo.getString(R.string.menu_deselect_all_button));
        mSolo.sleep(mSleeptime);

        mSolo.clickOnImage(0);
        Log.i(TAG, "test19_MenuOfNoteDelete: ending");
    }
    
    public void test20DeleteAllNote() {
        Log.i(TAG, "test21_DeleteAllNote: begining");
        int countBefore = getCurrentNoteNumber();
        if (countBefore == 0) {
            Log.e(TAG, "There is no note in the list");
            addNote(1);
        }
        mSolo.clickLongInList(0);
        mSolo.sleep(mSleeptime);
        
        mSolo.clickOnView(findView(R.id.select_items)); 
        mSolo.clickOnText(mSolo.getString(R.string.menu_select_all_button));
        mSolo.sleep(mSleeptime);
        mSolo.clickOnView(findView(R.id.menu_delete));

        mSolo.clickOnButton(mSolo.getString(android.R.string.ok));
        mSolo.sleep(mSleeptime);

        Log.i(TAG, "test21_DeleteAllNote: ending");
    }

    public void test21DisableMenuWhenNoNote() {
        Log.i(TAG, "test22_DisableMenuWhenNoNote: begining");
        if (getCurrentNoteNumber() == 0) {
            mSolo.sendKey(KeyEvent.KEYCODE_MENU);
            mSolo.sendKey(KeyEvent.KEYCODE_ENTER);
            mSolo.sleep(mSleeptime);
            assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_delete)));

            mSolo.sendKey(KeyEvent.KEYCODE_DPAD_DOWN);
            mSolo.sendKey(KeyEvent.KEYCODE_ENTER);
            mSolo.sleep(mSleeptime);
            assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_sort_by_modify)));
    
            mSolo.sendKey(KeyEvent.KEYCODE_DPAD_DOWN);
            mSolo.sendKey(KeyEvent.KEYCODE_ENTER);
            mSolo.sleep(mSleeptime);
            assertTrue(mSolo.searchText(mSolo.getString(R.string.menu_sort_by_tab)));
        }
        Log.i(TAG, "test22_DisableMenuWhenNoNote: ending");
    }
}


