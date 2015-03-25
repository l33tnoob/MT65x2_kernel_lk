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

package com.orangelabs.rcs.core.ims.service.capability;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.database.Cursor;
import android.os.Build;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;

import com.orangelabs.rcs.addressbook.AddressBookEventListener;
import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.contacts.ContactInfo;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import java.util.HashSet;
import java.util.List;
import java.util.HashMap;
import java.util.Map;



/**
 * Capability discovery service
 * 
 * @author jexa7410
 */
public class CapabilityService extends ImsService implements AddressBookEventListener {
	/**
	 * Capability refresh timeout in seconds
	 */
	private static final int CAPABILITY_REFRESH_PERIOD = RcsSettings.getInstance().getCapabilityRefreshTimeout();

	/**
	 * Options manager
	 */
	private OptionsManager optionsManager;

	/**
	 * Anonymous fetch manager
	 */
	private AnonymousFetchManager anonymousFetchManager;

	/**
	 * Polling manager
	 */
	private PollingManager pollingManager;

	/**
     * Flag: set during the address book changed procedure, if we are notified
     * of a change
     */
	private boolean isRecheckNeeded = false;

	/**
     * Flag indicating if a check procedure is in progress
     */
	private boolean isCheckInProgress = false;

	/**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /** M: add for SIP OPTION optimization @{ */
    /**
     * Numbers that are not already RCSe.
     */
    private ArrayList<String> mNotRcseNumbers = new ArrayList<String>();
    /**
     * Object used to synchronize
     */
    private Object mObject = new Object();

	private  Map<String, Long> mRequestContactTime = new HashMap<String, Long>();
    /**
     * Background service executor
     */
    private ExecutorService mExecutorService = null;

    /** @} */

    /**
     * Constructor
     * 
     * @param parent IMS module
     * @throws CoreException
     */
	public CapabilityService(ImsModule parent) throws CoreException {
        super(parent, true);

    	// Instanciate the polling manager
        pollingManager = new PollingManager(this);

    	// Instanciate the options manager
		optionsManager = new OptionsManager(parent);

    	// Instanciate the anonymous fetch manager
    	anonymousFetchManager = new AnonymousFetchManager(parent);

    	// Get capability extensions
    	CapabilityUtils.updateExternalSupportedFeatures(AndroidFactory.getApplicationContext());
	}

	/**
	 * Start the IMS service
	 */
	public synchronized void start() {
		if (isServiceStarted()) {
			// Already started
			return;
		}
		setServiceStarted(true);

		// Start options manager
		optionsManager.start();

		// Listen to address book changes

		// Start polling
		pollingManager.start();

        /** M: add for SIP OPTION optimization @{ */
		// Force a first capability check
        if (mExecutorService == null) {
            mExecutorService = Executors.newSingleThreadExecutor();
        }
        mExecutorService.execute(new Runnable() {
            @Override
			public void run() {
                handleAddressBookHasChanged(false);
                getImsModule().getCore().getAddressBookManager()
                        .addAddressBookListener(CapabilityService.this);
			}
        });
        /** @} */
	}

    /**
     * Stop the IMS service
     */
	public synchronized void stop() {
		if (!isServiceStarted()) {
			// Already stopped
			return;
		}
		setServiceStarted(false);

		// Stop options manager
		optionsManager.stop();

		// Stop polling
		pollingManager.stop();

        /** M: add for SIP OPTION optimization @{ */
        if (mExecutorService != null) {
            mExecutorService.shutdown();
            mExecutorService = null;
        }
        /** @} */

		// Stop listening to address book changes
		getImsModule().getCore().getAddressBookManager().removeAddressBookListener(this);
	}

	/**
     * Check the IMS service
     */
	public void check() {
	}

	/**
     * Get the options manager
     * 
     * @return Options manager
     */
	public OptionsManager getOptionsManager() {
		return optionsManager;
	}

