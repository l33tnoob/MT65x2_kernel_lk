package com.orangelabs.rcs.core.ims.service.ipcall;

import android.os.RemoteException;

import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.richcall.video.VideoSdpBuilder;
import com.orangelabs.rcs.utils.logger.Logger;


/**
 * Super class for IP Call Hold Manager 
 * 
 * @author O. Magnon
 */
public abstract class  HoldManager {
	/**
	 * Constant values for IPCall Hold states
	 */
	public static final int IDLE = 0; 
	public static final int HOLD_INPROGRESS = 1;
	public static final int HOLD = 2;
	public static final int UNHOLD_INPROGRESS = 3;
	public static final int REMOTE_HOLD_INPROGRESS = 4;
	public static final int REMOTE_HOLD = 5;
	public static final int REMOTE_UNHOLD_INPROGRESS = 6;
	
	/**
	 * Hold state
	 */
	protected static int  state;	
	
	/**
	 * session handled by Hold manager
	 */
	IPCallStreamingSession session ; 	
	
	/**
	 * The logger
	 */
	protected Logger logger = Logger.getLogger(this.getClass().getName());
	
	/**
	 * constructor
	 */
	public HoldManager(IPCallStreamingSession session){
		if (logger.isActivated()){
			logger.info("HoldManager()");
		}
		this.session = session;
	}
	
	/**
	 * get HoldManager state
	 * 
	 * @return int state
	 */
	public static int getState(){
		return state;
	}
	
	/**
	 * set HoldManager state
	 */
	public static void setState(int val){
		state = val;
	}
	
	/**
	 * set call on Hold/onResume (case local HoldManager)
	 * 
	 * @param calHoldAction hold action (true: call hold/false: call resume)
	 */
	public abstract void setCallHold(boolean callHoldAction);
	
	/**
	 * set call on Hold/onResume (case remote HoldManager)
	 * 
	 * @param calHoldAction hold action (true: call hold/false: call resume)
	 * @param reInvite reInvite SIP request received
	 */
	public abstract void setCallHold(boolean callHoldAction, SipRequest reInvite);
		
	/**
	 * set on Hold the media session
	 */
	public abstract void holdMediaSession();
	
	/**
	 * set resume the media session
	 */
	public abstract void resumeMediaSession();
	

}
