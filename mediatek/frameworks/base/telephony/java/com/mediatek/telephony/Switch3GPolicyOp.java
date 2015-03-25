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

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ISwitch3GPolicy;
import android.provider.Telephony.SIMInfo;
import android.provider.Telephony.SimInfo;
import android.os.SystemProperties;
import android.content.Context;
import android.util.Log;

/**
 * 3G Switch Policy (for Operator)
 * 
 * @hide
 */
public class Switch3GPolicyOp extends Switch3GPolicyDefault {
    
    private static final String LOG_TAG = "Switch3GPolicyOp";
    
    private int telephonyMode = 0;
    
    public Switch3GPolicyOp() {
    	// check current telephony mode
        if ( FeatureOption.MTK_GEMINI_SUPPORT && FeatureOption.MTK_UMTS_TDD128_MODE && !PhoneFactory.isDualTalkMode() )
        	telephonyMode = 2;
        Log.i(LOG_TAG, "policy object created. (telephonyMode="+telephonyMode+")");
    }
    
    @Override
    public int select3GCapability(Context ctx, String iccId3GSim, int id3GSim, String iccId1, String iccId2, String iccId3, String iccId4) {
        
        int selected = ISwitch3GPolicy.CAPABILITY_3G_INVALID;
        
        Log.i(LOG_TAG, "select3GCapability (telephonyMode="+telephonyMode+", iccId3GSim="+iccId3GSim+", id3GSim="+id3GSim+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        // ADD CUSTOM POLICY HERE
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
         
            Log.i(LOG_TAG, "select3GCapability (sim1Op="+sim1Op+", sim2Op="+sim2Op+")");
            
            if (iccId1 != null && iccId2 != null)
            {
                if (SimInfo.OPERATOR_OP01.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op))
                {
                    Log.i(LOG_TAG, "select3GCapability check OP01,OP02");
                    
                    if (id3GSim==-1) {
                        selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                    } else if (id3GSim==PhoneConstants.GEMINI_SIM_1 && iccId1.equals(iccId3GSim)) {
                        selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                    } else {
                        selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM1;
                    }
                    
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                    
                    Log.i(LOG_TAG, "select3GCapability check OP02,OP01");
                    
                    if (id3GSim==-1) {
                        selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                    } else if (id3GSim==PhoneConstants.GEMINI_SIM_2 && iccId2.equals(iccId3GSim)) {
                        selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                    } else {
                        selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM2;
                    }
                    
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                    
                    Log.i(LOG_TAG, "select3GCapability check OP02,OP02");
                    
                    selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                }
                
                // ELSE GO DEFAULT
                
            } else if (iccId1!=null || iccId2!=null) {
                
                if (iccId1!=null) {
                    if (SimInfo.OPERATOR_OP01.equals(sim1Op)) {
                        
                        Log.i(LOG_TAG, "select3GCapability check OP01,(null)");
                        
                        if (id3GSim==-1) {
                            selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                        } else if (id3GSim==PhoneConstants.GEMINI_SIM_1 && iccId1.equals(iccId3GSim)) {
                            selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                        } else {
                            selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM1;
                        }
                        
                    } else if (SimInfo.OPERATOR_OP02.equals(sim1Op)) {
                        
                        Log.i(LOG_TAG, "select3GCapability check OP02,(null)");
                        selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                        
                    }
                    
                    // ELSE GO DEFAULT
                    
                } else if (iccId2!=null) {
                    
                    if (SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                        Log.i(LOG_TAG, "select3GCapability check (null),OP01");
                        
                        if (id3GSim==-1) {
                            selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                        } else if (id3GSim==PhoneConstants.GEMINI_SIM_2 && iccId2.equals(iccId3GSim)) {
                            selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                        } else {
                            selected = ISwitch3GPolicy.CAPABILITY_3G_GEMINI_SIM2;
                        }
                        
                    } else if (SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                        
                        Log.i(LOG_TAG, "select3GCapability check (null),OP02");
                        selected = ISwitch3GPolicy.CAPABILITY_3G_NOCHANGE;
                        
                    }
                    
                    // ELSE GO DEFAULT
                    
                }
                
            }
        }
        
        // GO DEFAULT IF CUSTOM POLICY NOT MATCHED
        if (selected == ISwitch3GPolicy.CAPABILITY_3G_INVALID)
        {
            selected = super.select3GCapability(ctx, iccId3GSim, id3GSim, iccId1, iccId2, iccId3, iccId4);
        }
        
