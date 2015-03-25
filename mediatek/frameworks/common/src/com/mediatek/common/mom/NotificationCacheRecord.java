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

public class NotificationCacheRecord implements Parcelable {
    private static final String TAG = "NotificationCacheRecord";
    public String packageName = null;
    public boolean enable = false;
    
    public NotificationCacheRecord(String packageName, boolean enable) {
        this.packageName = packageName;
        this.enable = enable;
    }

    private NotificationCacheRecord(Parcel in) {
        this.packageName = in.readString();
        this.enable = in.readInt() == 1;
    }
    
    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int parcelableFlags) {
        out.writeString(this.packageName);
        out.writeInt(this.enable ? 1 : 0);
    }

    public String toString() {
        return "NotificationCacheRecord {"
            + this.packageName + ", " + this.enable + "}";
    }

    public boolean equals(Object o) {
        // Return true if the objects are identical.
        if (this == o) {
            return true;
        }
        // Return false if the other object has the wrong type.
        if (!(o instanceof NotificationCacheRecord)) {
            return false;
        }
        // Cast to the appropriate type.
        NotificationCacheRecord record = (NotificationCacheRecord) o;
        // Check each field.
        return this.packageName.equals(record.packageName) &&
                (this.enable == record.enable);
    }

    public static final Parcelable.Creator<NotificationCacheRecord> CREATOR
            = new Parcelable.Creator<NotificationCacheRecord>() {
        public NotificationCacheRecord createFromParcel(Parcel in) {
            return new NotificationCacheRecord(in);
        }

        public NotificationCacheRecord[] newArray(int size) {
            return new NotificationCacheRecord[size];
        }
    };
}

