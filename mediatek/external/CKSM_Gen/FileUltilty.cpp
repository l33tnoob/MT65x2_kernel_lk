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

#include "FileUltilty.h"
#include "Ini_Unit.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#define MAX_PATH 256
#endif

using std::cout;
using std::endl;
using std::vector;
using std::string;

// In this version, g_Bin_File is used for defult
BIN_FILE g_Bin_File[] = 
{
	{"preloader", "PRELOADER"},
	{"DSP_BL", "DSP_BL"},
	{"uboot", "UBOOT"},
    {"lk.bin", "UBOOT"}, // lk.bin will instead of uboot, if exist lk.bin, UBOOT checksum will be overwrite
	{"logo.bin", "LOGO"},
	{"boot.img", "BOOTIMG"},
	{"recovery.img", "RECOVERY"},
	{"secro.img", "SEC_RO"},
	{"system.img", "ANDROID"},
	{"userdata.img", "USRDATA"},
	{"PRO_INFO", "PRO_INFO"},
	{"MBR", "MBR"},
	{"EBR1", "EBR1"},
	{"EBR2", "EBR2"},
	{"cache.img", "CACHE"},
	{"fat", "FAT"}
};

#define LINE_LEN 256
#define MAX_BIN_FILE_NUM 30
//BIN_FILE g_Custom_Bin_File[MAX_BIN_FILE_NUM];

#define CHECKSUM_FILE "Checksum.ini"
#define CHECKSUM_SECTION "CheckSum"

#define CHECKSUM_GEN_FILE "CKSM_Gen.ini"
#define BIN_NUM_SECTION "BinNum"
#define BIN_NUM_KEY "Num"
#define BIN_FILE_LIST_SETION "BinFileList"
#define BIN_FILE_PREFIX "bin"


#define ENABLE_SECTION "IsEnableChecksum"
#define ENABLE_ITEM "CHECKSUM_SWITCH"
#if 0
#define ENABLE_SECTION_MDT "Config"
#define ENABLE_ITEM_MDT "Enable"
#endif

FU_VECTOR::FU_VECTOR()
{
	char path[MAX_PATH];
    int pos;
#ifdef WIN32
	::GetModuleFileName(NULL, path, MAX_PATH);
    m_Current_Dir = path;
	pos = m_Current_Dir.find_last_of('\\'); //m_Current_Dir.ReverseFind('\\'); 
    m_Current_Dir = m_Current_Dir.substr(0, pos);  //m_Current_Dir.Left(pos);
        
	m_ini_file = m_Current_Dir + "\\" + CHECKSUM_FILE;
    m_cksm_gen_file = m_Current_Dir + "\\" + CHECKSUM_GEN_FILE;
#else
    strcpy(path, "./");//getexecname();
    m_Current_Dir = path;
	pos = m_Current_Dir.find_last_of('/'); //m_Current_Dir.ReverseFind('\\'); 
    m_Current_Dir = m_Current_Dir.substr(0, pos);  //m_Current_Dir.Left(pos);
        
	m_ini_file = m_Current_Dir + "/" + CHECKSUM_FILE;
    m_cksm_gen_file = m_Current_Dir + "/" + CHECKSUM_GEN_FILE;
#endif
}
FU_VECTOR::~FU_VECTOR ()
{
}

string FU_VECTOR::GetAppDirectory ()
{
	return m_Current_Dir;
}

void FU_VECTOR::SetAppDirectory (string dir)
{
	m_Current_Dir = dir;
}

string FU_VECTOR::GetSrcDirectory ()
{
	return m_Src_Dir;
}

void FU_VECTOR::SetSrcDirectory (string dir)
{
	m_Src_Dir = dir;
    m_cksm_gen_file = m_Src_Dir; // m_cksm_gen_file in bin file folder(use scatterfile)
}
string FU_VECTOR::GetDestDirectory ()
{
	return m_Dest_Dir;
}

void FU_VECTOR::SetDestDirectory (string dir)
{
	m_Dest_Dir = dir;
}

void FU_VECTOR::SetIniDirectory (string dir)
{
#ifdef WIN32
	m_ini_file = dir + "\\" + CHECKSUM_FILE;
#else
    m_ini_file = dir + "/" + CHECKSUM_FILE;
#endif
}


