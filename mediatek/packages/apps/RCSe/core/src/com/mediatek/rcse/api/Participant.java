/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.api;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * This parcelable class describes an IM chat participant.
 */
public class Participant implements Parcelable {

    /**
     * The key of a bundle data in the intent calling the ChatActivity.
     */
    public static final String KEY_PARTICIPANT = "participant";
    /**
     * The key of a bundle data in the intent calling the ChatActivity.
     */
    public static final String KEY_PARTICIPANT_LIST = "participantList";

    /**
     * Constructor of Participant.
     * 
     * @param contact Typically a TEL or SIP URI.
     * @param displayName The name of the contact displayed in the ChatActvity.
     */
    public Participant(String contact, String displayName) {
        mContact = contact;
        mDisplayName = displayName;
    }

    protected Participant(Parcel source) {
        mContact = source.readString();
        mDisplayName = source.readString();
    }

    /**
     * Get the contact value of this participant.
     * 
     * @return The contact value, typically a TEL/SIP URI.
     */
    public String getContact() {
        return mContact;
    }

    /**
     * Get the display name of this participant.
     * 
     * @return The display name of this participant.
     */
    public String getDisplayName() {
        return mDisplayName;
    }

    String mContact = null;

    String mDisplayName = null;

    /**
     * Describe the kinds of special objects contained in this Parcelable's
     * marshalled representation
     * 
     * @return Integer
     */
    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * Write parcelable object
     * 
     * @param dest The Parcel in which the object should be written
     * @param flags Additional flags about how the object should be written
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mContact);
        dest.writeString(mDisplayName);
    }

    /**
     * Parcelable creator
     */
    public static final Parcelable.Creator<Participant> CREATOR =
            new Parcelable.Creator<Participant>() {
                public Participant createFromParcel(Parcel source) {
                    return new Participant(source);
                }

                public Participant[] newArray(int size) {
                    return new Participant[size];
                }
            };

    @Override
    public boolean equals(Object o) {
        if (o instanceof Participant) {
            if (null == this.mContact) {
                return null == ((Participant) o).mContact;
            } else {
                return this.mContact.equals(((Participant) o).mContact);
            }
        } else {
            return false;
        }
    }

    @Override
    public int hashCode() {
        if (null != mContact) {
            return mContact.hashCode();
        } else {
            return 0;
        }
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("Contant: ");
        builder.append(mContact);
        builder.append("    DisplayName: ");
        builder.append(mDisplayName);
        return builder.toString();
    }
}
