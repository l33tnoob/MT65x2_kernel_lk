/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.op.media;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Proxy;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.Settings;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.media.IOmaSettingHelper;
import com.mediatek.xlog.Xlog;

import java.util.HashMap;
import java.util.Map;

public class DefaultOmaSettingHelper implements IOmaSettingHelper {
    private static final String TAG = "DefaultOmaSettingHelper";
    private static final boolean LOG = true;
    
    private static final int UNKNOWN_PORT = -1;
    
    //rtsp info
    private static final String KEY_NAME = "NAME";
    private static final String KEY_PROVIDER_ID = "PROVIDER-ID";
    private static final String KEY_TO_PROXY = "TO-PROXY";
    private static final String KEY_TO_NAPID = "TO-NAPID";
    private static final String KEY_MAX_BANDWIDTH = "MAX-BANDWIDTH";
    private static final String KEY_NETINFO = "NETINFO";
    private static final String KEY_MIN_UDP_PORT = "MIN-UDP-PORT";
    private static final String KEY_MAX_UDP_PORT = "MAX-UDP-PORT";
    private static final String KEY_SIM_ID = "SIM-ID";
    
    private static final String KEY_RTSP_PROXY_HOST = "MTK-RTSP-PROXY-HOST";
    private static final String KEY_RTSP_PROXY_PORT = "MTK-RTSP-PROXY-PORT";
    private static final String KEY_HTTP_PROXY_HOST = "MTK-HTTP-PROXY-HOST";
    private static final String KEY_HTTP_PROXY_PORT = "MTK-HTTP-PROXY-PORT";
    
    private static final String KEY_HTTP_BUFFER_SIZE = "MTK-HTTP-CACHE-SIZE";
    private static final String KEY_RTSP_BUFFER_SIZE = "MTK-RTSP-CACHE-SIZE";
    private static final int DEFAULT_HTTP_BUFFER_SIZE = 10;//seconds
    private static final int DEFAULT_RTSP_BUFFER_SIZE = 4;//seconds

    /**
     * Fill the header according the feature option.
     * @param context
     * @param uri
     * @param headers
     * @return
     */
    public Map<String, String> setSettingHeader(Context context, Uri uri, Map<String, String> headers) {
        if (isOMAEnabled()) {
            int type = judgeStreamingType(uri);
            if (IOmaSettingHelper.STREAMING_RTSP == type || IOmaSettingHelper.STREAMING_HTTP == type) {
                return setOmaSettingHeader(context, headers);
            }
        }
        if (LOG) Xlog.v(TAG, "setSettingHeader: headers unchanged. uri=" + uri);
        return headers;
    }

    /**
     * Whether oma is supported or not.
     * @return true enabled, otherwise false.
     */
    protected boolean isOMAEnabled() {
        boolean enabled = FeatureOption.MTK_OMACP_SUPPORT || FeatureOption.MTK_DM_APP;
        if (LOG) Xlog.v(TAG, "isOMAEnabled: enabled=" + enabled);
        return enabled;
    }

    /**
     * Gets RTSP default buffer size.
     * @return RTSP default buffer size.
     */
    protected int getRtspDefaultBufferSize() {
        return DEFAULT_RTSP_BUFFER_SIZE;
    }

    /**
     * Judge the input Uri's streaming type.
     * @param uri
     * @return the current uri's streamign type.
     */
    private int judgeStreamingType(Uri uri) {
        if (LOG) Xlog.v(TAG, "judgeStreamingType(" + String.valueOf(uri) + ")");
        int streamingType = IOmaSettingHelper.STREAMING_UNKNOWN;
        if (uri == null) {
            Xlog.w(TAG, "judgeStreamingType: uri is null, cannot judge streaming type.");
            return streamingType;
        }
        String scheme = uri.getScheme();
        if ("rtsp".equalsIgnoreCase(scheme)) {
            streamingType = IOmaSettingHelper.STREAMING_RTSP;
        } else if ("http".equalsIgnoreCase(scheme)) {
            streamingType = IOmaSettingHelper.STREAMING_HTTP;
        } else {
            streamingType = IOmaSettingHelper.STREAMING_LOCAL;
        }
        if (LOG) Xlog.v(TAG, "judgeStreamingType: type=" + streamingType);
        return streamingType;
    }

