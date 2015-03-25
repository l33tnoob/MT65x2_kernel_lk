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

import android.util.Xml;

import com.mediatek.omacp.parser.ApplicationClass.AppAddr;
import com.mediatek.omacp.parser.ApplicationClass.AppAuth;
import com.mediatek.omacp.parser.ApplicationClass.Port;
import com.mediatek.omacp.parser.ApplicationClass.Resource;
import com.mediatek.omacp.parser.NapdefClass.NapAuthInfo;
import com.mediatek.omacp.parser.NapdefClass.Validity;
import com.mediatek.omacp.parser.ProxyClass.PxAuthInfo;
import com.mediatek.omacp.parser.ProxyClass.PxPhysical;
import com.mediatek.omacp.utils.MTKlog;

import org.kxml2.wap.WbxmlParser;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

public class OmacpParser {

    private class OmacpParserData {
        public String mName;

        public String mUri;

        public String mType;

        public String mListType;

        public String mParmName;

        public String mParmValue;

        public ApplicationClass mApplication;

        public AppAddr mAppAddr;

        public Port mPort;

        public AppAuth mAppAuth;

        public Resource mResource;

        public NapdefClass mNapdefClass;

        public NapAuthInfo mNapAuthInfo;

        public Validity mValidity;

        public ProxyClass mProxyClass;

        public PxAuthInfo mPxAuthInfo;

        public PxPhysical mPxPhysical;
    }

    private static final String TAG = "Omacp/OmacpParser";

    // constants for labels
    private static final String CHARACTERISTIC = "characteristic";

    private static final String PARM = "parm";

    private ArrayList<ApplicationClass> mApSectionList = new ArrayList<ApplicationClass>();

    private ArrayList<ProxyClass> mPxList = new ArrayList<ProxyClass>();

    private ArrayList<NapdefClass> mNapList = new ArrayList<NapdefClass>();

    private String mContextName;

    private Object mParser;

    public static final String[] TAG_TABLE_PAGE0 = {
            "wap-provisioningdoc", "characteristic", "parm"
    };

    public static final String[] TAG_TABLE_PAGE1 = {
            "", "characteristic", "parm"
    };

    public static final String [] ATTR_START_TABLE_PAGE0 = {
          "name", "value", "name=NAME", "name=NAP-ADDRESS", "name=NAP-ADDRTYPE", "name=CALLTYPE", //5~A
          "name=VALIDUNTIL", "name=AUTHTYPE", "name=AUTHNAME", "name=AUTHSECRET", "name=LINGER", //B~F
          "name=BEARER", "name=NAPID", "name=COUNTRY", "name=NETWORK", "name=INTERNET", "name=PROXY-ID", //10~15
          "name=PROXY-PROVIDER-ID", "name=DOMAIN", "name=PROVURL", "name=PXAUTH-TYPE", "name=PXAUTH-ID", //16~1A
          "name=PXAUTH-PW", "name=STARTPAGE", "name=BASAUTH-ID", "name=BASAUTH-PW", "name=PUSHENABLED", //1B~1F
          "name=PXADDR", "name=PXADDRTYPE", "name=TO-NAPID", "name=PORTNBR", "name=SERVICE", "name=LINKSPEED", //20~25
          "name=DNLINKSPEED", "name=LOCAL-ADDR", "name=LOCAL-ADDRTYPE", "name=CONTEXT-ALLOW", "name=TRUST", //26~2A
          "name=MASTER", "name=SID", "name=SOC", "name=WSP-VERSION", "name=PHYSICAL-PROXY-ID", //2B~2F
          "name=CLIENT-ID", "name=DELIVERY-ERR-SDU", "name=DELIVERY-ORDER", "name=TRAFFIC-CLASS", 
          "name=MAX-SDU-SIZE", "name=MAX-BITRATE-UPLINK", //30~35
          "name=MAX-BITRATE-DNLINK", "name=RESIDUAL-BER", "name=SDU-ERROR-RATIO", 
          "name=TRAFFIC-HANDL-PRIO", "name=TRANSFER-DELAY", //36~3A
          "name=GUARANTEED-BITRATE-UPLINK", "name=GUARANTEED-BITRATE-DNLINK", "name=PXADDR-FQDN", 
          "name=PROXY-PW", "name=PPGAUTH-TYPE", //3B~3F
          "", "", "", "", "", "version", "version=1.0", //40~46
          "name=PULLENABLED", "name=DNS-ADDR", "name=MAX-NUM-RETRY", "name=FIRST-RETRY-TIMEOUT", //47~4A
          "name=REREG-THRESHOLD", "name=T-BIT", "", "name=AUTH-ENTITY", "name=SPI", //4B~4F
          "type", "type=PXLOGICAL", "type=PXPHYSICAL", "type=PORT", "type=VALIDITY", "type=NAPDEF", //50~55
          "type=BOOTSTRAP", "type=VENDORCONFIG", "type=CLIENTIDENTITY", "type=PXAUTHINFO", 
          "type=NAPAUTHINFO", "type=ACCESS" //56~5B
    };

