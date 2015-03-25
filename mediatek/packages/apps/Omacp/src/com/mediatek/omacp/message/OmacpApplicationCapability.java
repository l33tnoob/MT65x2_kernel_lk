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

package com.mediatek.omacp.message;

public class OmacpApplicationCapability {

    // browser capability
    public static boolean sBrowser = false;

    public static boolean sBrowserBookMarkFolder = false;

    public static boolean sBrowserToProxy = false;

    public static boolean sBrowserToNapId = false;

    public static boolean sBrowserBookMarkName = false;

    public static boolean sBrowserBookMark = false;

    public static boolean sBrowserUserName = false;

    public static boolean sBrowserPassWord = false;

    public static boolean sBrowserHomePage = false;

    // mms capability
    public static boolean sMms = false;

    public static boolean sMmsMmscName = false;

    public static boolean sMmsToProxy = false;

    public static boolean sMmsToNapid = false;

    public static boolean sMmsMmsc = false;

    public static boolean sMmsCm = false;

    public static boolean sMmsRm = false;

    public static boolean sMmsMs = false;

    public static boolean sMmsPcAddr = false;

    public static boolean sMmsMa = false;

    // dm capability
    public static boolean sDm = false;

    public static boolean sDmProviderId = false;

    public static boolean sDmServerName = false;

    public static boolean sDmToProxy = false;

    public static boolean sDmToNapid = false;

    public static boolean sDmServerAddress = false;

    public static boolean sDmAddrType = false;

    public static boolean sDmPortNumber = false;

    public static boolean sDmAuthLevel = false;

    public static boolean sDmAuthType = false;

    public static boolean sDmAuthName = false;

    public static boolean sDmAuthSecret = false;

    public static boolean sDmAuthData = false;

    public static boolean sDmInit = false;

    // email capability
    public static boolean sEmail = false;

    public static boolean sEmailProviderId = false;

    public static boolean sEmailSettingName = false;

    public static boolean sEmailToNapid = false;

    public static boolean sEmailOutboundAddr = false;

    public static boolean sEmailOutboundAddrType = false;

    public static boolean sEmailOutboundPortNumber = false;

    public static boolean sEmailOutboundSecure = false;

    public static boolean sEmailOutboundAuthType = false;

    public static boolean sEmailOutboundUserName = false;

    public static boolean sEmailOutboundPassword = false;

    public static boolean sEmailFrom = false;

    public static boolean sEmailRtAddr = false;

    public static boolean sEmailInboundAddr = false;

    public static boolean sEmailInboundAddrType = false;

    public static boolean sEmailInboundPortNumber = false;

    public static boolean sEmailInboundSecure = false;

    public static boolean sEmailInboundAuthType = false;

    public static boolean sEmailInboundUserName = false;

    public static boolean sEmailInboundPassword = false;

    // rtsp capability
    public static boolean sRtsp = false;

    public static boolean sRtspProviderId = false;

    public static boolean sRtspName = false;

    public static boolean sRtspToProxy = false;

    public static boolean sRtspToNapid = false;

    public static boolean sRtspMaxBandwidth = false;

    public static boolean sRtspNetInfo = false;

    public static boolean sRtspMinUdpPort = false;

    public static boolean sRtspMaxUdpPort = false;

    // supl
    public static boolean sSupl = false;

    public static boolean sSuplProviderId = false;

    public static boolean sSuplServerName = false;

    public static boolean sSuplToNapid = false;

    public static boolean sSuplServerAddr = false;

    public static boolean sSuplAddrType = false;

    // ds
    public static boolean sDs = false;

    public static boolean sDsServerName = false;

    public static boolean sDsToProxy = false;

    public static boolean sDsToNapid = false;

    public static boolean sDsProviderId = false;

    public static boolean sDsServerAddress = false;

    public static boolean sDsAddressType = false;

    public static boolean sDsPortNumber = false;

    public static boolean sDsAuthLevel = false;

    public static boolean sDsAuthType = false;

    public static boolean sDsAuthName = false;

    public static boolean sDsAuthSecret = false;

    public static boolean sDsAuthData = false;

    public static boolean sDsDatabaseContentType = false;

    public static boolean sDsDatabaseUrl = false;

    public static boolean sDsDatabaseName = false;

    public static boolean sDsDatabaseAuthType = false;

    public static boolean sDsDatabaseAuthName = false;

    public static boolean sDsDatabaseAuthSecret = false;

    public static boolean sDsClientDatabaseUrl = false;

    public static boolean sDsSyncType = false;

    // imps
    public static boolean sImps = false;

    public static boolean sImpsProviderId = false;

    public static boolean sImpsServerName = false;

    public static boolean sImpsContentType = false;

    public static boolean sImpsServerAddress = false;

    public static boolean sImpsAddressType = false;

    public static boolean sImpsToProxy = false;

    public static boolean sImpsToNapid = false;

    public static boolean sImpsAuthLevel = false;

    public static boolean sImpsAuthName = false;

    public static boolean sImpsAuthSecret = false;

    public static boolean sImpsServices = false;

    public static boolean sImpsClientIdPrefix = false;

}
