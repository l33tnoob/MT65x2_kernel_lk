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

import com.mediatek.omacp.parser.ApplicationClass.AppAddr;
import com.mediatek.omacp.parser.ApplicationClass.AppAuth;
import com.mediatek.omacp.parser.ApplicationClass.Port;
import com.mediatek.omacp.parser.ApplicationClass.Resource;
import com.mediatek.omacp.parser.NapdefClass.NapAuthInfo;
import com.mediatek.omacp.parser.NapdefClass.Validity;
import com.mediatek.omacp.parser.ProxyClass.PxAuthInfo;
import com.mediatek.omacp.parser.ProxyClass.PxPhysical;
import com.mediatek.omacp.utils.MTKlog;

import java.util.ArrayList;

public class OmacpParserUtils {

    private static final String TAG = "Omacp/OmacpParserUtils";

    private static final boolean DEBUG = true;

    public static void handlePxParameters(String listType, String type, String parmName,
            String parmValue, ProxyClass px) {
        // TODO Auto-generated method stub
        if (px == null) {
            MTKlog.e(TAG, "OmacpParserUtils handlePxParameters px is null.");
            return;
        }

        ArrayList<PxAuthInfo> pxAuthInfoList = px.mPxauthinfo;
        ArrayList<Port> portList = px.mPort;
        ArrayList<PxPhysical> physicalList = px.mPxphysical;

        if (type.equalsIgnoreCase("PXLOGICAL")) {
            handlePxLogicalParams(parmName, parmValue, px);
        } else if (type.equalsIgnoreCase("PXAUTHINFO")) {
            handlePxAuthInfoParams(parmName, parmValue, pxAuthInfoList);
        } else if (type.equalsIgnoreCase("PORT")) {
            handlePortParams(parmName, parmValue, listType, portList, px);
        } else if (type.equalsIgnoreCase("PXPHYSICAL")) {
            handlePxPhysicalParams(parmName, parmValue, physicalList);
        }
    }

    private static void handlePxLogicalParams(String parmName, String parmValue, ProxyClass px) {
        if (parmName.equalsIgnoreCase("PROXY-ID") && px.mProxyId == null) {
            px.mProxyId = parmValue;
        } else if (parmName.equalsIgnoreCase("PROXY-PW") && px.mProxyPw == null) {
            px.mProxyPw = parmValue;
        } else if (parmName.equalsIgnoreCase("PPGAUTH-TYPE") && px.mPpgauthType == null) {
            px.mPpgauthType = parmValue;
        } else if (parmName.equalsIgnoreCase("PROXY-PROVIDER-ID") && px.mProxyProviderId == null) {
            px.mProxyProviderId = parmValue;
        } else if (parmName.equalsIgnoreCase("NAME") && px.mName == null) {
            px.mName = parmValue;
        } else if (parmName.equalsIgnoreCase("DOMAIN")) {
            px.mDomain.add(parmValue);
        } else if (parmName.equalsIgnoreCase("TRUST") && px.mTrust == null) {
            px.mTrust = "1"; // take no value, just exists
        } else {
            handleLowUsePxLogicalParams(parmName, parmValue, px);
        }
    }

    private static void handleLowUsePxLogicalParams(String parmName, String parmValue, ProxyClass px) {
        if (parmName.equalsIgnoreCase("MASTER") && px.mMaster == null) {
            px.mMaster = "1"; // take no value, just exists
        } else if (parmName.equalsIgnoreCase("STARTPAGE") && px.mStartpage == null) {
            px.mStartpage = parmValue;
        } else if (parmName.equalsIgnoreCase("BASAUTH-ID") && px.mBasauthId == null) {
            px.mBasauthId = parmValue;
        } else if (parmName.equalsIgnoreCase("BASAUTH-PW") && px.mBasauthPw == null) {
            px.mBasauthPw = parmValue;
        } else if (parmName.equalsIgnoreCase("WSP-VERSION") && px.mWspVersion == null) {
            px.mWspVersion = parmValue;
        } else if (parmName.equalsIgnoreCase("PUSHENABLED") && px.mPushenabled == null) {
            px.mPushenabled = parmValue;
        } else if (parmName.equalsIgnoreCase("PULLENBALED") && px.mPullenbaled == null) {
            px.mPullenbaled = parmValue;
        }
    }

