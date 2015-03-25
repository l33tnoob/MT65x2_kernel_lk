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

package com.orangelabs.rcs.core.ims.network.registration;

import java.util.List;
import java.util.ListIterator;
import java.util.Vector;

import javax.sip.header.ContactHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.header.Header;
import javax.sip.header.ViaHeader;

import android.os.SystemClock;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsError;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.ImsNetworkInterface;
import com.orangelabs.rcs.core.ims.network.sip.SipManager;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.registry.RegistryFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.DeviceUtils;
import com.orangelabs.rcs.utils.PeriodicRefresher;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Registration manager (register, re-register, un-register)
 *
 * @author JM. Auffret
 */
public class RegistrationManager extends PeriodicRefresher {
    /**
	 * Last min expire period key
	 */
	private static final String REGISTRY_MIN_EXPIRE_PERIOD = "MinRegisterExpirePeriod";
	
    /**
     * Expire period
     */
    private int expirePeriod;

    /**
     * Dialog path
     */
    private SipDialogPath dialogPath = null;

    /**
     * Supported feature tags
     */
    private List<String> featureTags;
    
    /**
     * IMS network interface
     */
    private ImsNetworkInterface networkInterface;
 
    /**
     * Registration procedure
     */
    private RegistrationProcedure registrationProcedure;

    /**
     * Instance ID
     */
    private String instanceId = null;
    
	/**
     * Registration flag
     */
    private boolean registered = false;

    /**
     * Registration pending flag
     */
    private boolean registering = false;

    /**
     * UnRegistration need flag
     */
    private boolean needUnregister = false;
	
	/**
	 * Number of 401 failures
	 */
	private int nb401Failures = 0;

     /**
     * M: add to revise registration response which is sometimes blocked by
     * network @{
     */
    /**
     * Max retry count
     */
    private final static int MAX_RETRY_COUNT = 3;


    /** @} */
    
    /**
     * M: add to do unregister when device power off or go to flight mode. @{
     */
    // Power off and flight mode waiting time : 1 second
    private static final int WAIT_TIME_OUT = 1;
    /** @} */

    /**
     * M:add to decide whether add the AccessNetworkInfo Headers(for T-Mobile).@{
     */
    private boolean mIsSecurity = false;
    /**
     * @}
     */

    /** M: Add to fix 403 issue@{ */
    private long mLastRegisteredTimestamp = System.currentTimeMillis();
    private final Object mReregistrationLock = new Object();
    /**
     * @}
     */
	
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    /**
     * Constructor
     * 
     * @param networkInterface IMS network interface
     * @param registrationProcedure Registration procedure
     */
    public RegistrationManager(ImsNetworkInterface networkInterface, RegistrationProcedure registrationProcedure) {
    	this.networkInterface = networkInterface;
        this.registrationProcedure = registrationProcedure;
        this.featureTags = RegistrationUtils.getSupportedFeatureTags();
		this.expirePeriod = RcsSettings.getInstance().getRegisterExpirePeriod();

        if (RcsSettings.getInstance().isGruuSupported()) {
            
            //DeviceUtils get instanceid handles whcih id needs to be used
            this.instanceId = DeviceUtils.getInstanceId(AndroidFactory.getApplicationContext());
        }

       	int defaultExpirePeriod = RcsSettings.getInstance().getRegisterExpirePeriod();
    	int minExpireValue = RegistryFactory.getFactory().readInteger(REGISTRY_MIN_EXPIRE_PERIOD, -1);
    	if ((minExpireValue != -1) && (defaultExpirePeriod < minExpireValue)) {
        	this.expirePeriod = minExpireValue;
    	} else {
    		this.expirePeriod = defaultExpirePeriod;
    	}
        /**
         * M: add to revise registration response which is sometimes blocked by
         * network @{
         */
        if (logger.isActivated()) {
            logger.debug("Current expirePeriod = " + this.expirePeriod);
        }
        /** @} */
    }
	
   
    /**
     * Init the registration procedure
     */
    public void init() {
    }
    
    /**
     * Returns registration procedure
     * 
     * @return Registration procedure
     */
    public RegistrationProcedure getRegistrationProcedure() {
    	return registrationProcedure;
    }
    
