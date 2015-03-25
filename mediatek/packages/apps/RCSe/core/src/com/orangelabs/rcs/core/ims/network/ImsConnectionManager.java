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

package com.orangelabs.rcs.core.ims.network;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.BatteryManager;
import android.os.Build;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.telephony.TelephonyManager;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnUtils;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.utils.NetworkSwitchInfo;

import java.util.Random;

/**
 * IMS connection manager
 *
 * @author JM. Auffret
 * @author Deutsche Telekom
 */
public class ImsConnectionManager implements Runnable {
	/**
     * IMS module
     */
    private ImsModule imsModule;
    
    /**
     * M: rcse only apn feature @{
     */
    /**
     * Network interfaces
     */
    private ImsNetworkInterface[] networkInterfaces = new ImsNetworkInterface[3];
    /** @} */
    /**
     * IMS network interface
     */
    private ImsNetworkInterface currentNetworkInterface;
    
    /**
     * IMS polling thread
     */
    private Thread imsPollingThread = null;

	private NetworkSwitchInfo netSwitchInfo = new NetworkSwitchInfo();

/**
     * IMS activation flag
     */
    private boolean imsActivationFlag = false;

    /**
     * IMS polling thread Id
     */
    private long imsPollingThreadID = -1;

    /**
     * Connectivity manager
     */
	private ConnectivityManager connectivityMgr;
	
	/**
	 * Network access type
	 */
	private int network;

	/**
	 * Operator
	 */
	private String operator;

	/**
	 * APN
	 */
	private String apn;

    /**
     * Battery level state
     */
    private boolean disconnectedByBattery = false;

    /**
     * IMS services already started
     */
    private boolean imsServicesStarted = false;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    /**
     * M: Added to achieve the auto configuration related feature. @{
     */
    private static final String PREFER_APN_URI = "content://telephony/carriers/preferapn";
    private static final String GEMINI_PREFER_APN_URI = "content://telephony/carriers_gemini/preferapn";
    private static final String APN = "apn";
    private static final boolean GEMINI_PHONE = FeatureOption.MTK_GEMINI_SUPPORT;
    /**
     * @}
     */

 /**
     * M: add for auto-rejoining group chat @{
     */
    /**
     * List of network connectivity listener.
     */
    private RemoteCallbackList<INetworkConnectivity> mListeners = new RemoteCallbackList<INetworkConnectivity>();
    /**
     * Lock used for synchronization
     */
    private Object mLock = new Object();

    /** @} */