    private static void handlePxAuthInfoParams(String parmName, String parmValue,
            ArrayList<PxAuthInfo> pxAuthInfoList) {
        if (null == pxAuthInfoList || pxAuthInfoList.size() == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handlePxParameters PXAUTHINFO size is 0.");
            return;
        }
        int size = pxAuthInfoList.size();
        if (parmName.equalsIgnoreCase("PXAUTH-TYPE")
                && pxAuthInfoList.get(size - 1).mPxauthType == null) {
            pxAuthInfoList.get(size - 1).mPxauthType = parmValue;
        } else if (parmName.equalsIgnoreCase("PXAUTH-ID")
                && pxAuthInfoList.get(size - 1).mPxauthId == null) {
            pxAuthInfoList.get(size - 1).mPxauthId = parmValue;
        } else if (parmName.equalsIgnoreCase("PXAUTH-PW")
                && pxAuthInfoList.get(size - 1).mPxauthPw == null) {
            pxAuthInfoList.get(size - 1).mPxauthPw = parmValue;
        }
    }

    private static void handlePortParams(String parmName, String parmValue, String listType,
            ArrayList<Port> portList, ProxyClass px) {
        int size = portList.size();
        if (listType.equalsIgnoreCase("PXLOGICAL")) {
            if (size == 0) {
                MTKlog.e(TAG, "OmacpParserUtils handlePxParameters PORT size is 0.");
                return;
            }
            if (parmName.equalsIgnoreCase("PORTNBR") && portList.get(size - 1).mPortnbr == null) {
                portList.get(size - 1).mPortnbr = parmValue;
            } else if (parmName.equalsIgnoreCase("SERVICE")) {
                portList.get(size - 1).mService.add(parmValue);
            }
        } else if (listType.equalsIgnoreCase("PXPHYSICAL")) {
            int pxPhysicalSize = px.mPxphysical.size();
            if (pxPhysicalSize == 0) {
                MTKlog.e(TAG, "OmacpParserUtils handlePxParameters PXPHYSICAL size is 0.");
                return;
            }
            int portSize = px.mPxphysical.get(pxPhysicalSize - 1).mPort.size();
            if (portSize == 0) {
                MTKlog.e(TAG, "OmacpParserUtils handlePxParameters PXPHYSICAL PORT size is 0.");
                return;
            }
            if (parmName.equalsIgnoreCase("PORTNBR")
                    && px.mPxphysical.get(pxPhysicalSize - 1).mPort.get(portSize - 1).mPortnbr == null) {
                px.mPxphysical.get(pxPhysicalSize - 1).mPort.get(portSize - 1).mPortnbr = parmValue;
            } else if (parmName.equalsIgnoreCase("SERVICE")) {
                px.mPxphysical.get(pxPhysicalSize - 1).mPort.get(portSize - 1).mService
                        .add(parmValue);
            }
        }
    }

    private static void handlePxPhysicalParams(String parmName, String parmValue,
            ArrayList<PxPhysical> physicalList) {
        int size = physicalList.size();
        if (size == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handlePxParameters PXPHYSICAL size is 0.");
            return;
        }
        if (parmName.equalsIgnoreCase("PHYSICAL-PROXY-ID")
                && physicalList.get(size - 1).mPhysicalProxyId == null) {
            physicalList.get(size - 1).mPhysicalProxyId = parmValue;
        } else if (parmName.equalsIgnoreCase("DOMAIN")) {
            physicalList.get(size - 1).mDomain.add(parmValue);
        } else if (parmName.equalsIgnoreCase("PXADDR")
                && physicalList.get(size - 1).mPxaddr == null) {
            physicalList.get(size - 1).mPxaddr = parmValue;
        } else if (parmName.equalsIgnoreCase("PXADDRTYPE")
                && physicalList.get(size - 1).mPxaddrtype == null) {
            physicalList.get(size - 1).mPxaddrtype = parmValue;
        } else {
            handleLowUsePxPhysicalParams(parmName, parmValue, physicalList, size);
        }
    }