    public static final String [] ATTR_START_TABLE_PAGE1 = {
          "name", "value", "name=NAME", "", "", "", //5~A
          "", "", "", "", "", //B~F
          "", "", "", "", "name=INTERNET", "", //10~15
          "", "", "", "", "", //16~1A
          "", "name=STARTPAGE", "", "", "", //1B~1F
          "", "", "name=TO-NAPID", "name=PORTNBR", "name=SERVICE", "", //20~25
          "", "", "", "", "", //26~2A
          "", "", "", "name=ACCEPT", "name=AAUTHDATA", //2B~2F
          "name=AAUTHLEVEL", "name=AAUTHNAME", "name=AAUTHSECRET", "name=AAUTHTYPE", "name=ADDR", "name=ADDRTYPE",//30~35
          "name=APPID", "name=ARPOTOCOL", "name=PROVIDER-ID", "name=TO-PROXY", "name=URI", //36~3A
          "name=RULE", "", "", "", "", //3B~3F
          "", "", "", "", "", "", "", //40~46
          "", "", "", "", //47~4A
          "", "", "", "", "", //4B~4F
          "type", "", "", "type=PORT", "", "type=APPLICATION", //50~55
          "type=APPADDR", "type=APPAUTH", "type=CLIENTIDENTITY", "type=RESOURCE", "", "" //56~5B
    };

    public static final String [] ATTR_VALUE_TABLE_PAGE0 = {
          "IPV4", "IPV6", "E164", "ALPHA", "APN", "SCODE", //85~8A
          "TETRA-ITSI", "MAN", "", "", "", //8B~8F
          "ANALOG-MODEM", "V.120", "V.110", "X.31", "BIT-TRANSPARENT", "DIRECT-ASYNCHRONOUS-DATA-SERVICE", //90~95
          "", "", "", "", "PAP", //96~9A
          "CHAP", "HTTP-BASIC", "HTTP-DIGEST", "WTLS-SS", "MD5", //9B~9F
          "", "", "GSM-USSD", "GSM-SMS", "ANSI-136-GUTS", "IS-95-CDMA-SMS", //A0~A5
          "IS-95-CDMA-CSD", "IS-95-CDMA-PACKET", "ANSI-136-CSD", "ANSI-136-GPRS", "GSM-CSD", //A6~AA
          "GSM-GPRS", "AMPS-CDPD", "PDC-CSD", "PDC-PACKET", "IDEN-SMS", //AB~AF
          "IDEN-CSD", "IDEN-PACKET", "FLEX/REFLEX", "PHS-SMS", "PHS-CSD", "TETRA-SDS", //B0~B5
          "TETRA-PACKET", "ANSI-136-GHOST", "MOBITEX-MPAK", "CDMA2000-1X-SIMPLE-IP", "CDMA2000-1X-MOBILE-IP", //B6~BA
          "", "", "", "", "", //BB~BF
          "", "", "", "", "", "AUTOBAUDING", //C0~C5
          "", "", "", "", "CL-WSP", //C6~CA
          "CO-WSP", "CL-SEC-WSP", "CO-SEC-WSP", "CL-SEC-WTA", "CO-SEC-WTA", //CB~CF
          "OTA-HTTP-TO", "OTA-HTTP-TLS-TO", "OTA-HTTP-PO", "OTA-HTTP-TLS-PO", //D0~D3
          "", "", "", "", "", "", "", //D4~DA
          "", "", "", "", "", //DB~DF
          "AAA", "HA"
        };

