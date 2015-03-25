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

package com.mediatek.datatransfer.modules;

import java.io.IOException;
import java.io.StringWriter;
import org.xmlpull.v1.XmlSerializer;
import android.util.Xml;

import com.mediatek.datatransfer.modules.MmsXmlInfo.MmsXml;

public class MmsXmlComposer {
    private XmlSerializer mSerializer = null;
    private StringWriter mStringWriter = null;

    public boolean startCompose() {
        boolean result = false;
        mSerializer = Xml.newSerializer();
        mStringWriter = new StringWriter();
        try {
            mSerializer.setOutput(mStringWriter);
            // serializer.startDocument("UTF-8", null);
            mSerializer.startDocument(null, false);
            mSerializer.startTag("", MmsXml.ROOT);
            result = true;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return result;
    }

    public boolean endCompose() {
        boolean result = false;
        try {
            mSerializer.endTag("", MmsXml.ROOT);
            mSerializer.endDocument();
            result = true;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }

    public boolean addOneMmsRecord(MmsXmlInfo record) {
        boolean result = false;
        try {
            mSerializer.startTag("", MmsXml.RECORD);
            //mSerializer.attribute("", MmsXml.CATEGORY, record.getCategory());
            mSerializer.attribute("", MmsXml.ID, record.getID());
            mSerializer.attribute("", MmsXml.ISREAD, record.getIsRead());
            //mSerializer.attribute("", MmsXml.LOCALDATE, record.getLocalDate());
            //mSerializer.attribute("", MmsXml.ST, record.getST());
            mSerializer.attribute("", MmsXml.MSGBOX, record.getMsgBox());
            mSerializer.attribute("", MmsXml.DATE, record.getDate());
            mSerializer.attribute("", MmsXml.SIZE, record.getSize());
            mSerializer.attribute("", MmsXml.SIMID, record.getSimId());
            mSerializer.attribute("", MmsXml.ISLOCKED, record.getIsLocked());
            mSerializer.endTag("", MmsXml.RECORD);

            result = true;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }

    public String getXmlInfo() {
        if (mStringWriter != null) {
            return mStringWriter.toString();
        }

        return null;
    }
}
