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

// CheckSum_Generate.cpp : Defines the entry point for the console application.
//

#include "CheckSum_Generate.h"
#include "FileUltilty.h"
#include <iostream>
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// The one and only application object

//CWinApp theApp;

using namespace std;

int main(int argc, char* argv[], char* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	//if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	//{
	//	// TODO: change error code to suit your needs
	//	cerr << _T("Fatal Error: MFC initialization failed") << endl;
	//	nRetCode = 1;
	//}
	//else
	//{
		// TODO: code your application's behavior here.
		FU_VECTOR fu_vec;
		
		cout <<endl;
		cout << "Check Check Generate begin." << endl;
		
		cout << "CMD: 1. Checksum_Generate.exe "<< endl;
		cout <<	"     2. Checksum_Generate.exe [Load Path]" << endl;
		cout <<	"     3. Checksum_Generate.exe [Load Path] [Ini Path]" << endl << endl;

		if (argc == 1) 
		{
			cout << (const char*)argv[0]<<(const char*)" "<<(const char*)fu_vec.GetAppDirectory().c_str() << endl;
			fu_vec.SetSrcDirectory (fu_vec.GetAppDirectory());
			fu_vec.SetDestDirectory (fu_vec.GetAppDirectory());
		}
		else if(argc == 2) 
		{
			cout << (const char*)argv[0]<<(const char*)" "<<(const char*)argv[1] << endl;
			fu_vec.SetSrcDirectory (argv[1]);
			fu_vec.SetDestDirectory (argv[1]);
		}
		else if(argc == 3) 
		{
			cout << (const char*)argv[0]<<(const char*)" "<<(const char*)argv[1] <<(const char*)" "<<(const char*)argv[2] << endl;
			fu_vec.SetSrcDirectory (argv[1]);
			fu_vec.SetDestDirectory (argv[2]);
		}
		else 
		{
			cout << "Error CMD: 1. Checksum_Generate.exe; "<< endl;
			cout <<	"           2. Checksum_Generate.exe [Load Path]" << endl;
			cout <<	"           3. Checksum_Generate.exe [Load Path] [Ini Path]" << endl << endl;
			return -1;

		}

        bool is_check_by_pkg(false);
		/*bool is_load_type_chosen(false);
		unsigned char check_mode(0);
		while(!is_load_type_chosen) {
			cout <<" Please choose rom load type(1 or 2): "<<endl
				<< " 1: Load rom file into buffer all in once; This is for MDT Now." <<endl
				<< " 2: Load rom file into buffer by packages. This is for FlashTool Now." << endl;
			std::cin >> check_mode;
			if ( '2' == check_mode) {
				is_check_by_pkg = true;
				is_load_type_chosen = true;
			} else if ( '1' == check_mode) {
				is_load_type_chosen = true;
			} else {
				continue;
			}
		}*/

        // In this version, always check by package
        is_check_by_pkg = true;

		cout << " Bin File     is From Path: "<< (const char*)fu_vec.GetSrcDirectory().c_str() << endl;
		cout <<	" CheckSum.ini is write to : "<< (const char*)fu_vec.GetDestDirectory().c_str() << endl;	
		
		fu_vec.FU_List_Init(is_check_by_pkg);
		
		fu_vec.SetIniDirectory (fu_vec.GetDestDirectory());
		fu_vec.Write_INI();  // write to  current tool folder

		cout << "Check Sum Write to ini File Finished." << endl << endl;

		//system("Pause");
	//}

	return nRetCode;
}

