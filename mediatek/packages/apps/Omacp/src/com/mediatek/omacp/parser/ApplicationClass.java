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

import java.util.ArrayList;

public class ApplicationClass {

    public String mAppid;

    public String mProviderId;

    public String mName;

    public String mAaccept;

    public String mAprotocol;

    public ArrayList<String> mToProxy = new ArrayList<String>();

    public ArrayList<String> mToNapid = new ArrayList<String>();

    public ArrayList<String> mAddr = new ArrayList<String>();

    public ArrayList<AppAddr> mAppaddr = new ArrayList<AppAddr>();

    public ArrayList<AppAuth> mAppauth = new ArrayList<AppAuth>();

    public ArrayList<Resource> mResource = new ArrayList<Resource>();

    // OMNA denifition attributes

    // MMS
    public String mCm;

    public String mRm;

    public String mMs;

    public String mPcAddr;

    public String mMa;

    // DM
    public String mInit;

    // SMTP
    public String mFrom;

    public String mRtAddr;

    // RTSP
    public String mMaxBandwidth;

    public ArrayList<String> mNetinfo = new ArrayList<String>();

    public String mMinUdpPort;

    public String mMaxUdpPort;

    // IMPS
    public String mServices;

    public String mCidprefix;

    public static class AppAddr {
        public String mAddr;

        public String mAddrtype;

        public ArrayList<Port> mPort = new ArrayList<Port>();

        @Override
        public String toString() {
            return "ADDR: " + mAddr + "\n"
            + "ADDRTYPE: " + mAddrtype + "\n"
            + "PORT: " + mPort  + "\n";
        }
    }

    public static class Port {
        public String mPortnbr;

        public ArrayList<String> mService = new ArrayList<String>();

        @Override
        public String toString() {
            return "PORTNBR: " + mPortnbr + "\n"
            + "SERVICE: " + mService  + "\n";
        }
    }

    public static class AppAuth {
        public String mAauthlevel;

        public String mAauthtype;

        public String mAauthname;

        public String mAauthsecret;

        public String mAauthdata;

        @Override
        public String toString() {
            return "AAUTHLEVEL: " + mAauthlevel + "\n"
            + "AAUTHTYPE: " + mAauthtype + "\n"
            + "AAUTHNAME: " + mAauthname + "\n"
            + "AAUTHSECRET: " + mAauthsecret + "\n"
            + "AAUTHDATA: " + mAauthdata  + "\n";
        }
    }

    public static class Resource {
        public String mUri;

        public String mName;

        public String mAaccept;

        public String mAauthtype;

        public String mAauthname;

        public String mAauthsecret;

        public String mAauthdata;

        public String mStartpage;

        // DS
        public String mCliuri;

        public String mSynctype;

        @Override
        public String toString() {
            return "URI: " + mUri + "\n"
            + "NAME: " + mName + "\n"
            + "AACCEPT: " + mAaccept + "\n"
            + "AAUTHTYPE: " + mAauthtype + "\n"
            + "AAUTHNAME: " + mAauthname + "\n"
            + "AAUTHSECRET: " + mAauthsecret + "\n"
            + "AAUTHDATA: " + mAauthdata + "\n"
            + "STARTPAGE: " + mStartpage  + "\n";
        }
    }

    @Override
    public String toString() {
        return "APPID: " + mAppid + "\n"
        + "PROVIDER_ID: " + mProviderId + "\n"
        + "NAME: " + mName + "\n"
        + "AACCEPT: " + mAaccept + "\n"
        + "APROTOCOL: " + mAprotocol + "\n"
        + "TO_PROXY: " + mToProxy + "\n"
        + "TO_NAPID: " + mToNapid + "\n"
        + "ADDR: " + mAddr + "\n"
        + "APPADDR: " + mAppaddr + "\n"
        + "APPAUTH: " + mAppauth + "\n"
        + "RESOURCE: " + mResource  + "\n";
    }

}