    /**
     * Is registered
     * 
     * @return Return True if the terminal is registered, else return False
     */
    public boolean isRegistered() {
        return registered;
    }

    /**
     * Restart registration procedure
     */
    public void restart() {
        Thread t = new Thread() {
            /**
             * Processing
             */
            public void run() {
                // Stop the current registration
                stopRegistration();

                // Start a new registration
                registration();
            }
        };
        t.start();
    }

    /**
     * Registration
     * 
     * @return Boolean status
     */
    public synchronized boolean registration() {    
        /**
         * M: Added to check the version and validity before registration. @{
         */
        String version = RcsSettings.getInstance().getProvisioningVersion();
        if (logger.isActivated()) {
            logger.info("registration()-the version is: " + version);
        }
        boolean disabled = ("-1".equals(version) || "0".equals(version));
        if (disabled) {
            if (!PhoneUtils.isUsingDummyAccount()) {
                if (logger.isActivated()) {
                    logger.debug("registration()-the version is 0, should do auto-config next bootup");
                }
                return false;
            } else {
                if (logger.isActivated()) {
                    logger.debug("registration()-the version is invalid but " +
                            "will still register because of debugging mode");
                }
            }
        }
        long validity = LauncherUtils.isProvisionValidity();
        if (validity < 0) {
            if (!PhoneUtils.isUsingDummyAccount()) {
                if (logger.isActivated()) {
                    logger.debug("registration()-the provision validity is invalid but still register by ignoring validity expire");
                }
                /**
                 * M: commented to bypass the validity expire state and continue to registration. @{
                 */
                
                  //return false;
                /**
                 * @}
                 */
                
            } else {
            if (logger.isActivated()) {
                    logger.debug("registration()-the provision validity is invalid but " +
                    		"will still register because of debugging mode");
                }
            }
        }
        /**
         * @}
         */
        registering = true;

        /**
         * M:set it false to decide can't add the AccessNetworkInfo
         * Headers.@{T-Mobile
         */
        mIsSecurity = false;
        /**
         * @}
         */

        /**
         * M: add to revise registration response which is sometimes blocked by
         * network @{
         */
        for (int retryCount = 0; retryCount < MAX_RETRY_COUNT; retryCount++) {
        try {
            // Create a dialog path if necessary
            if (dialogPath == null) {
            	// Reset the registration authentication procedure
            	registrationProcedure.init();
        		// Set Call-Id
            	String callId = networkInterface.getSipManager().getSipStack().generateCallId();

            	// Set target
            	String target = "sip:" + registrationProcedure.getHomeDomain();

                // Set local party
            	String localParty = registrationProcedure.getPublicUri();

                // Set remote party
            	String remoteParty = registrationProcedure.getPublicUri();

            	// Set the route path
            	Vector<String> route = networkInterface.getSipManager().getSipStack().getDefaultRoutePath();

            	// Create a dialog path
            	dialogPath = new SipDialogPath(
                		networkInterface.getSipManager().getSipStack(),
                		callId,
                		1,
                		target,
                		localParty,
                		remoteParty,
                		route);
            } else {
    	    	// Increment the Cseq number of the dialog path
    	        dialogPath.incrementCseq();
            }

        	// Reset the number of 401 failures
    		nb401Failures = 0;

    		// Create REGISTER request
            SipRequest register = SipMessageFactory.createRegister(dialogPath,
            		featureTags,
            		expirePeriod,
            		instanceId);

             /**@M : 
             * added security header in the first Registration request
             * 
             */
            // Set the security header
    		registrationProcedure.writeSecurityHeader(register);
             /**@*/


            // Send REGISTER request
            sendRegister(register);
                break;
            } catch (RegistrationTimeOutException e) {
                if (logger.isActivated()) {
                    logger.error("Registration has timed out", e);
                }
                if (retryCount == MAX_RETRY_COUNT - 1) {
                    handleError(new ImsError(ImsError.REGISTRATION_FAILED,
                            RegistrationTimeOutException.TIMEOUT_EXCEPTION));
                } else {
                    if (logger.isActivated()) {
                        logger.debug("Registration retry " + (retryCount + 1));
                    }
                    this.stopTimer();
                    this.resetDialogPath();
                }
        } catch (Exception e) {
        	if (logger.isActivated()) {
        		logger.error("Registration has failed", e);
        	}
        	handleError(new ImsError(ImsError.UNEXPECTED_EXCEPTION, e.getMessage()));
                /**
                 * M:Relase period wake lock here @{
                 */
            } finally {
                releasePeriodWakeLock();
            }
            /**
             * @}
             */
        }
        registering = false;
        /** @} */
        return registered;
    }
    
