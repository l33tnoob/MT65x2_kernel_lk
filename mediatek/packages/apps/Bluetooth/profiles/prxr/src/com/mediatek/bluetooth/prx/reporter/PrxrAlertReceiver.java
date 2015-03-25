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

package com.mediatek.bluetooth.prx.reporter;

import java.util.HashMap;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.NotificationFactory;
import com.mediatek.bluetooth.util.SystemUtils;

/**
 * @author Jerry Hsu
 * 
 */
public class PrxrAlertReceiver extends BroadcastReceiver {

	private static HashMap<String, Integer> NOTIFICATION_ID = new HashMap<String, Integer>(3);

	private static int CURRENT_NOTIFICATION_ID = 1;

	@Override
	public void onReceive( Context context, Intent intent ){

		BtLog.d( "PrxrAlertReceiver.onReceive()[+]" );

		String action = intent.getAction();

		if( PrxrConstants.ACTION_LINK_LOSS.equals(action) ){

			BtLog.d( "ProximityReporterReceiver get action: LINK_LOSS" );
			this.handleLossAction( context, intent, true );
		}
		else if( PrxrConstants.ACTION_PATH_LOSS.equals(action) ){

			BtLog.d( "ProximityReporterReceiver get action: PATH_LOSS" );
			this.handleLossAction( context, intent, false );
		}
	}

	/**
	 * 
	 * @param context
	 * @param intent
	 * @param isLinkLoss
	 */
	private void handleLossAction( Context context, Intent intent, boolean isLinkLoss ){

		// get extra: alert-level / remote-device
		byte alertLevel = intent.getByteExtra( PrxrConstants.EXTRA_ALERT_LEVEL, PrxrConstants.PRXR_ALERT_LEVEL_NULL );
		BluetoothDevice remoteDevice = intent.getParcelableExtra( BluetoothDevice.EXTRA_DEVICE );

		BtLog.d( "loss-action: level[" + alertLevel + "], device[" + remoteDevice + "]" );

		// check parameter
		if( alertLevel == PrxrConstants.PRXR_ALERT_LEVEL_NULL ){

			BtLog.w( "can't find alert-level in intent => no alert" );
			return;
		}

		// notify user
		NotificationManager nm = (NotificationManager)context.getSystemService( Context.NOTIFICATION_SERVICE );
		// cancel and re-send
		nm.cancel( this.getNotificationId( (remoteDevice==null) ? null : remoteDevice.getAddress() ) );
		if( alertLevel != PrxrConstants.PRXR_ALERT_LEVEL_NO ){

			// start alert
			Notification n = this.createAlertNotification( context, isLinkLoss, alertLevel, remoteDevice );
			nm.notify( this.getNotificationId( (remoteDevice==null) ? null : remoteDevice.getAddress() ), n );

			// screen on
			SystemUtils.screen( context, 3000 );
		}
	}

	private Notification createAlertNotification( Context context, boolean isLinkLoss, byte alertLevel, BluetoothDevice remoteDevice ){

		BtLog.d( "createAlertNotification[+]: isLinkLoss[" + isLinkLoss + "], alertLevel[" + alertLevel + "]" );
		
		// get resouce according to alert type
		int ticker = isLinkLoss ? R.string.bt_prxr_ll_notification_ticker : R.string.bt_prxr_pl_notification_ticker;
		int title = isLinkLoss ? R.string.bt_prxr_ll_notification_title : R.string.bt_prxr_pl_notification_title;
		int message = isLinkLoss ? R.string.bt_prxr_ll_notification_message : R.string.bt_prxr_pl_notification_message;
		int iconResId = isLinkLoss ? R.drawable.bt_prxr_link_loss_alert : R.drawable.bt_prxr_path_loss_alert;
		int soundResId = isLinkLoss ? R.raw.bt_prxr_link_loss_alert : R.raw.bt_prxr_path_loss_alert;

		// create notification object
		int profileNotificationId = this.getNotificationId( (remoteDevice==null) ? null : remoteDevice.getAddress() );

		Notification.Builder b = new Notification.Builder( context );
		b.setSmallIcon( iconResId );
		b.setTicker( context.getString( ticker ) );
		b.setAutoCancel( true );

		// config intent inside notification
		Intent intent = new Intent();
		PendingIntent pendingIntent = PendingIntent.getActivity( context,
						profileNotificationId,
						intent,
						PendingIntent.FLAG_UPDATE_CURRENT );

		// setup notification
		String deviceName = ( remoteDevice != null ) ? remoteDevice.getName() : "";
		b.setContentTitle( context.getString( title ) );
		b.setContentText( context.getString( message, deviceName ) );
		b.setContentIntent( pendingIntent );

		// config effect: sound + vibration
		b.setDefaults( Notification.DEFAULT_VIBRATE );

		// config period according to alert level
		Notification n = b.getNotification();
		if( alertLevel == PrxrConstants.PRXR_ALERT_LEVEL_HIGH ){

			n.sound = Uri.parse( "android.resource://" + Options.APPLICATION_PACKAGE_NAME + "/" + soundResId );
			n.flags |= Notification.FLAG_INSISTENT;
		}
		else {
			n.sound = null;
		}
		
		// return notification
		return n;
	}

	private int getNotificationId( String address ){

		if( address == null )	return NotificationFactory.getProfileNotificationId( BluetoothProfile.ID_PRXR, 0 );
		if( !NOTIFICATION_ID.containsKey(address) ){

			NOTIFICATION_ID.put( address, CURRENT_NOTIFICATION_ID++ );
		}
		return NotificationFactory.getProfileNotificationId( BluetoothProfile.ID_PRXR, NOTIFICATION_ID.get( address ) );
	}
}
