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

// FileUnit.cpp: implementation of the FileUnit class.
//
//////////////////////////////////////////////////////////////////////
#include <exception>
#include <iostream>
#include "CheckSum_Generate.h"
#include "FileUnit.h"
#include <stdio.h>
#include <string.h>

using std::string;
using std::cout;
using std::endl;


#ifdef WIN32
#include <windows.h>
#else
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#ifndef __FUNCTION__
     #define __FUNCTION__ "Global"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileUnit::FileUnit () :
	fu_checksum(0),
	fu_checksum_by_package(0),
	fu_size(0),
	fu_buf(NULL),
	is_check_by_pkg(true)
{
	memset(&bin_def, 0, sizeof(BIN_FILE));
}
FileUnit::FileUnit (const string& path) :
    fu_name(path),
	fu_checksum(0),
	fu_checksum_by_package(0),
	fu_size(0),
	fu_buf(NULL),
	is_check_by_pkg(true)
{
	memset(&bin_def, 0, sizeof(BIN_FILE));	
}

FileUnit::FileUnit (const FileUnit& fu)
{
	bin_def = fu.bin_def;
	fu_name = fu.fu_name;
	fu_checksum = fu.fu_checksum;
	fu_checksum_by_package = fu.fu_checksum_by_package;
	fu_size = fu.fu_size;
	is_check_by_pkg = fu.is_check_by_pkg;
	//fu_buf = NULL;
	//fu_buf = new char [fu_size];
	//memcpy (fu_buf, fu.fu_buf, fu_size);
}

FileUnit& FileUnit::operator= (const FileUnit &fu)
{
	bin_def = fu.bin_def;
        fu_name = fu.fu_name;
	fu_checksum = fu.fu_checksum;
	fu_checksum_by_package = fu.fu_checksum_by_package;
	fu_size = fu.fu_size;
	is_check_by_pkg = fu.is_check_by_pkg;
	//fu_buf = NULL;
	//fu_buf = new char [fu_size];
	//memcpy (fu_buf, fu.fu_buf, fu_size);
	return *this;
}
/*
void* FileUnit::operator new (unsigned int num)
{
	void *p_fu = malloc (num*sizeof(FileUnit));
	return p_fu;
}

void FileUnit::operator delete (void *p)
{
	free (p);
}
*/

FileUnit::~FileUnit()
{
	//delete[] fu_buf;
}

void FileUnit::Open (const string& path)
{
	fu_name = path;
}

unsigned int FileUnit::Calc_Checksum(const bool is_check_by_pkg)
{
	return is_check_by_pkg ? CalcCheckSumByPackage() : CalcCheckSumByBuf();
}

unsigned int FileUnit::CalcCheckSumByBuf()
{
    FILE *file;
    if( (file = fopen(fu_name.c_str(), "rb")) == NULL)
    {
        cout << "Can't open file: " << fu_name.c_str() << endl;
        return -1;
    }
    fseek(file, 0, SEEK_END);
	fu_size = ftell(file);
    fseek(file, 0, 0);

	//Padding to even for buffer_size
	unsigned int buf_size = PaddingFilesize2Even(fu_size);
	std::cout<<__FUNCTION__
		<<" File size: "<<fu_size<<" Buffer size: "<<buf_size<<"."
		<<std::endl;
	//system("Pause");
	try {
		MallocfileBuf(buf_size);
		//unsigned int nBytesRead = file.Read(fu_buf, fu_size);
        unsigned int nBytesRead = fread(fu_buf, 1, fu_size, file);
		PaddingBuf(fu_size, fu_buf);

		unsigned int i = 0;
		fu_checksum = 0;
		
		for (i=0; i<buf_size; i++) {
			fu_checksum += fu_buf[i];
		}
	} catch (std::exception& ex) {
		//file.Close();
        fclose(file);
		FreefileBuf();
		std::cout<<"Runtime exception! Error hint: "
			<<ex.what()<<std::endl;
	}
    
	//file.Close();
    fclose(file);
	FreefileBuf();

    if(fu_size % 2) fu_checksum += 0xff;
    return fu_checksum;
}

