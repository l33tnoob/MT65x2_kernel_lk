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

#ifndef ANDROID_SRS_PARAMS
#define ANDROID_SRS_PARAMS

namespace android {
	
struct SRS_ParamBlock;
struct SRS_ParamSource;
typedef void (*Param_Set)(SRS_ParamBlock* pPB, SRS_ParamSource* pSrc, int bank, int param, const char* pValue);
typedef const char* (*Param_Get)(SRS_ParamBlock* pPB, SRS_ParamSource* pSrc, int bank, int param);
	
struct SRS_ParamSource {	// Implement for IO Operations
	void*		pSourceToken;	// Misc additional data
	Param_Set	SetParam;
	Param_Get	GetParam;
};

enum SRS_ParamTypes {	// Any writable type must follow _CFG - readonly before _CFG
	SRS_PTYP_INFO = 0,	// Readonly - optional serialized
	SRS_PTYP_DEBUG,		// Readonly - debug serialized
	SRS_PTYP_CFG,		// Read/write - config serialized
	SRS_PTYP_PREF,		// Read/write - user-pref serialized
	SRS_PTYP_RUNTIME,	// Read/write - never serialized (often used to store things like setup paths, etc)
	SRS_PTYP_ALIAS,		// Read/write - input serialized only (follows permission of type aliased)
};

enum SRS_ParamTypeFlags {
	SRS_PTFLAG_INFO = (1<<SRS_PTYP_INFO),
	SRS_PTFLAG_DEBUG = (1<<SRS_PTYP_DEBUG),
	SRS_PTFLAG_CFG = (1<<SRS_PTYP_CFG),
	SRS_PTFLAG_PREF = (1<<SRS_PTYP_PREF),
	SRS_PTFLAG_RUNTIME = (1<<SRS_PTYP_RUNTIME),
	SRS_PTFLAG_ALIAS = (1<<SRS_PTYP_ALIAS),
};

enum SRS_ParamFormats {
	SRS_PFMT_STATIC = -1,	// Info, etc
	SRS_PFMT_BOOL = 0,
	SRS_PFMT_INT,
	SRS_PFMT_FLOAT,
	SRS_PFMT_ENUM,
	SRS_PFMT_STRING,
	SRS_PFMT_INTARRAY,
	SRS_PFMT_FLOATARRAY,
};

struct SRS_Param {
	int EnumID;
	int Type;			// Propery, Info, etc
	int Format;			// Int, Float, Enum, etc
	float DefValue;
	float Min;
	float Max;
	const char* pKey;	// Param name for code
	const char* pName;	// Param name for humans
	const char* pInfo;	// Info for humans
	const char* pEnums;	// a,b,c,d style for enum format - single int for 'array' formats
	unsigned int KeyCRC;	// Zero
};

struct SRS_ParamBank {
	int EnumID;
	int Index;	// Speeds using multiple IP Banks
	const char* pType;	// WOWHD, MAXV, Info, Routing, etc
	const char* pPrefix;
	const char* pInfo;
	SRS_Param* pParams;
	int ParamCount;
	unsigned int PrefixCRC;	// Zero
	int PrefixLen;	// Zero
};

enum SRS_ParamBlockFlags {
	SRS_PBFS_FILLED = 0x0001,			// Initial dynamic pass on params data performed (queries for lib versions, defaults overrides, etc)
	SRS_PBFS_PARAMSCACHED = 0x0002,		// String of all Params for passing to Java apps was created.
};

struct SRS_ParamBlock {
	int ConfigFlags;	// Always starts 0 - allows one-time config of a Param Set, even if a thread or usages causes a restart more than once per boot.
	SRS_ParamBank* pBanks;
	int BankCount;
	const char* pParamCache;
	
	void FillPreCalcs();
	SRS_Param* FindParam(const char* pKey, int& bank, int& param);
	const char* GetParamCache();
	
	// IO Support
	bool ConfigRead(const char* pPath, SRS_ParamSource* pSource, uint32_t typeMask=0xFFFFFFFF);
	void ConfigWrite(const char* pPath, SRS_ParamSource* pSource, uint32_t typeMask=0xFFFFFFFF);
	bool ConfigParse(char* data, SRS_ParamSource* pSource, uint32_t typeMask=0xFFFFFFFF);
	
	// CRC Support
	static unsigned int CRC_Bank[256];
	static unsigned int CalcCRC(unsigned char* pData, unsigned int dataLen);
};

struct HELP_ParamIn {	// Limit the amount of duplicated string in/out processing we do...
	static bool GetBool(const char* pV);
	static int GetInt(const char* pV);
	static float GetFloat(const char* pV);
};

struct HELP_ParamOut {	// Limit the amount of duplicated string in/out processing we do...
	static char Workspace[512];
	static const char* FromBool(bool tV);
	static const char* FromInt(int tV);
	static const char* FromFloat(float tV);
};

};

#endif	// ANDROID_SRS_PARAMS