    /**
     * Get the options manager
     * 
     * @return Options manager
     */
	public AnonymousFetchManager getAnonymousFetchManager() {
		return anonymousFetchManager;
	}

	/**
     * Request contact capabilities
     * 
     * @param contact Contact
     * @return Capabilities
     */
	public synchronized Capabilities requestContactCapabilities(String contact) {
    	if (logger.isActivated()) {
    		logger.debug("Request capabilities to " + contact);
    	}

		if(null != mRequestContactTime){
			mRequestContactTime = new HashMap<String, Long>();
		}

    	// Extract contact phone number
		contact = PhoneUtils.extractNumberFromUri(contact);
        /** M: add for SIP OPTION optimization @{ */
        String myself = PhoneUtils.extractNumberFromUri(ImsModule.IMS_USER_PROFILE.getPublicUri());
        if (contact != null && contact.equalsIgnoreCase(myself)) {
            if (logger.isActivated()) {
                logger.debug("contact is myself, so do not exchange capability");
            }
            return new Capabilities();
        }
        if (!PhoneUtils.isVodafoneValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug("requestContactCapabilities " + contact
                        + " is not RCSe contact, so do not send SIP OPTION");
            }
            return new Capabilities();
        }
        /** @} */
		// Read capabilities from the database
		Capabilities capabilities = ContactsManager.getInstance().getContactCapabilities(contact);
		
		if(!(mRequestContactTime.isEmpty())){


			
				long lastTime = mRequestContactTime.get(contact);


				
				long currentTime = System.currentTimeMillis();		
				long delta = (currentTime  - lastTime )/1000 ;
				if(!(delta > 0.8 || delta < 0)){
					return capabilities;
					}
			}
				