    private static void handleLowUsePxPhysicalParams(String parmName, String parmValue,
            ArrayList<PxPhysical> physicalList, int size) {
        if (parmName.equalsIgnoreCase("PXADDR-FQDN")
                && physicalList.get(size - 1).mPxaddrFqdn == null) {
            physicalList.get(size - 1).mPxaddrFqdn = parmValue;
        } else if (parmName.equalsIgnoreCase("WSP-VERSION")
                && physicalList.get(size - 1).mWspVersion == null) {
            physicalList.get(size - 1).mWspVersion = parmValue;
        } else if (parmName.equalsIgnoreCase("PUSHENABLED")
                && physicalList.get(size - 1).mPushenabled == null) {
            physicalList.get(size - 1).mPushenabled = parmValue;
        } else if (parmName.equalsIgnoreCase("PULLENABLED")
                && physicalList.get(size - 1).mPullenabled == null) {
            physicalList.get(size - 1).mPullenabled = parmValue;
        } else if (parmName.equalsIgnoreCase("TO-NAPID")) {
            physicalList.get(size - 1).mToNapid.add(parmValue);
        }
    }

    public static void handleNapParameters(String type, String parmName, String parmValue,
            NapdefClass nap) {
        // TODO Auto-generated method stub
        if (nap == null) {
            MTKlog.e(TAG, "OmacpParserUtils handleNapParameters nap is null.");
            return;
        }

        ArrayList<NapAuthInfo> napAuthInfoList = nap.mNapauthinfo;
        ArrayList<Validity> validityList = nap.mValidity;

        if (type.equalsIgnoreCase("NAPDEF")) {
            handleNapDefParams(parmName, parmValue, nap);
        } else if (type.equalsIgnoreCase("NAPAUTHINFO")) {
            handleNapAuthInfoParams(parmName, parmValue, napAuthInfoList);
        } else if (type.equalsIgnoreCase("VALIDITY")) {
            handleValidityParams(parmName, parmValue, validityList);
        }
    }

    private static void handleNapDefParams(String parmName, String parmValue, NapdefClass nap) {
        if (parmName.equalsIgnoreCase("NAPID") && nap.mNapid == null) {
            nap.mNapid = parmValue;
        } else if (parmName.equalsIgnoreCase("BEARER")) {
            nap.mBearer.add(parmValue);
        } else if (parmName.equalsIgnoreCase("NAME") && nap.mName == null) {
            nap.mName = parmValue;
        } else if (parmName.equalsIgnoreCase("INTERNET") && nap.mInternet == null) {
            nap.mInternet = "1"; // take no value, just exists
        } else {
            handleNapDefAddrParams(parmName, parmValue, nap);
        }
    }

    private static void handleNapDefAddrParams(String parmName, String parmValue, NapdefClass nap) {
        if (parmName.equalsIgnoreCase("NAP-ADDRESS") && nap.mNapaddress == null) {
            nap.mNapaddress = parmValue;
        } else if (parmName.equalsIgnoreCase("NAP-ADDRTYPE") && nap.mNapaddrtype == null) {
            nap.mNapaddrtype = parmValue;
        } else if (parmName.equalsIgnoreCase("DNS-ADDR")) {
            nap.mDnsaddr.add(parmValue);
        } else if (parmName.equalsIgnoreCase("CALLTYPE") && nap.mCalltype == null) {
            nap.mCalltype = parmValue;
        } else if (parmName.equalsIgnoreCase("LOCAL_ADDR") && nap.mLocaladdr == null) {
            nap.mLocaladdr = parmValue;
        } else if (parmName.equalsIgnoreCase("LOCAL_ADDRTYPE") && nap.mLocaladdrtype == null) {
            nap.mLocaladdrtype = parmValue;
        } else {
            handleNapDefLinkParams(parmName, parmValue, nap);
        }
    }

