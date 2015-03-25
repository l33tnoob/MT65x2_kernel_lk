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

#ifndef ANDROID_SRS_TECH_CSHP
#define ANDROID_SRS_TECH_CSHP

namespace android {

struct SRS_Tech_CSHP {
	float IGain, OGain, BGain;
	bool DoTB, DoTBSplit, DoDef, DoDecode, DoDialog, DoLimit, Skip;
	float TBMin, TBWindow, TBSlide, TBCompress;
	float DefMin, DefWindow, DefSlide, Dialog, LimitMGain;
	int DecodeMode, TBFreq, TBAnalyze, TBMode;
	
	float TBSlideUDef, DefSlideUDef;
};

struct SRS_Source_CSHP;

// Instead of virtuals - or a 'public' class - we'll hide all of CSHP behind this...
extern SRS_Source_CSHP* SRS_Create_CSHP(SRS_Source_Out* pOut);
extern void SRS_Destroy_CSHP(SRS_Source_CSHP* pSrc, SRS_Source_Out* pOut);
extern void SRS_Config_CSHP(SRS_Source_CSHP* pSrc, SRS_Source_Out* pOut, SRS_Tech_CSHP* pCFG, bool bBypass);

extern void SRS_Process_CSHP_256(SRS_Source_CSHP* pSrc, SRSSamp* pData);

extern char* SRS_GetVersion_CSHP(char* pWork, size_t maxBytes);
extern SRS_Param* SRS_GetBank_CSHP(int& paramCount);
extern void SRS_SetParam_CSHP(SRS_Tech_CSHP* pCFG, SRS_Param* pParam, const char* pValue);
extern const char* SRS_GetParam_CSHP(SRS_Tech_CSHP* pCFG, SRS_Param* pParam);
extern void SRS_Default_CSHP(SRS_Tech_CSHP* pCFG);
extern void SRS_UserDefault_CSHP(SRS_Tech_CSHP* pCFG);

};

#endif	// ANDROID_SRS_TECH_CSHP