    public static final String [] ATTR_VALUE_TABLE_PAGE1 = {
          "", "", "NAME", "", "", "", //5~A
          "", "", "", "", "", //B~F
          "", "", "", "", "INTERNET", "", //10~15
          "", "", "", "", "", //16~1A
          "", "STARTPAGE", "", "", "", //1B~1F
          "", "", "TO-NAPID", "PORTNBR", "SERVICE", "", //20~25
          "", "", "", "", "", //26~2A
          "", "", "", "ACCEPT", "AAUTHDATA", //2B~2F
          "AAUTHLEVEL", "AAUTHNAME", "AAUTHSECRET", "AAUTHTYPE", "ADDR", "ADDRTYPE",//30~35
          "APPID", "ARPOTOCOL", "PROVIDER-ID", "TO-PROXY", "URI", //36~3A
          "RULE", "", "", "", "", //3B~3F
          "", "", "", "", "", "", "", //40~46
          "", "", "", "", //47~4A
          "", "", "", "", "", //4B~4F
          "", "", "", "PORT", "", "APPLICATION", //50~55
          "APPADDR", "APPAUTH", "CLIENTIDENTITY", "RESOURCE", "", "", //56~5B
          "", "", "", "", //5C~5F
          "", "", "", "", "", "", //60~65
          "", "", "", "", "", //66~6A
          "", "", "", "", "", //6B~6F
          "", "", "", "", "", "", //70~75
          "", "", "", "", "", //76~7A
          "", "", "", "", "", //7B~7F
          "", "", "", "", "", //80~84

          "", "IPV6", "E164", "ALPHA", "", "", //85~8A
          "", "", "APPSRV", "OBEX", "", //8B~8F
          ",(comma character", "HTTP-", "BASIC", "DIGEST", "", "", //90~95
        };

    public void setParser(Object parser) {
        mParser = parser;
    }

    public static XmlPullParser getTextParser() {
        XmlPullParser parser = Xml.newPullParser();
        return parser;
    }

    public static WbxmlParser getWbxmlParser() {
        WbxmlParser parser = new WbxmlParser();
        parser.setTagTable(0, TAG_TABLE_PAGE0);
        parser.setAttrStartTable(0, ATTR_START_TABLE_PAGE0);
        parser.setAttrValueTable(0, ATTR_VALUE_TABLE_PAGE0);

        parser.setTagTable(1, TAG_TABLE_PAGE1);
        parser.setAttrStartTable(1, ATTR_START_TABLE_PAGE1);
        parser.setAttrValueTable(1, ATTR_VALUE_TABLE_PAGE1);

        return parser;
    }

    public void parse(byte[] data) {

        if (mParser == null) {
            MTKlog.e(TAG, "OmacpParserBase mParser is null.");
            return;
        }

        InputStream in = new ByteArrayInputStream(data);
        OmacpParserData omacpParserData = new OmacpParserData();
        try {
            if (mParser instanceof XmlPullParser) {
                ((XmlPullParser) mParser).setInput(in, null);
            } else if (mParser instanceof WbxmlParser) {
                ((WbxmlParser) mParser).setInput(in, null);
            } else {
                MTKlog.e(TAG, "OmacpParserBase unknown parser type.");
            }

            int eventType = -1;
            eventType = getEventType();

            while (eventType != XmlPullParser.END_DOCUMENT) {
                getParserName(omacpParserData);
                switch (eventType) {
                    case XmlPullParser.START_DOCUMENT:
                        break;

                    case XmlPullParser.START_TAG:
                        getStartTagData(omacpParserData);
                        break;

                    case XmlPullParser.END_TAG:
                        getEndTagData(omacpParserData);
                        break;

                    default:
                        MTKlog.e(TAG, "OmacpParserBase parse eventType error, eventType is : " + eventType);
                        break;
                }
                eventType = getEventType();
            }
        } catch (Exception e) {
            MTKlog.e(TAG, "OmacpParserBase parse Exception, e is : " + e);
            mApSectionList.clear();
            mPxList.clear();
            mNapList.clear();
        }
    }

