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


package com.mediatek.common.mom;

import android.os.Parcel;
import android.os.Parcelable;

public class ReceiverRecord implements Parcelable {
    public final String packageName;
    public boolean enabled = true;

    public ReceiverRecord(String _packageName) {
        packageName = _packageName;
        enabled = true;
    }

    public ReceiverRecord(String _packageName, boolean _enable) {
        packageName = _packageName;
        enabled = _enable;
    }

    public ReceiverRecord(ReceiverRecord data) {
        packageName = data.packageName;
        enabled = data.enabled;
    }

    private ReceiverRecord(Parcel in) {
        this.packageName = in.readString();
        this.enabled = in.readInt() == 1;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int parcelableFlags) {
        out.writeString(this.packageName);
        out.writeInt(this.enabled ? 1 : 0);
    }

    public String toString() {
        return "ReceiverRecord {"
                + this.packageName + ", " + this.enabled + "}";
    }

    public boolean equals(Object o) {
        // Return true if the objects are identical.
        if (this == o) {
            return true;
        }
        // Return false if the other object has the wrong type.
        if (!(o instanceof ReceiverRecord)) {
            return false;
        }
        // Cast to the appropriate type.
        ReceiverRecord record = (ReceiverRecord) o;
        // Check each field.
        return this.packageName.equals(record.packageName) &&
                (this.enabled == record.enabled);
    }

    public static final Parcelable.Creator<ReceiverRecord> CREATOR
            = new Parcelable.Creator<ReceiverRecord>() {
        public ReceiverRecord createFromParcel(Parcel in) {
            return new ReceiverRecord(in);
        }

        public ReceiverRecord[] newArray(int size) {
            return new ReceiverRecord[size];
        }
    };
}