		if (capabilities == null) {
	    	if (logger.isActivated()) {
	    		logger.debug("No capability exist for " + contact);
	    	}

			mRequestContactTime.put(contact, System.currentTimeMillis());
            // New contact: request capabilities from the network
    		optionsManager.requestCapabilities(contact);
		} else {
	    	if (logger.isActivated()) {
	    		logger.debug("Capabilities exist for " + contact);
	    	}
			long delta = (System.currentTimeMillis()-capabilities.getTimestamp())/1000;
			if (logger.isActivated()) {
	    		logger.debug("requestContactCapabilities delata for conatct" + contact + "is" + delta);
	    	}
			if ((delta >= CAPABILITY_REFRESH_PERIOD) || (delta < 0)) {
		    	if (logger.isActivated()) {
		    		logger.debug("Capabilities have expired for " + contact);
		    	}

				mRequestContactTime.put(contact, System.currentTimeMillis());
		    	// Capabilities are too old: request capabilities from the network
	    		optionsManager.requestCapabilities(contact);
			}
		}
		return capabilities;
    }

    /**
     * Request capabilities for a list of contacts
     * 
     * @param contactList List of contacts
     */
	public void refreshContactCapabilities(String contact) {

	    if (logger.isActivated()) {
    		logger.debug("Refresh  capabilities to " + contact);
    	}
	    String myself = PhoneUtils.extractNumberFromUri(ImsModule.IMS_USER_PROFILE.getPublicUri());
        if (contact != null && contact.equalsIgnoreCase(myself)) {
            if (logger.isActivated()) {
                logger.debug("contact is myself, so do not exchange capability");
            }
        }  	
		else
    	    optionsManager.requestCapabilities(contact);
	}

    /**
     * Request capabilities for a list of contacts
     * 
     * @param contactList List of contacts
     */
	public void requestContactCapabilities(List<String> contactList) {
    	if ((contactList != null) && (contactList.size() > 0)) {
        	if (logger.isActivated()) {
        		logger.debug("Request capabilities for " + contactList.size() + " contacts");
        	}
			HashSet<String> setContacts = new HashSet<String>(contactList);
			for (String contact : setContacts) {
				requestContactCapabilities(contact);
			}
    		//optionsManager.requestCapabilities(contactList);
    	}
		if (logger.isActivated()) {
        	logger.debug("requestContactCapabilities for list exit");
    	}
	}	
	
    /**
     * Receive a capability request (options procedure)
     * 
     * @param options Received options message
     */
    public void receiveCapabilityRequest(SipRequest options) {
    	optionsManager.receiveCapabilityRequest(options);
    }

	/**
     * Receive a notification (anonymous fecth procedure)
     * 
     * @param notify Received notify
     */
    public void receiveNotification(SipRequest notify) {
    	anonymousFetchManager.receiveNotification(notify);
    }

    /** M: add for SIP OPTION optimization @{ */
    /**
     * Notify when no response for option is returned.
     * 
     * @param contact The contact whose option retrieve no response.
     */
    public void optionWithoutResponse(String contact) {
        if (logger.isActivated()) {
            logger.debug("optionWithoutResponse contact: " + contact);
        }
        String number = PhoneUtils.formatNumberToInternational(contact);
        if (logger.isActivated()) {
            logger.debug("optionWithoutResponse formated number: " + number);
        }
        synchronized (mObject) {
            if (mNotRcseNumbers.contains(number)) {
                mNotRcseNumbers.remove(number);
            }
        }
    }

	/**
	 * Address book content has changed
	 */
	public void handleAddressBookHasChanged() {
        handleAddressBookHasChanged(true);
    }

    /**
     * Address book content has changed
     * 
     * @param needCheck If true, it should check that numbers has changed,
     *            otherwise do not check.
     */
    public void handleAddressBookHasChanged(boolean needCheck) {
		// Update capabilities for the contacts that have never been queried
		if (isCheckInProgress) {
            if (logger.isActivated()) {
                logger.debug("handleAddressBookHasChanged isCheckInProgress is true, wait to update");
            }
			isRecheckNeeded = true;
			return;
		}

		// We are beginning the check procedure
		isCheckInProgress = true;

		// Reset recheck flag
		isRecheckNeeded = false;

		// Check all phone numbers and query only the new ones
        String[] projection = {
                Phone._ID, Phone.NUMBER, Phone.RAW_CONTACT_ID 
                };
        Cursor phonesCursor = AndroidFactory.getApplicationContext().getContentResolver().query(
                Phone.CONTENT_URI,
                projection,
                null,
                null,
                null);

        if (phonesCursor == null) {
            if (logger.isActivated()) {
                logger.debug("handleAddressBookHasChanged phonesCursor is null");
            }
            isCheckInProgress = false;
            if (isRecheckNeeded) {
                handleAddressBookHasChanged();
            }
            return;
        }
		// List of unique number that will have to be queried for capabilities
		ArrayList<String> toBeTreatedNumbers = new ArrayList<String>();

		// List of unique number that have already been queried
        ArrayList<String> alreadyRcsNumbers = new ArrayList<String>();
        List<String> rcsContacts = ContactsManager.getInstance().getRcsContacts();
        int rcsContactsSize = rcsContacts.size();
        for (int i = 0; i < rcsContactsSize; i++) {
            alreadyRcsNumbers.add(PhoneUtils.formatNumberToInternational(rcsContacts.get(i)));
        }
        // We add "My number" to the numbers that are already RCS, so we don't
        // query it if it is present in the address book
        alreadyRcsNumbers.add(PhoneUtils.extractNumberFromUri(ImsModule.IMS_USER_PROFILE
                .getPublicUri()));

        ArrayList<String> allNumbers = new ArrayList<String>();
        ArrayList<String> notRcsContacts = new ArrayList<String>();
        ArrayList<String> numbers = new ArrayList<String>();
        try {
		while(phonesCursor.moveToNext()) {
                String phoneNumber = PhoneUtils.formatNumberToInternational(phonesCursor
                        .getString(1));
                allNumbers.add(phoneNumber);
            }
        } finally {
            phonesCursor.close();
        }
        int size = allNumbers.size();
        for (int i = 0; i < size; i++) {
            String number = allNumbers.get(i);
            if (!alreadyRcsNumbers.contains(number)) {
                notRcsContacts.add(number);
            }
        }
        synchronized (mObject) {
            if (needCheck) {
                size = notRcsContacts.size();
                for (int i = 0; i < size; i++) {
                    String number = notRcsContacts.get(i);
                    if (!mNotRcseNumbers.contains(number)) {
                        numbers.add(number);
		            }				
				}
            } else {
                numbers = notRcsContacts;
		            }				
            mNotRcseNumbers = notRcsContacts;
				}
        size = numbers.size();
        for (int i = 0; i < size; i++) {
            String phoneNumber = numbers.get(i);
            // If this number is not considered RCS valid or has
            // already
            // an entry with RCS, skip it
if (ContactsManager.getInstance().isRcsValidNumber(phoneNumber)){
					if (logger.isActivated()) {
		                logger.info("Not Rcs NUMBER"+phoneNumber);
		            }				
				}
				if (!ContactsManager.getInstance().isOnlySimAssociated(phoneNumber)){
					if (logger.isActivated()) {
		                logger.info("sim ASSOCIATED  NUMBER"+phoneNumber);
		            }				
				}
                if (ContactsManager.getInstance().isRcsValidNumber(phoneNumber)
                    && (!ContactsManager.getInstance().isOnlySimAssociated(phoneNumber) || (Build.VERSION.SDK_INT > 10))
                    && PhoneUtils.isVodafoneValidNumber(phoneNumber)) {
					toBeTreatedNumbers.add(phoneNumber);
				} else {
					toBeTreatedNumbers.remove(phoneNumber);
				}
		}
			 if (logger.isActivated()) {
            logger.debug("handleAddressBookHasChanged needCheck: " + needCheck
                    + " allNumbers size: " + allNumbers.size() + " mNotRcseNumbers size: "
                    + mNotRcseNumbers.size() + " toBeTreatedNumbers size: "
                    + toBeTreatedNumbers.size() + " numbers size: " + numbers.size());
            }
        // Get the capabilities for the numbers that haven't got a RCS
        // associated contact
		requestContactCapabilities(toBeTreatedNumbers);
		// End of the check procedure
		isCheckInProgress = false;

		// Check if we have to make another check
		if (isRecheckNeeded) {
			handleAddressBookHasChanged();
		}
	}

    /** @} */

	/**
     * Reset the content sharing capabities for a given contact
     * 
     * @param contact Contact
     */
	public void resetContactCapabilitiesForContentSharing(String contact) {
		Capabilities capabilities = ContactsManager.getInstance().getContactCapabilities(contact);
		if (capabilities != null) {
            // Force a reset of content sharing capabilities
			capabilities.setImageSharingSupport(false);
			capabilities.setVideoSharingSupport(false);

		 	// Update the database capabilities
	        ContactsManager.getInstance().setContactCapabilities(contact, capabilities);
	        ContactInfo info = ContactsManager.getInstance().getContactInfo(contact);
            /**
             * M: Added to resolve the issue that The RCS-e icon does not
             * display in contact list of People.@{
             */
            if (info.getRcsStatus() == ContactInfo.NO_INFO
                    || info.getRcsStatus() == ContactInfo.NOT_RCS) {
                capabilities.setRcseContact(false);
                if (logger.isActivated()) {
                    logger.debug("resetContactCapabilitiesForContentSharing setRcseContact contact: "
                            + contact + " false");
                }
            } else {
                capabilities.setRcseContact(true);
                if (logger.isActivated()) {
                    logger.debug("resetContactCapabilitiesForContentSharing setRcseContact contact: "
                            + contact + " true");
                }
            }
            /**
             * @}
             */
		 	// Notify listener
		 	getImsModule().getCore().getListener().handleCapabilitiesNotification(contact, capabilities);
		}
	 }
}