bool FU_VECTOR::SearchFile(string &files,
		string pattern, COMPARE_METHOD compare_m) 
{
#ifdef WIN32
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	int bIsDirectory;
	string file_name, file_path, search_path;
	search_path = m_Src_Dir + "\\*.*";
	bool bFound = false;

	hFind = FindFirstFile((const char*)search_path.c_str(), &find_data);

	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}

	do {
		bIsDirectory = ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!= 0);
		file_name = find_data.cFileName;

		if (!bIsDirectory) 
		{
			if (compare_m == NULL) 
			{
				if (pattern == "") 
				{
					//add all files if no pattern defined
					files = file_name;	
					bFound = true;
                } else if ( -1 != file_name.find(pattern))
				{
					files = file_name;
					bFound = true;
				}
			} else {
				if (compare_m(file_name, pattern)) {
					files = file_name;
					bFound = true;
				}
			}

		}
		
	} while (FindNextFile(hFind, &find_data));

    FindClose(hFind);
	return bFound;

#else   /*Linux*/
    struct dirent *ent = NULL;
    DIR *pDir = NULL;
    char dir[512];
    struct stat statbuf;
    bool bFound = false;

    string file_name, file_path;// search_path;
	//search_path = m_Src_Dir + "\\*.*";

    if( (pDir = opendir((const char*) m_Src_Dir.c_str())) == NULL)
    {
        cout << "Can't open directory: " << m_Src_Dir.c_str() << endl;
        return false;
    }

    while( (ent = readdir(pDir)) != NULL)
    {
        //file_name = m_Src_Dir + "\\" + ent->d_name;
        file_name = ent->d_name;
        file_path = m_Src_Dir + "/" + ent->d_name;
        //lstat(file_name.c_str(), &statbuf);
        lstat(file_path.c_str(), &statbuf);

        if( !S_ISDIR(statbuf.st_mode) )
        {
            if (compare_m == NULL) 
            {
                if (pattern == "") 
                {
                    //add all files if no pattern defined
                    files = file_name;	
                    bFound = true;
                } else if ( -1 != file_name.find(pattern))
                {
                    files = file_name;
                    bFound = true;
                }
            } else {
                if (compare_m(file_name, pattern)) {
                    files = file_name;
                    bFound = true;
                }
            }
        }
    }

    closedir(pDir);
    return bFound;
#endif
}


FileUnit FU_VECTOR::Append_FileUnit(const FileUnit &fu)
{
	m_fu_vector.push_back(fu);
	return fu;
}

