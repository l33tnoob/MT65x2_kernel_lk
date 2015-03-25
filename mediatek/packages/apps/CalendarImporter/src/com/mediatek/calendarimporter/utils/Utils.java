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

package com.mediatek.calendarimporter.utils;

import android.R;
import android.accounts.Account;
import android.accounts.AccountManager;
import android.content.Context;
import android.content.res.Resources;

//import com.mediatek.featureoption.FeatureOption;

public final class Utils {
    public static final String MAIL_TYPE_EXCHANGE = "com.android.exchange";
    public static final String MAIL_TYPE_GOOGLE = "com.google";

    public static final int DEFAULT_COLOR = R.color.holo_blue_light;

    private Utils() {
    }

    /**
     * Judge whether has Google or Exchange Account
     * 
     * @param context
     *            the context
     * @return true if there has Exchange or Google Account on the device,else
     *         false
     */
    public static boolean hasExchangeOrGoogleAccount(Context context) {
        final Account[] account = AccountManager.get(context.getApplicationContext()).getAccounts();
        for (int i = 0; i < account.length; i++) {
            if (isExchangeOrGoogleAccount(account[i])) {
                return true;
            }
        }
        return false;
    }

    /**
     * Judge whether the given account is a exchange or google type
     * 
     * @param account
     *            the accout going to check
     * @return true if the account's type is exchange or google type,else false.
     */
    public static boolean isExchangeOrGoogleAccount(Account account) {
        boolean b = account.type.equals(MAIL_TYPE_EXCHANGE) || account.type.equals(Utils.MAIL_TYPE_GOOGLE);

        return b;
    }

    /**
     * get the color managed by ThemeManager
     * 
     * @param context
     * @param defaultColor
     *            if failed,return this color
     * @return
     */
    public static int getThemeMainColor(Context context, int defaultColor) {
       // MTK will phase out Theme Manager on KitKat.
       // if (true /* FeatureOption.MTK_THEMEMANAGER_APP */) {
       //     Resources res = context.getResources();
       //     int colorValue = res.getThemeMainColor();
       //     if (colorValue != 0) {
       //         return colorValue;
       //     }
       // }
        return defaultColor;
    }
}