    /**
     * Read OMA RTSP settings and fill these info into headers.
     * If headers is null, new Map<String, String> object will be created and returned.
     * Otherwise, key value will be filled into headers.
     * @param context
     * @param headers
     * @return filled headers
     */
    private Map<String, String> setOmaSettingHeader(Context context, Map<String, String> headers) {
        if (LOG) Xlog.i(TAG, "setOmaSettingHeader(" + context + "," + headers + ")");
        if (context == null) {
            Xlog.e(TAG, "setOmaSettingHeader: Null context!");
            return headers;
        }
        //get oma rtsp setting
        //Settings supports client and server cache mechanism,
        //so here we needn't to cache the values.
        ContentResolver cr = context.getContentResolver();
        Map<String, String> tempHeaders = headers;
        if (tempHeaders == null) {
            tempHeaders = new HashMap<String, String>();
        }
        
        int minUdpPort = UNKNOWN_PORT;
        int maxUdpPort = UNKNOWN_PORT;
        int rtspProxyEnable = 0;//0 false, 1 true
        String rtspProxyHost = null;
        int rtspProxyPort = UNKNOWN_PORT;
        int httpProxyEnable = 0;//0 false, 1 true
        String httpProxyHost = null;
        int httpProxyPort = UNKNOWN_PORT;
        
        //get buffer size info
        int httpBufferSize = Settings.System.getInt(cr, KEY_HTTP_BUFFER_SIZE, DEFAULT_HTTP_BUFFER_SIZE);
        fillHeader(tempHeaders, KEY_HTTP_BUFFER_SIZE, String.valueOf(httpBufferSize));
        int rtspBufferSize = Settings.System.getInt(cr, KEY_RTSP_BUFFER_SIZE, getRtspDefaultBufferSize());
        fillHeader(tempHeaders, KEY_RTSP_BUFFER_SIZE, String.valueOf(rtspBufferSize));
        
        //get rtsp udp port info
        minUdpPort = Settings.System.getInt(cr, MediaStore.Streaming.Setting.MIN_UDP_PORT, UNKNOWN_PORT);
        maxUdpPort = Settings.System.getInt(cr, MediaStore.Streaming.Setting.MAX_UDP_PORT, UNKNOWN_PORT);
        if (minUdpPort != UNKNOWN_PORT && maxUdpPort != UNKNOWN_PORT) {
            fillHeader(tempHeaders, KEY_MIN_UDP_PORT, String.valueOf(minUdpPort));
            fillHeader(tempHeaders, KEY_MAX_UDP_PORT, String.valueOf(maxUdpPort));
        }
        //get rtsp proxy info
        rtspProxyEnable = Settings.System.getInt(cr, MediaStore.Streaming.Setting.RTSP_PROXY_ENABLED, 0);
        if (rtspProxyEnable == 1) {
            rtspProxyHost = Settings.System.getString(cr, MediaStore.Streaming.Setting.RTSP_PROXY_HOST);
            rtspProxyPort = Settings.System.getInt(cr, MediaStore.Streaming.Setting.RTSP_PROXY_PORT, UNKNOWN_PORT);
            if (rtspProxyHost != null && rtspProxyPort != UNKNOWN_PORT) { 
                fillHeader(tempHeaders, KEY_RTSP_PROXY_HOST, rtspProxyHost);
                fillHeader(tempHeaders, KEY_RTSP_PROXY_PORT, String.valueOf(rtspProxyPort));
            }
        }

        //get http proxy info
        httpProxyEnable = Settings.System.getInt(cr, MediaStore.Streaming.Setting.HTTP_PROXY_ENABLED, 0);
        if (httpProxyEnable == 1) {
            httpProxyHost = Settings.System.getString(cr, MediaStore.Streaming.Setting.HTTP_PROXY_HOST);
            httpProxyPort = Settings.System.getInt(cr, MediaStore.Streaming.Setting.HTTP_PROXY_PORT, UNKNOWN_PORT);
        }
        //If not enable streaming http proxy or not set streaming proxy,
        //pass the wifi or gprs's proxy to stagefright.
        //Otherwise, use streaming http proxy instead.
        if (httpProxyEnable != 1 || httpProxyPort == UNKNOWN_PORT) {
            //Proxy will returns corresponding proxy host and port according 
            //to the connection type(mobile or wifi) since 4.0.
            httpProxyHost = Proxy.getHost(context);
            httpProxyPort = Proxy.getPort(context);
        }
        
        if (httpProxyHost != null && httpProxyPort != UNKNOWN_PORT) {
            fillHeader(tempHeaders, KEY_HTTP_PROXY_HOST, httpProxyHost);
            fillHeader(tempHeaders, KEY_HTTP_PROXY_PORT, String.valueOf(httpProxyPort));
        }

        if (LOG) {
            StringBuilder sb = new StringBuilder();
            sb.append("minUdpPort=").append(minUdpPort);
            sb.append("minUdpPort=").append(minUdpPort);
            sb.append("maxUdpPort=").append(maxUdpPort);
            sb.append("rtspProxyEnable=").append(rtspProxyEnable);
            sb.append("rtspProxyHost=").append(rtspProxyHost);
            sb.append("rtspProxyPort=").append(rtspProxyPort);
            sb.append("httpProxyEnable=").append(httpProxyEnable);
            sb.append("httpProxyHost=").append(httpProxyHost);
            sb.append("httpProxyPort=").append(httpProxyPort);
            sb.append("httpBufferSize=").append(httpBufferSize);
            sb.append("rtspBufferSize=").append(rtspBufferSize);
            Xlog.v(TAG, "setOmaSettingHeader: params:" + sb.toString());
        }
        return tempHeaders;
    }

    private void fillHeader(Map<String, String> headers, String key, String value) {
        if (value != null && !"".equals(value.trim())) {
            headers.put(key, value);
        } else {
            Xlog.w(TAG, "fillHeader: cannot fill key=" + key + ", value=" + value);
        }
    }
}
