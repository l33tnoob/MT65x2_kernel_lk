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

package com.mediatek.telephony;

import com.mediatek.common.telephony.ISwitch3GPolicy;
import com.mediatek.common.telephony.ISwitch3GPolicyWrapper;
import android.util.Log;


/**
 * This is the wrapper class for smart 3G switch policy object creation. 
 * 
 * {@hide}
 */
public class Switch3GPolicyWrapper implements ISwitch3GPolicyWrapper {

	private static final String TAG = "Switch3GPolicyWrapper";
	
	public ISwitch3GPolicy createInstance(String type) {
		ISwitch3GPolicy policyObj = null;
		
        try {
            Object obj = null;
            Log.i(TAG, "createInstance type="+type);
            
        	if (type != null) {
                if (type.equals(ISwitch3GPolicy.POLICY_OP)) {
                    Class<?> clz = Class.forName("com.mediatek.telephony.Switch3GPolicyOp");
                    if (clz != null)
                    	obj = clz.newInstance();
                    Log.i(TAG, "createInstance (Op) : " + obj);
                } else if (type.equals(ISwitch3GPolicy.POLICY_DEFAULT)) {
                    Class<?> clz = Class.forName("com.mediatek.telephony.Switch3GPolicyDefault");
                    if (clz != null)
                    	obj = clz.newInstance();
                    Log.i(TAG, "createInstance (Default) : " + obj);
                }
            }
            if (obj == null) {
                Log.i(TAG, "createInstance (ERROR) : create request not mapped to any implementation.");
            }
            
            policyObj = (ISwitch3GPolicy) obj;
            
        } catch (Exception e) {
            Log.e(TAG, "createInstance: got exception for Switch 3G Policy");
            e.printStackTrace();
        }
        
        return policyObj;
	}
}
