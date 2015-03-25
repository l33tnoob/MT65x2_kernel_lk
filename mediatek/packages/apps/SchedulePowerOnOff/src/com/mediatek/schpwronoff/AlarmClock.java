/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.schpwronoff;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.CheckBox;
import android.widget.CursorAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.schpwronoff.R;
import com.mediatek.xlog.Xlog;

import java.text.DateFormatSymbols;
import java.util.Calendar;

/**
 * AlarmClock application.
 */
public class AlarmClock extends PreferenceActivity implements OnItemClickListener {
    private static final String TAG = "AlarmClock";
    static final String PREFERENCES = "AlarmClock";
    static final String PREF_CLOCK_FACE = "face";
    static final String PREF_SHOW_CLOCK = "show_clock";

    /** Cap alarm count at this number */
    static final int MAX_ALARM_COUNT = 12;

    /**
     * This must be false for production. If true, turns on logging, test code, etc.
     */
    static final boolean DEBUG = true;

    private LayoutInflater mFactory;
    private ListView mAlarmsList;
    private Cursor mCursor;
    private String mAm;
    private String mPm;

    /*
     * FIXME: it would be nice for this to live in an xml config file.
     */

    private class AlarmTimeAdapter extends CursorAdapter {
        public AlarmTimeAdapter(Context context, Cursor cursor) {
            super(context, cursor);
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            View ret = mFactory.inflate(R.layout.schpwr_alarm_time, parent, false);
            ((TextView) ret.findViewById(R.id.am)).setText(mAm);
            ((TextView) ret.findViewById(R.id.pm)).setText(mPm);

            DigitalClock digitalClock = (DigitalClock) ret.findViewById(R.id.digitalClock);
            if (digitalClock != null) {
                digitalClock.setLive(false);
            }
            Xlog.d(TAG, "newView " + cursor.getPosition());
            return ret;
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {
            Xlog.d(TAG, "bindView");
            final Alarm alarm = new Alarm(cursor);
            final Context cont = context;
            CheckBox onButton = (CheckBox) view.findViewById(R.id.alarmButton);
            if (onButton != null) {
                onButton.setChecked(alarm.mEnabled);
                onButton.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        boolean isChecked = ((CheckBox) v).isChecked();
                        Alarms.enableAlarm(cont, alarm.mId, isChecked);
                        if (isChecked) {
                            SetAlarm.popAlarmSetToast(cont, alarm.mHour, alarm.mMinutes, alarm.mDaysOfWeek, alarm.mId);
                        }
                    }
                });
            }

            ImageView onOffView = (ImageView) view.findViewById(R.id.power_on_off);
            if (onOffView != null) {
                onOffView.setImageDrawable(getResources().getDrawable(
                        (alarm.mId == 1) ? R.drawable.ic_settings_schpwron : R.drawable.ic_settings_schpwroff));
            }

            DigitalClock digitalClock = (DigitalClock) view.findViewById(R.id.digitalClock);

            // set the alarm text
            final Calendar c = Calendar.getInstance();
            c.set(Calendar.HOUR_OF_DAY, alarm.mHour);
            c.set(Calendar.MINUTE, alarm.mMinutes);
            if (digitalClock != null) {
                digitalClock.updateTime(c);
            }

