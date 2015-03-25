package com.orangelabs.rcs.service.api.client.media;

import com.orangelabs.rcs.service.api.client.media.IAudioEventListener;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;

/**
 * Audio RTP renderer
 */
interface IAudioRenderer {
	// Open the renderer
	void open(in String remoteHost, in int remotePort);

	// Close the renderer
	void close();

	// Start the renderer
	void start();

	// Stop the renderer
	void stop();

	// Returns the local RTP port
	int getLocalRtpPort();

	// Add an audio event listener
	void addListener(in IAudioEventListener listener);

	// Remove an audio event listeners
	void removeAllListeners();

	// Get supported audio codecs
	MediaCodec[] getSupportedAudioCodecs();

	// Get audio codec
	MediaCodec getAudioCodec();

	// Set audio codec
	void setAudioCodec(in MediaCodec mediaCodec);
	
}