    private static void handleNapDefLinkParams(String parmName, String parmValue, NapdefClass nap) {
        if (parmName.equalsIgnoreCase("LINKSPEED") && nap.mLinkspeed == null) {
            nap.mLinkspeed = parmValue;
        } else if (parmName.equalsIgnoreCase("DNLINKSPEED") && nap.mDnlinkspeed == null) {
            nap.mDnlinkspeed = parmValue;
        } else if (parmName.equalsIgnoreCase("LINGER") && nap.mLinger == null) {
            nap.mLinger = parmValue;
        } else if (parmName.equalsIgnoreCase("DELIVERY-ERR-SDU") && nap.mDeliveryerrsdu == null) {
            nap.mDeliveryerrsdu = parmValue;
        } else if (parmName.equalsIgnoreCase("DELIVERY-ORDER") && nap.mDeliveryorder == null) {
            nap.mDeliveryorder = parmValue;
        } else if (parmName.equalsIgnoreCase("TRAFFIC-CLASS") && nap.mTrafficclass == null) {
            nap.mTrafficclass = parmValue;
        } else {
            handleNapDefMaxParams(parmName, parmValue, nap);
        }
    }

    private static void handleNapDefMaxParams(String parmName, String parmValue, NapdefClass nap) {
        if (parmName.equalsIgnoreCase("MAX-SDU-SIZE") && nap.mMaxsdusize == null) {
            nap.mMaxsdusize = parmValue;
        } else if (parmName.equalsIgnoreCase("MAX-BITRATE-UPLINK") && nap.mMaxbitrateuplink == null) {
            nap.mMaxbitrateuplink = parmValue;
        } else if (parmName.equalsIgnoreCase("MAX-BITRATE-DNLINK") && nap.mMaxbitratednlink == null) {
            nap.mMaxbitratednlink = parmValue;
        } else if (parmName.equalsIgnoreCase("RESIDUAL-BER") && nap.mResidualber == null) {
            nap.mResidualber = parmValue;
        } else if (parmName.equalsIgnoreCase("SDU-ERROR-RATIO") && nap.mSduerrorratio == null) {
            nap.mSduerrorratio = parmValue;
        } else if (parmName.equalsIgnoreCase("TRAFFIC-HANDL-PROI") && nap.mTraffichandlproi == null) {
            nap.mTraffichandlproi = parmValue;
        } else {
            handleLowUseNapDefParams(parmName, parmValue, nap);
        }
    }

    private static void handleLowUseNapDefParams(String parmName, String parmValue, NapdefClass nap) {
        if (parmName.equalsIgnoreCase("TRANSFER-DELAY") && nap.mTransferdelay == null) {
            nap.mTransferdelay = parmValue;
        } else if (parmName.equalsIgnoreCase("GUARANTEED-BITRATE-UPLINK")
                && nap.mGuaranteedbitrateuplink == null) {
            nap.mGuaranteedbitrateuplink = parmValue;
        } else if (parmName.equalsIgnoreCase("GUARANTEED-BITRATE-DNLINK")
                && nap.mGuaranteedbitratednlink == null) {
            nap.mGuaranteedbitratednlink = parmValue;
        } else if (parmName.equalsIgnoreCase("MAX-NUM-RETRY") && nap.mMaxnumretry == null) {
            nap.mMaxnumretry = parmValue;
        } else if (parmName.equalsIgnoreCase("FIRST-RETRY-TIMEOUT")
                && nap.mFirstretrytimeout == null) {
            nap.mFirstretrytimeout = parmValue;
        } else if (parmName.equalsIgnoreCase("REREG-THRESHOLD") && nap.mReregthreshold == null) {
            nap.mReregthreshold = parmValue;
        } else if (parmName.equalsIgnoreCase("T-BIT") && nap.mTbit == null) {
            nap.mTbit = "1"; // take no value, just exists
        }
    }

    private static void handleNapAuthInfoParams(String parmName, String parmValue,
            ArrayList<NapAuthInfo> napAuthInfoList) {
        int size = napAuthInfoList.size();
        if (size == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handleNapParameters NAPAUTHINFO size is 0.");
            return;
        }
        if (parmName.equalsIgnoreCase("AUTHTYPE")
                && napAuthInfoList.get(size - 1).mAuthtype == null) {
            napAuthInfoList.get(size - 1).mAuthtype = parmValue;
        } else if (parmName.equalsIgnoreCase("AUTHNAME")
                && napAuthInfoList.get(size - 1).mAuthname == null) {
            napAuthInfoList.get(size - 1).mAuthname = parmValue;
        } else if (parmName.equalsIgnoreCase("AUTHSECRET")
                && napAuthInfoList.get(size - 1).mAuthsecret == null) {
            napAuthInfoList.get(size - 1).mAuthsecret = parmValue;
        } else if (parmName.equalsIgnoreCase("AUTH_ENTITY")) {
            napAuthInfoList.get(size - 1).mAuthentity.add(parmValue);
        } else if (parmName.equalsIgnoreCase("SPI") && napAuthInfoList.get(size - 1).mSpi == null) {
            napAuthInfoList.get(size - 1).mSpi = parmValue;
        }
    }

