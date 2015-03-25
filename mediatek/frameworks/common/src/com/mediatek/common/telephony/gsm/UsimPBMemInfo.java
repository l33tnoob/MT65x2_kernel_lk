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
 * PB Mem Ext
 */

package com.mediatek.common.telephony.gsm;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * {@hide}
 */
public class UsimPBMemInfo implements Parcelable {
    private int mSliceIndex;
    private int mAdnLength;
    private int mAdnUsed;
    private int mAdnTotal;
    private int mAdnType;
    private int mExt1Length;
    private int mExt1Used;
    private int mExt1Total;
    private int mExt1Type;
    private int mGasLength;
    private int mGasUsed;
    private int mGasTotal;
    private int mGasType;
    private int mAnrLength;
    private int mAnrUsed;
    private int mAnrTotal;
    private int mAnrType;
    private int mAasLength;
    private int mAasUsed;
    private int mAasTotal;
    private int mAasType;
    private int mSneLength;
    private int mSneUsed;
    private int mSneTotal;
    private int mSneType;
    private int mEmailLength;
    private int mEmailUsed;
    private int mEmailTotal;
    private int mEmailType;
    private int mCcpLength;
    private int mCcpUsed;
    private int mCcpTotal;
    private int mCcpType;

    public static final int INT_NOT_SET = -1;
    public static final String STRING_NOT_SET = "";

    public UsimPBMemInfo() {
        mSliceIndex = INT_NOT_SET;
        mAdnLength = INT_NOT_SET;
        mAdnUsed = INT_NOT_SET;
        mAdnTotal = INT_NOT_SET;
        mAdnType = INT_NOT_SET;
        mExt1Length = INT_NOT_SET;
        mExt1Used = INT_NOT_SET;
        mExt1Total = INT_NOT_SET;
        mExt1Type = INT_NOT_SET;
        mGasLength = INT_NOT_SET;
        mGasUsed = INT_NOT_SET;
        mGasTotal = INT_NOT_SET;
        mGasType = INT_NOT_SET;
        mAnrLength = INT_NOT_SET;
        mAnrUsed = INT_NOT_SET;
        mAnrTotal = INT_NOT_SET;
        mAnrType = INT_NOT_SET;
        mAasLength = INT_NOT_SET;
        mAasUsed = INT_NOT_SET;
        mAasTotal = INT_NOT_SET;
        mAasType = INT_NOT_SET;
        mSneLength = INT_NOT_SET;
        mSneUsed = INT_NOT_SET;
        mSneTotal = INT_NOT_SET;
        mSneType = INT_NOT_SET;
        mEmailLength = INT_NOT_SET;
        mEmailUsed = INT_NOT_SET;
        mEmailTotal = INT_NOT_SET;
        mEmailType = INT_NOT_SET;
        mCcpLength = INT_NOT_SET;
        mCcpUsed = INT_NOT_SET;
        mCcpTotal = INT_NOT_SET;
        mCcpType = INT_NOT_SET;
    }

    public static final Parcelable.Creator<UsimPBMemInfo> CREATOR = new Parcelable.Creator<UsimPBMemInfo>() {
        public UsimPBMemInfo createFromParcel(Parcel source) {
            return UsimPBMemInfo.createFromParcel(source);
        }

        public UsimPBMemInfo[] newArray(int size) {
            return new UsimPBMemInfo[size];
        }
    };

    public static UsimPBMemInfo createFromParcel(Parcel source) {
        UsimPBMemInfo p = new UsimPBMemInfo();
        p.mSliceIndex = source.readInt();
        p.mAdnLength = source.readInt();
        p.mAdnUsed = source.readInt();
        p.mAdnTotal = source.readInt();
        p.mAdnType = source.readInt();
        p.mExt1Length = source.readInt();
        p.mExt1Used = source.readInt();
        p.mExt1Total = source.readInt();
        p.mExt1Type = source.readInt();
        p.mGasLength = source.readInt();
        p.mGasUsed = source.readInt();
        p.mGasTotal = source.readInt();
        p.mGasType = source.readInt();
        p.mAnrLength = source.readInt();
        p.mAnrUsed = source.readInt();
        p.mAnrTotal = source.readInt();
        p.mAnrType = source.readInt();
        p.mAasLength = source.readInt();
        p.mAasUsed = source.readInt();
        p.mAasTotal = source.readInt();
        p.mAasType = source.readInt();
        p.mSneLength = source.readInt();
        p.mSneUsed = source.readInt();
        p.mSneTotal = source.readInt();
        p.mSneType = source.readInt();
        p.mEmailLength = source.readInt();
        p.mEmailUsed = source.readInt();
        p.mEmailTotal = source.readInt();
        p.mEmailType = source.readInt();
        p.mCcpLength = source.readInt();
        p.mCcpUsed = source.readInt();
        p.mCcpTotal = source.readInt();
        p.mCcpType = source.readInt();

        return p;
    }

