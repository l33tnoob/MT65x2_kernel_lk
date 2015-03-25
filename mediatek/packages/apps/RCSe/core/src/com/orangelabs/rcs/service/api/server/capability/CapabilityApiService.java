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

package com.orangelabs.rcs.service.api.server.capability;

import java.util.List;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.capability.ICapabilityApi;
import com.orangelabs.rcs.service.api.server.ServerApiException;
import com.orangelabs.rcs.service.api.server.ServerApiUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Capability API service
 */
public class CapabilityApiService extends ICapabilityApi.Stub {
    /**
	 * The logger
	 */
	private Logger logger = Logger.getLogger(this.getClass().getName());

	/**
	 * Constructor
	 */
	public CapabilityApiService() {
		if (logger.isActivated()) {
			logger.info("Capability API service is loaded");
		}
	}

	/**
	 * Close API
	 */
	public void close() {
	}
    
	/**
	 * Request capabilities for a given contact
	 * 
	 * @param contact Contact
	 * @return Capabilities
	 * @throws ServerApiException
	 */
	public Capabilities requestCapabilities(String contact) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Request capabilities for contact " + contact);
		}

    	// Check permission
		ServerApiUtils.testPermission();

		// Test IMS connection
		ServerApiUtils.testIms();

		try {
			// Request contact capabilities
			return Core.getInstance().getCapabilityService().requestContactCapabilities(contact);
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
	}
	
	/**
	 * Refresh capabilities for all contacts
	 * 
	 * @throws ServerApiException
	 */
	public void refreshAllCapabilities() throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Refresh capabilities for all contacts");
		}

    	// Check permission
		ServerApiUtils.testPermission();

		// Test IMS connection
		ServerApiUtils.testIms();

		try {
	    	// Request all contacts capabilities
			List<String> contactList = ContactsManager.getInstance().getAllContacts();
			Core.getInstance().getCapabilityService().requestContactCapabilities(contactList);
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
	}

	/**
	 * Refresh capabilities for all contacts
	 * 
	 * @throws ServerApiException
	 */
	public void refreshContactCapabilities(String contact) throws ServerApiException {
		if (logger.isActivated()) {
			logger.info("Refresh capabilities for all contact"+ contact);
		}

    	// Check permission
		ServerApiUtils.testPermission();

		// Test IMS connection
		ServerApiUtils.testIms();

		try {
	    	// Request all contacts capabilities
			Core.getInstance().getCapabilityService().requestContactCapabilities(contact);
		} catch(Exception e) {
			if (logger.isActivated()) {
				logger.error("Unexpected error", e);
			}
			throw new ServerApiException(e.getMessage());
		}
	}
}