    private static void handleValidityParams(String parmName, String parmValue,
            ArrayList<Validity> validityList) {
        int size = validityList.size();
        if (size == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handleNapParameters VALIDITY size is 0.");
            return;
        }
        if (parmName.equalsIgnoreCase("COUNTRY") && validityList.get(size - 1).mCountry == null) {
            validityList.get(size - 1).mCountry = parmValue;
        } else if (parmName.equalsIgnoreCase("NETWORK")
                && validityList.get(size - 1).mNetwork == null) {
            validityList.get(size - 1).mNetwork = parmValue;
        } else if (parmName.equalsIgnoreCase("SID") && validityList.get(size - 1).mSid == null) {
            validityList.get(size - 1).mSid = parmValue;
        } else if (parmName.equalsIgnoreCase("SOC") && validityList.get(size - 1).mSoc == null) {
            validityList.get(size - 1).mSoc = parmValue;
        } else if (parmName.equalsIgnoreCase("VALIDUNTIL")
                && validityList.get(size - 1).mValiduntil == null) {
            validityList.get(size - 1).mValiduntil = parmValue;
        }

    }

    public static void handleApParameters(String type, String parmName, String parmValue,
            ApplicationClass application) {
        if (application == null) {
            MTKlog.e(TAG, "OmacpParserUtils handleApParameters application is null.");
            return;
        }

        ArrayList<AppAddr> appAddrList = application.mAppaddr;
        ArrayList<AppAuth> appAuthList = application.mAppauth;
        ArrayList<Resource> resourceList = application.mResource;

        if (type.equalsIgnoreCase("APPLICATION")) {
            handleApplicationParams(type, parmName, parmValue, application);
        } else if (type.equalsIgnoreCase("APPADDR")) {
            handleAppAddrParams(parmName, parmValue, appAddrList, application);
        } else if (type.equalsIgnoreCase("PORT")) {
            handleApPortParam(parmName, parmValue, appAddrList);
        } else if (type.equalsIgnoreCase("APPAUTH")) {
            handleAppAuthParams(parmName, parmValue, appAuthList);
        } else if (type.equalsIgnoreCase("RESOURCE")) {
            handleResourceParams(parmName, parmValue, resourceList);
        }
    }

    private static void handleApplicationParams(String type, String parmName, String parmValue,
            ApplicationClass application) {
        if (parmName.equalsIgnoreCase("APPID") && application.mAppid == null) {
            application.mAppid = parmValue;
        } else if (parmName.equalsIgnoreCase("PROVIDER-ID") && application.mProviderId == null) {
            application.mProviderId = parmValue;
        } else if (parmName.equalsIgnoreCase("NAME") && application.mName == null) {
            application.mName = parmValue;
        } else if (parmName.equalsIgnoreCase("AACCEPT") && application.mAaccept == null) {
            application.mAaccept = parmValue;
        } else if (parmName.equalsIgnoreCase("APROTOCOL") && application.mAprotocol == null) {
            application.mAprotocol = parmValue;
        } else {
            handleOtherApplicationParams(type, parmName, parmValue, application);
        }
    }

    private static void handleOtherApplicationParams(String type, String parmName,
            String parmValue, ApplicationClass application) {
        if (parmName.equalsIgnoreCase("TO-PROXY")) {
            application.mToProxy.add(parmValue);
        } else if (parmName.equalsIgnoreCase("TO-NAPID")) {
            application.mToNapid.add(parmValue);
        } else if (parmName.equalsIgnoreCase("ADDR") && type.equalsIgnoreCase("APPLICATION")) {
            application.mAddr.add(parmValue);
        } else if (parmName.equalsIgnoreCase("CM") && application.mCm == null) {
            application.mCm = parmValue;
        } else if (parmName.equalsIgnoreCase("RM") && application.mRm == null) {
            application.mRm = parmValue;
        } else if (parmName.equalsIgnoreCase("MS") && application.mMs == null) {
            application.mMs = parmValue;
        } else {
            handleOtherAddrApplicationParams(type, parmName, parmValue, application);
        }
    }

