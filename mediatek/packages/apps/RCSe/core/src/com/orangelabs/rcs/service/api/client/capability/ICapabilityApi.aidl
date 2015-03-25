package com.orangelabs.rcs.service.api.client.capability;

import com.orangelabs.rcs.service.api.client.capability.Capabilities;

/**
 * Capability API
 */
interface ICapabilityApi {
	// Request capabilities for a contact
	Capabilities requestCapabilities(in String contact);

	// Refresh capabilities for all contacts
	void refreshAllCapabilities();
	
	// Refresh capabilities for one contact
	void refreshContactCapabilities(in String contact);
}