    /**
     * Stop the registration manager without unregistering from IMS
     */
    public synchronized void stopRegistration() {
    	if (!registered) {
			// Already unregistered
			return;
    	}    	

    	// Stop periodic registration
        stopTimer();

        // Force registration flag to false
        registered = false;

        // Reset dialog path attributes
        resetDialogPath();
        
        // Notify event listener
        networkInterface.getImsModule().getCore().getListener().handleRegistrationTerminated();
    }
    
    /**
     * Unregistration
     */
    public synchronized void unRegistration() {
        if (registered) {
            doUnRegistration();
        } else if (registering) {
            needUnregister = true;
        }
    }

    /**
     * Unregistration
     */
    private synchronized void doUnRegistration() {
        needUnregister = false;
    	if (!registered) {
			// Already unregistered
			return;
    	}    	
    	
        try {
            // Stop periodic registration
            stopTimer();

            // Increment the Cseq number of the dialog path
            dialogPath.incrementCseq();
            
            // Create REGISTER request with expire 0
            SipRequest register = SipMessageFactory.createRegister(dialogPath,
            		featureTags,
            		0,
            		instanceId);

            // Send REGISTER request
            sendRegister(register);

        } catch (Exception e) {
        	if (logger.isActivated()) {
        		logger.error("Unregistration has failed", e);
        	}
        }

        // Force registration flag to false
        registered = false;

        // Reset dialog path attributes
        resetDialogPath();
        
        // Notify event listener
        networkInterface.getImsModule().getCore().getListener().handleRegistrationTerminated();
    }

	/**
	 * Send REGISTER message
	 * 
	 * @param register SIP REGISTER
	 * @throws SipException
	 * @throws CoreException
	 */
    private void sendRegister(SipRequest register) throws Exception {
         /**
         * M: Acquire WakeLock @{
         */
        acquirePeriodWakeLock();
        //Release network wake lock if it's held
        releaseNetworkWakeLock();
        /**
         * @}
         */
         if (logger.isActivated()) {
        	logger.info("Send REGISTER, expire=" + register.getExpires());
        }
        /**
         * M: add WakeLock @{
         */
        SipTransactionContext ctx = null;
        try {
            if (registered) {
        // Set the security header
        registrationProcedure.writeSecurityHeader(register);
            }

        // Send REGISTER request
         ctx = networkInterface.getSipManager().sendSipMessageAndWait(register);

        } finally {
            /**
             * 
             * M: Release the wake lock acquired by period process @{
             */
            releasePeriodWakeLock();
            /**
             * @}
             */
        }
        // Wait response
        if (logger.isActivated()) {
            logger.info("Wait response");
        }
        if (ctx != null) {
            ctx.waitResponse(SipManager.TIMEOUT);
        }
        /**@}*/

        // Analyze the received response 
        if (ctx.isSipResponse()) {
            /**
             * M: add WakeLock @{
             */
            acquireNetworkWakeLock();
            try {
        	// A response has been received
            if (ctx.getStatusCode() == 200) {
                    // Reset the number of 401 failures
                    nb401Failures = 0;

        		// 200 OK
        		if (register.getExpires() != 0) {
        			handle200OK(ctx);
        		} else {
        			handle200OkUnregister(ctx);
        		}
            } else
            if (ctx.getStatusCode() == 302) {
        		// 302 Moved Temporarily
            	handle302MovedTemporarily(ctx);
            } else
            if (ctx.getStatusCode() == 401) {
        		// Increment the number of 401 failures
        		nb401Failures++;

                    /**
                    * M: changed the 401 failure limit so on the third time no more request needs tobe 
                    * sent @{
                    */

        		// Check number of failures
                    if (nb401Failures < 2) {
                	// 401 Unauthorized
                	handle401Unauthorized(ctx);
            	     } 
                     /**
                     * @}
                    */
                 else { 
                        // We reached 3 successive 401 failures, stop
                        // registration
                        // retries
                        handleError(new ImsError(ImsError.REGISTRATION_FAILED, "too many 401"));
                    
                        // Reset the number of 401 failures
                        nb401Failures = 0;
                  	}
            }else
            if (ctx.getStatusCode() == 423) {
            	// 423 Interval Too Brief
            	handle423IntervalTooBrief(ctx);
            } else {
            	// Other error response
                    handleError(new ImsError(ImsError.REGISTRATION_FAILED, ctx.getStatusCode()
                            + " " + ctx.getReasonPhrase()));
                }
            } finally {
                releaseNetworkWakeLock();
            }
            /**
             * @}
             */
        } else {
            /**
             * M: add to revise registration response which is sometimes blocked
             * by network @{
             */
        	// No response received: timeout   
                throw new RegistrationTimeOutException();
              /** @} */
        	//handleError(new ImsError(ImsError.REGISTRATION_FAILED, "timeout"));
        }
	}    

