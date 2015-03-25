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
package com.mediatek.common.telephony;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.net.ConnectivityManager;
import com.mediatek.common.featureoption.FeatureOption;


// MTK multiuser in 3gdatasms
public interface IOnlyOwnerSimSupport {

    public static String MTK_NORMALUSER_SMS_ACTION = "mediatek.Telephony.NORMALUSER_SMS_RECEIVED";
    public static String MTK_NORMALUSER_MMS_ACTION = "mediatek.Telephony.NORMALUSER_MMS_RECEIVED";
    public static String MTK_NORMALUSER_CB_ACTION = "mediatek.Telephony.NORMALUSER_CB_RECEIVED";
    /**
       * Check feature (MTK_ONLY_OWNER_SIM_SUPPORT)  is ON  or OFF.
       *
       * @return Returns true if MTK_ONLY_OWNER_SIM_SUPPORT is ON.
       */
    public boolean isOnlyOwnerSimSupport();

 
    /**
       * Check  CurrentUser is Owner or NormalUser. 
       * In this method ,we will firstly call isOnlyOwnerSimSupportOn() to ensure feature (MTK_ONLY_OWNER_SIM_SUPPORT) is ON.
       *
       * @return Returns true if MTK_ONLY_OWNER_SIM_SUPPORT is OFF, or CurrentUser is  Owner and MTK_ONLY_OWNER_SIM_SUPPORT is on .
       */
    public boolean isCurrentUserOwner();

     
    /**
       *  Check the message dispatch to owner or not. If  current user is not Owner, the SMS/MMS/CB need to be dispatch to owner 
       *  In this method ,we will firstly call isCurrentUserOwner() to ensure CurrentUser is NormalUser.
       *  And this method will be called in SMSDispatch.dispatch(). 
       *  The parameters(intent , permission and appOp ) must be from the params of SMSDispatch.dispatch().
       *
       * @param intent intent to broadcast.
       * @param permission Receivers are required to have this permission.
       * @return Returns true if Message is SMS/MMS/CB and CurrentUser  is  NormalUser.
       */
    public boolean isMsgDispatchOwner(Intent intent, String permission, int appOp);


    /**
       * Check  Network type is Mobile or not. 
       * In this method ,we will firstly call isCurrentUserOwner to ensure CurrentUser is NormalUser.
       * The Network types  can be found in ConnectivityManager.
       *
       * @param networkType  Network Type.
       * @return Returns true if networkType is TYPE_MOBILE or TYPE_MOBILE_XXX  and CurrentUser is NormalUser.
       */
    public boolean isNetworkTypeMobile(int networkType);

    /**
       *Intercept Message/SIM operation. 
       *
       * @param obj  PendingIntent/ArrayList<PengingIntent>,broadcast when the message is successfully sent, or failed.
       * @param resultCode Result code to supply back to the PendingIntent's target.
       */
    public void intercept(Object obj, int resultCode);

    /**
       * Normal user receive  SMS/MMS/CB. dispatch to owner
       *This method will be called in SMSDispatch.dispatch(). 
       * And the parameters(intent , permission and appOp) must be from the params of SMSDispatch.dispatch().
       *
       * @param intent intent to broadcast.
       * @simId  SIM card the user would like to access.
       * @param permission Receivers are required to have this permission.
       */
    public void dispatchMsgOwner(Intent intent, int simId, String permission, int appOp) ;

    
}

