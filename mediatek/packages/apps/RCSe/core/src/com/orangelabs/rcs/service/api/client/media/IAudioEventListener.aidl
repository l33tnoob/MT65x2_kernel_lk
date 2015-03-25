package com.orangelabs.rcs.service.api.client.media;

/**
 * Audio event listener
 */
interface IAudioEventListener {
	// Audio is opened
	void audioOpened();

	// Audio is closed
	void audioClosed();

	// Audio is started
	void audioStarted();
	
	// Audio is stopped
	void audioStopped();

	// Audio has failed
	void audioError(in String error);
}
