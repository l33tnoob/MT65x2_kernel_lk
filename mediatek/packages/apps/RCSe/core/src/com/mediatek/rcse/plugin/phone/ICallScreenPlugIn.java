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

package com.mediatek.rcse.plugin.phone;

import com.android.internal.telephony.CallManager;

public interface ICallScreenPlugIn {

    /**
     * Start share.
     * 
     * @param phone number
     */
    void start(String number);

    /**
     * Stop share.
     */
    void stop();

    int getCurrentState();
    

    /**
     * set ICallScreenHost interface for calling back functions.
     * 
     * @param call screen host interface
     */
    void setCallScreenHost(ICallScreenHost callScreenHost);

    /**
     * get ICallScreenHost interface which is set before
     * 
     * @return call screen host interface
     */
    ICallScreenHost getCallScreenHost();

    /**
     * Get the capability of sharing with specified phone number.
     * 
     * @return whether specified phone number has capability
     * @param phone number
     */
    boolean getCapability(String number);

    /**
     * Get the capability of Image sharing with specified phone number.
     * 
     * @return whether specified phone number has capability
     * @param phone number
     */
    boolean isImageShareSupported(String number);
    
    /**
     * Get the capability of Image sharing with specified phone number.
     * 
     * @return whether specified phone number has capability
     * @param phone number
     */
    boolean isVideoShareSupported(String number);
    
    /**
     * Register phone number for getting notification of capability change.
     * 
     * @param phone number
     */
    void registerForCapabilityChange(String number);

    /**
     * Register phone number for canceling getting notification of capability
     * change.
     * 
     * @param phone number
     */
    void unregisterForCapabilityChange(String number);

    /**
     * Query the state of plug-in
     * 
     * @return state
     */
    int getState();

    /**
     * Query the state of plug-in //to be  used only by IS
     * 
     * @return state
     */
    int getStatus();

    /**
     * Dismiss dialogs
     * 
     * @return whether host no need handle dismiss dialog again
     */
    boolean dismissDialog();

    /**
     * Notify phone state change
     * 
     * @param call manager
     */
    void onPhoneStateChange(CallManager cm);

    
    /**
     * Save Dialogs
     * 
     */
    void saveAlertDialogs();
    
    /**
     * Show Dialogs
     * 
     */
    void showAlertDialogs();
    
    /**
     * Clear Dialogs
     * 
     */
    void clearSavedDialogs();
}
