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

package com.mediatek.bluetooth.prx.monitor;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.CompoundButton.OnCheckedChangeListener;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.service.BluetoothPrxmDevice;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 * 
 * 1. persistence for screen rotate
 * 2. config threshold and pass back to activity
 * 3. 
 *
 */
public class PrxmPathLossThresholdPreference extends DialogPreference implements OnCheckedChangeListener {

	// define the granularity of the seekbar
	protected static final int MAX_PATH_LOSS_THRESHOLD = 100;

	// ui components
	private SeekBar currentRssiSeekbar;		// current rssi
	private CheckBox usingCurrentRssiCheckbox;	// using current rssi as threshold
	private TextView pathLossThresholdTitle;	// path loss threshold title
	private SeekBar pathLossThresholdSeekbar;	// path loss threshold value

	private PrxmDeviceSettingActivity parent;	// parent component to provide model

	private int curPathLoss;

	/**
	 * Constructor
	 * 
	 * @param context
	 * @param attrs
	 */
	public PrxmPathLossThresholdPreference( Context context, AttributeSet attrs ){

		super( context, attrs );

		this.setDialogLayoutResource( R.layout.bt_prxm_path_loss_threshold );
	}

	/**
	 * parent component - data model provider
	 * 
	 * @param parent
	 */
	public void setParent( PrxmDeviceSettingActivity parent ){

		this.parent = parent;
	}
	
	@Override
	protected void onBindDialogView( View view ){

		BtLog.w( "onBindDialogView()[+]" );

		super.onBindDialogView(view);

		// current rssi
		this.currentRssiSeekbar = (SeekBar)view.findViewById( R.id.current_rssi_seekbar );
		this.currentRssiSeekbar.setEnabled( false );	// only can be updated by service
		this.currentRssiSeekbar.setMax( MAX_PATH_LOSS_THRESHOLD );

		// using current rssi checkbox
		this.usingCurrentRssiCheckbox = (CheckBox)view.findViewById( R.id.using_current_rssi_cb );
		this.usingCurrentRssiCheckbox.setChecked( this.getPersistedBoolean(false) );
		this.usingCurrentRssiCheckbox.setOnCheckedChangeListener(this);

		// path loss threshold
		this.pathLossThresholdTitle = (TextView)view.findViewById( R.id.path_loss_threshold_title );
		this.pathLossThresholdSeekbar = (SeekBar)view.findViewById( R.id.path_loss_threshold_seekbar );

		// update data model
		if( this.parent != null && this.parent.getDeviceInfo() != null ){
			BluetoothPrxmDevice pdi = this.parent.getDeviceInfo();
			this.curPathLoss = PrxmUtils.getPathLoss( pdi.getCurrentRssi(), pdi.getRemoteTxPower() );
			this.pathLossThresholdSeekbar.setProgress( pdi.getPathLossThreshold() );
		}
		this.setPathLossThresholdVisibility( !this.usingCurrentRssiCheckbox.isChecked() );
		view.setFocusableInTouchMode( true );
	}

	private void reset(){

		this.usingCurrentRssiCheckbox.setOnCheckedChangeListener( null );
	}

	/**
	 * enable / disable path_loss_threshold setting components.
	 * 
	 * @param visible
	 */
	private void setPathLossThresholdVisibility( boolean visible ){

		int visibility = visible ? View.VISIBLE : View.GONE;
		this.pathLossThresholdTitle.setVisibility( visibility );
		this.pathLossThresholdSeekbar.setVisibility( visibility );
	}

	/**
	 * implements OnCheckedChangeListener
	 */
	public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ){

		// change visibility according to checkbox
		this.setPathLossThresholdVisibility( !isChecked );
	}

	@Override
	protected void onDialogClosed( boolean positiveResult ){

		super.onDialogClosed( positiveResult );

		// user confirm the changes
		if( positiveResult ){

			boolean usingRssi = this.usingCurrentRssiCheckbox.isChecked();

			// save using_rssi_as_path_loss_threshold into shared-preference
			this.persistBoolean( usingRssi );

			// sync path loss threshold from current rssi
			if( usingRssi ){

				this.pathLossThresholdSeekbar.setProgress( this.currentRssiSeekbar.getProgress() );
			}

			// callback to parent
			this.callChangeListener( this.pathLossThresholdSeekbar.getProgress() );
		}

		// release resource
		this.reset();
	}

	public void updateCurPathLoss( int curPathLoss ){

		this.curPathLoss = curPathLoss;
		if( this.currentRssiSeekbar != null ){

			this.currentRssiSeekbar.setProgress( this.curPathLoss );
		}
	}
}