    private void getStartTagData(OmacpParserData omacpParserData) {
        if (omacpParserData.mName.equalsIgnoreCase(CHARACTERISTIC)) {
            // Get the value of the type attribute
            getCharacteristicParams(omacpParserData);
        } else if (omacpParserData.mName.equalsIgnoreCase(PARM)) {
            // Get the value of the type attribute
            // Get the value of name and value attributes
            getAttributeParams(omacpParserData);
        }

    }

    private void getEndTagData(OmacpParserData omacpParserData) {
        // To avoid non-ordered block definition in the big
        // block
        if (omacpParserData.mName.equalsIgnoreCase("CHARACTERISTIC")) {
            if (omacpParserData.mType == null) {
                MTKlog.e(TAG, "OmacpParserBase END_TAG type is null.");
                return;
            }

            if (omacpParserData.mType.equalsIgnoreCase("PORT")) {
                if (omacpParserData.mListType.equalsIgnoreCase("APPLICATION")) {
                    omacpParserData.mType = "APPADDR";
                } else if (omacpParserData.mListType.equalsIgnoreCase("PXPHYSICAL")) {
                    omacpParserData.mType = "PXPHYSICAL";
                } else if (omacpParserData.mListType.equalsIgnoreCase("PXLOGICAL")) {
                    omacpParserData.mType = "PXLOGICAL";
                }
            } else if (omacpParserData.mType.equalsIgnoreCase("APPADDR") 
                    || omacpParserData.mType.equalsIgnoreCase("APPAUTH")
                    || omacpParserData.mType.equalsIgnoreCase("RESOURCE")) {
                omacpParserData.mType = "APPLICATION";
            } else if (omacpParserData.mType.equalsIgnoreCase("NAPAUTHINFO") 
                    || omacpParserData.mType.equalsIgnoreCase("VALIDITY")) {
                omacpParserData.mType = "NAPDEF";
            } else if (omacpParserData.mType.equalsIgnoreCase("PXAUTHINFO") 
                    || omacpParserData.mType.equalsIgnoreCase("PXPHYSICAL")) {
                omacpParserData.mType = "PXLOGICAL";
            }
        }
    }

    private int getEventType() {
        int eventType = -1;
        try {
            if (mParser instanceof XmlPullParser) {
                eventType = ((XmlPullParser) mParser).next();
            } else if (mParser instanceof WbxmlParser) {
                eventType = ((WbxmlParser) mParser).next();
            } else {
                MTKlog.e(TAG, "OmacpParserBase unknown parser type.");
            }
        } catch (XmlPullParserException e) {
            MTKlog.e(TAG, "OmacpParser get eventType Exception, e is : " + e);
        } catch (IOException e) {
            MTKlog.e(TAG, "OmacpParser get eventType Exception, e is : " + e);
        }
        return eventType;
    }

    private void getParserName(OmacpParserData omacpParserData) {
        if (mParser instanceof XmlPullParser) {
            omacpParserData.mName = ((XmlPullParser) mParser).getName();
        } else if (mParser instanceof WbxmlParser) {
            omacpParserData.mName = ((WbxmlParser) mParser).getName();
        } else {
            MTKlog.e(TAG, "OmacpParserBase unknown parser type.");
        }

        if (omacpParserData.mName == null) {
            MTKlog.e(TAG, "OmacpParserBase START_TAG name is null.");
            return;
        }
    }