	/**
	 * Handle 200 0K response 
	 * 
	 * @param ctx SIP transaction context
	 * @throws SipException
	 * @throws CoreException
	 */
	private void handle200OK(SipTransactionContext ctx) throws Exception {
        // 200 OK response received
    	if (logger.isActivated()) {
    		logger.info("200 OK response received");
    	}

        /** M: Add to fix 403 issue@{ */
    	mLastRegisteredTimestamp = System.currentTimeMillis();
        /** @} */



    	SipResponse resp = ctx.getSipResponse();
    	
        // Set the associated URIs
		ListIterator<Header> associatedHeader = resp.getHeaders(SipUtils.HEADER_P_ASSOCIATED_URI);
		ImsModule.IMS_USER_PROFILE.setAssociatedUri(associatedHeader);
		
		// Set the GRUU
		networkInterface.getSipManager().getSipStack().setInstanceId(instanceId);			
		ListIterator<Header> contacts = resp.getHeaders(ContactHeader.NAME);
		while(contacts.hasNext()) {
			ContactHeader contact = (ContactHeader)contacts.next();
			String contactInstanceId = contact.getParameter(SipUtils.SIP_INSTANCE_PARAM);
			if ((contactInstanceId != null) && (instanceId != null) && (instanceId.contains(contactInstanceId))) {
				String pubGruu = contact.getParameter(SipUtils.PUBLIC_GRUU_PARAM);
				networkInterface.getSipManager().getSipStack().setPublicGruu(pubGruu);			
				String tempGruu = contact.getParameter(SipUtils.TEMP_GRUU_PARAM);
				networkInterface.getSipManager().getSipStack().setTemporaryGruu(tempGruu);
			}
		}
		
        // Set the service route path
		ListIterator<Header> routes = resp.getHeaders(SipUtils.HEADER_SERVICE_ROUTE);
		networkInterface.getSipManager().getSipStack().setServiceRoutePath(routes);
		
    	// If the IP address of the Via header in the 200 OK response to the initial
        // SIP REGISTER request is different than the local IP address then there is a NAT 
    	String localIpAddr = networkInterface.getNetworkAccess().getIpAddress();
    	ViaHeader respViaHeader = ctx.getSipResponse().getViaHeaders().next();
    	String received = respViaHeader.getParameter("received");
    	if (!respViaHeader.getHost().equals(localIpAddr) || ((received != null) && !received.equals(localIpAddr))) {
    		networkInterface.setNatTraversal(true);
    		networkInterface.setNatPublicAddress(received);
    		String viaRportStr = respViaHeader.getParameter("rport");
    		int viaRport = -1;
    		if (viaRportStr != null) {
	    		try {
	    			viaRport = Integer.parseInt(viaRportStr);
	    		} catch (NumberFormatException e) {
	    			if (logger.isActivated()) {
	    				logger.warn("Non-numeric rport value \"" + viaRportStr + "\"");
	    			} 
	    		}
	    	}
    		networkInterface.setNatPublicPort(viaRport);
    		if (logger.isActivated()) {
    			logger.debug("NAT public interface detected: " + received + ":" + viaRport);
    		}      
    	} else {
    	networkInterface.setNatTraversal(false);
    		networkInterface.setNatPublicAddress(null);
    		networkInterface.setNatPublicPort(-1);
    	}        	
        if (logger.isActivated()) {
            logger.debug("NAT traversal detection: " + networkInterface.isBehindNat());
        }
		
        // Read the security header
    	registrationProcedure.readSecurityHeader(resp);

        // Retrieve the expire value in the response
        retrieveExpirePeriod(resp);
        registered = true;
        
        // Start the periodic registration
        /**
         * M: Reduce the frequency of re-registration and meet with IOT
         * ID_RCSE_2_1_1@{
         */
        if (logger.isActivated()) {
            logger.debug("expirePeriod = " + expirePeriod);
        }
        startTimer(expirePeriod - 60, 1);
        //Release network wake lock asap
        releaseNetworkWakeLock();
//        if (expirePeriod <= 1200 ) {
//            startTimer(expirePeriod, 0.5);
//        } else {
//            startTimer(expirePeriod-600);
//        }
        /** @} */

        // Notify event listener
        networkInterface.getImsModule().getCore().getListener().handleRegistrationSuccessful();

        // Start unregister procedure if necessary
        if (needUnregister) {
            doUnRegistration();
        }
        if (logger.isActivated()) {
            logger.info("200 OK response received for registered end at " + SystemClock.elapsedRealtime());
        }
    }