    public void writeToParcel(Parcel dest) {
        dest.writeInt(mSliceIndex);

        dest.writeInt(mAdnLength);
        dest.writeInt(mAdnUsed);
        dest.writeInt(mAdnTotal);
        dest.writeInt(mAdnType);
        dest.writeInt(mExt1Length);
        dest.writeInt(mExt1Used);
        dest.writeInt(mExt1Total);
        dest.writeInt(mExt1Type);
        dest.writeInt(mGasLength);
        dest.writeInt(mGasUsed);
        dest.writeInt(mGasTotal);
        dest.writeInt(mGasType);
        dest.writeInt(mAnrLength);
        dest.writeInt(mAnrUsed);
        dest.writeInt(mAnrTotal);
        dest.writeInt(mAnrType);
        dest.writeInt(mAasLength);
        dest.writeInt(mAasUsed);
        dest.writeInt(mAasTotal);
        dest.writeInt(mAasType);
        dest.writeInt(mSneLength);
        dest.writeInt(mSneUsed);
        dest.writeInt(mSneTotal);
        dest.writeInt(mSneType);
        dest.writeInt(mEmailLength);
        dest.writeInt(mEmailUsed);
        dest.writeInt(mEmailTotal);
        dest.writeInt(mEmailType);
        dest.writeInt(mCcpLength);
        dest.writeInt(mCcpUsed);
        dest.writeInt(mCcpTotal);
        dest.writeInt(mCcpType);

    }

    public void writeToParcel(Parcel dest, int flags) {
        writeToParcel(dest);
    }

    public int describeContents() {
        return 0;
    }

    public String toString() {

        return super.toString()
                + " mSliceIndex: " + mSliceIndex
                + " mAdnLength: " + mAdnLength
                + " mAdnUsed: " + Integer.toString(mAdnUsed)
                + " mAdnTotal:" + Integer.toString(mAdnTotal)
                + " mAdnType:" + Integer.toString(mAdnType)
                + " mExt1Length:" + Integer.toString(mExt1Length)
                + " mExt1Used:" + Integer.toString(mExt1Used)
                + " mExt1Total" + Integer.toString(mExt1Total)
                + " mExt1Type" + Integer.toString(mExt1Type)
                + " mGasLength" + Integer.toString(mGasLength)
                + " mGasUsed" + Integer.toString(mGasUsed)
                + " mGasTotal: " + Integer.toString(mGasTotal)
                + " mGasType: " + Integer.toString(mGasType)
                + " mAnrLength: " + Integer.toString(mAnrLength)
                + " mAnrUsed: " + Integer.toString(mAnrUsed)
                + " mAnrTotal: " + Integer.toString(mAnrTotal)
                + " mAnrType: " + Integer.toString(mAnrType)
                + " mEmailLength: " + Integer.toString(mEmailLength)
                + " mEmailUsed: " + Integer.toString(mEmailUsed)
                + " mEmailTotal: " + Integer.toString(mEmailTotal)
                + " mEmailType: " + Integer.toString(mEmailType);
    }

    public int getSliceIndex() {
        return mSliceIndex;
    }

    public int getAdnLength() {
        return mAdnLength;
    }

    public int getAdnUsed() {
        return mAdnUsed;
    }

    public int getAdnTotal() {
        return mAdnTotal;
    }

    public int getAdnType() {
        return mAdnType;
    }

    public int getAdnFree() {
        return (mAdnTotal - mAdnUsed);
    }

    public int getExt1Length() {
        return mExt1Length;
    }

    public int getExt1Used() {
        return mExt1Used;
    }

    public int getExt1Total() {
        return mExt1Total;
    }

    public int getExt1Type() {
        return mExt1Type;
    }

    public int getExt1Free() {
        return (mExt1Total - mExt1Used);
    }

