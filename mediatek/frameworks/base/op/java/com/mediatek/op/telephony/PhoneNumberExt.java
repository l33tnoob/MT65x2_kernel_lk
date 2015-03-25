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

package com.mediatek.op.telephony;

import android.telephony.Rlog;
import android.text.TextUtils;
import android.telephony.PhoneNumberUtils;
import android.os.ServiceManager;
import android.content.Context;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.mediatek.common.telephony.IPhoneNumberExt;
import com.android.internal.telephony.ITelephony;
import com.mediatek.common.featureoption.FeatureOption;

public class PhoneNumberExt implements IPhoneNumberExt {
    static final String TAG = "PhoneNumberExt";

    public PhoneNumberExt() {
    }

    public boolean isCustomizedEmergencyNumber(String number, String plusNumber, String numberPlus) {
        return false;
    }

    public boolean isSpecialEmergencyNumber(String dialString) {
        if (FeatureOption.MTK_CTA_SET) {
            String [] emergencyCTAList = {"120","122"};
            String numberPlus = null;
            for (String emergencyNum : emergencyCTAList) {
                numberPlus = emergencyNum + "+";
                if (emergencyNum.equals(dialString)
                    || numberPlus.equals(dialString)) {
                    Rlog.d(TAG, "[isSpecialEmergencyNumber] CTA list: " + "return true");
                    return true;
                }
            }
        }
        return false;
    }

    public boolean isCustomizedEmergencyNumberExt(String number, String plusNumber, String numberPlus) {
        return false;
    }

   /** 
    * @hide
    */
    public String prependPlusToNumber(String number) {
        // This is an "international number" and should have
        // a plus prepended to the dialing number. But there
        // can also be Gsm MMI codes as defined in TS 22.030 6.5.2
        // so we need to handle those also.
        //
        // http://web.telia.com/~u47904776/gsmkode.htm is a
        // has a nice list of some of these GSM codes.
        //
        // Examples are:
        //   **21*+886988171479#
        //   **21*8311234567#
        //   *21#
        //   #21#
        //   *#21#
        //   *31#+11234567890
        //   #31#+18311234567
        //   #31#8311234567
        //   18311234567
        //   +18311234567#
        //   +18311234567
        // Odd ball cases that some phones handled
        // where there is no dialing number so they
        // append the "+"
        //   *21#+
        //   **21#+
        StringBuilder ret;
        String retString = number.toString();
        Pattern p = Pattern.compile("(^[#*])(.*)([#*])(.*)(#)$");
        Matcher m = p.matcher(retString);
        if (m.matches()) {
            if ("".equals(m.group(2))) {
                // Started with two [#*] ends with #
                // So no dialing number and we'll just
                // append a +, this handles **21#+
                ret = new StringBuilder();
                ret.append(m.group(1));
                ret.append(m.group(3));
                ret.append(m.group(4));
                ret.append(m.group(5));
                ret.append("+");
            } else {
                // Starts with [#*] and ends with #
                // Assume group 4 is a dialing number
                // such as *21*+1234554#
                ret = new StringBuilder();
                ret.append(m.group(1));
                ret.append(m.group(2));
                ret.append(m.group(3));
                ret.append("+");
                ret.append(m.group(4));
                ret.append(m.group(5));
            }
        } else {
            p = Pattern.compile("(^[#*])(.*)([#*])(.*)");
            m = p.matcher(retString);
            if (m.matches()) {
                // Starts with [#*] and only one other [#*]
                // Assume the data after last [#*] is dialing
                // number (i.e. group 4) such as *31#+11234567890.
                // This also includes the odd ball *21#+
                ret = new StringBuilder();
                ret.append(m.group(1));
                ret.append(m.group(2));
                ret.append(m.group(3));
                ret.append("+");
                ret.append(m.group(4));
            } else {
                // Does NOT start with [#*] just prepend '+'
                ret = new StringBuilder();
                ret.append('+');
                ret.append(retString);
            }
        }

        return ret.toString();
    }

   /**
    * isVoiceMailNumber: checks a given number against the voicemail
    *   number provided by the RIL and SIM card. The caller must have
    *   the READ_PHONE_STATE credential.
    *
    * @param number the number to look up.
    * @param simId the SIM card ID
    * @return true if the number is in the list of voicemail. False
    * otherwise, including if the caller does not have the permission
    * to read the VM number.
    * @hide TODO: pending API Council approval
    */
    public boolean isVoiceMailNumber(String number, int simId) {
        String vmNumber;

        Rlog.d(TAG, "isVoiceMailNumber, number " + number + " simId: " + simId);

        try {
            ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
            vmNumber = iTel.getVoiceMailNumber(simId);
        } catch (Exception ex) {
            return false;
        }

        // Strip the separators from the number before comparing it
        // to the list.
        number = PhoneNumberUtils.extractNetworkPortionAlt(number);

        // compare tolerates null so we need to make sure that we
        // don't return true when both are null.
        return !TextUtils.isEmpty(number) && PhoneNumberUtils.compare(number, vmNumber);
    }

    public int getMinMatch() {
        return 7;
    }

    public boolean isPauseOrWait(char c) {
        return false;
    }

    public void log(String text) {
        Rlog.d(TAG, text);
    }
}
