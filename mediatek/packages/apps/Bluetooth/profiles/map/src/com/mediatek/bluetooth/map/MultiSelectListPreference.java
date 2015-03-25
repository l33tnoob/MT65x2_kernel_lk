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

package com.mediatek.bluetooth.map;

import android.app.AlertDialog.Builder;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnMultiChoiceClickListener;
import android.content.res.TypedArray;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.AttributeSet;
import android.preference.ListPreference;
import java.util.ArrayList;
import android.util.Log;


/*temp solution for multi choice list preference
	if android support it, standard class has to be used instead of this class
 */
public class MultiSelectListPreference extends ListPreference {
	private static final String SEPERATOR = "99899"; 
	private boolean isDialogShowing = false;
	private boolean[] mClickedDialogEntryItems;
	private boolean[] mClickedItemsInHistory;
	private OnMultiChoiceClickListener mMultiChoiceListener = 
		new DialogInterface.OnMultiChoiceClickListener(){
			public void onClick(DialogInterface dialog, int which, boolean val){
				mClickedDialogEntryItems[which] = val;
			}
		};
    
    public MultiSelectListPreference(Context context, AttributeSet attrs) {
        super(context, attrs);        
    }
	@Override
	public void setEntries(CharSequence[] entries){
		super.setEntries(entries);
	}	
	public boolean[] getSelectedItems(){
		return mClickedDialogEntryItems;
	}

	public boolean[] reverseSelectedItems(String value) {
		return parseRecords(value);
	}
	public void setSelectedItems(boolean[] values) {
		StringBuilder summary = new StringBuilder();
		CharSequence[] entries = getEntries();
		setValue(composeRecords(values));
		for (int index = 0; values != null && index < values.length; index ++){
			if (values[index] && entries != null && index < entries.length) {
				summary.append(entries[index]+" ");
				Log.v("MultiSelect", "entries[index]"+entries[index]);
			} 
		}
		setSummary(summary.toString());
	}
    
    @Override
    protected void onPrepareDialogBuilder(Builder builder) {
    	CharSequence[] values = getEntryValues();
		CharSequence[] entries = getEntries();
    	if (entries == null || values == null) {
			return;
		}
	
    		if (isDialogShowing) {
			mClickedDialogEntryItems = mClickedItemsInHistory;
			mClickedItemsInHistory = null;			
		} else {
			String oldItems = getValue();
			mClickedDialogEntryItems = parseRecords(oldItems);
		}

        builder.setMultiChoiceItems(entries, mClickedDialogEntryItems,mMultiChoiceListener);
	isDialogShowing = true;
    }

	

    @Override
    protected void onDialogClosed(boolean positiveResult) {
    	CharSequence[] entries = getEntries();
		isDialogShowing = false;
		if (!positiveResult || entries == null){
			return;
		}
	//	String value = composeRecords(mClickedDialogEntryItems);		
        
        if (callChangeListener(null)) {				
        	setSelectedItems(mClickedDialogEntryItems);
        }		
    }
	private String composeRecords(boolean[] items){
		StringBuilder value = new StringBuilder();
		CharSequence[] entries = getEntries();

		for(int index = 0; items != null && index < items.length; index++){
			if (items[index]) {
				value.append(entries[index].toString()).append(SEPERATOR);	
			}
		}
		return value.toString();
	}
	private boolean[] parseRecords(String value) {	
		int index;
		String[] items;
		boolean[] selectedItems;
		CharSequence[] entries = getEntries();
		if (entries == null) {
			return null;
		}
		selectedItems = new boolean[entries.length];
		if (value == null) {
			return selectedItems;
		}
		items = value.split(SEPERATOR);
		for (String oldItem : items) {			
			for (index = 0; index < entries.length; index++) {
				if (oldItem.equals(entries[index].toString())) {
					break;
				}
			}
			if (index < entries.length) {
				selectedItems[index] = true;	
			}
		}
		return selectedItems;
	}
	
	@Override
    protected Parcelable onSaveInstanceState() {
        final Parcelable superState = super.onSaveInstanceState();
        if (!isDialogShowing) {
            // No need to save instance state if dialig is not showing
            return superState;
        }
        final SavedState myState = new SavedState(superState);
        myState.value = mClickedDialogEntryItems;
		myState.length = (mClickedDialogEntryItems == null)? 0 : mClickedDialogEntryItems.length;
		return myState;
    }

    @Override
    protected void onRestoreInstanceState(Parcelable state) {
        if (state == null || !state.getClass().equals(SavedState.class)) {
            // Didn't save state for us in onSaveInstanceState
            super.onRestoreInstanceState(state);
            return;
        }	
        SavedState myState = (SavedState) state;
		/*Notes: mClickedItemsInHistory has to be saved first, and then call onRestoreInstanceState of parents*/
		/*because onRestoreInstanceState will cause onPrepareDialogBuilder is called*/
        mClickedItemsInHistory = myState.value;
	isDialogShowing = true;
        super.onRestoreInstanceState(myState.getSuperState());
    }
    
    private static class SavedState extends BaseSavedState {
        public boolean[] value;
		public int length;
	
        public SavedState(Parcel source) {
            super(source);
	    length = source.readInt();
            value = new boolean[length];
	    source.readBooleanArray(value);
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
			dest.writeInt(length);
            dest.writeBooleanArray(value);	    
        }

        public SavedState(Parcelable superState) {
            super(superState);
        }

        public static final Parcelable.Creator<SavedState> CREATOR =
                new Parcelable.Creator<SavedState>() {
            public SavedState createFromParcel(Parcel in) {
                return new SavedState(in);
            }

            public SavedState[] newArray(int size) {
                return new SavedState[size];
            }
        };
    }

   
    
}
