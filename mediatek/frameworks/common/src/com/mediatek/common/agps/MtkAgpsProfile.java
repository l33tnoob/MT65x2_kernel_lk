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

package com.mediatek.common.agps;

import android.os.Parcel;
import android.os.Parcelable;

public final class MtkAgpsProfile implements Parcelable {
    public String name;     // 1. AGPS profile identifier name
    public String addr;     // 2. SLP server address, could be IP or FQDN
    public int port;        // 3.
    public int tls;         // 4.
    public String code;     // 5.
    public String backupSlpNameVar;// 6. string resource name of this SLP (mainly for MUI) 
    public int showType;    // 7. whether to show this profile in current mode(user/engineer)
    public String addrType; // 8.
    public String providerId;   // 9.
    public String defaultApn;   // 10.
    public String optionApn;    // 11.
    public String optionApn2;   // 12.
    public String appId;       // 13.
    public String mccMnc;       // 14.

    public String toString() {
        String str = new String();
        str = " MtkAgpsProfile name=[" + name + "] addr=[" + addr + "] port=[" + port + 
            "] tls=[" + tls + "] backupSlpNameVar=[" + backupSlpNameVar + 
            "] showType=[" + showType + "] code=[" + code + "] addrType=[" + addrType + 
            "] providerId=[" + providerId + "] defaultApn=[" + defaultApn + "] optionApn=[" + optionApn + 
            "] optionApn2=[" + optionApn2 + "] appId=[" + appId + "] mccMnc=[" + mccMnc + "]";
        return str;
    }

    public static final Parcelable.Creator<MtkAgpsProfile> CREATOR = new Parcelable.Creator<MtkAgpsProfile>() {
        public MtkAgpsProfile createFromParcel(Parcel in) {
            MtkAgpsProfile profile = new MtkAgpsProfile();
            profile.readFromParcel(in);
            return profile;
        }
        public MtkAgpsProfile[] newArray(int size) {
            return new MtkAgpsProfile[size];
        }
    };

    public MtkAgpsProfile() {}
    public MtkAgpsProfile(String name, String addr, int port, int tls, String code, String backup,
            int showType, String addrType, String providerId, String defaultApn, String optionApn,
            String optionApn2, String appId, String mccMnc) {
        this.name = name;
        this.addr = addr;
        this.port = port;
        this.tls  = tls;
        this.code = code;
        this.backupSlpNameVar = backup;
        this.showType = showType;
        this.addrType = addrType;
        this.providerId = providerId;
        this.defaultApn = defaultApn;
        this.optionApn = optionApn;
        this.optionApn2 = optionApn2;
        this.appId = appId;
        this.mccMnc = mccMnc;
    }

    //@Override
    public int describeContents() {
        return 0;
    }

    //@Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(name);
        out.writeString(addr);
        out.writeInt(port);
        out.writeInt(tls);
        out.writeString(code);
        out.writeString(backupSlpNameVar);
        out.writeInt(showType);
        out.writeString(addrType);
        out.writeString(providerId);
        out.writeString(defaultApn);
        out.writeString(optionApn);
        out.writeString(optionApn2);
        out.writeString(appId);
        out.writeString(mccMnc);
    }

    //@Override
    public void readFromParcel(Parcel in) {
        name = in.readString();
        addr = in.readString();
        port = in.readInt();
        tls  = in.readInt();
        code = in.readString();
        backupSlpNameVar = in.readString();
        showType = in.readInt();
        addrType = in.readString();
        providerId = in.readString();
        defaultApn = in.readString();
        optionApn = in.readString();
        optionApn2 = in.readString();
        appId = in.readString();
        mccMnc = in.readString();
    }
}