    private static void handleOtherAddrApplicationParams(String type, String parmName,
            String parmValue, ApplicationClass application) {
        if (parmName.equalsIgnoreCase("PC-ADDR") && application.mPcAddr == null) {
            application.mPcAddr = parmValue;
        } else if (parmName.equalsIgnoreCase("Ma") && application.mMa == null) {
            application.mMa = parmValue;
        } else if (parmName.equalsIgnoreCase("INIT") && application.mInit == null) {
            application.mInit = "1";
        } else if (parmName.equalsIgnoreCase("FROM") && application.mFrom == null) {
            application.mFrom = parmValue;
        } else if (parmName.equalsIgnoreCase("RT-ADDR") && application.mRtAddr == null) {
            application.mRtAddr = parmValue;
        } else {
            handleLowUseApplicationParams(type, parmName, parmValue, application);
        }
    }

    private static void handleLowUseApplicationParams(String type, String parmName,
            String parmValue, ApplicationClass application) {
        if (parmName.equalsIgnoreCase("MAX-BANDWIDTH") && application.mMaxBandwidth == null) {
            application.mMaxBandwidth = parmValue;
        } else if (parmName.equalsIgnoreCase("NETINFO")) {
            application.mNetinfo.add(parmValue);
        } else if (parmName.equalsIgnoreCase("MIN-UDP-PORT") && application.mMinUdpPort == null) {
            application.mMinUdpPort = parmValue;
        } else if (parmName.equalsIgnoreCase("MAX-UDP-PORT") && application.mMaxUdpPort == null) {
            application.mMaxUdpPort = parmValue;
        } else if (parmName.equalsIgnoreCase("SERVICES") && application.mServices == null) {
            application.mServices = parmValue;
        } else if (parmName.equalsIgnoreCase("CIDPREFIX") && application.mCidprefix == null) {
            application.mCidprefix = parmValue;
        }
    }

    private static void handleAppAddrParams(String parmName, String parmValue,
            ArrayList<AppAddr> appAddrList, ApplicationClass application) {
        int size = appAddrList.size();
        if (size == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handleApParameters APPADDR size is 0.");
            return;
        }
        if (parmName.equalsIgnoreCase("ADDR") && appAddrList.get(size - 1).mAddr == null) {
            appAddrList.get(size - 1).mAddr = parmValue;
        } else if (parmName.equalsIgnoreCase("ADDRTYPE")
                && application.mAppaddr.get(size - 1).mAddrtype == null) {
            appAddrList.get(size - 1).mAddrtype = parmValue;
        }
    }

    private static void handleApPortParam(String parmName, String parmValue,
            ArrayList<AppAddr> appAddrList) {
        int addrSize = appAddrList.size();
        if (addrSize == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handleApParameters APPADDR size is 0.");
            return;
        }
        int size = appAddrList.get(addrSize - 1).mPort.size();
        if (size == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handleApParameters PORT size is 0.");
            return;
        }
        if (parmName.equalsIgnoreCase("PORTNBR")
                && appAddrList.get(addrSize - 1).mPort.get(size - 1).mPortnbr == null) {
            appAddrList.get(addrSize - 1).mPort.get(size - 1).mPortnbr = parmValue;
        } else if (parmName.equalsIgnoreCase("SERVICE")) {
            appAddrList.get(addrSize - 1).mPort.get(size - 1).mService.add(parmValue);
        }
    }