        return selected;
    }

    @Override
    public boolean isManualModeEnabled(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        
        boolean bRet = super.isManualModeEnabled(ctx, iccId1, iccId2, iccId3, iccId4);
        
        Log.i(LOG_TAG, "isManualModeEnabled (telephonyMode="+telephonyMode+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
            
            Log.i(LOG_TAG, "isManualModeEnabled (sim1Op="+sim1Op+", sim2Op="+sim2Op+")");
    
            if (iccId1 != null && iccId2 != null)
            {
                if (SimInfo.OPERATOR_OP01.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeEnabled check OP01,OP02");
                    bRet = true;
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeEnabled check OP02,OP01");
                    bRet = true;
                } else if (SimInfo.OPERATOR_OP01.equals(sim1Op) && SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeEnabled check OP01,OP01");
                    bRet = true;
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeEnabled check OP02,OP02");
                    bRet = false;
                }
                
                // ELSE GO DEFAULT
                
            } else if (iccId1!=null || iccId2!=null) {
                if (iccId1!=null) {
                    if (SimInfo.OPERATOR_OP01.equals(sim1Op)) {
                        Log.i(LOG_TAG, "isManualModeEnabled check OP01,(null)");
                        bRet = true;
                    } else if (SimInfo.OPERATOR_OP02.equals(sim1Op)) {
                        Log.i(LOG_TAG, "isManualModeEnabled check OP02,(null)");
                        bRet = false;
                    }
                    
                    // ELSE GO DEFAULT
                    
                } else if (iccId2!=null) {
                    if (SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeEnabled check (null),OP01");
                        bRet = true;
                    } else if (SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeEnabled check (null),OP02");
                        bRet = false;
                    }
                    
                    // ELSE GO DEFAULT
                    
                }
                
            }
        }
        
        return bRet;
    }
    
    public boolean isManualModeChange3GAllowed(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        boolean bRet = super.isManualModeChange3GAllowed(ctx, iccId1, iccId2, iccId3, iccId4);
        
        Log.i(LOG_TAG, "isManualModeChange3GAllowed (telephonyMode="+telephonyMode+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
            
            Log.i(LOG_TAG, "isManualModeChange3GAllowed (sim1Op="+sim1Op+", sim2Op="+sim2Op+")");
    
            if (iccId1 != null && iccId2 != null)
            {
                if (SimInfo.OPERATOR_OP01.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeChange3GAllowed check OP01,OP02");
                    bRet = false;
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeChange3GAllowed check OP02,OP01");
                    bRet = false;
                } else if (SimInfo.OPERATOR_OP01.equals(sim1Op) && SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeChange3GAllowed check OP01,OP01");
                    bRet = true;
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                    Log.i(LOG_TAG, "isManualModeChange3GAllowed check OP02,OP02");
                    bRet = false;
                }
                
                // ELSE GO DEFAULT
                
            } else if (iccId1!=null || iccId2!=null) {
                if (iccId1!=null) {
                    if (SimInfo.OPERATOR_OP01.equals(sim1Op)) {
                        Log.i(LOG_TAG, "isManualModeChange3GAllowed check OP01,(null)");
                        bRet = false;
                    } else if (SimInfo.OPERATOR_OP02.equals(sim1Op)) {
                        Log.i(LOG_TAG, "isManualModeChange3GAllowed check OP02,(null)");
                        bRet = false;
                    }
                    
                    // ELSE GO DEFAULT
                    
                } else if (iccId2!=null) {
                    if (SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeChange3GAllowed check (null),OP01");
                        bRet = false;
                    } else if (SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                        Log.i(LOG_TAG, "isManualModeChange3GAllowed check (null),OP02");
                        bRet = false;
                    }
                    
                    // ELSE GO DEFAULT
                    
                }
                
            }
        }

        return bRet;
    };
    
    public int getAllowedSwitch3GSlots(Context ctx, String iccId1, String iccId2, String iccId3, String iccId4) {
        int nRet = 0;
        
        Log.i(LOG_TAG, "getAllowedSwitch3GSlots (telephonyMode="+telephonyMode+", iccId1="+iccId1+", iccId2="+iccId2+", iccId3="+iccId3+", iccId4="+iccId4+")");
        
        if (telephonyMode == 2 && (iccId3 == null && iccId4 == null))
        {
        	String sim1Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_1, iccId1);
        	String sim2Op = getSimOperator(ctx, PhoneConstants.GEMINI_SIM_2, iccId2);
            
            Log.i(LOG_TAG, "getAllowedSwitch3GSlots (sim1Op="+sim1Op+", sim2Op="+sim2Op+")");
    
            if (iccId1 != null && iccId2 != null)
            {
                if (SimInfo.OPERATOR_OP01.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                    Log.i(LOG_TAG, "getAllowedSwitch3GSlots check OP01,OP02");
                    nRet = 1;
                    
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                    Log.i(LOG_TAG, "getAllowedSwitch3GSlots check OP02,OP01");
                    nRet = 2;
                    
                } else if (SimInfo.OPERATOR_OP01.equals(sim1Op) && SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                    Log.i(LOG_TAG, "getAllowedSwitch3GSlots check OP01,OP01");
                    nRet = 3;
                    
                } else if (SimInfo.OPERATOR_OP02.equals(sim1Op) && SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                    Log.i(LOG_TAG, "getAllowedSwitch3GSlots check OP02,OP02");
                    nRet = 0;
                    
                } else {
                    // ELSE GO DEFAULT
                    nRet = super.getAllowedSwitch3GSlots(ctx, iccId1, iccId2, iccId3, iccId4);
                }
                
            } else if (iccId1!=null || iccId2!=null) {
                if (iccId1!=null) {
                    if (SimInfo.OPERATOR_OP01.equals(sim1Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitch3GSlots check OP01,(null)");
                        nRet = 1;
                        
                    } else if (SimInfo.OPERATOR_OP02.equals(sim1Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitch3GSlots check OP02,(null)");
                        nRet = 0;
                        
                    } else {
                        // ELSE GO DEFAULT
                        nRet = super.getAllowedSwitch3GSlots(ctx, iccId1, iccId2, iccId3, iccId4);
                    }
                    
                } else if (iccId2!=null) {
                    if (SimInfo.OPERATOR_OP01.equals(sim2Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitch3GSlots check (null),OP01");
                        nRet = 2;
                        
                    } else if (SimInfo.OPERATOR_OP02.equals(sim2Op)) {
                        Log.i(LOG_TAG, "getAllowedSwitch3GSlots check (null),OP02");
                        nRet = 0;
                        
                    } else {
                        // ELSE GO DEFAULT
                        nRet = super.getAllowedSwitch3GSlots(ctx, iccId1, iccId2, iccId3, iccId4);
                    }
                    
                }
                
            } else {
                // ELSE GO DEFAULT
                nRet = super.getAllowedSwitch3GSlots(ctx, iccId1, iccId2, iccId3, iccId4);
            }
            
        } else {
            nRet = super.getAllowedSwitch3GSlots(ctx, iccId1, iccId2, iccId3, iccId4);
        }

        return nRet;
    };
    
    private String getEarlySimOperatorById(int simIdx) {
    	String opStr = "";
        if(FeatureOption.MTK_RILD_READ_IMSI == true) {
        	String propName = "gsm.sim.ril.op" + (simIdx==PhoneConstants.GEMINI_SIM_1 ? "" : "."+(simIdx+1));
        	opStr = SystemProperties.get(propName, "");
        	if (opStr==null) opStr = "";
        }
    	Log.d(LOG_TAG, "getEarlySimOperatorById (simIdx="+simIdx+", opStr="+opStr+")");
    	return opStr;
    }
    
    private String getSimOperatorByICCId(Context ctx, String iccId) {
    	SIMInfo simInfo = SIMInfo.getSIMInfoByICCId(ctx, iccId);
		String opStr = ((simInfo==null || simInfo.mOperator==null) ? "" : simInfo.mOperator);
		Log.d(LOG_TAG, "getSimOperatorByICCId (iccId="+iccId+", opStr="+opStr+")");
		return opStr;
    }
    
    private String getSimOperator(Context ctx, int simIdx, String iccId) {
    	String opStr = getSimOperatorByICCId(ctx, iccId);
    	if(FeatureOption.MTK_RILD_READ_IMSI == true) {
        	if (opStr == null || "".equals(opStr)) {
        		opStr = getEarlySimOperatorById(simIdx);
        	}
    	}
    	Log.d(LOG_TAG, "getSimOperator (simIdx="+simIdx+", iccId="+iccId+", opStr="+opStr+")");
    	return opStr;
    }
}