unsigned short FileUnit::CalcCheckSumByPackage()
{
    FILE *file;
    //CFile file((const char*)fu_name.c_str(),  CFile::modeRead | CFile::typeBinary);
    if( (file = fopen(fu_name.c_str(), "rb")) == NULL)
    {
        cout << "Can't open file: " << fu_name.c_str() << endl;
        return -1;
    }

    fseek(file, 0, SEEK_END);
	fu_size = ftell(file);
    fseek(file, 0, 0);

    std::cout<<__FUNCTION__
		<<" File size: "<<fu_size<<std::endl;

    const unsigned int kTransferBufSize(1024*1024);//1024*1024
    try {
        MallocfileBuf(kTransferBufSize);
        memset(this->fu_buf, 0, kTransferBufSize);
        unsigned int i(0);
        unsigned int curser(0);
        unsigned int pkg_len(0);
        fu_checksum_by_package = 0;
        while (curser < fu_size) {
            //file.Seek(curser, CFile::begin);
            fseek(file, curser, 0);
            if (fu_size < kTransferBufSize) {
                pkg_len = fu_size;
            } else if (fu_size - curser > kTransferBufSize) {
                pkg_len = kTransferBufSize;
            } else {
                pkg_len = (fu_size - curser);
            }
            //file.Read(fu_buf, pkg_len);	
            fread(fu_buf, 1, pkg_len, file);
            
            for (i=0; i < pkg_len; i++) {
                fu_checksum_by_package += (unsigned short)fu_buf[i];
            }
            memset(this->fu_buf, 0, kTransferBufSize);
            curser += kTransferBufSize;
        }		
    } catch (std::exception& ex) {
        //file.Close();
        fclose(file);
        FreefileBuf();
        std::cout<<"Runtime exception! Error hint: "
            <<ex.what()<<std::endl;
    }
    
    fclose(file);
    FreefileBuf();

    if(fu_size % 2) fu_checksum_by_package += 0xff; // Fix bug

	std::cout<<"fu_checksum_by_package is : "
			<<fu_checksum_by_package<<std::endl;
	return fu_checksum_by_package;
}

void FileUnit::Set_File_Name(string fs_name, BIN_FILE bf)
{
	fu_name = fs_name;
	bin_def = bf;
}

unsigned int FileUnit::Get_Checksum()
{
	std::cout<<"FileUnit is_check_by_pkg is "<< this->is_check_by_pkg <<std::endl;
	if(this->is_check_by_pkg)
	{
		return this->fu_checksum_by_package;
	}
	else
	{
		return this->fu_checksum;
	}
	
}

unsigned int FileUnit::PaddingFilesize2Even(const unsigned int size)
{
	unsigned int buf_size(size);
	if ( 0 != buf_size%2 ) {
		//Padding file length to even
		buf_size += 1;
	}
	return buf_size;
}

void FileUnit::MallocfileBuf(const unsigned int buf_size)
{
	if (NULL == this->fu_buf) {
		try {
		    this->fu_buf = new unsigned char[buf_size];
		    memset(this->fu_buf, 1, buf_size);
		} catch (std::bad_alloc& mallc_ex) {
			if (NULL != fu_buf) {
				delete[] fu_buf;
			}
			std::cout<<__FUNCTION__<<" malloc memory error, your PC memory is low..."<<std::endl;
			std::cout<<__FUNCTION__<<" Error Hint: "<<mallc_ex.what()<<std::endl;
		}
	} else {
		return;
	}
}

void FileUnit::PaddingBuf(const unsigned int file_len, unsigned char* buf)
{
	//Padding last byte to 0xFF
	if (0 != file_len%2) {
		*(buf+file_len) = 0xFF;
	}
}

void FileUnit::FreefileBuf(void)
{
	if (NULL != this->fu_buf) {
		delete[] fu_buf;
		fu_buf = NULL;
	}
}
