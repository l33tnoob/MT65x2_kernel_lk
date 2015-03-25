/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.systemupdate;

import android.util.Log;
import android.util.Xml;

import org.xmlpull.v1.XmlSerializer;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class XmlWriter {

    private static final String ENCODING_UTF8 = "UTF-8";
    private static final String TAG = "SystemUpdate/XmlWriter";
    private XmlSerializer mSerializer;
    private FileOutputStream mFileos;
    private String mFileName;

    // private String TAG = "DmXMLWriter";

    /**
     * Constructor of DmXMLWriter. Initiate xml writer.
     * 
     * @param String
     *            path - the full path of file
     */
    public XmlWriter(String path) {
        mFileName = path;
        File file = new File(path);

        try {
            mFileos = new FileOutputStream(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            Log.e(TAG, "can't create FileOutputStream");
        }

        mSerializer = Xml.newSerializer();
        try {
            mSerializer.setOutput(mFileos, ENCODING_UTF8);
            mSerializer.startDocument(null, true);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Write back to the file and release the resources.
     */
    public void close() {
        Log.d(TAG, "[close]XML writer close");
        try {
            if (mSerializer != null) {
                mSerializer.endDocument();
                mSerializer.flush();
            }
            if (mFileos != null) {
                mFileos.getFD().sync();
                Log.i(TAG, "[close] mFileos.getFD().sync()");
                mFileos.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "[close]error occurred while close xml file");
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        }
    }

    /**
     * Write start tag
     * 
     * @param String
     *            tag - the start tag name
     */
    public void writeStartTag(String tag) throws IOException {
        if (tag == null || mSerializer == null) {
            Log.e(TAG, "could not write null start tag");
            return;
        }
        mSerializer.startTag(null, tag);
    }

    /**
     * Write end tag
     * 
     * @param String
     *            tag - the end tag name
     */
    public void writeEndTag(String tag) throws IOException {
        if (tag == null || mSerializer == null) {
            Log.e(TAG, "could not write null end tag");
            return;
        }
        mSerializer.endTag(null, tag);
    }

    /**
     * Add value in xml file
     * 
     * @param String
     *            tag - the tag name
     * @param String
     *            value - the value of the tag
     */
    public void addValue(String tag, String value) throws IOException {
        if ((tag == null) || (value == null) || mSerializer == null) {
            Log.e(TAG, "[addValue] could not write null tag or value");
            return;
        }
        Log.w(TAG, "[addValue] TAG: " + tag + ", value: " + value);
        try {
            mSerializer.startTag(null, tag);
            mSerializer.text(value);
            mSerializer.endTag(null, tag);
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
    }
}