    private void getCharacteristicParams(OmacpParserData omacpParserData) {
        // get parser's name and type
        getParserUriAndType(omacpParserData);
        // If the type is one of the following cases, then
        // start a new record
        if (omacpParserData.mType.equalsIgnoreCase("PXLOGICAL")) {
            omacpParserData.mProxyClass = new ProxyClass();
            mPxList.add(omacpParserData.mProxyClass);
            omacpParserData.mListType = omacpParserData.mType;
        } else if (omacpParserData.mType.equalsIgnoreCase("PXPHYSICAL")) {
            omacpParserData.mPxPhysical = new PxPhysical();
            if (omacpParserData.mProxyClass != null) {
                omacpParserData.mProxyClass.mPxphysical.add(omacpParserData.mPxPhysical);
            }
            omacpParserData.mListType = omacpParserData.mType;
        } else if (omacpParserData.mType.equalsIgnoreCase("PXAUTHINFO")) {
            omacpParserData.mPxAuthInfo = new PxAuthInfo();
            if (omacpParserData.mProxyClass != null) {
                omacpParserData.mProxyClass.mPxauthinfo.add(omacpParserData.mPxAuthInfo);
            }
        } else if (omacpParserData.mType.equalsIgnoreCase("NAPDEF")) {
            omacpParserData.mNapdefClass = new NapdefClass();
            mNapList.add(omacpParserData.mNapdefClass);
            omacpParserData.mListType = omacpParserData.mType;
        } else {
            getAppCharacteristicParams(omacpParserData);
        }
    }

    private void getAppCharacteristicParams(OmacpParserData omacpParserData) {
        if (omacpParserData.mType.equalsIgnoreCase("APPLICATION")) {
            omacpParserData.mApplication = new ApplicationClass();
            mApSectionList.add(omacpParserData.mApplication);
            omacpParserData.mListType = omacpParserData.mType;
        } else if (omacpParserData.mType.equalsIgnoreCase("APPADDR")) {
            omacpParserData.mAppAddr = new AppAddr();
            if (omacpParserData.mApplication != null) {
                omacpParserData.mApplication.mAppaddr.add(omacpParserData.mAppAddr);
            }
        } else if (omacpParserData.mType.equalsIgnoreCase("PORT")) {
            getPortParams(omacpParserData);
        } else if (omacpParserData.mType.equalsIgnoreCase("APPAUTH")) {
            omacpParserData.mAppAuth = new AppAuth();
            if (omacpParserData.mApplication != null) {
                omacpParserData.mApplication.mAppauth.add(omacpParserData.mAppAuth);
            }
        } else if (omacpParserData.mType.equalsIgnoreCase("RESOURCE")) {
            omacpParserData.mResource = new Resource();
            if (omacpParserData.mApplication != null) {
                omacpParserData.mApplication.mResource.add(omacpParserData.mResource);
            }
        } else if (omacpParserData.mType.equalsIgnoreCase("NAPAUTHINFO")) {
            omacpParserData.mNapAuthInfo = new NapAuthInfo();
            if (omacpParserData.mNapdefClass != null) {
                omacpParserData.mNapdefClass.mNapauthinfo.add(omacpParserData.mNapAuthInfo);
            }
        } else if (omacpParserData.mType.equalsIgnoreCase("VALIDITY")) {
            omacpParserData.mValidity = new Validity();
            if (omacpParserData.mNapdefClass != null) {
                omacpParserData.mNapdefClass.mValidity.add(omacpParserData.mValidity);
            }
        }
    }

    private void getParserUriAndType(OmacpParserData omacpParserData) {
        if (mParser instanceof XmlPullParser) {
            omacpParserData.mUri = ((XmlPullParser) mParser).getNamespace();
            omacpParserData.mType = ((XmlPullParser) mParser).getAttributeValue(omacpParserData.mUri, "type");
        } else if (mParser instanceof WbxmlParser) {
            omacpParserData.mUri = ((WbxmlParser) mParser).getNamespace();
            omacpParserData.mType = ((WbxmlParser) mParser).getAttributeValue(omacpParserData.mUri, "type");
        } else {
            MTKlog.e(TAG, "OmacpParserBase unknown parser type.");
        }

        if (omacpParserData.mType == null) {
            MTKlog.e(TAG, "OmacpParserBase START_TAG type is null.");
            throw new IllegalArgumentException();
        }
    }

