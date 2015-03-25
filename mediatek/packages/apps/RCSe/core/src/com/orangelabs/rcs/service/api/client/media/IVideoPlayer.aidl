package com.orangelabs.rcs.service.api.client.media;

import com.orangelabs.rcs.service.api.client.media.IVideoEventListener;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;

/**
 * Video RTP player
 */
interface IVideoPlayer {
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

	// Add a media listener
	void addListener(in IVideoEventListener listener);

	// Remove media listeners
	void removeAllListeners();

	// Get supported video codecs
	MediaCodec[] getSupportedVideoCodecs();

	// Get video codec
	MediaCodec getVideoCodec();

	// Set video codec
	void setVideoCodec(in MediaCodec mediaCodec);

    // Set extension header orientation id
    void setOrientationHeaderId(int headerId);
}