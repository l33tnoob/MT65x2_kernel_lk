/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 */
package com.mediatek.telephony;

import android.text.Editable;
import android.text.Selection;
import android.text.TextWatcher;
import android.widget.TextView;

import java.util.Locale;

/**
 * Watches a TextView and if a phone number is entered will format it. The formatting
 * is based on the current system default sim iso. when this object is created and future
 * sim changes may not take effect on this instance.
 * @hide
 */
public class PhoneNumberFormattingTextWatcherEx implements TextWatcher {

    static private int sFormatType;
    //static private Locale sCachedLocale;
    private String sCachedSimIso;
    private boolean mFormatting;
    private boolean mDeletingHyphen;
    private int mHyphenStart;
    private boolean mDeletingBackward;


    public PhoneNumberFormattingTextWatcherEx() {
    	/*
        if (sCachedLocale == null || sCachedLocale != Locale.getDefault()) {
            sCachedLocale = Locale.getDefault();
            sFormatType = PhoneNumberFormatUtilEx.getFormatTypeForLocale(sCachedLocale);
        }
        */
        if(sCachedSimIso == null){
        	sCachedSimIso = PhoneNumberFormatUtilEx.getDefaultSimCountryIso();
        	sFormatType = PhoneNumberFormatUtilEx.getFormatTypeFromCountryCode(sCachedSimIso);
        }
    }

    /**
     * Format the input string after the text is changed.
     * @param text The text editor to be formatted.
     * @hide
     */
    public synchronized void afterTextChanged(Editable text) {
        // Make sure to ignore calls to afterTextChanged caused by the work done below
        if (!mFormatting) {
            mFormatting = true;

            // If deleting the hyphen, also delete the char before or after that
            if (mDeletingHyphen && mHyphenStart > 0) {
                if (mDeletingBackward) {
                    if (mHyphenStart - 1 < text.length()) {
                        text.delete(mHyphenStart - 1, mHyphenStart);
                    }
                } else if (mHyphenStart < text.length()) {
                    text.delete(mHyphenStart, mHyphenStart + 1);
                }
            }

            PhoneNumberFormatUtilEx.formatNumber(text, sFormatType);
            mFormatting = false;
        }
    }

    /**
     * Check if the user is deleting a hyphen.
     * @param s The string to be checked.
     * @param start start value of selection
     * @param count The count of character to be deleted
     * @param after Check if for deleting
     * @hide
     */
    public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        // Check if the user is deleting a hyphen
        if (!mFormatting) {
            // Make sure user is deleting one char, without a selection
            final int selStart = Selection.getSelectionStart(s);
            final int selEnd = Selection.getSelectionEnd(s);
            if (s.length() > 1 // Can delete another character
                    && count == 1 // Deleting only one character
                    && after == 0 // Deleting
                    && s.charAt(start) == '-' // a hyphen
                    && selStart == selEnd) { // no selection
                mDeletingHyphen = true;
                mHyphenStart = start;
                // Check if the user is deleting forward or backward
                if (selStart == start + 1) {
                    mDeletingBackward = true;
                } else {
                    mDeletingBackward = false;
                }
            } else {
                mDeletingHyphen = false;
            }
        }
    }

    /**
     * Not used.
     * @hide
     */
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        // Does nothing
    }
}

