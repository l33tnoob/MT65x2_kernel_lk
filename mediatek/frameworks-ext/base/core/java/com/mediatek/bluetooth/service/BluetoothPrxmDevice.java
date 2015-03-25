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

package com.mediatek.bluetooth.service;

import android.bluetooth.BluetoothDevice;
import android.os.Parcel;
import android.os.Parcelable;

/**
 * @author Jerry Hsu
 */
public class BluetoothPrxmDevice implements Parcelable {

	private BluetoothDevice device;	// bluetooth device info
	private int id = -1;		// ContentProvider implementation
	private byte currentState;	// PrxConstants.PRXM_STATE_
	private byte capability;	// PrxConstants.PRXM_CAP_NONE
	private byte linkLossLevel;	// local config
	private byte pathLossLevel;	// local config
	private byte pathLossThreshold;	// local config
	private byte remoteTxPower;	// remote tx power
	private byte currentRssi;	// current rssi

	private boolean isPathLost = false;	// keep the path loss state before current update
	private int updateState = 0;		// keep the attribute update state: 1: txPower / 2: currentRssi

	/**
	 * Constructor
	 * 
	 * @param device
	 */
	public BluetoothPrxmDevice( BluetoothDevice device ){

		this.device = device;
	}

	/**
	 * get device-name from BluetoothDevice
	 * 
	 * @return
	 */
	public String getName(){

		return this.device.getName();
	}

	/**
	 * get device-address from BluetoothDevice
	 * 
	 * @return
	 */
	public String getAddress(){

		return this.device.getAddress();
	}

	/**
	 * reset update-state when going to start next round of update
	 */
	public void resetUpdateState(){
		this.updateState = 0;
	}
	/**
	 * check all of the attributes have been updated 
	 * 
	 * @return
	 */
	public boolean isUpdateDone(){
		return ( this.updateState == 1 );	// 00000001
	}
	/**
	 * update field bit state when value updated
	 * 
	 * @param field
	 */
	private void commitUpdate( int field ){
		this.updateState |= field;	// 1: currentRssi
	}
	/**
	 * update isPathLost flag according current PathLoss and PathLossThreshold
	 * 
	 * @return
	 */
	public boolean checkPathLoss( byte pathLoss ){

		this.isPathLost = ( this.pathLossThreshold < pathLoss );
		return this.isPathLost;
	}

	public int describeContents(){

		return 0;
	}

	public void writeToParcel( Parcel dest, int flags ){

		dest.writeParcelable( this.device, flags );
		dest.writeByte( this.capability );
		dest.writeByte( this.linkLossLevel );
		dest.writeByte( this.pathLossLevel );
		dest.writeByte( this.pathLossThreshold );
		dest.writeByte( this.remoteTxPower );
		dest.writeByte( this.currentRssi );
	}

	public static final Parcelable.Creator<BluetoothPrxmDevice> CREATOR = new Parcelable.Creator<BluetoothPrxmDevice>(){

		public BluetoothPrxmDevice createFromParcel( Parcel in ){

			BluetoothPrxmDevice pdi = new BluetoothPrxmDevice( (BluetoothDevice)in.readParcelable( null ) );
			pdi.capability = in.readByte();
			pdi.linkLossLevel = in.readByte();
			pdi.pathLossLevel = in.readByte();
			pdi.pathLossThreshold = in.readByte();
			pdi.remoteTxPower = in.readByte();
			pdi.currentRssi = in.readByte();
			return pdi;
		}
		public BluetoothPrxmDevice[] newArray( int size ){

			return new BluetoothPrxmDevice[size];
		}
	};

	@Override
	public boolean equals( Object o ){

		if( o != null && o instanceof BluetoothPrxmDevice ){

			return this.device.equals( ((BluetoothPrxmDevice)o).device );
		}
		return false;
	}

	@Override
	public int hashCode(){

		return this.device.hashCode();
	}

	// Getter / Setter ***************************************************************************************
	public int getId(){
		return this.id;
	}
	public void setId( int id ){
		this.id = id;
	}
	public BluetoothDevice getDevice() {
		return device;
	}
	public void setDevice(BluetoothDevice device) {
		this.device = device;
	}
	public byte getCurrentState() {
		return currentState;
	}
	public void setCurrentState(byte currentState) {
		this.currentState = currentState;
	}
	public byte getCapability() {
		return capability;
	}
	public void setCapability(byte capability) {
		this.capability = capability;
	}
	public byte getLinkLossLevel() {
		return linkLossLevel;
	}
	public void setLinkLossLevel(byte linkLossLevel) {
		this.linkLossLevel = linkLossLevel;
	}
	public byte getPathLossLevel() {
		return pathLossLevel;
	}
	public void setPathLossLevel(byte pathLossLevel) {
		this.pathLossLevel = pathLossLevel;
	}
	public byte getPathLossThreshold() {
		return pathLossThreshold;
	}
	public void setPathLossThreshold(byte pathLossThreshold) {
		this.pathLossThreshold = pathLossThreshold;
	}
	public byte getRemoteTxPower(){
		return this.remoteTxPower;
	}
	public void setRemoteTxPower(byte remoteTxPower) {
		this.remoteTxPower = remoteTxPower;
	}
	public byte getCurrentRssi(){
		return this.currentRssi;
	}
	public void setCurrentRssi(byte currentRssi) {
		this.currentRssi = currentRssi;
		this.commitUpdate(1);
	}
	public boolean isPathLost(){
		return isPathLost;
	}
}
