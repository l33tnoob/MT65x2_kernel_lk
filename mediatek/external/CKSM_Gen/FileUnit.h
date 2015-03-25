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

// FileUnit.h: interface for the FileUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEUNIT_H__AAED5943_60D1_4164_BD6B_826A5BB7CD27__INCLUDED_)
#define AFX_FILEUNIT_H__AAED5943_60D1_4164_BD6B_826A5BB7CD27__INCLUDED_

/*
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/
#include <string>
typedef bool (*COMPARE_METHOD)(const std::string &file_name, std::string &pattern);

typedef struct _BIN_FILE
{
	//char* file_name;
    //char* cfg_item_name;
    char file_name[32]; 
    char cfg_item_name[32];	
} BIN_FILE;

class FileUnit
{
public:
	FileUnit ();
	FileUnit (const FileUnit& fu);
	FileUnit& operator= (const FileUnit& fu);
	//void* operator new (unsigned int num);
	//void operator delete (void* p);
	FileUnit (const std::string& path);
	virtual ~FileUnit();
	
	void Open (const std::string& path);
	unsigned int Calc_Checksum(const bool is_check_by_pkg);
	
	void Set_File_Name (std::string fs_name, BIN_FILE bf);
	unsigned int Get_Checksum ();
	unsigned int Get_Size () {return fu_size;}
	std::string Get_Item_Name(){return bin_def.cfg_item_name;}
	bool is_check_by_pkg;

private:
	unsigned int PaddingFilesize2Even(const unsigned int size);
	void PaddingBuf(const unsigned int file_len, unsigned char* buf);
	void MallocfileBuf(const unsigned int buf_size);
	void FreefileBuf(void);
	unsigned int CalcCheckSumByBuf();
	unsigned short CalcCheckSumByPackage();
protected:
	BIN_FILE bin_def;
	std::string fu_name;
	unsigned int fu_checksum;
	unsigned short fu_checksum_by_package;
	unsigned int fu_size;
	unsigned char* fu_buf;
};

#endif // !defined(AFX_FILEUNIT_H__AAED5943_60D1_4164_BD6B_826A5BB7CD27__INCLUDED_)
