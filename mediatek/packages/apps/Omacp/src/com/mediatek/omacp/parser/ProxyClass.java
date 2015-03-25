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

package com.mediatek.omacp.parser;

import com.mediatek.omacp.parser.ApplicationClass.Port;

import java.util.ArrayList;

public class ProxyClass {

    public String mProxyId;

    public String mProxyPw;

    public String mPpgauthType;

    public String mProxyProviderId;

    public String mName;

    public ArrayList<String> mDomain = new ArrayList<String>();

    public String mTrust;

    public String mMaster;

    public String mStartpage;

    public String mBasauthId;

    public String mBasauthPw;

    public String mWspVersion;

    public String mPushenabled;

    public String mPullenbaled;

    public ArrayList<PxAuthInfo> mPxauthinfo = new ArrayList<PxAuthInfo>();

    public ArrayList<Port> mPort = new ArrayList<Port>();

    public ArrayList<PxPhysical> mPxphysical = new ArrayList<PxPhysical>();

    public static class PxAuthInfo {
        public String mPxauthType;

        public String mPxauthId;

        public String mPxauthPw;

        @Override
        public String toString() {
            return "PXAUTH_TYPE: " + mPxauthType + "\n"
            + "PXAUTH_ID: " + mPxauthId + "\n"
            + "PXAUTH_PW: " + mPxauthPw + "\n";
        }
    }

    public static class PxPhysical {
        public String mPhysicalProxyId;

        public ArrayList<String> mDomain = new ArrayList<String>();

        public String mPxaddr;

        public String mPxaddrtype;

        public String mPxaddrFqdn;

        public String mWspVersion;

        public String mPushenabled;

        public String mPullenabled;

        public ArrayList<String> mToNapid = new ArrayList<String>();

        public ArrayList<Port> mPort = new ArrayList<Port>();

        @Override
        public String toString() {
            return "PHYSICAL_PROXY_ID: " + mPhysicalProxyId + "\n"
            + "DOMAIN: " + mDomain + "\n"
            + "PXADDR: " + mPxaddr + "\n"
            + "PXADDRTYPE: " + mPxaddrtype + "\n"
            + "PXADDR_FQDN: " + mPxaddrFqdn + "\n"
            + "WSP_VERSION: " + mWspVersion + "\n"
            + "PUSHENABLED: " + mPushenabled + "\n"
            + "PULLENABLED: " + mPullenabled + "\n"
            + "TO_NAPID: " + mToNapid + "\n"
            + "PORT: " + mPort + "\n";
        }
    }

    @Override
    public String toString() {
        return "PROXY_ID: " + mProxyId + "\n"
        + "PROXY_PW: " + mProxyPw + "\n"
        + "PPGAUTH_TYPE: " + mPpgauthType + "\n"
        + "PROXY_PROVIDER_ID: " + mProxyProviderId + "\n"
        + "NAME: " + mName + "\n"
        + "DOMAIN: " + mDomain + "\n"
        + "TRUST: " + mTrust + "\n"
        + "MASTER: " + mMaster + "\n"
        + "STARTPAGE: " + mStartpage + "\n"
        + "BASAUTH_ID: " + mBasauthId + "\n"
        + "BASAUTH_PW: " + mBasauthPw + "\n"
        + "WSP_VERSION: " + mWspVersion + "\n"
        + "PUSHENABLED: " + mPushenabled + "\n"
        + "PULLENBALED: " + mPullenbaled + "\n"
        + "PXAUTHINFO: " + mPxauthinfo + "\n"
        + "PORT: " + mPort + "\n"
        + "PXPHYSICAL: " + mPxphysical + "\n";
    }

}