            // Set the repeat text or leave it blank if it does not repeat.
            TextView daysOfWeekView = (TextView) digitalClock.findViewById(R.id.daysOfWeek);
            final String daysOfWeekStr = alarm.mDaysOfWeek.toString(context, false);
            if (daysOfWeekView != null) {
                if (daysOfWeekStr != null && daysOfWeekStr.length() != 0) {
                    daysOfWeekView.setText(daysOfWeekStr);
                    daysOfWeekView.setVisibility(View.VISIBLE);
                } else {
                    daysOfWeekView.setVisibility(View.GONE);
                }
            }
        }
    };

    @Override
    public boolean onContextItemSelected(final MenuItem item) {
        final AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
        if (item.getItemId() == R.id.enable_alarm) {
            final Cursor c = (Cursor) mAlarmsList.getAdapter().getItem(info.position);
            final Alarm alarm = new Alarm(c);
            Alarms.enableAlarm(this, alarm.mId, !alarm.mEnabled);
            if (!alarm.mEnabled) {
                SetAlarm.popAlarmSetToast(this, alarm.mHour, alarm.mMinutes, alarm.mDaysOfWeek, alarm.mId);
            }
            return true;
        }
        return super.onContextItemSelected(item);
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        String[] ampm = new DateFormatSymbols().getAmPmStrings();
        mAm = ampm[0];
        mPm = ampm[1];
        mFactory = LayoutInflater.from(this);
        mCursor = Alarms.getAlarmsCursor(this.getContentResolver());
        Xlog.d(TAG, "mCursor.getCount() " + mCursor.getCount());
        
        //add which is in onCreateView()
        View v = mFactory.inflate(R.layout.schpwr_alarm_clock, null);
        setContentView(v);
        mAlarmsList = (ListView) v.findViewById(android.R.id.list);
        if (mAlarmsList != null) {
            mAlarmsList.setAdapter(new AlarmTimeAdapter(this, mCursor));
            mAlarmsList.setVerticalScrollBarEnabled(true);
            mAlarmsList.setOnItemClickListener(this);
            mAlarmsList.setOnCreateContextMenuListener(this);
        }
        registerForContextMenu(mAlarmsList);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        View viewFocus = getCurrentFocus();
        int viewId = -1;
        int position = -1;
        if (viewFocus != null) {
            viewId = viewFocus.getId();
            if (viewFocus instanceof ListView) {
                position = ((ListView) viewFocus).getSelectedItemPosition();
            }
        }

        super.onConfigurationChanged(newConfig);
        // updateLayout();

        if (viewId >= 0 && position >= 0) {
            ListView mListView = (ListView)findViewById(viewId);
            mListView.requestFocus();
            mListView.setSelection(position);
        }
    }

    private void updateLayout() {
        LayoutInflater inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View v = inflater.inflate(R.layout.schpwr_alarm_clock, null);
        mAlarmsList = (ListView) v.findViewById(android.R.id.list);
        if (mAlarmsList != null) {
            mAlarmsList.setAdapter(new AlarmTimeAdapter(this, mCursor));
            mAlarmsList.setVerticalScrollBarEnabled(true);
            mAlarmsList.setOnItemClickListener(this);
            mAlarmsList.setOnCreateContextMenuListener(this);
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mCursor.close();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View view, ContextMenuInfo menuInfo) {
        // Inflate the menu from xml.
        getMenuInflater().inflate(R.menu.schpwr_context_menu, menu);

        // Use the current item to create a custom view for the header.
        final AdapterContextMenuInfo info = (AdapterContextMenuInfo) menuInfo;
        final Cursor c = (Cursor) mAlarmsList.getAdapter().getItem(info.position);
        final Alarm alarm = new Alarm(c);

        // Construct the Calendar to compute the time.
        final Calendar cal = Calendar.getInstance();
        cal.set(Calendar.HOUR_OF_DAY, alarm.mHour);
        cal.set(Calendar.MINUTE, alarm.mMinutes);
        final String time = Alarms.formatTime(this, cal);

        // Inflate the custom view and set each TextView's text.
        final View v = mFactory.inflate(R.layout.schpwr_context_menu_header, null);
        TextView textView = (TextView) v.findViewById(R.id.header_time);
        if (textView != null) {
            textView.setText(time);
        }

        // Set the custom view on the menu.
        menu.setHeaderView(v);
        // Change the text to "disable" if the alarm is already enabled.
        if (alarm.mEnabled) {
            menu.findItem(R.id.enable_alarm).setTitle(R.string.disable_schpwr);
        } else {
            menu.findItem(R.id.enable_alarm).setTitle(R.string.enable_schpwr);
        }
    }

    @Override
    public void onItemClick(AdapterView parent, View v, int pos, long id) {
		Xlog.d(TAG, "onItemClick, id is " + id);		
	    Intent intent = new Intent();
		intent.setClass(this, com.mediatek.schpwronoff.SetAlarm.class);
	    final Bundle bundle = new Bundle();
		bundle.putInt(Alarms.ALARM_ID, (int) id);
        intent.putExtras(bundle);
        startActivity(intent);
    }
}
