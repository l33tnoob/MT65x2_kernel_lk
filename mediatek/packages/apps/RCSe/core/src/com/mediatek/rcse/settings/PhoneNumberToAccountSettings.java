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

package com.mediatek.rcse.settings;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;

import android.content.DialogInterface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.AdapterView.OnItemSelectedListener;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Set;

/**
 * Set up the relationship between phone number and vodafone account.
 */
public class PhoneNumberToAccountSettings extends Activity {
    private LayoutInflater mLayoutInflater = null;
    private AutoCompleteTextView mNumber = null;
    private Spinner mAccount = null;
    private String vf_account = ProvisionProfileSettings.ACCOUNTS[0];
    private ArrayList<Member> mNumberAccountList = new ArrayList<Member>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLayoutInflater = LayoutInflater.from(this);
        PhoneUtils.initContext(getApplicationContext());
        HashMap<String, String> map = PhoneUtils.NORMAL_NUMBER_TO_VODAFONE_ACCOUNT;
        Set<Entry<String, String>> entrys = map.entrySet();
        for (Entry<String, String> entry : entrys) {
            mNumberAccountList.add(new Member(entry.getKey(), entry.getValue()));
        }
        SettingsDialog dialog = new SettingsDialog();
        dialog.show(getFragmentManager(), SettingsDialog.TAG);
    }

    public final class SettingsDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        public static final String TAG = "SettingsDialog";
        private ListView mListView = null;
        private NumberAccountAdapter mAdapter = null;

        public SettingsDialog() {

        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.d(TAG, "onClick which: " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                String number = mNumber.getText().toString();
                String account = mAccount.getSelectedItem().toString();
                Member member = new Member(number, account);
                if (mNumberAccountList.contains(member)) {
                    mNumberAccountList.remove(member);
                }
                mNumberAccountList.add(member);
                mAdapter.notifyDataSetChanged();
                PhoneUtils.addMapEntry(number, account);
            }
            dialog.dismiss();
            finish();
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            dialog.dismiss();
            finish();
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            View view =
                    LayoutInflater.from(getActivity()).inflate(R.layout.number_to_account, null);
            mNumber = (AutoCompleteTextView) view.findViewById(R.id.phone_number_content);
            mNumber.requestFocus();
            mAccount = (Spinner) view.findViewById(R.id.vf_account_content);
            mListView = (ListView) view.findViewById(R.id.mapped_data);
            mAdapter = new NumberAccountAdapter();
            mListView.setAdapter(mAdapter);
            OnItemSelectedListener accountListener= new OnItemSelectedListener(){

                public void onItemSelected(AdapterView<?> parent, View view, int pos,
                        long id) {
                	
                	vf_account = ProvisionProfileSettings.ACCOUNTS[pos];
                    
                }
                public void onNothingSelected(AdapterView<?> arg0) {
                    //nothing selected

                }
            };
            mAccount.setOnItemSelectedListener(accountListener);
            
            ArrayAdapter<String> numberAdapter = new ArrayAdapter<String>(PhoneNumberToAccountSettings.this,
                    android.R.layout.select_dialog_item, ProvisionProfileSettings.GSM_NUMBERS);
            mNumber.setThreshold(1);
            mNumber.setAdapter(numberAdapter);
            mNumber.setOnTouchListener(new View.OnTouchListener() {
				
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					mNumber.showDropDown();
					return false;
				}
			});
            
            ArrayAdapter<String> accountAdapter = new ArrayAdapter<String>(PhoneNumberToAccountSettings.this,
	        		android.R.layout.simple_spinner_item, ProvisionProfileSettings.ACCOUNTS);
	        accountAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    	mAccount.setAdapter(accountAdapter);
            final AlertDialog alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setView(view);
            alertDialog.setTitle(R.string.number_to_account_settings);
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }
    }

    private class NumberAccountAdapter extends BaseAdapter {

        @Override
        public int getCount() {
            return mNumberAccountList.size();
        }

        @Override
        public Object getItem(int position) {
            return mNumberAccountList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View result;
            if (convertView == null) {
                result = mLayoutInflater.inflate(R.layout.number_to_account_item, null);
            } else {
                result = convertView;
            }
            final Member member = mNumberAccountList.get(position);
            TextView numberView = (TextView) result.findViewById(R.id.item_number);
            numberView.setText(member.mNumber);
            TextView accountView = (TextView) result.findViewById(R.id.item_account);
            accountView.setText(member.mAccount);
            result.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mNumber.setText(member.mNumber);
                    Integer i = (Integer) v.getTag();
                    Member member = mNumberAccountList.get(i);
                    int index = Arrays.asList(ProvisionProfileSettings.ACCOUNTS).indexOf(member.mAccount);
                    mAccount.setSelection(index);
                }
            });
            result.setTag(new Integer(position));
            return result;
        }

    }

    private static final class Member {
        public String mNumber;
        public String mAccount;

        public Member(String number, String account) {
            mNumber = number;
            mAccount = account;
        }
    }
}