	/**
	 * Handle 200 0K response of UNREGISTER
	 * 
	 * @param ctx SIP transaction context
	 */
	private void handle200OkUnregister(SipTransactionContext ctx) {
        // 200 OK response received
        if (logger.isActivated()) {
            logger.info("200 OK response received");
        }

         // Reset the NAT parameters as we are not expecting any more messages
        // for this registration
    	networkInterface.setNatPublicAddress(null);
    	networkInterface.setNatPublicPort(-1);


	}
	
	/**
	 * Handle 302 response
	 * 
	 * @param ctx SIP transaction context
	 * @throws SipException
	 * @throws CoreException
	 */
	private void handle302MovedTemporarily(SipTransactionContext ctx) throws Exception {
        // 302 Moved Temporarily response received
        if (logger.isActivated()) {
            logger.info("302 Moved Temporarily response received");
        }
        
        // Extract new target URI from Contact header of the received response
		SipResponse resp = ctx.getSipResponse();
        ContactHeader contactHeader = (ContactHeader)resp.getStackMessage().getHeader(ContactHeader.NAME);
		String newUri = contactHeader.getAddress().getURI().toString();
		dialogPath.setTarget(newUri);

		// Increment the Cseq number of the dialog path
		dialogPath.incrementCseq();

		// Create REGISTER request with security token
		if (logger.isActivated()) {
			logger.info("Send REGISTER to new address");
		}
		SipRequest register = SipMessageFactory.createRegister(dialogPath,
				featureTags,
				ctx.getTransaction().getRequest().getExpires().getExpires(),
				instanceId);

		// Send REGISTER request
		sendRegister(register);
	}

	/**
	 * Handle 401 response 
	 * 
	 * @param ctx SIP transaction context
	 * @throws SipException
	 * @throws CoreException
	 */
	private void handle401Unauthorized(SipTransactionContext ctx) throws Exception {
		// 401 response received
    	if (logger.isActivated()) {
    		logger.info("401 response received, nbFailures=" + nb401Failures);
    	}

		/**
		 * M:set it true to decide can add the AccessNetworkInfo
		 * Headers.@{T-Mobile
		 */
		mIsSecurity = true;
		/**
		 * @}
		 */

		SipResponse resp = ctx.getSipResponse();

		// Read the security header
		registrationProcedure.readSecurityHeader(resp);

		// Increment the Cseq number of the dialog path
		dialogPath.incrementCseq();

		// Create REGISTER request with security token
		if (logger.isActivated()) {
			logger.info("Send REGISTER with security token");
		}
		SipRequest register = SipMessageFactory.createRegister(dialogPath,
				featureTags,
				ctx.getTransaction().getRequest().getExpires().getExpires(),
				instanceId);

		// Set the security header
		registrationProcedure.writeSecurityHeader(register);

		// Send REGISTER request
		sendRegister(register);
	}	

