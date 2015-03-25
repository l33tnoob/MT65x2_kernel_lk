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

package com.android.ex.chips;

import android.text.util.Rfc822Token;
import android.text.util.Rfc822Tokenizer;

/**
 * M: Represents one contact with basic information.
 */
public class MTKRecipient {

    /* ID for the person */
    private long mContactId;
    /* ID for the destination */
    private long mDataId;
    /* Display name for the person */
    private String mDisplayName;
    /* Destination for this contact. Would be an email address or a phone number. Don't need to contain '<' and '>'. */
    private String mDestination;

    public MTKRecipient() {
        mContactId = -3;
        mDataId = -3;
        mDisplayName = "";
        mDestination = "";
    }

    public MTKRecipient(String displayName, String destination) {
        this(-1, -1, displayName, destination);
    }

    public MTKRecipient(long contactId, long dataId, String displayName, String destination) {
        mContactId = contactId;
        mDataId = dataId;
        mDisplayName = displayName;
        mDestination = destination;
    }

    public long getContactId() {
        return mContactId;
    }

    public long getDataId() {
        return mDataId;
    }

    public String getDisplayName() {
        return mDisplayName;
    }

    public String getDestination() {
        if (mContactId == RecipientEntry.GENERATED_CONTACT) {
            return tokenizeDestination(mDestination);
        }
        return mDestination;
    }

    public String getFormatString() {
        String str = "";
        if ((mDestination != null) && (!textIsAllBlank(mDestination))) {
            if ((mDisplayName != null) && !textIsAllBlank(mDisplayName)) {
                /// M: Pre-process display name. @{
                String displayName = mDisplayName;
                displayName = displayName.replaceAll("\\\\", "\\\\\\\\"); /// M: replace all \ with \\
                displayName = displayName.replaceAll("\"", "\\\\\"");     /// M: replace all " with \"
                displayName = displayName.replaceAll("\n", " ");     /// M: replace all \n with " "
                if (displayName.matches(".*[\\(\\)<>@,;:\\\\\".\\[\\]].*")) {
                    if (!displayName.matches("^\".*\"$")) {
                        displayName = "\"" + displayName + "\"";  /// M: Add "" when needed
                    }
                }
                /// @}
                str = displayName + " <" + mDestination + ">, ";
            } else {
                str = mDestination + ", ";
            }
        }
        return str;
    }

    private String tokenizeDestination(String destination) {
        Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(destination);
        if (tokens != null && tokens.length > 0) {
            return tokens[0].getAddress();
        }
        return destination;
    }

    /// M: To judge wheather text just have blank.
    private boolean textIsAllBlank(String str) {
        if (str != null) {
            for (int i = 0; i < str.length(); i++) {
                if (str.charAt(i) != ' ') {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
}
