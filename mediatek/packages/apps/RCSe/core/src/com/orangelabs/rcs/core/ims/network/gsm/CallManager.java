/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
package com.orangelabs.rcs.core.ims.network.gsm;

import java.util.Timer;
import java.util.TimerTask;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Call manager. Note: for outgoing call the capability request is initiated
 * only when we receive the OPTIONS from the remote because the call state goes
 * directly to CONNETED even if the remote has not ringing. For the incoming
 * call, the capability are requested when phone is ringing.
 * 
 * @author jexa7410
 */
public class CallManager {
	/**
	 * Call state unknown
	 */
	public final static int UNKNOWN = 0;

	/**
	 * Call state ringing
	 */
	public final static int RINGING = 1;

	/**
	 * Call state connected
	 */
	public final static int CONNECTED = 2;

	/**
	 * Call state disconnected
	 */
	public final static int DISCONNECTED = 3;

	/**
	 * IMS module
	 */
	private ImsModule imsModule;

    /**
     * Call state
     */
    private int callState = UNKNOWN;

    /**
     * Remote party
     */
    private static String remoteParty = null;

    /**
     * Multiparty call
     */
    private boolean multipartyCall = false;
    
    /**
     * Call hold
     */
    private boolean callHold = false;

    /**
     * Telephony manager
     */
	private TelephonyManager tm;

	/**
     * M: fix the issue related to rich call capability @{
     */
    /**
     * Connectivity manager
     */
    private ConnectivityManager mConnectivityManager = null;
    /**
     * Outgoing call receiver.
     */
    private OutgoingCallReceiver mOutgoingCallReceiver = null;

    /** @} */

	/**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     * 
     * @param core Core
     */
	public CallManager(ImsModule parent) throws CoreException {
		this.imsModule = parent;

        /**
         * M: fix the issue related to rich call capability @{
         */
		// Instantiate the telephony manager
        tm = (TelephonyManager) AndroidFactory.getApplicationContext().getSystemService(
                Context.TELEPHONY_SERVICE);
        // Instantiate the connectivity manager.
        mConnectivityManager = (ConnectivityManager) AndroidFactory.getApplicationContext()
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        /** @} */
	}

