package com.orangelabs.rcs.service.api.client.messaging;

/**
 * Message delivery listener
 */
interface IMessageDeliveryListener {
	// Message delivery status
	/** M: add server date for delivery status @{ */
	void handleMessageDeliveryStatus(in String contact, in String msgId, in String status, in long date);
	/** @} */
        
    // File Transfer delivery status
     // In FToHTTP, Delivered status is done just after download information are received by the
    // terminating, and Displayed status is done when the file is downloaded.
    // In FToMSRP, the two status are directly done just after MSRP transfer complete.
    void handleFileDeliveryStatus(in String ftSessionId, in String status,in String contact);
}
