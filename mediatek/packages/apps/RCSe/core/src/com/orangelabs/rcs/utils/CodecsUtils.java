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

package com.orangelabs.rcs.utils;

import java.util.Vector;

import com.orangelabs.rcs.core.ims.protocol.rtp.codec.audio.amr.AMRWBConfig;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.H264Config;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.JavaPacketizer;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.profiles.H264Profile1_2;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.profiles.H264Profile1_3;
import com.orangelabs.rcs.core.ims.protocol.rtp.codec.video.h264.profiles.H264Profile1b;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.audio.AmrWbAudioFormat;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.video.H264VideoFormat;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;
import com.orangelabs.rcs.service.api.client.media.audio.AudioCodec;
import com.orangelabs.rcs.service.api.client.media.video.VideoCodec;

/**
 * Codecs utility functions
 *
 * @author hlxn7157
 * @author yplo6403
 *
 */
public class CodecsUtils {

    /**
     * Get list of supported video codecs according to current network
     *
     * @return Codecs list
     */
    public static MediaCodec[] getRendererCodecList() {
        return getSupportedCodecList();
    }

    /**
     * Get list of supported video codecs according to current network
     *
     * @return Codecs list
     */
    public static MediaCodec[] getPlayerCodecList() {
        return getSupportedCodecList();
    }

    /**
     * Get list of supported video codecs according to current network
     *
     * @param cif true if available
     * @param qvga true if available
     * @return Codecs list
     */
    private static MediaCodec[] getSupportedCodecList() {
        int networkLevel = NetworkUtils.getNetworkAccessType();
        int payload_count = H264VideoFormat.PAYLOAD - 1;
        Vector<MediaCodec> list = new Vector<MediaCodec>();

        // Add codecs settings (ordered list)
        /*
         * 3G/3g+ -> level 1.B: profile-level-id=42900b, frame_rate=15, frame_size=QCIF, bit_rate=96k
         *
         * WIFI   -> level 1.2: profile-level-id=42800c, frame_rate=15, frame_size=QVGA, bit_rate=384k
         * WIFI   -> level 1.2: profile-level-id=42800c, frame_rate=15, frame_size=CIF, bit_rate=384k
         * WIFI   -> level 1.3: profile-level-id=42800d, frame_rate=15, frame_size=CIF, bit_rate=384k
         */

        if (networkLevel == NetworkUtils.NETWORK_ACCESS_WIFI || networkLevel == NetworkUtils.NETWORK_ACCESS_4G) {
//TODO check with removing this if server accepts , did not work on 2.5.12	

	/*	list.add(new VideoCodec(H264Config.CODEC_NAME,
                    ++payload_count,
                    H264Config.CLOCK_RATE,
                    H264Config.CODEC_PARAM_PROFILEID + "=" + H264Profile1_3.BASELINE_PROFILE_ID + ";" + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=" + JavaPacketizer.H264_ENABLED_PACKETIZATION_MODE,
                    15,
                    256000,
                    H264Config.CIF_WIDTH, 
                    H264Config.CIF_HEIGHT).getMediaCodec());
            list.add(new VideoCodec(H264Config.CODEC_NAME,
                    ++payload_count,
                    H264Config.CLOCK_RATE,
                    H264Config.CODEC_PARAM_PROFILEID + "=" + H264Profile1_2.BASELINE_PROFILE_ID + ";" + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=" + JavaPacketizer.H264_ENABLED_PACKETIZATION_MODE,
                    15,
                    176000,
                    H264Config.CIF_WIDTH, 
                    H264Config.CIF_HEIGHT).getMediaCodec());
            list.add(new VideoCodec(H264Config.CODEC_NAME,
                    ++payload_count,
                    H264Config.CLOCK_RATE,
                    H264Config.CODEC_PARAM_PROFILEID + "=" + H264Profile1_2.BASELINE_PROFILE_ID + ";" + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=" + JavaPacketizer.H264_ENABLED_PACKETIZATION_MODE,
                    15,
                    176000,
                    H264Config.QVGA_WIDTH, 
                    H264Config.QVGA_HEIGHT).getMediaCodec());
		*/}
        list.add(new VideoCodec(H264Config.CODEC_NAME,
                ++payload_count,
                H264Config.CLOCK_RATE,
                H264Config.CODEC_PARAM_PROFILEID + "=" + H264Profile1b.BASELINE_PROFILE_ID + ";" + H264Config.CODEC_PARAM_PACKETIZATIONMODE + "=" + JavaPacketizer.H264_ENABLED_PACKETIZATION_MODE,
                15,
                96000,
                H264Config.QCIF_WIDTH, 
                H264Config.QCIF_HEIGHT).getMediaCodec());

        return (MediaCodec[]) list.toArray(new MediaCodec[list.size()]);
    }

    /**
     * Get list of supported audio codecs
     *
     * @return Codecs list
     */
    public static MediaCodec[] getSupportedAudioCodecList() {
        int i = -1;

        // Set number of codecs
        int size = 1; // default

        MediaCodec[] supportedMediaCodecs = new MediaCodec[size];

        // Add codecs settings (ordered list)        
        supportedMediaCodecs[++i] = new AudioCodec(AMRWBConfig.CODEC_NAME,
        		AmrWbAudioFormat.PAYLOAD,
                "",
                AMRWBConfig.SAMPLE_RATE).getMediaCodec();

        return supportedMediaCodecs;
    }
    
	/**
	 * Retrieve the video codec with profile 1B from list of media codecs
	 * 
	 * @param mediaCodecs
	 *            list of media codecs
	 * @return the video codec with profile 1B or null
	 */
	public static VideoCodec getVideoCodecProfile1b(MediaCodec[] mediaCodecs) {
		for (MediaCodec mediaCodec : mediaCodecs) {
			VideoCodec videoCodec = new VideoCodec(mediaCodec);
			if (H264Profile1b.BASELINE_PROFILE_ID
					.compareToIgnoreCase(H264Config.getCodecProfileLevelId(videoCodec.getCodecParams())) == 0) {
				return videoCodec;
			}
		}
		return null;
	}
}
