package com.orangelabs.rcs.service.api.client.media;

import com.orangelabs.rcs.service.api.client.media.IAudioEventListener;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;

/**
 * Audio RTP player
 */
interface IAudioPlayer {
	// Open the player
	void open(in String remoteHost, in int remotePort);

	// Close the player
	void close();

	// Start the player
	void start();

	// Stop the player
	void stop();

	// Returns the local RTP port
	int getLocalRtpPort();

	// Add an audio event listener
	void addListener(in IAudioEventListener listener);

	// Remove audio event listeners
	void removeAllListeners();

	// Get supported audio codecs
	MediaCodec[] getSupportedAudioCodecs();

	// Get audio codec
	MediaCodec getAudioCodec();

	// Set audio codec
	void setAudioCodec(in MediaCodec mediaCodec);

}