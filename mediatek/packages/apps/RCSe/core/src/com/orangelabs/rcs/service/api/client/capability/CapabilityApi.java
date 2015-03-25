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

package com.orangelabs.rcs.service.api.client.capability;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApi;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.CoreServiceNotAvailableException;
import com.orangelabs.rcs.service.api.client.contacts.ContactInfo;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.List;

/**
 * Capability API
 */
public class CapabilityApi extends ClientApi {

	/**
	 * Core service API
	 */
	private ICapabilityApi coreApi = null;

    /**
     * Constructor
     * 
     * @param ctx Application context
     */
    public CapabilityApi(Context ctx) {
    	super(ctx);
    	
    	// Initialize contacts provider
    	ContactsManager.createInstance(ctx);    	
    	
    	// Initialize settings provider
		RcsSettings.createInstance(ctx);    	
    }

    /**
     * Connect API
     */
    public void connectApi() {
    	super.connectApi();

    	ctx.bindService(new Intent(ICapabilityApi.class.getName()), apiConnection, 0);
    }
    
    /**
     * Disconnect API
     */
    public void disconnectApi() {
    	super.disconnectApi();
		
    	try {
    		ctx.unbindService(apiConnection);
        } catch (IllegalArgumentException e) {
        	// Nothing to do
        }
    }
    
	/**
	 * Core service API connection
	 */
	private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            coreApi = ICapabilityApi.Stub.asInterface(service);

            // Notify event listener
        	notifyEventApiConnected();
        }

        public void onServiceDisconnected(ComponentName className) {
        	// Notify event listener
        	notifyEventApiDisconnected();

        	coreApi = null;
        }
    };
    
    /**
     * Get my capabilities
     * 
     * @return Capabilities
     */
    public Capabilities getMyCapabilities() {
    	return RcsSettings.getInstance().getMyCapabilities();
    }
    
    /**
     * Get contact capabilities
     * 
     * @param contact Contact
     * @return Capabilities
     */
    public Capabilities getContactCapabilities(String contact) {
        /**
         * M: add for number format @{
         */
        contact = PhoneUtils.formatNumberToInternational(contact);
        /** @} */
    	ContactInfo contactInfo = ContactsManager.getInstance().getContactInfo(contact);
    	if (contactInfo != null) {
    		return contactInfo.getCapabilities();
    	} else {
    		return null;
    	}
    }    
    
    /**
     * Get contact info
     * 
     * @param contact Contact
     * @return Contact info
     */
    public ContactInfo getContactInfo(String contact) {
    	return ContactsManager.getInstance().getContactInfo(contact);
    }    

    /**
     * M: Added to return all the RCS-e contacts. @{
     */
    /**
     * Get RCS-e contacts.
     * 
     * @return The RCS-e contacts.
     */
    public List<String> getRcsContacts() {
        return ContactsManager.getInstance().getRcsContacts();
    }
    /**
     * @}
     */

	/**
	 * Request capabilities for a given contact
	 * 
	 * @param contact Contact
	 * @return Capabilities
	 * @throws ClientApiException
	 */
	public Capabilities requestCapabilities(String contact) throws ClientApiException {
		if (coreApi != null) {
			try {
				// Request capabilities
				return coreApi.requestCapabilities(contact);
			} catch(Exception e) {
				throw new ClientApiException(e.getMessage());
			}
		} else {
			throw new CoreServiceNotAvailableException();
		}
	}

	/**
	 * Refresh capabilities for all contacts
	 * 
	 * @throws ClientApiException
	 */
	public void refreshAllCapabilities() throws ClientApiException {
		if (coreApi != null) {
			try {
				// Start refresh
				coreApi.refreshAllCapabilities();
			} catch(Exception e) {
				throw new ClientApiException(e.getMessage());
			}
		} else {
			throw new CoreServiceNotAvailableException();
		}
	}	

     /**
	 * Refresh capabilities for all contacts
	 * 
	 * @throws ClientApiException
	 */
	public void refreshContactCapabilities(String contact) throws ClientApiException {
		if (coreApi != null) {
			try {
				// Start refresh
				coreApi.refreshContactCapabilities(contact);
			} catch(Exception e) {
				throw new ClientApiException(e.getMessage());
			}
		} else {
			throw new CoreServiceNotAvailableException();
		}
	}	
}
