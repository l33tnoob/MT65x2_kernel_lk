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

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 
/*
 * Phonebook Entry
 */

package com.mediatek.common.telephony.gsm;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * {@hide}
 */
public class PBEntry implements Parcelable {
    private int mIndex1;
    private String mNumber;
    private int mType;
    private String mText;
    private int mHidden;
    private String mGroup;
    private String mAdnumber;
    private int mAdtype;
    private String mSecondtext;
    private String mEmail;

    public static final int INT_NOT_SET = -1;
    public static final String STRING_NOT_SET = "";

    public PBEntry() {
        mIndex1 = INT_NOT_SET;
        mNumber = STRING_NOT_SET;
        mType = INT_NOT_SET;
        mText = STRING_NOT_SET;
        // only support 0
        mHidden = 0;
        mGroup = STRING_NOT_SET;
        mAdnumber = STRING_NOT_SET;
        mAdtype = INT_NOT_SET;
        mSecondtext = STRING_NOT_SET;
        mEmail = STRING_NOT_SET;
    }

    public static final Parcelable.Creator<PBEntry> CREATOR = new Parcelable.Creator<PBEntry>() {
        public PBEntry createFromParcel(Parcel source) {
            return PBEntry.reateFromParcel(source);
        }

        public PBEntry[] newArray(int size) {
            return new PBEntry[size];
        }
    };

    public static PBEntry reateFromParcel(Parcel source) {
        PBEntry p = new PBEntry();
        p.mIndex1 = source.readInt();
        p.mNumber = source.readString();
        p.mType = source.readInt();
        p.mText = source.readString();
        p.mHidden = source.readInt();
        p.mGroup = source.readString();
        p.mAdnumber = source.readString();
        p.mAdtype = source.readInt();
        p.mSecondtext = source.readString();
        p.mEmail = source.readString();
        return p;
    }

    public void writeToParcel(Parcel dest) {
        dest.writeInt(mIndex1);
        dest.writeString(mNumber);
        dest.writeInt(mType);
        dest.writeString(mText);
        dest.writeInt(mHidden);
        dest.writeString(mGroup);
        dest.writeString(mAdnumber);
        dest.writeInt(mAdtype);
        dest.writeString(mSecondtext);
        dest.writeString(mEmail);
    }

    public void writeToParcel(Parcel dest, int flags) {
        writeToParcel(dest);
    }

    public int describeContents() {
        return 0;
    }

    @Override
    public String toString() {
        return super.toString() + ", index1: " + mIndex1
                + ", number: " + mNumber
                + ", type:" + mType
                + ", text:" + mText
                + ", hidden:" + mHidden
                + ", group:" + mGroup
                + ", adnumber:" + mAdnumber
                + ", adtype:" + mAdtype
                + ", secondtext:" + mSecondtext
                + ", email:" + mEmail;
    }

    public void setIndex1(int iIndex1) {
        mIndex1 = iIndex1;
    }

    public void setNumber(String sNumber) {
        mNumber = sNumber;
    }

    public void setType(int iType) {
        mType = iType;
    }

    public void setText(String sText) {
        if (sText != null) {
            mText = sText;
        }
    }

    public void setHidden(int iHidden) {
        mHidden = iHidden;
    }

    public void setGroup(String sGroup) {
        mGroup = sGroup;
    }

    public void setAdnumber(String sAdnumber) {
        if (sAdnumber != null) {
            mAdnumber = sAdnumber;
        }
    }

    public void setAdtype(int iAdtype) {
        mAdtype = iAdtype;
    }

    public void setSecondtext(String sSecondtext) {
        mSecondtext = sSecondtext;
    }

    public void setEmail(String sEmail) {
        if (sEmail != null) {
            mEmail = sEmail;
        }
    }

    public int getIndex1() {
        return mIndex1;
    }

    public String getNumber() {
        return mNumber;
    }

    public int getType() {
        return mType;
    }

    public String getText() {
        return mText;
    }

    public int getHidden() {
        return mHidden;
    }

    public String getGroup() {
        return mGroup;
    }

    public String getAdnumber() {
        return mAdnumber;
    }

    public int getAdtype() {
        return mAdtype;
    }

    public String getSecondtext() {
        return mSecondtext;
    }

    public String getEmail() {
        return mEmail;
    }
}
