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

package com.mediatek.xlog;

public class BLogPacker {

    private long binBuf;

    private int index;

    public BLogPacker() {
	binBuf = initBinaryBuffer();
	index = 0;
    }

    public BLogPacker append(boolean b) {
	index = writeBoolean(binBuf, index, b);
	return this;
    }

    public BLogPacker append(char cValue) {
	index = writeChar(binBuf, index, cValue);
	return this;
    }

    public BLogPacker append(double dValue) {
	index = writeDouble(binBuf, index, dValue);
	return this;
    }

    public BLogPacker append(float fValue) {
	index = writeDouble(binBuf, index, fValue);
	return this;
    }

    public BLogPacker append(int intValue) {
	index = writeInt(binBuf, index, intValue);
	return this;
    }

    public BLogPacker append(long lng) {
	index = writeLong(binBuf, index, lng);
	return this;
    }

    public BLogPacker append(String s) {
	index = writeString(binBuf, index, s);
	return this;
    }

    public BLogPacker append(Object o) {
	if (o != null) {
	    index = writeString(binBuf, index, o.toString());
	}
	else {
	    index = writeString(binBuf, index, "null");
	}
	return this;
    }

    protected native void finalize();

    private static native long initBinaryBuffer();

    private static native int writeBoolean(long binBuf, int index, boolean value);

    private static native int writeChar(long binBuf, int index, char value);

    private static native int writeInt(long binBuf, int index, int value);

    private static native int writeLong(long binBuf, int index, long value);

    private static native int writeString(long binBuf, int ndex, String value);

    private static native int writeDouble(long binBuf, int index, double value);
}
