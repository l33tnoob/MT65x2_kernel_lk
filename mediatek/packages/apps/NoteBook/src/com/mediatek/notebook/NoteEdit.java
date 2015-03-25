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

package com.mediatek.notebook;

import android.app.ActionBar;
import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.res.ColorStateList;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.text.InputFilter;
import android.text.Spanned;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.notebook.NotePad.Notes;

import java.util.Calendar;

public class NoteEdit extends Activity {
    private static final String TAG = "NoteEdit";
    private Spinner mSpinner; 
    private Context mContext;
    private String mNotegroup;
    private Cursor mCursor;
    private EditText mText;
    private Uri mUri = null;
    private Toast mMaxNoteToast = null;
    private int mMaxLength = 1501;
    private String mNote = null;
    private String mGroup = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        int position = 0;
        super.onCreate(savedInstanceState);
        setContentView(R.layout.noteslist_item_editor);
        mContext = this;
        mSpinner = (Spinner)findViewById(R.id.spinner1);
        ActionBar ab = getActionBar();
        ab.setIcon(R.drawable.ic_title_bar_done);
        ab.setHomeButtonEnabled(true);
        String noteData[] = getData(); 
        LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View customActionBarView = inflater.inflate(
                R.layout.editor_custom_action_bar, null);
        View saveMenuItem = customActionBarView
                .findViewById(R.id.save_menu_item);
        saveMenuItem.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                doSaveAction();
            }
        });
        View cancelMenuItem = customActionBarView
                .findViewById(R.id.cancel_menu_item);
        cancelMenuItem.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        ab.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_SHOW_HOME
                        | ActionBar.DISPLAY_SHOW_TITLE);
        ab.setCustomView(customActionBarView);
        BaseAdapter ba = new BaseAdapter() {
            @Override
            public int getCount() {
                return 5;
            }

            @Override
            public Object getItem(int arg0) {
                return null;
            }

            @Override
            public long getItemId(int arg0) {
                return 0;
            }

            @Override
            public View getView(int arg0, View arg1, ViewGroup arg2) {
                String noteDataUI[] = getData();
                Resources resource = getBaseContext()
                        .getResources();
                ColorStateList textColor = (ColorStateList) resource
                        .getColorStateList(R.color.text);
                LinearLayout layout = new LinearLayout(NoteEdit.this);
                layout.setOrientation(LinearLayout.HORIZONTAL);
                TextView tvcolor = new TextView(NoteEdit.this);
                tvcolor.setPadding(0, 5, 0, 0);
                tvcolor.setWidth((int) (mContext.getResources()
                        .getDisplayMetrics().density * 6 + 0.5f));
                tvcolor.setHeight((int) (mContext.getResources()
                        .getDisplayMetrics().density * 48 + 0.5f));
                switch(arg0) {
                    case 0:
                        tvcolor.setBackgroundResource(R.color.none);
                        break;
                    case 1:
                        tvcolor.setBackgroundResource(R.color.work);
                        break;
                    case 2:
                        tvcolor.setBackgroundResource(R.color.personal);
                        break;
                    case 3:
                        tvcolor.setBackgroundResource(R.color.family);
                        break;
                    case 4:
                        tvcolor.setBackgroundResource(R.color.study);
                        break;
                    default:
                        break;
                }
                layout.addView(tvcolor);
                TextView tvgroup = new TextView(NoteEdit.this);
                tvgroup.setGravity(Gravity.CENTER_VERTICAL|Gravity.RIGHT);
                tvgroup.setText(noteDataUI[arg0]);
                tvgroup.setTextSize(16);
                tvgroup.setPadding(0, 0, (int) (mContext.getResources()
                        .getDisplayMetrics().density * 9 + 0.5f), 0);
                tvgroup.setLayoutParams(new LayoutParams(
                        LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
                tvgroup.setTextColor(textColor);
                layout.addView(tvgroup);
                return layout;
            }
        };
        mSpinner.setAdapter(ba);  
        mSpinner.setOnItemSelectedListener(new SpinnerSelectedListener()); 
        mText = (EditText) findViewById(R.id.note);
        mText.setFilters(new InputFilter[] { new MaxLengthFilter(mMaxLength) });
        mUri = getIntent().getData();
        if (mUri == null
                || mUri.toString().indexOf(Notes.CONTENT_URI.toString()) < 0) {
            mText.setText("");
        } else {
            mCursor = managedQuery(mUri, NotePad.Notes.PROJECTION, null, null,
                    null);
            mCursor.moveToFirst();
            int colNoteIndex = mCursor
                    .getColumnIndex(NotePad.Notes.COLUMN_NAME_NOTE);
            int groupNoteIndex = mCursor
                    .getColumnIndex(NotePad.Notes.COLUMN_NAME_GROUP);
            mNote = mCursor.getString(colNoteIndex);
            mGroup = mCursor.getString(groupNoteIndex);
            String gp = getGroup(mGroup);
            mText.setText(mNote);
            mText.setSelection(mNote.length());
            for (int i = 0; i < noteData.length; i++) {
                if (noteData[i].equals(gp)) {
                    position = i;
                }
            }
        }
        mSpinner.setSelection(position);
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            doSaveAction();
        }
        return super.onKeyDown(keyCode, event);
    }

    class MaxLengthFilter implements InputFilter {
        private int mMaxLength;

        public MaxLengthFilter(int max) {
            mMaxLength = max - 1;
            mMaxNoteToast = Toast.makeText(NoteEdit.this, R.string.editor_full,
                    Toast.LENGTH_SHORT);
        }

        @Override
        public CharSequence filter(CharSequence source, int start, int end,
                Spanned dest, int dstart, int dend) {
        int keep = mMaxLength - (dest.length() - (dend - dstart));
        if (keep < (end - start)) {
            mMaxNoteToast.show();
        }
        if (keep <= 0) {
            return "";
        } else if (keep >= end - start) {
            return null;
        } else {
            return source.subSequence(start, start + keep);
        }
      }
    }

    /*
     * @Override protected void onResume() { super.onResume(); if
     * (mSpinner.isPopupShowing()) { mSpinner.dismissPopup(); } }
     * 
     * @Override public void onConfigurationChanged(Configuration newConfig){
     * super.onConfigurationChanged(newConfig); if (mSpinner.isPopupShowing()) {
     * mSpinner.dismissPopup(); } }
     */
    class SpinnerSelectedListener implements OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> arg0, View arg1, int which,
                long arg3) {
            String noteData[] = getData(); 
            mNotegroup = noteData[which];
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
        }   
    }  

    public String getGroup(String i) {
        Resources resource = this.getResources();
        String groupWork = (String) resource.getString(R.string.menu_work); 
        String groupPersonal = (String) resource
                .getString(R.string.menu_personal);
        String groupFamily = (String) resource.getString(R.string.menu_family);
        String groupStudy = (String) resource.getString(R.string.menu_study);
        if (i.equals("1")) {
            return groupWork;
        } else if (i.equals("2")) {
            return groupPersonal;
        } else if (i.equals("3")) {
            return groupFamily;
        } else if (i.equals("4")) {
            return groupStudy;
        } else {
            return "";
        }
    }

    private void doSaveAction() {
        String text = mText.getText().toString();
        String noteData[] = getData(); 
        int i = 0;
        for (i = 0; i < 5; i ++) {
            if (mNotegroup.equals(noteData[i])) {
                break;
            }
        }
        if (mUri == null
                || mUri.toString().indexOf(Notes.CONTENT_URI.toString()) < 0) {
            mUri = Notes.CONTENT_URI;
            ContentValues values = new ContentValues();
            if (text.equals("")) {
                finish();
                Notes.sSaveNoNote = true;
                return;
            } else {
                values.put(Notes.COLUMN_NAME_GROUP, String.valueOf(i));
                values.put(Notes.COLUMN_NAME_NOTE, text);
                values.put(Notes.COLUMN_NAME_TITLE, text);
            }
            Uri retrunUri = getContentResolver().insert(mUri, values);
            if (retrunUri == null) {
                mUri = null;
                Toast.makeText(this, R.string.sdcard_full, Toast.LENGTH_LONG)
                        .show();
            } else {
                Notes.sFlagNotesChanged = true;
                this.getWindow().setSoftInputMode(
                        WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
                finish();
                Notes.sSaveNoteFlag = true;
            }
        } else {
            if (text.equals("")) {
                Toast.makeText(this, R.string.empty_note, Toast.LENGTH_LONG)
                        .show();
                return;
            } else {
                updateNote(text, null, String.valueOf(i));
                Notes.sFlagNotesChanged = true;
                this.getWindow().setSoftInputMode(
                        WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
                finish();
                Log.i(TAG, "context_before = " + mNote + ","
                        + "context_after = " + text);
                Log.i(TAG, "group_before = " + mGroup + "," + "group_after = "
                        + i);
                if (!text.equals(mNote) || !mGroup.equals(String.valueOf(i))) {
                    Notes.sSaveNoteFlag = true;
                }
            }
        }
    }

    private String[] getData() {
        Resources resource = getBaseContext().getResources();
        String groupWork = (String)resource.getString(R.string.menu_work);
        String groupNone = (String)resource.getString(R.string.menu_none);
        String groupPersonal = (String) resource
                .getString(R.string.menu_personal);
        String groupFamily = (String)resource.getString(R.string.menu_family);
        String groupStudy = (String)resource.getString(R.string.menu_study);
        String[] noteData = { groupNone, groupWork, groupPersonal, groupFamily,
                groupStudy };
        return noteData;
    }

    private void updateNote(String text, String title, String group) {
        String year;
        String month;
        String day;
        String hour;
        String minute;
        int i = 0;
        ContentValues values = new ContentValues();
        values.put(NotePad.Notes.COLUMN_NAME_MODIFICATION_DATE,
                System.currentTimeMillis());
        Calendar c = Calendar.getInstance();
        year = String.valueOf(c.get(Calendar.YEAR));
        month = String.valueOf(c.get(Calendar.MONTH) + 1);
        day = String.valueOf(c.get(Calendar.DAY_OF_MONTH));
        hour = String.valueOf(c.get(Calendar.HOUR_OF_DAY));
        minute = String.valueOf(c.get(Calendar.MINUTE));
        if (c.get(Calendar.HOUR_OF_DAY) < 10) {
            hour = "0" + hour;
        }
        if (c.get(Calendar.MINUTE) < 10) {
            minute = "0" + minute;
        }
        String modifyTime = String.valueOf(year) + " " + month + " " + day
                + " " + hour + ":" + minute;
        values.put(Notes.COLUMN_NAME_CREATE_DATE, modifyTime);
        values.put(Notes.COLUMN_NAME_NOTE, text);
        String noteData[] = getData();
        for (i = 0; i < 5; i ++) {
            if (mNotegroup == noteData[i]) {
                break;
            }
        }
        values.put(Notes.COLUMN_NAME_GROUP, String.valueOf(i));
        getContentResolver().update(mUri, values, null, null);
        Notes.sFlagNotesChanged = true;
    }
}
