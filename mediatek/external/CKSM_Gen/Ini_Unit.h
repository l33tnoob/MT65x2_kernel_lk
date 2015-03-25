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

// Ini_Unite.h: interface for the Ini_Unite class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INI_UNITE_H__49839613_BD1F_420A_A22F_C05382CBCFF6__INCLUDED_)
#define AFX_INI_UNITE_H__49839613_BD1F_420A_A22F_C05382CBCFF6__INCLUDED_
/*
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/
#include <string>

class Ini_Unit  
{
public:
	Ini_Unit();
	Ini_Unit(std::string path);
	virtual ~Ini_Unit();

	int Write (const char* lpSection, const char* lpKeyName, const char* lpString);
	int Read (const char* section, const char* lpKeyName, const char* lpDefault,
		        char* lpReturnedString, unsigned short nSize);
    //int WriteProfileString (const char* path, const char* section, 
    //                    const char* lpKeyName, const char* lpString);
    //int ReadProfileString (const char* path, const char* section,
    //                   const char* lpKeyName, const char* lpDefault,
	//	                char* lpReturnedString, unsigned short nSize);

protected:
	std::string ini_path;
    //char *writeBuf;
    //char *readBuf;

    int WriteProfileString (const char* path, const char* section, 
                        const char* lpKeyName, const char* lpString);
    int ReadProfileString (const char* path, const char* section,
                       const char* lpKeyName, const char* lpDefault,
		                char* lpReturnedString, unsigned short nSize);
    int StrCopyWithoutSpace(char *out, const char *src, unsigned int n);
};

#endif // !defined(AFX_INI_UNITE_H__49839613_BD1F_420A_A22F_C05382CBCFF6__INCLUDED_)