  /**
	 * Constructor
	 * 
	 * @param imsModule IMS module
	 * @throws CoreException
	 */
	public ImsConnectionManager(ImsModule imsModule) throws CoreException {
		this.imsModule = imsModule;

		// Get network access parameters
        RcseOnlyApnUtils.createInstance(AndroidFactory.getApplicationContext());
		network = RcsSettings.getInstance().getNetworkAccess();

		// Get network operator parameters
		operator = RcsSettings.getInstance().getNetworkOperator();
		logger.info("operator = " + operator);
		apn = RcsSettings.getInstance().getNetworkApn();
		
		// Set the connectivity manager
		connectivityMgr = (ConnectivityManager)AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
		
        // Instanciates the IMS network interfaces
        networkInterfaces[0] = new MobileNetworkInterface(imsModule);
        networkInterfaces[1] = new WifiNetworkInterface(imsModule);
        /**
         * M: rcse only apn feature @{
         */
        networkInterfaces[2] = new OnlyApnNetworkInterface(imsModule);
        /** @} */

        // Set the mobile network interface by default
		currentNetworkInterface = getMobileNetworkInterface();

		// Load the user profile
		loadUserProfile();
		
		// Register network state listener
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);	
/**
         * M: Modified to resolve the issue that RCS-e server can not be
         * connected. @{
         */
        if (null == networkStateListener) {
            synchronized (mListenerLock) {
                networkStateListener = new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, final Intent intent) {
                        Thread t = new Thread() {
                            public void run() {
                                logger.debug("networkStateListener onReceive() intent = " + intent);
                                connectionEvent(intent);
                            }
                        };
                        t.start();
                    }
                };
                AndroidFactory.getApplicationContext().registerReceiver(networkStateListener,
                        intentFilter);
            }
        } else {
            logger.error("Constructor() networkStateListener is not null");
        }
        /**
         * @}
         */

        // Battery management
        AndroidFactory.getApplicationContext().registerReceiver(batteryLevelListener, new IntentFilter(Intent.ACTION_BATTERY_CHANGED)); 
	}
	
	/**
     * Returns the current network interface
     * 
     * @return Current network interface
     */
	public ImsNetworkInterface getCurrentNetworkInterface() {
		return currentNetworkInterface;
	}

	/**
     * Returns the mobile network interface
     * 
     * @return Mobile network interface
     */
	public ImsNetworkInterface getMobileNetworkInterface() {
		return networkInterfaces[0];
	}
	
	/**
     * Returns the Wi-Fi network interface
     * 
     * @return Wi-Fi network interface
     */
	public ImsNetworkInterface getWifiNetworkInterface() {
		return networkInterfaces[1];
	}
	
	/**
     * M: rcse only apn feature @{
     */
    /**
     * Returns the only apn network interface
     * 
     * @return only apn network interface
     */
    public ImsNetworkInterface getOnlyApnNetworkInterface() {
        return networkInterfaces[2];
    }

    /** @} */

	/**
     * Is connected to Wi-Fi
     * 
     * @return Boolean
     */
	public boolean isConnectedToWifi() {
		if (currentNetworkInterface == getWifiNetworkInterface()) {
			return true;
		} else {
			return false;
		}
	}
	
	/**
     * Is connected to mobile
     * 
     * @return Boolean
     */
	public boolean isConnectedToMobile() {
		if (currentNetworkInterface == getMobileNetworkInterface()) {
			return true;
		} else {
			return false;
		}
	}	

    /**
     * Is disconnected by battery
     *
     * @return Returns true if disconnected by battery, else returns false
     */
    public boolean isDisconnectedByBattery() {
        return disconnectedByBattery;
    }	
	
	/**
	 * Load the user profile associated to the network interface
	 */
	private void loadUserProfile() {
    	ImsModule.IMS_USER_PROFILE = currentNetworkInterface.getUserProfile();
		if (logger.isActivated()) {
			logger.debug("User profile has been reloaded");
	                 logger.info("IMS user profile: " + ImsModule.IMS_USER_PROFILE.toString());
		}
	}
	
	/**
     * Terminate the connection manager
     */
    public void terminate() {
    	if (logger.isActivated()) {
    		logger.info("Terminate the IMS connection manager");
    	}

        // Unregister battery listener
        try {
            AndroidFactory.getApplicationContext().unregisterReceiver(batteryLevelListener);
        } catch (IllegalArgumentException e) {
            // Nothing to do
        }

        // Unregister network state listener
    	try {
    	   /**
         * M: Modified to resolve the issue that RCS-e server can not be
         * connected. @{
         */
        if (null != networkStateListener) {
            synchronized (mListenerLock) {
                if (null != networkStateListener) {
                    logger.debug("terminate() networkStateListener is not null");
                    // Unregister network state listener
                    AndroidFactory.getApplicationContext().unregisterReceiver(networkStateListener);
                    /**
                     * M: Modified to resolve the issue that RCSe service need
                     * to start twice when installed. @{
                     */
                    networkStateListener = null;
                    /**
                     * @}
                     */
                } else {
                    logger.error("terminate() networkStateListener is null in double check");
                }
            }
        } else {
            logger.error("terminate() networkStateListener is null");
        }
        /**
         * @}
         */
        } catch (IllegalArgumentException e) {
        	// Nothing to do
        }

    	// Stop the IMS connection manager
    	stopImsConnection();
    	
    	/**M
    	 * added to how notification of connecting and disconnecting states during registration
    	 */	
    	//send notification to notication bar
    	imsModule.getCore().getListener().handleTryDeregister();
    	/**@*/
    	
    	// Unregister from the IMS
		currentNetworkInterface.unregister();
		    	
    	if (logger.isActivated()) {
    		logger.info("IMS connection manager has been terminated");
    	}
    }

    /**
     * M: Modified to resolve the issue that RCS-e server can not be connected. @{
     */
    /**
     * Network state listener
     */
    private volatile BroadcastReceiver networkStateListener = null;
    /*
	private BroadcastReceiver networkStateListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
        	Thread t = new Thread() {
        		public void run() {
                          logger.debug("networkStateListener onReceive() intent = " + intent);
        			connectionEvent(intent);
        		}
        	};
        	t.start();
        }
    };    
    */
    
    private Object mListenerLock = new Object();
    /**
     * @}
     */

    /**
     * Connection event
     * 
     * @param intent Intent
     */
    private synchronized void connectionEvent(Intent intent) {
    	if (disconnectedByBattery) {
    		return;
    	}
    	
		if (!intent.getAction().equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
			return;
		}
		
		boolean connectivity = intent.getBooleanExtra(ConnectivityManager.EXTRA_NO_CONNECTIVITY, false);
        String reason = intent.getStringExtra(ConnectivityManager.EXTRA_REASON);
        boolean failover = intent.getBooleanExtra(ConnectivityManager.EXTRA_IS_FAILOVER, false);
		if (logger.isActivated()) {
			logger.debug("Connectivity event change: failover=" + failover + ", connectivity=" + !connectivity + ", reason=" + reason);
		}

		// Check received network info
/**
             * M: rcse only apn feature @{
             */
            NetworkInfo onlyApnNetworkInfo = connectivityMgr
                    .getNetworkInfo(ConnectivityManager.TYPE_MOBILE_RCSE);
            if (onlyApnNetworkInfo != null) {
                boolean isConnected = onlyApnNetworkInfo.isConnected();
                if (logger.isActivated()) {
                    logger.debug("connectionEvent() onlyApnNetworkInfo isConnected: " + isConnected);
                }
                if (isConnected) {
                    String localIpAddr = NetworkFactory.getFactory().getLocalIpAddress(onlyApnNetworkInfo.getType());
                    if (currentNetworkInterface == getOnlyApnNetworkInterface()) {
                        // check whether ip has changed.
                        if ((localIpAddr != null)
                                && !localIpAddr.equals(currentNetworkInterface.getNetworkAccess()
                                        .getIpAddress())) {
                            // Disconnect from IMS network interface
                            if (logger.isActivated()) {
                                logger.debug("connectionEvent() onlyApnNetworkInfo IP address has changed");
                            }
							netSwitchInfo.set_ims_off_by_network();
                            disconnectFromIms();
                        }
                    } else {
                        currentNetworkInterface = getOnlyApnNetworkInterface();
                    }
                    if(RcsSettings.getInstance().isRcsOnlyRoamingAuthorized()||!(currentNetworkInterface == getOnlyApnNetworkInterface()) ){
                    connectToIms(localIpAddr);
                    }
                    else if(currentNetworkInterface == getOnlyApnNetworkInterface()&& !RcsSettings.getInstance().isRcsOnlyRoamingAuthorized()){
                    	if(OnlyApnRoamingState.getInstance().displayPopupAndWaitResponse(AndroidFactory.getApplicationContext())){
                    		
                    		connectToIms(localIpAddr);
                    	}
                    	
                    }
                    return;
                }
            }
	    	NetworkInfo networkInfo = connectivityMgr.getActiveNetworkInfo();
            // if it previously use only apn, and now the mobile/wifi network is
            // recovered, stop using only apn.
            if (RcsSettings.getInstance().isRcseOnlyApnEnabled()
                        && !RcseOnlyApnUtils.getInstance().isRcsOnlyApnStarted()) {
                    RcseOnlyApnUtils.getInstance().switchRcseOnlyApn();
            }
            /** @} */
			if ((networkInfo == null) || (currentNetworkInterface == null)) {
			// Disconnect from IMS network interface
			if (logger.isActivated()) {
				logger.debug("Disconnect from IMS: no network (e.g. air plane mode)");
			}
			netSwitchInfo.set_ims_off_by_network();
			disconnectFromIms();
			return;
		}

        // Check if SIM account has changed (i.e. hot SIM swap)
		if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
	        String lastUserAccount = LauncherUtils.getLastUserAccount(AndroidFactory.getApplicationContext());
	        String currentUserAccount = LauncherUtils.getCurrentUserAccount(AndroidFactory.getApplicationContext());
	        if (lastUserAccount != null) {
	            if ((currentUserAccount == null) || !currentUserAccount.equalsIgnoreCase(lastUserAccount)) {
	                imsModule.getCoreListener().handleSimHasChanged();
	                return;
	            }
	        }
		}
		
		// Get the current local IP address
		String localIpAddr = NetworkFactory.getFactory().getLocalIpAddress(networkInfo.getType());
		if (logger.isActivated()) {
			logger.debug("Local IP address for interface " + networkInfo.getTypeName() + " is " + localIpAddr);
		}   				

		// Check if the network access type has changed 
		if (networkInfo.getType() != currentNetworkInterface.getType()) {
			// Network interface changed
			if (logger.isActivated()) {
				logger.info("Data connection state: NETWORK ACCESS CHANGED");
			}

			// Disconnect from current IMS network interface
			if (logger.isActivated()) {
				logger.debug("Disconnect from IMS: network access has changed");
			}
			netSwitchInfo.set_ims_off_by_network();
			disconnectFromIms();
		

// Change current network interface
			if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
				if (logger.isActivated()) {
					logger.debug("Change the network interface to mobile");
				}
				currentNetworkInterface = getMobileNetworkInterface();
			} else
			if (networkInfo.getType() == ConnectivityManager.TYPE_WIFI) {
				if (logger.isActivated()) {
					logger.debug("Change the network interface to Wi-Fi");
				}
				currentNetworkInterface = getWifiNetworkInterface();
			}				

			// Load the user profile for the new network interface
			loadUserProfile();
		} else {
			// Check if the IP address has changed
			if ((localIpAddr != null) &&
					!localIpAddr.equals(currentNetworkInterface.getNetworkAccess().getIpAddress())) {
				// Disconnect from current IMS network interface
				if (logger.isActivated()) {
					logger.debug("Disconnect from IMS: IP address has changed");
				}
					if (logger.isActivated()) {
                        logger.debug("localIpAddr: " + localIpAddr + " current: " + currentNetworkInterface.getNetworkAccess().getIpAddress());
                    }
					netSwitchInfo.set_ims_off_by_network();
				disconnectFromIms();
			}
		}
		
		// Check if there is an IP connectivity
		if (networkInfo.isConnected() && (localIpAddr != null)) {
			if (logger.isActivated()) {
				logger.info("Data connection state: CONNECTED to " + networkInfo.getTypeName());
			}

	// Test roaming
				if (networkInfo.isRoaming() &&
					(networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) &&
						(!RcsSettings.getInstance().isRoamingAuthorized())) {
					if (logger.isActivated()) {
						logger.warn("RCS not authorized in roaming");
					}
					return;
				}
							
// Test network access type
			if ((network != RcsSettingsData.ANY_ACCESS) && (network != networkInfo.getType())) {
				if (logger.isActivated()) {
					logger.warn("Network access " + networkInfo.getTypeName() + " is not authorized");
				}
				return;
			}

			// Test the operator id
			TelephonyManager tm = (TelephonyManager)AndroidFactory.getApplicationContext().getSystemService(Context.TELEPHONY_SERVICE);
			String currentOpe = tm.getSimOperatorName();
			if ((operator.length() > 0) && !currentOpe.equalsIgnoreCase(operator)) {
				if (logger.isActivated()) {
					logger.warn("Operator not authorized");
				}
				return;
			}

            if (Build.VERSION.SDK_INT < 17) { // From Android 4.2, the management of APN is only for system app 
				// Test the default APN configuration if mobile network
/**
                 * M: Added to achieve the auto configuration related feature. @{
                 */
				if (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE) {
		// Test the default APN configuration			
                    ContentResolver cr = AndroidFactory.getApplicationContext()
                            .getContentResolver();
					String currentApn = null;
                    Cursor cursor = null;
                    try {
                        if (logger.isActivated()) {
                            logger.debug("connectionEvent GEMINI_PHONE: " + GEMINI_PHONE);
                        }
                        if (GEMINI_PHONE) {
                            cursor = cr.query(Uri.parse(GEMINI_PREFER_APN_URI), new String[] {
                                APN
                            }, null, null, null);
                        } else {
                            cursor = cr.query(Uri.parse(PREFER_APN_URI), new String[] {
                                APN
                            }, null, null, null);
						}
                        if (cursor != null) {
                            final int apnIndex = cursor.getColumnIndexOrThrow("apn");
                            if (cursor.moveToFirst()) {
                                currentApn = cursor.getString(apnIndex);
                            }
                        }
                    } finally {
                        if (cursor != null) {
                            cursor.close();
                        }
                    }
                    if (logger.isActivated()) {
                        logger.warn("connectionEvent apn = " + apn + ", currentApn = " + currentApn);
					}
					if ((apn.length() > 0) && !apn.equalsIgnoreCase(currentApn)) {
						if (logger.isActivated()) {
							logger.warn("APN not authorized");
						}
//                        return;
					}
				}
              /**
                 * @}
                 */
            }
			// Test the configuration
			if (!currentNetworkInterface.isInterfaceConfigured()) {
				if (logger.isActivated()) {
					logger.warn("IMS network interface not well configured");
				}
				return;
			}
			// Connect to IMS network interface
			if (logger.isActivated()) {
				logger.debug("Connect to IMS");
			}
			connectToIms(localIpAddr);
		} else {
			if (logger.isActivated()) {
				logger.info("Data connection state: DISCONNECTED from " + networkInfo.getTypeName());
			}

			// Disconnect from IMS network interface
			if (logger.isActivated()) {
				logger.debug("Disconnect from IMS: IP connection lost");
			}
			netSwitchInfo.set_ims_off_by_network();
			disconnectFromIms();
			
    	}
    }    
    
    /**
     * Connect to IMS network interface
     * 
     * @param ipAddr IP address
     */
    private void connectToIms(String ipAddr) {
    	// Connected to the network access
		currentNetworkInterface.getNetworkAccess().connect(ipAddr);

		/**
		 * M:update Access Network Info to database. @{T-Mobile
		 */
		currentNetworkInterface.setAccessNetworkInfo();
		/**
		 * @}
		 */
		// Start the IMS connection
		startImsConnection();
    }
    
    /**
     * Disconnect from IMS network interface
     */
    private void disconnectFromIms() {
        /**
         * M: add for auto-rejoining group chat @{
         */
        if (logger.isActivated()) {
            logger.debug("ready to notify prepareToDisconnect");
        }
        final int N = mListeners.beginBroadcast();
        for (int i = 0; i < N; i++) {
            try {
                mListeners.getBroadcastItem(i).prepareToDisconnect();
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mListeners.finishBroadcast();
        /** @} */
		// Stop the IMS connection
		stopImsConnection();

		// Registration terminated 
		currentNetworkInterface.registrationTerminated();
		
		// Disconnect from the network access
		currentNetworkInterface.getNetworkAccess().disconnect();
    }
    
	/**
     * M: add for auto-rejoining group chat @{
     */
    /**
     * Add network connectivity listener
     * 
     * @param listener Listener
     */
    public void addNetworkConnectivityListener(INetworkConnectivity listener) {
        if (logger.isActivated()) {
            logger.info("Add network connectivity listener");
        }

        synchronized (mLock) {
            mListeners.register(listener);
        }
    }

    /**
     * Remove network connectivity listener
     * 
     * @param listener Listener
     */
    public void removeNetworkConnectivityListener(INetworkConnectivity listener) {
        if (logger.isActivated()) {
            logger.info("Remove network connectivity listener");
        }
        synchronized (mLock) {
            mListeners.unregister(listener);
        }
    }

    /** @} */

	/**
	 * Start the IMS connection
	 */
	private synchronized void startImsConnection() {	
if (imsActivationFlag) {
			// Already connected
			return;
		}

	if (imsPollingThreadID >= 0) {
			// Already connected
			return;
		}		
		// Set the connection flag
    	if (logger.isActivated()) {
    		logger.info("Start the IMS connection manager");
    	}
		imsActivationFlag = true;

    	// Start background polling thread
		try {
			imsPollingThread = new Thread(this);
                     imsPollingThreadID = imsPollingThread.getId();
			imsPollingThread.start();
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Internal exception while starting IMS polling thread", e);
			}
		}
	}
	
	/**
	 * Stop the IMS connection
	 */
	private synchronized void stopImsConnection() {		
	if (!imsActivationFlag) {
			// Already disconnected
			return;
		}

if (imsPollingThreadID == -1) {
			// Already disconnected
			return;
		}
		// Set the connection flag
		if (logger.isActivated()) {
    		logger.info("Stop the IMS connection manager");
    	}
        imsPollingThreadID = -1;
	imsActivationFlag = false;

    	// Stop background polling thread
		try {
			imsPollingThread.interrupt();
			imsPollingThread = null;
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Internal exception while stopping IMS polling thread", e);
			}
		}
		
		// Stop IMS services
        if (imsServicesStarted) {
		imsModule.stopImsServices();
            imsServicesStarted = false;
        }
	}

	/**
	 * Background processing
	 */
	public void run() {
    	if (logger.isActivated()) {
    		logger.debug("Start polling of the IMS connection");
    	}
    	
		int servicePollingPeriod = RcsSettings.getInstance().getImsServicePollingPeriod();
		int regBaseTime = RcsSettings.getInstance().getRegisterRetryBaseTime();
		int regMaxTime = RcsSettings.getInstance().getRegisterRetryMaxTime();
		Random random = new Random();
		int nbFailures = 0;

        while (imsActivationFlag && (imsPollingThreadID == Thread.currentThread().getId())) {
	    	if (logger.isActivated()) {
	    		logger.debug("Polling: check IMS connection");
	    	}

	    	// Connection management
    		try {
    	    	// Test IMS registration
    			if (!currentNetworkInterface.isRegistered()) {
    				if (logger.isActivated()) {
    					logger.debug("Not yet registered to IMS: try registration");
    				}
    				/**M
    				 * added to how notification of connecting and disconnecting states during registration
    				 */	
    				//send notification
    				imsModule.getCore().getListener().handleTryRegister();
    				/**@*/
    				
    				// Try to register to IMS
    	    		if (currentNetworkInterface.register()) {
                        // InterruptedException thrown by stopImsConnection() may be caught by one
                        // of the methods used in currentNetworkInterface.register() above
                        if (imsPollingThreadID != Thread.currentThread().getId()) {
                            logger.debug("IMS connection polling thread race condition");
                            break;
                        } else {
                            if (logger.isActivated()) {
                                logger.debug("Registered to the IMS with success: start IMS services");
                            }    	
                            if (imsModule.isReady() && !imsServicesStarted) {
                                imsModule.startImsServices();
                                imsServicesStarted = true;
                        /**
                         * M: add for auto-rejoining group chat @{
                         */
                        if (logger.isActivated()) {
                            logger.debug("ready to notify connect");
                        }
                        final int N = mListeners.beginBroadcast();
                        for (int i = 0; i < N; i++) {
                            try {
                                mListeners.getBroadcastItem(i).connect();
                            } catch (RemoteException e) {
                                if (logger.isActivated()) {
                                    logger.error("Can't notify listener", e);
                                }
                            }
                        }
                        mListeners.finishBroadcast();

                        /** @} */


                            }
    
                            // Reset number of failures
                            nbFailures = 0;
                        }
    	    		} else {
    	            	if (logger.isActivated()) {
    	            		logger.debug("Can't register to the IMS");
    	            	}
    	            	
    	            	// Increment number of failures
    	            	nbFailures++;
    	    		}
    			} else {
                    if (imsModule.isReady()) {
                        if (!imsServicesStarted) {
                            if (logger.isActivated()) {
                                logger.debug("Already registered to IMS: start IMS services");
                            }
                            imsModule.startImsServices();
                            imsServicesStarted = true;
                        } else {
    	        	if (logger.isActivated()) {
    	        		logger.debug("Already registered to IMS: check IMS services");
    	        	}
    	        	imsModule.checkImsServices();
    			}
                    } else {
                        if (logger.isActivated()) {
                            logger.debug("Already registered to IMS: IMS services not yet started");
                        }
                    }
                }
            } catch (Exception e) {
				if (logger.isActivated()) {
		    		logger.error("Internal exception", e);
		    	}
			}

            // InterruptedException thrown by stopImsConnection() may be caught by one
            // of the methods used in currentNetworkInterface.register() above
            if (imsPollingThreadID != Thread.currentThread().getId()) {
                logger.debug("IMS connection polling thread race condition");
                break;
            }

			// Make a pause before the next polling
	    	try {
    			if (!currentNetworkInterface.isRegistered()) {
    				// Pause before the next register attempt
    				double w = Math.min(regMaxTime, (regBaseTime * Math.pow(2, nbFailures)));
    				double coeff = (random.nextInt(51) + 50) / 100.0; // Coeff between 50% and 100%
    				int retryPeriod = (int)(coeff * w);
    	        	if (logger.isActivated()) {
    	        		logger.debug("Wait " + retryPeriod + "s before retry registration (failures=" + nbFailures + ", coeff="+ coeff + ")");
    	        	}
    	        	/**
    	        	* M:Add for closing open port when idle
    	        	*/
    	        	//close the sipstack if until the next regsiter request
    	        	currentNetworkInterface.closeSipStack();
    	        	/**
    	        	* M @}
    	        	*/
    	        	
    				Thread.sleep(retryPeriod * 1000);
	    		} else if (!imsServicesStarted) {
                    int retryPeriod = 5;
                    if (logger.isActivated()) {
                        logger.debug("Wait " + retryPeriod + "s before retry to start services");
                    }
                    Thread.sleep(retryPeriod * 1000);
	    		} else {
    				// Pause before the next service check
	    			Thread.sleep(servicePollingPeriod * 1000);
	    		}
            } catch(InterruptedException e) {
                break;
            }
		}

		if (logger.isActivated()) {
    		logger.debug("IMS connection polling is terminated");
    	}
	}

    /**
     * Battery level listener
     */
    private BroadcastReceiver batteryLevelListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int batteryLimit = RcsSettings.getInstance().getMinBatteryLevel();
            if (batteryLimit > 0) {
                int batteryLevel = intent.getIntExtra("level", 0);
                int batteryPlugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 1);
                if (logger.isActivated()) {
                    logger.info("Battery level: " + batteryLevel + "% plugged: " + batteryPlugged);
                }
                if ((batteryLevel <= batteryLimit) && (batteryPlugged == 0)) {
                    if (!disconnectedByBattery) {
                        disconnectedByBattery = true;

                        // Disconnect
                        disconnectFromIms();
                    }
                } else {
                    if (disconnectedByBattery) {
                        disconnectedByBattery = false;
                        
                        // Reconnect with a connection event
                        connectionEvent(new Intent(ConnectivityManager.CONNECTIVITY_ACTION));
                    }
                }
            } else {
                disconnectedByBattery = false;
            }
        }
    };
}
