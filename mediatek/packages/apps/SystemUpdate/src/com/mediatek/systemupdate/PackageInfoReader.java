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

package com.mediatek.systemupdate;

import android.content.Context;
import android.util.Log;

import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

/**
 * Get update PackageInfo from update.zip or xml files restored by PackageInfoWriter
 * 
 * @author mtk80800
 * 
 */
class PackageInfoReader {
    private static final String TAG = "SystemUpdate/InfoReader";
    private XmlParser mConfigureParser;

    public PackageInfoReader(Context context, String file) {
        File configFileInSystem = new File(file);
        if (configFileInSystem.exists()) {
            Log.d(TAG, "[PackageInfoReader]configure.xml exists in system");
            try {
                mConfigureParser = new XmlParser(file);
            } catch (SAXException e) {
                Xlog.e(TAG, "[PackageInfoReader] SAXException");
            } catch (IOException e) {
                Xlog.e(TAG, "[PackageInfoReader] IOException");
            }
        }

    }

    public String getOem() {
        String oem = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser oem from " + Util.InfoXmlTags.XML_TAG_OEM);
            oem = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_OEM);
            Log.d(TAG, "oem = " + oem);
        }
        return oem;
    }

    public String getProduct() {
        String product = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser Product from " + Util.InfoXmlTags.XML_TAG_PRODUCT);
            product = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_PRODUCT);
            Log.d(TAG, "Product = " + product);
        }
        return product;
    }

    public String getFlavor() {
        String flavor = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser flavor from " + Util.InfoXmlTags.XML_TAG_FLAVOR);
            flavor = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_FLAVOR);
            Log.d(TAG, "flavor = " + flavor);
        }
        return flavor;
    }

    public String getLanguage() {
        String language = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser language from " + Util.InfoXmlTags.XML_TAG_LANGUAGE);
            language = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_LANGUAGE);
            Log.d(TAG, "language = " + language);
        }
        return language;
    }

    public String getOperator() {
        String operator = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser operator from " + Util.InfoXmlTags.XML_TAG_OPERATOR);
            operator = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_OPERATOR);
            Log.d(TAG, "operator = " + operator);
        }
        return operator;
    }

    public String getPublishTime() {
        String publishTime = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser Publish Time from " + Util.InfoXmlTags.XML_TAG_PUBLISH_TIME);
            publishTime = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_PUBLISH_TIME);
            Log.d(TAG, "Publish Time = " + publishTime);
        }
        return publishTime;
    }

    public String getAndroidNum() {
        String androidNum = "Android 4.2";
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser getAndroidNum from " + Util.InfoXmlTags.XML_TAG_ANDROID_NUM);
            androidNum = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_ANDROID_NUM);
            Log.d(TAG, "getAndroidNum = " + androidNum);
        }
        return androidNum;
    }

    public String getVersionName() {
        String versionName = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser getVersionName from " + Util.InfoXmlTags.XML_TAG_VERSION_NAME);
            versionName = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_VERSION_NAME);
            Log.d(TAG, "getVersionName = " + versionName);
        }
        return versionName;
    }

    public String getNotes() {
        String notes = null;
        if (mConfigureParser != null) {
            Log.d(TAG, "Parser Notes from " + Util.InfoXmlTags.XML_TAG_NOTES);
            notes = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_NOTES);
            Log.d(TAG, "Notes = " + notes);
        }
        return notes;
    }

//    public String getFingerprint() {
//        String fingerprint = null;
//        if (mConfigureParser != null) {
//            Log.d(TAG, "Parser fingerprint from " + Util.InfoXmlTags.XML_TAG_FINGERPRINT);
//            fingerprint = mConfigureParser.getValByTagName(Util.InfoXmlTags.XML_TAG_FINGERPRINT);
//            Log.d(TAG, "fingerprint = " + fingerprint);
//        }
//        return fingerprint;
//    }

    public UpdatePackageInfo getInfo(int id) {
        Xlog.d(TAG, "[getInfo] begin " + id);

        if (mConfigureParser == null) {
            return null;
        }

        List<Node> nodeList = new ArrayList<Node>();
        mConfigureParser.getChildNodeAtLevel(nodeList, 2);
        if (id >= nodeList.size() || id < 0) {
            Xlog.e(TAG, "[getInfo], order >= nodeList.size() || id < 0, size is = " + nodeList.size());
            return null;
        }

        Node node = nodeList.get(id);

        return getInfoFromNode(node);
    }

    public List<UpdatePackageInfo> getInfoList() {
        Xlog.d(TAG, "[getUpdateInfoList] begin");
        List<UpdatePackageInfo> infoList = new ArrayList<UpdatePackageInfo>();

        if (mConfigureParser != null) {
            List<Node> nodeList = new ArrayList<Node>();
            mConfigureParser.getChildNodeAtLevel(nodeList, 2);

            Xlog.d(TAG, "[getInfoList]node list's length: " + nodeList.size());

            if (!nodeList.isEmpty()) {
                for (Node node : nodeList) {
                    UpdatePackageInfo info = getInfoFromNode(node);
                    if (info != null) {
                        infoList.add(info);
                    }
                }
            }

        }

        return infoList;
    }

    private UpdatePackageInfo getInfoFromNode(Node node) {
        if (node == null) {
            Xlog.w(TAG, "[getInfoFromNode]node is null");
            return null;
        }

        Xlog.v(TAG, "[getInfoFromNode]node name: " + node.getNodeName());
        NodeList childNodeList = node.getChildNodes();/* new ArrayList<Node>(); */
        int length = childNodeList.getLength();
        Xlog.v(TAG, "[getInfoFromNode]childNodeList list's length: " + length);

        if (length != UpdatePackageInfo.FIELD_NUMBER) {
            Xlog.w(TAG, "[getInfoFromNode], child node number is not currect");
            return null;
        }

        UpdatePackageInfo info = new UpdatePackageInfo();
        for (int i = 0; i < length; ++i) {

            Node child = childNodeList.item(i);
            String nodeName = child.getNodeName();

            if (Util.InfoXmlTags.XML_TAG_PUBLISH_TIME.equals(nodeName)) {
                info.publishTime = child.getTextContent();
                Xlog.v(TAG, "[getInfoFromNode],Item (" + i + "): [getInfoList]publish time:"
                        + info.publishTime);
                // if (!TextUtils.isEmpty(timeString)) {
                // info.mPublishTime = Long.parseLong(timeString);
                // }
            } else if (Util.InfoXmlTags.XML_TAG_ANDROID_NUM.equals(nodeName)) {
                info.androidNumber = child.getTextContent();
                Xlog.v(TAG, "[getInfoFromNode],Item (" + i + "): [getInfoList]mAndroidNum:"
                        + info.androidNumber);
            } else if (Util.InfoXmlTags.XML_TAG_VERSION_NAME.equals(nodeName)) {
                info.version = child.getTextContent();
                Xlog.v(TAG, "[getInfoFromNode],Item (" + i + "): [getInfoList]version:"
                        + info.version);
            } else if (Util.InfoXmlTags.XML_TAG_NOTES.equals(nodeName)) {
                info.notes = child.getTextContent();
                Xlog.v(TAG, "[getInfoFromNode],Item (" + i + "): [getInfoList] have notes");
            } else if (Util.InfoXmlTags.XML_TAG_PATH.equals(nodeName)) {
                info.path = child.getTextContent();
                Xlog.v(TAG, "[getInfoFromNode],Item (" + i + "): [getInfoList]mPath:" + info.path);
            } else {
                Xlog.w(TAG, "[getInfoFromNode], child node TAG is not currect");
                return null;
            }
        }

        return info;
    }

}
