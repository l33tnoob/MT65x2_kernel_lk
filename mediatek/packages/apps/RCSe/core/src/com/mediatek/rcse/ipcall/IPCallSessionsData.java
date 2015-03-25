package com.mediatek.rcse.ipcall;

import java.util.ArrayList;
import java.util.HashMap;

import android.os.IBinder;

import com.orangelabs.rcs.service.api.client.ClientApiListener;
import com.orangelabs.rcs.service.api.client.ImsEventListener;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallEventListener;
import com.orangelabs.rcs.service.api.client.ipcall.IIPCallSession;
import com.orangelabs.rcs.service.api.client.ipcall.IPCallApi;
import com.orangelabs.rcs.utils.logger.Logger;

public class IPCallSessionsData {
	
	/**
	 * Constant values for session direction type
	 */
	public static final int TYPE_INCOMING_IPCALL = 16;
	public static final int TYPE_OUTGOING_IPCALL = 17;


	private static IPCallSessionsData instance = null;
	
	/**
	 * list of established sessions
	 */
	public ArrayList<IBinder> sessions = null;
	
	/**
	 * IP call API object
	 */
	public IPCallApi callApi;

	/**
	 * IP call API connected status
	 */
	public boolean isCallApiConnected;
	
	/**
	 * Client API listener
	 */
	public ClientApiListener callApiListener;
	
	/**
	 * Ims Event listener
	 */
	public ImsEventListener imsEventListener;	
	
	/**
	 * constructor
	 */
	public IPCallSessionsData() {
		this.sessions = null;
		this.callApi = null;
		this.isCallApiConnected= false;
		this.callApiListener = null;
		this.imsEventListener = null;
		
	}
	
	public static IPCallSessionsData getInstance(){
		if (instance == null) {
			instance = new IPCallSessionsData();
		}

		return instance;
	}

}