	/**
     * Start call monitoring
     */
	public void startCallMonitoring() {
		if (logger.isActivated()) {
			logger.info("Start call monitoring");
		}

		// Monitor phone state
	    tm.listen(listener, PhoneStateListener.LISTEN_CALL_STATE);
        /**
         * M: fix the issue related to rich call capability @{
         */
        mOutgoingCallReceiver = new OutgoingCallReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_NEW_OUTGOING_CALL);
        AndroidFactory.getApplicationContext()
                .registerReceiver(mOutgoingCallReceiver, intentFilter);
        /** @} */
	}

    /**
     * Stop call monitoring
     */
	public void stopCallMonitoring() {
		if (logger.isActivated()) {
			logger.info("Stop call monitoring");
		}

		// Unmonitor phone state
	    tm.listen(listener, PhoneStateListener.LISTEN_NONE);
        /**
         * M: fix the issue related to rich call capability @{
         */
        AndroidFactory.getApplicationContext().unregisterReceiver(mOutgoingCallReceiver);
        /** @} */
	}

	/**
	 * Phone state listener
	 */
	private PhoneStateListener listener = new PhoneStateListener() {
		public void onCallStateChanged(int state, String incomingNumber) {
			switch(state) {
				case TelephonyManager.CALL_STATE_RINGING:
					if (callState == CallManager.CONNECTED) {
						// Tentative of multipaty call
						return;
					}

					if (logger.isActivated()) {
						logger.debug("Call is RINGING: incoming number=" + incomingNumber);
					}
					
					// Phone is ringing: this state is only used for incoming call
					callState = CallManager.RINGING;

                    /**
                     * M: fix the issue related to rich call capability @{
                     */
					// Set remote party
				    remoteParty = incomingNumber;
                    if (LauncherUtils.getDebugMode(AndroidFactory.getApplicationContext())) {
                        String tmpNumber = PhoneUtils.NORMAL_NUMBER_TO_VODAFONE_ACCOUNT
                                .get(incomingNumber);
                        if (tmpNumber != null) {
                            remoteParty = tmpNumber;
                        }
                    }
                    requestCapabilities(remoteParty);
                    /** @} */

					break;

				case TelephonyManager.CALL_STATE_IDLE:
					if (logger.isActivated()) {
						logger.debug("Call is IDLE: last number=" + remoteParty);
					}
					
					// No more call in progress
					callState = CallManager.DISCONNECTED;
				    multipartyCall = false;
				    callHold = false;

				    // Abort pending richcall sessions
			    	imsModule.getRichcallService().abortAllSessions();

                    if (remoteParty != null) {
				    // Disable content sharing capabilities
						imsModule.getCapabilityService().resetContactCapabilitiesForContentSharing(remoteParty);

                        // Request capabilities to the remote
                        imsModule.getCapabilityService().requestContactCapabilities(remoteParty);
					}

					// Reset remote party
					remoteParty = null;
					break;

				case TelephonyManager.CALL_STATE_OFFHOOK:
					if (callState == CallManager.CONNECTED) {
					    // Request capabilities only if not a multiparty call or call hold
						if (logger.isActivated()) {
							logger.debug("Multiparty call established");
						}
						return;
					}

					if (logger.isActivated()) {
						logger.debug("Call is CONNECTED: connected number=" + remoteParty);
					}

					// Both parties are connected
					callState = CallManager.CONNECTED;

                    // Delay option request 2 seconds according to implementation guideline ID_4_20
                    Timer timer = new Timer();
                    timer.schedule(new TimerTask() {
                        @Override
                        public void run() {
                            // Request capabilities
                            requestCapabilities(remoteParty);
                        }
                    }, 2000);
					break;

				default:
					if (logger.isActivated()) {
						logger.debug("Unknown call state " + state);
					}
					break;
			}
		}
    };

	/**
     * M: fix the issue related to rich call capability @{
     */
    /**
     * Set the remote phone number
     * 
     * @param number Phone number
     */
    public static void setRemoteParty(String number) {
        remoteParty = number;
	}

    /** @} */

	/**
     * Get the remote phone number
     * 
     * @return Phone number
     */
	public String getRemotePhoneNumber() {
        /**
         * M: fix the issue related to rich call capability @{
         */
        if (logger.isActivated()) {
            logger.debug("getRemotePhoneNumber() callState: " + callState + " remoteParty: "
                    + remoteParty);
        }
        /** @} */
		if (callState == CallManager.DISCONNECTED) {
			return null;
		} else {
			return remoteParty;
		}
	}

	/**
     * Returns the calling remote party
     * 
     * @return MSISDN
     */
	public String getRemoteParty() {
        /**
         * M: fix the issue related to rich call capability @{
         */
        if (logger.isActivated()) {
            logger.debug("getRemoteParty() remoteParty: " + remoteParty);
        }
        /** @} */
		return remoteParty;
	}

	/**
     * Is call connected
     * 
     * @return Boolean
     */
	public boolean isCallConnected() {
		return (callState == CONNECTED || callState == RINGING);
	}

	/**
     * M: fix the issue related to rich call capability @{
     */
    /**
     * Is call connected with a given contact
     * 
     * @param contact Contact
     * @return Boolean
     */
	public boolean isCallConnectedWith(String contact) {
        String remoteNumber = getRemotePhoneNumber();
        if (logger.isActivated()) {
            logger.debug("isCallConnectedWith() contact: " + contact + " remoteNumber: "
                    + remoteNumber);
        }
        return (isCallConnected() && PhoneUtils.compareNumbers(contact, remoteNumber));
	}

    /** @} */

    /**
     * M: fix the issue related to rich call capability @{
     */
	/**
	 * Is richcall supported with a given contact
	 * 
	 * @param contact Contact
	 * @return Boolean
	 */
	public boolean isRichcallSupportedWith(String contact) {
        boolean richCall = false;
		if (this.multipartyCall || this.callHold) {
            if (logger.isActivated()) {
                logger.debug("isRichcallSupportedWith() multipartyCall: " + multipartyCall
                        + "callHold: " + callHold);
            }
            return richCall;
        }
        richCall = isRichCallSupportedByNetwork();
        if (logger.isActivated()) {
            logger.debug("isRichcallSupportedWith() richCall: " + richCall);
        }
        return richCall;
    }

    private boolean isRichCallSupportedByNetwork() {
        NetworkInfo onlyApnNetworkInfo = mConnectivityManager
                .getNetworkInfo(ConnectivityManager.TYPE_MOBILE_RCSE);
        if (onlyApnNetworkInfo != null) {
            boolean isConnected = onlyApnNetworkInfo.isConnected();
            if (logger.isActivated()) {
                logger.debug("isRichCallSupportedByNetwork() isConnected: " + isConnected);
            }
            if (isConnected) {
                return true;
            }
        }
        NetworkInfo networkInfo = mConnectivityManager.getActiveNetworkInfo();
        if (networkInfo == null) {
            if (logger.isActivated()) {
                logger.debug("isRichCallSupportedByNetwork() getActiveNetworkInfo is null");
            }
            return false;
        }
        int type = networkInfo.getType();
        if (logger.isActivated()) {
            logger.debug("isRichCallSupportedByNetwork() type: " + type);
        }
        if (type == ConnectivityManager.TYPE_WIFI) {
            return true;
        }
        int subType = tm.getNetworkType();
        if (logger.isActivated()) {
            logger.debug("isRichCallSupportedByNetwork() subType: " + subType);
        }
        if (subType == TelephonyManager.NETWORK_TYPE_HSPA
                || subType == TelephonyManager.NETWORK_TYPE_LTE) {
            return true;
        } else if (subType == TelephonyManager.NETWORK_TYPE_EDGE
                || subType == TelephonyManager.NETWORK_TYPE_GPRS) {
			return false;
        } else {
            // TODO to handle 3G type, to enable ONLY ONE WAY.
            return true;
        }
		}
		
    /**
     * @}
     */
	
	/**
     * Is a multiparty call
     * 
     * @return Boolean
     */
	public boolean isMultipartyCall() {
		return multipartyCall;
	}	
	
	/**
     * Is call hold
     * 
     * @return Boolean
     */
	public boolean isCallHold() {
		return callHold;
	}

	/**
     * Request capabilities to a given contact
     * 
     * @param contact Contact
     */
	private void requestCapabilities(String contact) {
		 if ((contact != null) && (contact.length() > 0) && imsModule.getCapabilityService().isServiceStarted()) {
			if (logger.isActivated()) {
				logger.debug("Request capabilities to " + contact);
			}
			imsModule.getCapabilityService().requestContactCapabilities(contact);
		 }
    }
	
	/**
	 * Call leg has changed
	 */
	private void callLegHasChanged() {
		if (multipartyCall | callHold) {
		    // Abort pending richcall sessions if call hold or multiparty call
	    	imsModule.getRichcallService().abortAllSessions();
    	}
		
		// Request new capabilities
    	requestCapabilities(remoteParty);
	}
	
	/**
	 * Set multiparty call
	 * 
	 * @param state State
	 */
	public void setMultiPartyCall(boolean state) {
		if (logger.isActivated()) {
			logger.info("Set multiparty call to " + state);
		}
		this.multipartyCall = state;
		
		callLegHasChanged();	
	}

	/**
	 * Set call hold
	 * 
	 * @param state State
	 */
	public void setCallHold(boolean state)  {
		if (logger.isActivated()) {
			logger.info("Set call hold to " + state);
		}
		this.callHold = state;
		
		callLegHasChanged();	
	}
	
	/**
	 * Connection event
	 * 
	 * @param connected Connection state
	 */
	public void connectionEvent(boolean connected) {
		if (remoteParty == null) {
			return;
		}
		
		if (connected) {
			if (logger.isActivated()) {
				logger.info("Connectivity changed: update content sharing capabilities");
			}

			// Update content sharing capabilities
			requestCapabilities(remoteParty);
		} else {
			if (logger.isActivated()) {
				logger.info("Connectivity changed: disable content sharing capabilities");
			}

			// Disable content sharing capabilities
			imsModule.getCapabilityService().resetContactCapabilitiesForContentSharing(remoteParty);
		}
	}
     /**
     * M:  convert to dummy numbers , this is needed for running with non RCS numbers @{
     */

    private class OutgoingCallReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String number = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
            if (logger.isActivated()) {
                logger.info("onReceive() number: " + number);
            }
            if (LauncherUtils.getDebugMode(AndroidFactory.getApplicationContext())) {
                String tmpNumber = PhoneUtils.NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.get(number);
                if (tmpNumber != null) {
                    number = tmpNumber;
                }
            }
            if (logger.isActivated()) {
                logger.info("onReceive() associated vodafone account: " + number);
            }
            setRemoteParty(number);
        }
        /** @} */
    }
}
