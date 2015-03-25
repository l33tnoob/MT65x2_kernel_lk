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
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.ColorStateList;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.BaseColumns;
import android.text.method.ScrollingMovementMethod;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.notebook.NotePad.Notes;

import java.util.Calendar;
import java.util.Locale;

public class NoteReading extends Activity {
    private Uri mUri;
    private static Uri sCurrentUri;
    private Cursor mCursor;
    private AlertDialog mDialog;
    private ProgressDialog mDeleting;
    protected static final String BUNDLE_KEY_DELETE_DIALOG_VISIBLE = "key_delete_dialog_visible";
    private StringBuilder mFilter;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.noteslist_item_reading);
        ActionBar actionbar = getActionBar();
        actionbar.setHomeButtonEnabled(true);
        actionbar.setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP
                | ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_TITLE
                | ActionBar.DISPLAY_USE_LOGO);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (Notes.sNoteCount == 0) {
            finish();
            return;
        }
        if (Notes.sSaveNoteFlag) {
            Toast.makeText(this, R.string.note_saved, Toast.LENGTH_LONG).show();
            Notes.sSaveNoteFlag = false;
        }
        Resources resource = getBaseContext().getResources();
        ColorStateList colorWork = (ColorStateList) resource
                .getColorStateList(R.color.work);
        ColorStateList colorPersonal = (ColorStateList) resource
                .getColorStateList(R.color.personal);
        ColorStateList colorFamily = (ColorStateList) resource
                .getColorStateList(R.color.family);
        ColorStateList colorStudy = (ColorStateList) resource
                .getColorStateList(R.color.study);
        mUri = getIntent().getData();
        mCursor = managedQuery(mUri, Notes.PROJECTION, null, null, null);
        mCursor.moveToFirst();
        TextView mTextGroup = (TextView) findViewById(R.id.group);
        TextView mTextModify = (TextView) findViewById(R.id.modify_time);
        TextView mTextContext = (TextView) findViewById(R.id.context);
        int contextNoteIndex = mCursor.getColumnIndex(Notes.COLUMN_NAME_NOTE);
        int idNoteIndex = mCursor.getColumnIndex(BaseColumns._ID);
        int groupNoteIndex = mCursor.getColumnIndex(Notes.COLUMN_NAME_GROUP);
        int modifyNoteIndex = mCursor
                .getColumnIndex(Notes.COLUMN_NAME_CREATE_DATE);
        String note = mCursor.getString(contextNoteIndex);
        String group = mCursor.getString(groupNoteIndex);
        String modify = mCursor.getString(modifyNoteIndex);
        int noteId = mCursor.getInt(idNoteIndex);
        sCurrentUri = Uri.parse(Notes.CONTENT_URI + "/" + noteId);
        mFilter = new StringBuilder("_id in ('");
        mFilter.append(String.valueOf(sCurrentUri.getPathSegments().get(
                Notes.NOTE_ID_PATH_POSITION)));
        mFilter.append("')");
        mTextContext.setText(note);
        mTextContext.setMovementMethod(ScrollingMovementMethod.getInstance());
        modify = currentDay(modify, this);
        mTextModify.setText(modify);
        String gp = getGroup(group);
        mTextGroup.setText(gp);
        if (gp.equals(getString(R.string.menu_personal))) {
            mTextGroup.setTextColor(colorPersonal);
        } else if (gp.equals(getString(R.string.menu_work))) {
            mTextGroup.setTextColor(colorWork);
        } else if (gp.equals(getString(R.string.menu_family))) {
            mTextGroup.setTextColor(colorFamily);
        } else if (gp.equals(getString(R.string.menu_study))) {
            mTextGroup.setTextColor(colorStudy);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
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

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.note_reading_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.menu_edit_current:
            Intent it = new Intent(this, NoteEdit.class);
            it.setData(sCurrentUri);
            this.startActivity(it);
            break;
        case R.id.menu_delete_current:
            AlertDialog.Builder bld = new AlertDialog.Builder(this);
            bld.setPositiveButton(getString(R.string.delete_confirm_ok),
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            deleteCurrentNote();
                        }
                    });
            bld.setNegativeButton(getString(R.string.delete_confirm_cancel),
                    null);
            bld.setCancelable(true);
            bld.setIconAttribute(android.R.attr.alertDialogIcon);
            bld.setMessage(getString(R.string.delete_confirm_current));
            bld.setTitle(getString(R.string.delete_confirm_title));
            mDialog = bld.create();
            mDialog.show();
            break;
        default:
            finish();
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    public String currentDay(String modifyDay, Context context) {
        Calendar c = Calendar.getInstance();
        String currentDay = String.valueOf(c.get(Calendar.DAY_OF_MONTH));
        String currentYear = String.valueOf(c.get(Calendar.YEAR));
        String currentMonth = String.valueOf(c.get(Calendar.MONTH) + 1);
        String saveYear;
        String saveMonth;
        String saveDay;
        String[] mt = modifyDay.split(" ");
        Resources resource = context.getResources();

        if (Locale.getDefault().getLanguage().equals("en")) {
            saveMonth = mt[1];
            saveDay = mt[2];
            saveYear = mt[0];
            if (mt[1].equals("Jan")) {
                mt[1] = "1";
            } else if (mt[1].equals("Feb")) {
                mt[1] = "2";
            } else if (mt[1].equals("Mar")) {
                mt[1] = "3";
            } else if (mt[1].equals("Apr")) {
                mt[1] = "4";
            } else if (mt[1].equals("May")) {
                mt[1] = "5";
            } else if (mt[1].equals("June")) {
                mt[1] = "6";
            } else if (mt[1].equals("July")) {
                mt[1] = "7";
            } else if (mt[1].equals("Aug")) {
                mt[1] = "8";
            } else if (mt[1].equals("Sep")) {
                mt[1] = "9";
            } else if (mt[1].equals("Oct")) {
                mt[1] = "10";
            } else if (mt[1].equals("Nov")) {
                mt[1] = "11";
            } else if (mt[1].equals("Dec")) {
                mt[1] = "12";
            }
            if (currentYear.equals(saveYear)) {
                if (currentMonth.equals(saveMonth)
                        && currentDay.equals(saveDay)) {
                    return mt[3];
                } else {
                    return Notes.MONTH[Integer.valueOf(mt[1]) - 1] + " "
                            + mt[2] + " " + mt[3];
                }
            } else {
                return Notes.MONTH[Integer.valueOf(mt[1]) - 1] + " " + mt[2]
                        + "," + mt[0] + " " + mt[3];
            }
        } else {
            String displayMonth = (String) resource.getString(R.string.month);
            String displayDay = (String) resource.getString(R.string.day);
            saveYear = mt[0];
            saveMonth = mt[1];
            saveDay = mt[2];
            if (currentYear.equals(saveYear)) {
                if (currentMonth.equals(saveMonth)
                        && currentDay.equals(saveDay)) {
                    return mt[3];
                } else {
                    return mt[1] + displayMonth + mt[2] + displayDay + " "
                            + mt[3];
                }
            } else {
                return mt[0] + "-" + mt[1] + "-" + mt[2] + " " + mt[3];
            }
        }
    }
   
    private void deleteCurrentNote() {
        mDeleting = ProgressDialog.show(this, "",
                getString(R.string.delete_progress), true);
        QueryHandler qh = new QueryHandler(this.getContentResolver(), this);
        qh.startDelete(Notes.NOTEREADING_DELETE_TOKEN, mDeleting,
                Notes.CONTENT_URI, String.valueOf(mFilter), null);
    }
}