    public int getGasLength() {
        return mGasLength;
    }

    public int getGasUsed() {
        return mGasUsed;
    }

    public int getGasTotal() {
        return mGasTotal;
    }

    public int getGasType() {
        return mGasType;
    }

    public int getAnrLength() {
        return mAnrLength;
    }

    public int getAnrUsed() {
        return mAnrUsed;
    }

    public int getAnrTotal() {
        return mAnrTotal;
    }

    public int getAnrType() {
        return mAnrType;
    }

    public int getAnrFree() {
        return (mAnrTotal - mAnrUsed);
    }

    public int getAasLength() {
        return mAasLength;
    }

    public int getAasUsed() {
        return mAasUsed;
    }

    public int getAasTotal() {
        return mAasTotal;
    }

    public int getAasType() {
        return mAasType;
    }

    public int getSneLength() {
        return mSneLength;
    }

    public int getSneUsed() {
        return mSneUsed;
    }

    public int getSneTotal() {
        return mSneTotal;
    }

    public int getSneType() {
        return mSneType;
    }

    public int getEmailLength() {
        return mEmailLength;
    }

    public int getEmailUsed() {
        return mEmailUsed;
    }

    public int getEmailTotal() {
        return mEmailTotal;
    }

    public int getEmailType() {
        return mEmailType;
    }

    public int getEmailFree() {
        return (mEmailTotal - mEmailUsed);
    }

    public int getCcpLength() {
        return mCcpLength;
    }

    public int getCcpUsed() {
        return mCcpUsed;
    }

    public int getCcpTotal() {
        return mCcpTotal;
    }

    public int getCcpType() {
        return mCcpType;
    }

    public int getCcpFree() {
        return (mCcpTotal - mCcpUsed);
    }

    public int getGasFree() {
        return (mGasTotal - mGasUsed);
    }

    public int getAasFree() {
        return (mAasTotal - mAasUsed);
    }

    public int getSneFree() {
        return (mSneTotal - mSneUsed);
    }

    public void setSliceIndex(int value) {
        mSliceIndex = value;
    }

    public void setAdnLength(int value) {
        mAdnLength = value;
    }

    public void setAdnUsed(int value) {
        mAdnUsed = value;
    }

    public void setAdnTotal(int value) {
        mAdnTotal = value;
    }

    public void setAdnType(int value) {
        mAdnType = value;
    }

    public void setExt1Length(int value) {
        mExt1Length = value;
    }

    public void setExt1Used(int value) {
        mExt1Used = value;
    }

    public void setExt1Total(int value) {
        mExt1Total = value;
    }

    public void setExt1Type(int value) {
        mExt1Type = value;
    }

    public void setGasLength(int value) {
        mGasLength = value;
    }

    public void setGasUsed(int value) {
        mGasUsed = value;
    }

    public void setGasTotal(int value) {
        mGasTotal = value;
    }

    public void setGasType(int value) {
        mGasType = value;
    }

    public void setAnrLength(int value) {
        mAnrLength = value;
    }

    public void setAnrUsed(int value) {
        mAnrUsed = value;
    }

    public void setAnrTotal(int value) {
        mAnrTotal = value;
    }

    public void setAnrType(int value) {
        mAnrType = value;
    }

    public void setAasLength(int value) {
        mAasLength = value;
    }

    public void setAasUsed(int value) {
        mAasUsed = value;
    }

    public void setAasTotal(int value) {
        mAasTotal = value;
    }

    public void setAasType(int value) {
        mAasType = value;
    }

    public void setSneLength(int value) {
        mSneLength = value;
    }

    public void setSneUsed(int value) {
        mSneUsed = value;
    }

    public void setSneTotal(int value) {
        mSneTotal = value;
    }

    public void setSneType(int value) {
        mSneType = value;
    }

    public void setEmailLength(int value) {
        mEmailLength = value;
    }

    public void setEmailUsed(int value) {
        mEmailUsed = value;
    }

    public void setEmailTotal(int value) {
        mEmailTotal = value;
    }

    public void setEmailType(int value) {
        mEmailType = value;
    }

    public void setCcpLength(int value) {
        mCcpLength = value;
    }

    public void setCcpUsed(int value) {
        mCcpUsed = value;
    }

    public void setCcpTotal(int value) {
        mCcpTotal = value;
    }

    public void setCcpType(int value) {
        mCcpType = value;
    }

}
