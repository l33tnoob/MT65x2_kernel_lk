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

package com.mediatek.engineermode.lcm;

import java.io.IOException;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

//import com.mediatek.engineermode.ChipSupport;
import android.R.integer;

import com.mediatek.xlog.Xlog;
//import com.mediatek.internalem.emsvr.ShellExe;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;

public class EmDsenseLcmAssit {
    // constants below.
    public final static String TAG = "LCM/EmDsenseLcmAssit";

    private final static int FB0_LCDWriteCycleGetMinVal = 1;
    private final static int FB0_LCDWriteCycleGetCurrentVal = 2;
    private final static int FB0_LCDWriteCycleSetVal = 3;

	private final static int FB0_LCMPowerON = 4;
	private final static int FB0_LCMPowerOFF = 5;
		
	private final static int FB0_LCM_Get_Tm = 6;
	private final static int FB0_LCM_Set_Tm = 7;

	private final static int FB0_LCM_Get_MIPI_clock = 8;
	private final static int FB0_LCM_Set_MIPI_clock = 9;
	private final static int FB0_LCM_Get_Interface_Type = 10;
	
    private static int FB0_Fucntion(int... param) {
        AFMFunctionCallEx A = new AFMFunctionCallEx();
        boolean result = A
                .startCallFunctionStringReturn(AFMFunctionCallEx.FUNCTION_EM_FB0_IOCTL);
        A.writeParamNo(param.length);
        for (int i : param) {
            A.writeParamInt(i);
        }

        if (!result) {
            return -1;
        }

        int valueRet = -1;
        FunctionReturn r;
        do {
            r = A.getNextResult();            
            Xlog.v(TAG, "default result --returnString--"+r.mReturnString);
            if (r.mReturnString == "") {
                break;
            } else {
                if (r.mReturnString.equalsIgnoreCase("FFFFFFFF")) {
                    valueRet = -1;
                    break;
                }
                try {
                    valueRet = Integer.valueOf(r.mReturnString);
                } catch (NumberFormatException e) {
                    Xlog.d(TAG, e.getMessage());
                    valueRet = -1;
                }
            }
        } while (r.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);

        if (r.mReturnCode == AFMFunctionCallEx.RESULT_IO_ERR) {
            // error
            return -1;
        } else {
            return valueRet;
        }
    }

    
	public static int LCMPowerON() {
		return FB0_Fucntion(FB0_LCMPowerON);
	}

	public static int LCMPowerOFF() {
		return FB0_Fucntion(FB0_LCMPowerOFF);
	}

    public static int LCDWriteCycleGetMinVal() {
		Xlog.v(TAG, "-->LCDWriteCycleGetMinVal");
        return FB0_Fucntion(FB0_LCDWriteCycleGetMinVal);
    }

    public static int LCDWriteCycleGetCurrentVal() {
		Xlog.v(TAG, "-->LCDWriteCycleGetCurrentVal");
        return FB0_Fucntion(FB0_LCDWriteCycleGetCurrentVal);
    }

    public static int LCDWriteCycleSetVal(int cycle) {
    	Xlog.v(TAG, "-->LCDWriteCycleSetVal");
        return FB0_Fucntion(FB0_LCDWriteCycleSetVal, cycle);
    }

    // LCM TM
	public static int LCMGetTm() {
		Xlog.v(TAG, "-->LCMGetTm");
		return FB0_Fucntion(FB0_LCM_Get_Tm);
	}

	public static int LCMSetTm(int tm) {
		Xlog.v(TAG, "-->LCMSetTm");
		return FB0_Fucntion(FB0_LCM_Set_Tm, tm);
	}

    // LCM MIPI
	public static int LCMGetMipi() {
		Xlog.v(TAG, "-->LCMGetMipi");
		return FB0_Fucntion(FB0_LCM_Get_MIPI_clock);
	}

	public static int LCMSetMipi(int mipi) {
		Xlog.v(TAG, "-->LCMSetMipi");
		return FB0_Fucntion(FB0_LCM_Set_MIPI_clock, mipi);
	}
	
	public static int LCMGetInterfaceType() {
		Xlog.v(TAG, "-->LCMGetInterfaceType");
		return FB0_Fucntion(FB0_LCM_Get_Interface_Type);
	}
	
	
   
}