	/**
	 * Handle 423 response 
	 *
	 * @param ctx SIP transaction context
	 * @throws SipException
	 * @throws CoreException
	 */
	private void handle423IntervalTooBrief(SipTransactionContext ctx) throws Exception {
		// 423 response received
    	if (logger.isActivated()) {
    		logger.info("423 response received");
    	}

    	SipResponse resp = ctx.getSipResponse();

		// Increment the Cseq number of the dialog path
		dialogPath.incrementCseq();
		
        // Extract the Min-Expire value
        int minExpire = SipUtils.getMinExpiresPeriod(resp);
        if (minExpire == -1) {
            if (logger.isActivated()) {
            	logger.error("Can't read the Min-Expires value");
            }
        	handleError(new ImsError(ImsError.UNEXPECTED_EXCEPTION, "No Min-Expires value found"));
        	return;
        }
        
        // Save the min expire value in the terminal registry
        RegistryFactory.getFactory().writeInteger(REGISTRY_MIN_EXPIRE_PERIOD, minExpire);
        
        // Set the expire value
    	expirePeriod = minExpire;
        
        // Create a new REGISTER with the right expire period
        if (logger.isActivated()) {
        	logger.info("Send new REGISTER");
        }
        SipRequest register = SipMessageFactory.createRegister(dialogPath,
        		featureTags,
        		expirePeriod,
        		instanceId);
        
        // Set the security header
        registrationProcedure.writeSecurityHeader(register);

        // Send REGISTER request
        sendRegister(register);
	}	
	
	/**
	 * Handle error response 
	 * 
	 * @param error Error
	 */
	private void handleError(ImsError error) {
        // Error
    	if (logger.isActivated()) {
    		logger.info("Registration has failed: " + error.getErrorCode() + ", reason=" + error.getMessage());
    	}
        registered = false;
        
        // Registration has failed, stop the periodic registration
		stopTimer();
    	
        // Reset dialog path attributes
        resetDialogPath();
        
        // Notify event listener
        networkInterface.getImsModule().getCore().getListener().handleRegistrationFailed(error);
        
        /**
         * M: Reset the configuration validity and version as 0 when register failed for 3
         * times@{
         */
        if (error.getErrorCode() == ImsError.REGISTRATION_FAILED) {
            if (logger.isActivated()) {
                logger.debug("handleError(), REGISTRATION_FAILED error, so do auto config after next bootup");
            }
            RcsSettings.getInstance().setProvisionValidity(0);
            RcsSettings.getInstance().setProvisioningVersion("0");
            this.stopRegistration();
            /**
             * Added to resolve issue of
             * ALPS00451581-[SW.MT6577JB_PLATFORM][RCSe][Register] when server
             * no response cause register failed,RCSe will cycle
             * login-configuration in the background. @{
             */
            Core.terminateCore();
            /**
             * @}
             */
        } else {
            if (logger.isActivated()) {
                logger.debug("handleError(), Other type error");
            }
        }
        /** @} */
	}
	
	/**
     * Reset the dialog path
     */
    private void resetDialogPath() {
        dialogPath = null;
    }

    /**
     * Retrieve the expire period
     * 
     * @param response SIP response
     */
    private void retrieveExpirePeriod(SipResponse response) {
    	// Extract expire value from Contact header
        ListIterator<Header> contacts = response.getHeaders(ContactHeader.NAME);
	    if (contacts != null) {
	    	while(contacts.hasNext()) {
		    	ContactHeader contact = (ContactHeader)contacts.next();
		    	if (contact.getAddress().getHost().equals(networkInterface.getNetworkAccess().getIpAddress())) {
			    	int expires = contact.getExpires();
				    if (expires != -1) {
			    		expirePeriod = expires;
			    	}
				    return;
		    	}
	    	}
	    }
	    
        // Extract expire value from Expires header
        ExpiresHeader expiresHeader = (ExpiresHeader)response.getHeader(ExpiresHeader.NAME);
    	if (expiresHeader != null) {
    		int expires = expiresHeader.getExpires();
		    if (expires != -1) {
	    		expirePeriod = expires;
	    	}
        }
    }