    private void getPortParams(OmacpParserData omacpParserData) {
        omacpParserData.mPort = new Port();

        if (omacpParserData.mListType == null) {
            MTKlog.e(TAG, "OmacpParserBase START_TAG listType is null.");
            return;
        }

        if (omacpParserData.mListType.equalsIgnoreCase("APPLICATION")) {
            if (omacpParserData.mApplication != null) {
                int size = omacpParserData.mApplication.mAppaddr.size();
                if (size != 0) {
                    omacpParserData.mApplication.mAppaddr.get(size - 1).mPort.add(omacpParserData.mPort);
                } else {
                    MTKlog.e(TAG, "OmacpParserBase invalid APPADDR definition");
                }
            }
        } else if (omacpParserData.mListType.equalsIgnoreCase("PXLOGICAL")) {
            if (omacpParserData.mProxyClass != null) {
                omacpParserData.mProxyClass.mPort.add(omacpParserData.mPort);
            }
        } else if (omacpParserData.mListType.equalsIgnoreCase("PXPHYSICAL")) {
            if (omacpParserData.mProxyClass != null) {
                int size = omacpParserData.mProxyClass.mPxphysical.size();
                if (size != 0) {
                    omacpParserData.mProxyClass.mPxphysical.get(size - 1).mPort.add(omacpParserData.mPort);
                } else {
                    MTKlog.e(TAG,
                            "OmacpParserBase invalid PXPHYSICAL definition");
                }
            }
        }
    }

    private void getAttributeParams(OmacpParserData omacpParserData) {
        if (mParser instanceof XmlPullParser) {
            omacpParserData.mUri = ((XmlPullParser) mParser).getNamespace();
            omacpParserData.mParmName = ((XmlPullParser) mParser).getAttributeValue(omacpParserData.mUri, "name");
            omacpParserData.mParmValue = ((XmlPullParser) mParser).getAttributeValue(omacpParserData.mUri, "value");
        } else if (mParser instanceof WbxmlParser) {
            omacpParserData.mUri = ((WbxmlParser) mParser).getNamespace();
            omacpParserData.mParmName = ((WbxmlParser) mParser).getAttributeValue(omacpParserData.mUri, "name");
            omacpParserData.mParmValue = ((WbxmlParser) mParser).getAttributeValue(omacpParserData.mUri, "value");
        } else {
            MTKlog.e(TAG, "OmacpParserBase unknown parser type.");
        }

        if (omacpParserData.mListType != null) {
            if (omacpParserData.mListType.equalsIgnoreCase("APPLICATION")) {
                OmacpParserUtils.handleApParameters(omacpParserData.mType, omacpParserData.mParmName, 
                                                    omacpParserData.mParmValue, omacpParserData.mApplication);
            } else if (omacpParserData.mListType.equalsIgnoreCase("NAPDEF")) {
                OmacpParserUtils.handleNapParameters(omacpParserData.mType, omacpParserData.mParmName, 
                                                     omacpParserData.mParmValue, omacpParserData.mNapdefClass);
            } else if (omacpParserData.mListType.equalsIgnoreCase("PXLOGICAL")
                    || omacpParserData.mListType.equalsIgnoreCase("PXPHYSICAL")) {
                OmacpParserUtils.handlePxParameters(omacpParserData.mListType, omacpParserData.mType, 
                                                    omacpParserData.mParmName, omacpParserData.mParmValue,
                                                    omacpParserData.mProxyClass);
            }
        }

        if (omacpParserData.mType != null && omacpParserData.mType.equalsIgnoreCase("BOOTSTRAP")
                && omacpParserData.mParmName.equalsIgnoreCase("NAME")) {
            mContextName = omacpParserData.mParmValue;
        }
    }

    public ArrayList<ApplicationClass> getApSectionList() {
        return OmacpParserUtils.removeInvalidApSettings(mApSectionList);
    }

    public ArrayList<ProxyClass> getPxList() {
        return mPxList;
    }

    public ArrayList<NapdefClass> getNapList() {
        return mNapList;
    }

    public String getContextName() {
        return mContextName;
    }

}