FileUnit FU_VECTOR::FU_List_Init(const bool is_check_by_pkg)
{
	string file_name;
	FileUnit fu;
	fu.is_check_by_pkg = is_check_by_pkg;

    /*char ini_read[LINE_LEN] = {0};
    int no_cksm_gen_file = -1;
    if( !(no_cksm_gen_file = m_cksm_gen_file.Read(BIN_NUM_SECTION, BIN_NUM_KEY, NULL, ini_read, LINE_LEN)) && (atoi(ini_read) > 0) )
    {
        char temp_bin_file[20];
        int bin_num = atoi(ini_read);
        if (bin_num > MAX_BIN_FILE_NUM)
            bin_num = MAX_BIN_FILE_NUM;

        //cout << bin_num << endl;
        //system("pause");

        for (int i = 0; i < bin_num; i++)
        {
            sprintf(temp_bin_file, "%s%d", BIN_FILE_PREFIX, i + 1); 
            if( m_cksm_gen_file.Read(BIN_FILE_LIST_SETION, temp_bin_file, NULL, ini_read, LINE_LEN) ) continue;

            //cout << ini_read << endl;
            //system("pause");

            // To find the value, maybe these code need a fuction implemention
            int first = 0;
            while( ini_read[first] )
            {
                if(ini_read[first] == ',') break;
                g_Custom_Bin_File[i].file_name[first] = ini_read[first];
                first ++;
            }
            g_Custom_Bin_File[i].file_name[first+1] = '\0';

            int second = 0;
            first ++;
            while( ini_read[first] )
            {
                if(ini_read[first] == ',') break;
                g_Custom_Bin_File[i].cfg_item_name[second] = ini_read[first];
                //cout << g_Custom_Bin_File[i].cfg_item_name[m] << endl;
                first ++;
                second ++;
            }
            g_Custom_Bin_File[i].cfg_item_name[second] = '\0';

            cout << "SearchFile...: " << g_Custom_Bin_File[i].file_name << endl;
            //cout << "SearchFile...: " << g_Custom_Bin_File[i].cfg_item_name << endl;
            //system("pause");

            if(SearchFile(file_name, g_Custom_Bin_File[i].file_name, NULL))
            {
                cout << file_name << " is found;" << endl; 
#ifdef WIN32
                fu.Open (m_Src_Dir + "\\" + file_name);
#else
                fu.Open (m_Src_Dir + "/" +file_name);
#endif
                cout << file_name << " is Calc_Checksum" << endl; 
                fu.Calc_Checksum (is_check_by_pkg);
                fu.Set_File_Name (file_name, g_Custom_Bin_File[i]);
                cout << "fu.fu_checksum_by_package ========"<< fu.Get_Checksum()<<endl;
                Append_FileUnit (fu);
                cout << file_name << " is Append to List." << endl<<endl;
            }
            else
                cout << g_Custom_Bin_File[i].file_name << " is not found;" << endl << endl; 
        }
    }
    else*/
    {
        // If the CKSM_Gen.ini doesn't exist, create it use default list
        /*if( no_cksm_gen_file )
        {
            char temp_bin[20];
            char temp_list_value[LINE_LEN];
            m_cksm_gen_file.Write(BIN_NUM_SECTION, BIN_NUM_KEY, "0");
            for (int i = 0; i < sizeof(g_Bin_File)/sizeof(BIN_FILE); i++)
            {
                sprintf(temp_bin, "%s%d", BIN_FILE_PREFIX, i + 1);
                sprintf(temp_list_value, "%s, %s", g_Bin_File[i].file_name, g_Bin_File[i].cfg_item_name);

                m_cksm_gen_file.Write(BIN_FILE_LIST_SETION, temp_bin, temp_list_value);
            }
        }*/

        // Use default bin file list gen
        for (int i=0; i< sizeof(g_Bin_File)/sizeof(BIN_FILE); i++)
        {
            cout << "SearchFile...: " << g_Bin_File[i].file_name << endl; 
            if(SearchFile(file_name, g_Bin_File[i].file_name, NULL))
            {
                cout << file_name << " is found;" << endl; 
#ifdef WIN32
                fu.Open (m_Src_Dir + "\\" + file_name);
#else
                fu.Open (m_Src_Dir + "/" +file_name);
#endif
                cout << file_name << " is Calc_Checksum" << endl; 
                fu.Calc_Checksum (is_check_by_pkg);
                fu.Set_File_Name (file_name, g_Bin_File[i]);
                cout << "fu.fu_checksum_by_package ========"<< fu.Get_Checksum()<<endl;
                Append_FileUnit (fu);
                cout << file_name << " is Append to List." << endl<<endl;
            }
            else
                cout << g_Bin_File[i].file_name << " is not found;" << endl << endl; 
        }
    }
	return fu;
}

int FU_VECTOR::Write_INI()
{
	std::vector<FileUnit>::iterator iter;
	//string str;
    char temp[32];

	m_ini_file.Write (ENABLE_SECTION, ENABLE_ITEM, "1");
#if 0
	m_ini_file.Write (ENABLE_SECTION_MDT, ENABLE_ITEM_MDT, "1");
#endif	
	for (iter=m_fu_vector.begin(); iter!=m_fu_vector.end(); iter++)
	{
        memset(temp, 0, 32);
        sprintf(temp, "0x%08x", iter->Get_Checksum());
		//str.append("0x%08x", iter->Get_Checksum());
		cout << " iter->Get_Checksum() ==== " << iter->Get_Checksum() << endl; 
		//m_ini_file.Write (CHECKSUM_SECTION, (const char*)iter->Get_Item_Name().c_str(), (const char*)str.c_str());
        m_ini_file.Write (CHECKSUM_SECTION, (const char*)iter->Get_Item_Name().c_str(), (const char*)temp);
		
		cout << iter->Get_Item_Name()<<" = " << temp <<";  size = " << iter->Get_Size() <<endl;
	}
	cout << endl;
	return true;
}