	/**
     * Registration processing
     */
    public void periodicProcessing() {
        /**
         * M: Acquire WakeLock @{
         */
        acquirePeriodWakeLock();
        /**
         * @}
         */
        // Make a registration
    	if (logger.isActivated()) {
    		logger.info("Execute re-registration, begin at " + SystemClock.elapsedRealtime());
    	}
        registration();
        if (logger.isActivated()) {
            logger.info("Execute re-registration, end at " + SystemClock.elapsedRealtime());
        }
    }

    /**
     * M: add to revise registration response which is sometimes blocked by
     * network @{
     */
    private static final class RegistrationTimeOutException extends Exception {
        private static final long serialVersionUID = 1L;

        public static final String TIMEOUT_EXCEPTION = "Registration timeout";

        public RegistrationTimeOutException() {
            super(TIMEOUT_EXCEPTION);
        }
    }
    /** @} */
    
    /**
     * M: add to do unregister when device power off or go to flight mode. @{
     */
    /**
     * Unregistration
     */
    public synchronized void unNormalUnRegistration() {
        if(logger.isActivated()){
            logger.debug("unNormalUnRegistration()");
        }
        if (registered) {
            doUnNormalUnRegistration();
        } else if (registering) {
            needUnregister = true;
        }
    }
    
    /**
     * Unregistration
     */
    private synchronized void doUnNormalUnRegistration() {
        needUnregister = false;
        if (!registered) {
            // Already unregistered
            return;
        }
        try {
            // Stop periodic registration
            stopTimer();
            // Increment the Cseq number of the dialog path
            dialogPath.incrementCseq();
            // Create REGISTER request with expire 0
            SipRequest register = SipMessageFactory.createRegister(dialogPath, featureTags, 0,
                    instanceId);
            // Send REGISTER request
            sendUnNormalRegister(register);

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unregistration has failed", e);
            }
        }
        // Force registration flag to false
        registered = false;
        // Reset dialog path attributes
        resetDialogPath();
        // Notify event listener
        networkInterface.getImsModule().getCore().getListener().handleRegistrationTerminated();
    }

    /**
     * Send REGISTER message and do not wait response.
     * 
     * @param register SIP REGISTER
     * @throws Exception
     */
    private void sendUnNormalRegister(SipRequest register) throws Exception {
        if (logger.isActivated()) {
            logger.info("sendUnNormalRegister() Send REGISTER, expire=" + register.getExpires());
        }
        if (registered) {
            // Set the security header
            registrationProcedure.writeSecurityHeader(register);
        }
        // Send REGISTER request
        SipTransactionContext ctx = networkInterface.getSipManager()
                .sendSipMessageAndWait(register);
        // Wait response
        if (logger.isActivated()) {
            logger.info("Wait response: 1 seconds");
        }
        ctx.waitResponse(WAIT_TIME_OUT);
        if (logger.isActivated()) {
            logger.info("After wait response: 1 seconds");
        }
    }
    /** @} */

    /**
     * M:add to decide whether add the AccessNetworkInfo Headers.@{T-Mobile
     */
    public boolean isSecurity() {
        return mIsSecurity;
    }
    /**
     * @}
     */

    /**
     * M:add to resolve the issue that sip options response 403 error.@{
     */
    /**
     * Do re-registration for 403 response.
     * @return Return true if do re-registration successfully, otherwise return false.
     */
    public boolean doReregistrationFor403() {
        if (logger.isActivated()) {
            logger.debug("doReregistrationFor403() entry");
        }
        synchronized (mReregistrationLock) {
            long currentTimeStamp = System.currentTimeMillis();
            if (registered
                    && (currentTimeStamp - mLastRegisteredTimestamp) < expirePeriod * 0.5) {
                if (logger.isActivated()) {
                    logger.debug("doReregistrationFor403() : No need to do re-registration");
                }
                return true;
            } else {
                if (logger.isActivated()) {
                    logger.debug("doReregistrationFor403() : Do re-registration for 403 response");
                }
                return registration();
            }
        }
    }
    /**
     * @}
     */
}
