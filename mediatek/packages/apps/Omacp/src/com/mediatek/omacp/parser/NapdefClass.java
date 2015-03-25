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

public class NapdefClass {

    public String mNapid;

    public ArrayList<String> mBearer = new ArrayList<String>();

    public String mName;

    public String mInternet;

    public String mNapaddress;

    public String mNapaddrtype;

    public ArrayList<String> mDnsaddr = new ArrayList<String>();

    public String mCalltype;

    public String mLocaladdr;

    public String mLocaladdrtype;

    public String mLinkspeed;

    public String mDnlinkspeed;

    public String mLinger;

    public String mDeliveryerrsdu;

    public String mDeliveryorder;

    public String mTrafficclass;

    public String mMaxsdusize;

    public String mMaxbitrateuplink;

    public String mMaxbitratednlink;

    public String mResidualber;

    public String mSduerrorratio;

    public String mTraffichandlproi;

    public String mTransferdelay;

    public String mGuaranteedbitrateuplink;

    public String mGuaranteedbitratednlink;

    public String mMaxnumretry;

    public String mFirstretrytimeout;

    public String mReregthreshold;

    public String mTbit;

    public ArrayList<NapAuthInfo> mNapauthinfo = new ArrayList<NapAuthInfo>();

    public ArrayList<Validity> mValidity = new ArrayList<Validity>();

    public static class NapAuthInfo {
        public String mAuthtype;

        public String mAuthname;

        public String mAuthsecret;

        public ArrayList<String> mAuthentity = new ArrayList<String>();

        public String mSpi;

        @Override
        public String toString() {
            return "AUTHTYPE: " + mAuthtype + "\n"
            + "AUTHNAME: " + mAuthname + "\n"
            + "AUTHSECRET: " + mAuthsecret + "\n"
            + "AUTH_ENTITY: " + mAuthentity + "\n"
            + "SPI: " + mSpi + "\n";
        }
    }

    public static class Validity {
        public String mCountry;

        public String mNetwork;

        public String mSid;

        public String mSoc;

        public String mValiduntil;

        @Override
        public String toString() {
            return "COUNTRY: " + mCountry + "\n"
            + "NETWORK: " + mNetwork + "\n"
            + "SID: " + mSid + "\n"
            + "SOC: " + mSoc + "\n"
            + "VALIDUNTIL: " + mValiduntil + "\n";
        }
    }

    @Override
    public String toString() {
        return "APPID: " + mNapid + "\n"
        + "BEARER: " + mBearer + "\n"
        + "NAME: " + mName + "\n"
        + "INTERNET: " + mInternet + "\n"
        + "NAP_ADDRESS: " + mNapaddress + "\n"
        + "NAP_ADDRTYPE: " + mNapaddrtype + "\n"
        + "DNS_ADDR: " + mDnsaddr + "\n"
        + "CALLTYPE: " + mCalltype + "\n"
        + "LOCAL_ADDR: " + mLocaladdr + "\n"
        + "LOCAL_ADDRTYPE: " + mLocaladdrtype + "\n"
        + "LINKSPEED: " + mLinkspeed + "\n"
        + "DNLINKSPEED: " + mDnlinkspeed + "\n"
        + "LINGER: " + mLinger + "\n"
        + "DELIVERY_ERR_SDU: " + mDeliveryerrsdu + "\n"
        + "DELIVERY_ORDER: " + mDeliveryorder + "\n"
        + "TRAFFIC_CLASS: " + mTrafficclass + "\n"
        + "MAX_SDU_SIZE: " + mMaxsdusize + "\n"
        + "MAX_BITRATE_UPLINK: " + mMaxbitrateuplink + "\n"
        + "MAX_BITRATE_DNLINK: " + mMaxbitratednlink + "\n"
        + "RESIDUAL_BER: " + mResidualber + "\n"
        + "SDU_ERROR_RATIO: " + mSduerrorratio + "\n"
        + "TRAFFIC_HANDL_PROI: " + mTraffichandlproi + "\n"
        + "TRANSFER_DELAY: " + mTransferdelay + "\n"
        + "GUARANTEED_BITRATE_UPLINK: " + mGuaranteedbitrateuplink + "\n"
        + "GUARANTEED_BITRATE_DNLINK: " + mGuaranteedbitratednlink + "\n"
        + "MAX_NUM_RETRY: " + mMaxnumretry + "\n"
        + "FIRST_RETRY_TIMEOUT: " + mFirstretrytimeout + "\n"
        + "REREG_THRESHOLD: " + mReregthreshold + "\n"
        + "T_BIT: " + mTbit + "\n"
        + "NAPAUTHINFO: " + mNapauthinfo + "\n"
        + "VALIDITY: " + mValidity + "\n";
    }
}
