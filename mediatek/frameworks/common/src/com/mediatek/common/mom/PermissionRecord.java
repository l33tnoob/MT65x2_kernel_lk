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

import com.mediatek.common.mom.IMobileManager;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public class PermissionRecord implements Parcelable {
    private static final String TAG = "PermissionRecord";
    public String mPackageName = null;
    public String mPermissionName = null;
    private int mStatus = IMobileManager.PERMISSION_STATUS_GRANTED;
    
    public PermissionRecord(String packageName, String permissionName, int status) {
        mPackageName = packageName;
        mPermissionName = permissionName;
        mStatus = status;
    }

    private PermissionRecord(Parcel in) {
        mPackageName = in.readString();
        mPermissionName = in.readString();
        mStatus = in.readInt();
    }
    
    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int parcelableFlags) {
        out.writeString(mPackageName);
        out.writeString(mPermissionName);
        out.writeInt(mStatus);
    }

    public String toString() {
        return "PermissionRecord {"
            + mPackageName + ", " + mPermissionName + ", " + mStatus + "}";
    }

    public boolean equals(Object o) {
        // Return true if the objects are identical.
        if (this == o) {
            return true;
        }
        // Return false if the other object has the wrong type.
        if (!(o instanceof PermissionRecord)) {
            return false;
        }
        // Cast to the appropriate type.
        PermissionRecord record = (PermissionRecord) o;
        // Check each field.
        return mPackageName.equals(record.mPackageName) &&
             mPermissionName.equals(record.mPermissionName) &&
             (mStatus == record.mStatus);
    }

    public static final Parcelable.Creator<PermissionRecord> CREATOR
            = new Parcelable.Creator<PermissionRecord>() {
        public PermissionRecord createFromParcel(Parcel in) {
            return new PermissionRecord(in);
        }

        public PermissionRecord[] newArray(int size) {
            return new PermissionRecord[size];
        }
    };

    public void setStatus(int status) {
        if(status == IMobileManager.PERMISSION_STATUS_GRANTED ||
            status == IMobileManager.PERMISSION_STATUS_DENIED ||
            status == IMobileManager.PERMISSION_STATUS_CHECK) {
            mStatus = status;
        } else {
            Log.e(TAG, "Invalid permission status: " + status);
        }
    }

    public int getStatus() {
        return mStatus;
    }
}