    private static void handleAppAuthParams(String parmName, String parmValue,
            ArrayList<AppAuth> appAuthList) {
        int size = appAuthList.size();
        if (size == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handleApParameters APPAUTH size is 0.");
            return;
        }
        if (parmName.equalsIgnoreCase("AAUTHLEVEL")
                && appAuthList.get(size - 1).mAauthlevel == null) {
            appAuthList.get(size - 1).mAauthlevel = parmValue;
        } else if (parmName.equalsIgnoreCase("AAUTHTYPE")
                && appAuthList.get(size - 1).mAauthtype == null) {
            appAuthList.get(size - 1).mAauthtype = parmValue;
        } else if (parmName.equalsIgnoreCase("AAUTHNAME")
                && appAuthList.get(size - 1).mAauthname == null) {
            appAuthList.get(size - 1).mAauthname = parmValue;
        } else if (parmName.equalsIgnoreCase("AAUTHSECRET")
                && appAuthList.get(size - 1).mAauthsecret == null) {
            appAuthList.get(size - 1).mAauthsecret = parmValue;
        } else if (parmName.equalsIgnoreCase("AAUTHDATA")
                && appAuthList.get(size - 1).mAauthdata == null) {
            appAuthList.get(size - 1).mAauthdata = parmValue;
        }
    }

    private static void handleResourceParams(String parmName, String parmValue,
            ArrayList<Resource> resourceList) {
        int size = resourceList.size();
        if (size == 0) {
            MTKlog.e(TAG, "OmacpParserUtils handleApParameters RESOURCE size is 0.");
            return;
        }
        if (parmName.equalsIgnoreCase("URI") && resourceList.get(size - 1).mUri == null) {
            resourceList.get(size - 1).mUri = parmValue;
        } else if (parmName.equalsIgnoreCase("NAME") && resourceList.get(size - 1).mName == null) {
            resourceList.get(size - 1).mName = parmValue;
        } else if (parmName.equalsIgnoreCase("AACCEPT")
                && resourceList.get(size - 1).mAaccept == null) {
            resourceList.get(size - 1).mAaccept = parmValue;
        } else if (parmName.equalsIgnoreCase("AAUTHTYPE")
                && resourceList.get(size - 1).mAauthtype == null) {
            resourceList.get(size - 1).mAauthtype = parmValue;
        } else if (parmName.equalsIgnoreCase("AAUTHNAME")
                && resourceList.get(size - 1).mAauthname == null) {
            resourceList.get(size - 1).mAauthname = parmValue;
        } else {
            handleOtherResourceParams(parmName, parmValue, resourceList, size);
        }
    }

    private static void handleOtherResourceParams(String parmName, String parmValue,
            ArrayList<Resource> resourceList, int size) {
        if (parmName.equalsIgnoreCase("AAUTHSECRET")
                && resourceList.get(size - 1).mAauthsecret == null) {
            resourceList.get(size - 1).mAauthsecret = parmValue;
        } else if (parmName.equalsIgnoreCase("AAUTHDATA")
                && resourceList.get(size - 1).mAauthdata == null) {
            resourceList.get(size - 1).mAauthdata = parmValue;
        } else if (parmName.equalsIgnoreCase("STARTPAGE")
                && resourceList.get(size - 1).mStartpage == null) {
            // take no value, just exists
            resourceList.get(size - 1).mStartpage = "1";
        } else if (parmName.equalsIgnoreCase("CLIURI")
                && resourceList.get(size - 1).mCliuri == null) {
            // take no value, just exists
            resourceList.get(size - 1).mCliuri = parmName;
        } else if (parmName.equalsIgnoreCase("SYNCTYPE")
                && resourceList.get(size - 1).mSynctype == null) {
            // take no value, just exists
            resourceList.get(size - 1).mSynctype = parmName;
        }
    }

    public static ArrayList<ApplicationClass> removeInvalidApSettings(
            ArrayList<ApplicationClass> apList) {
        if (apList == null) {
            MTKlog.e(TAG, "OmacpParserUtils removeDuplicateApSettings apList is null.");
            return null;
        }

        // remove duplicate application settings
        for (int i = 0; i < apList.size(); i++) {
            String appId = apList.get(i).mAppid;
            if (null == appId) {
                apList.remove(i);
                continue;
            }

            for (int j = i + 1; j < apList.size(); j++) {
                if (apList.get(j).mAppid.equalsIgnoreCase(appId)) {
                    if (DEBUG) {
                        MTKlog.i(TAG,
                                "OmacpParserUtils removeDuplicateApSettings duplicate application settings, "
                                        + "will remove " + j + " " + "element");
                    }

                    apList.remove(j);
                    j--;
                }
            }
        }

        return apList;
    }

}
