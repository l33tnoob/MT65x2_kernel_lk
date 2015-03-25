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

package com.orangelabs.rcs.service.api.client.media.audio;

import com.orangelabs.rcs.service.api.client.media.MediaCodec;

/**
 * Audio codec
 * 
 * @author opob7414
 */
public class AudioCodec {

    /**
     * Media codec
     */
    private MediaCodec mediaCodec;

    /**
     * Payload key
     */
    private static final String PAYLOAD = "payload";

    /**
     * Codec param key
     */
    private static final String CODECPARAMS = "codecParams";

    /**
     * Sample rate key
     */
    private static final String SAMPLERATE = "samplerate";

    /**
     * Constructor
     *
     * @param codecName Codec name
     * @param clockRate Clock rate
     * @param codecParams Codec parameters
     * @param samplerate Sample rate
     * @param bitrate Bit rate
     */
    public AudioCodec(String codecName, int payload, String codecParams, int samplerate) {
        mediaCodec = new MediaCodec(codecName);
        mediaCodec.setIntParam(PAYLOAD, payload);
        mediaCodec.setStringParam(CODECPARAMS, codecParams);
        mediaCodec.setIntParam(SAMPLERATE, samplerate);
    }

    /**
     * Constructor
     * 
     * @param mediaCodec Media codec
     */
    public AudioCodec(MediaCodec mediaCodec) {
        this.mediaCodec = mediaCodec;
    }

    /**
     * Get media codec
     * 
     * @return Media codec
     */
    public MediaCodec getMediaCodec() {
        return mediaCodec;
    }

    /**
     * Get codec name
     * 
     * @return Codec name
     */
    public String getCodecName() {
        return mediaCodec.getCodecName();
    }

    /**
     * Get payload
     * 
     * @return Payload
     */
    public int getPayload() {
        return mediaCodec.getIntParam(PAYLOAD, 96);
    }

    /**
     * Get audio codec parameters
     * 
     * @return Audio codec parameters
     */
    public String getCodecParams() {
        return mediaCodec.getStringParam(CODECPARAMS);
    }

    /**
     * Get audio sample rate
     * 
     * @return Audio sample rate
     */
    public int getSamplerate() {
        return mediaCodec.getIntParam(SAMPLERATE, 15);
    }

    /**
     * Compare codec encodings
     *
     * @param codec Codec to compare
     * @return True if codecs are equals
     */
    public boolean compare(AudioCodec codec) {
    	boolean ret = false;
        if (getCodecName().equalsIgnoreCase(codec.getCodecName())) {
        	ret = true;
        }
        return ret;
    }

    /**
     * Check if a codec is in a list
     *
     * @param supportedCodecs List of supported codec
     * @param codec Selected codec
     * @return True if the codec is in the list
     */
    public static boolean checkAudioCodec(MediaCodec[] supportedCodecs, AudioCodec codec) {
        for (int i = 0; i < supportedCodecs.length; i++) {
            if (codec.compare(new AudioCodec(supportedCodecs[i]))) {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns string representation of the video codec
     * 
     * @return String
     */
    public String toString() {
        return "Codec " + getCodecName() + " " + getPayload() + " " +
                getCodecParams() + " " + getSamplerate();
    }
}
