/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   File_OP.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Nick Huang (mtk02183)
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * Jun 18 2009 mtk80306
 * [DUMA00120904] optimize the power button and nvram
 * 
 *
 * Apr 29 2009 mtk80306
 * [DUMA00116080] revise the customization of nvram
 * revise nvram customization
 *
 * Mar 21 2009 mtk80306
 * [DUMA00112158] fix the code convention.
 * fix the codeing convention.
 *
 * Mar 15 2009 mtk80306
 * [DUMA00111629] add camera nvram files
 * add the check of bin region.
 *
 * Feb 23 2009 mtk80306
 * [DUMA00109277] add meta _battery mode.
 * 
 *
 * Nov 24 2008 mbj08139
 * [DUMA00105099] create meta code
 * 
 *
 * Oct 29 2008 mbj08139
 * [DUMA00105099] create meta code
 * 
 *
 * 
 *
 *******************************************************************************/

//
// FILE_OP driver. 
//
#include <stdbool.h>

#ifndef __FILE_OP_H__
#define __FILE_OP_H__

//*****************************************************************************
//
//                          TTT Driver MACRO def
//
//*****************************************************************************

//-------------------------------------
// underlying COM port related defines
//-------------------------------------

//*****************************************************************************
//
//                          FILE_OP Driver data stru def
//
//*****************************************************************************

//*****************************************************************************
//
//                          FILE_OP Driver var def
//
//*****************************************************************************


//*****************************************************************************
//                          FILE_OP Driver General Functions Prototypes
//
//*****************************************************************************
/*
#ifdef _WIN32
#define LOGD(x)
#else
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "NVBACKUP"
#endif

#define NVBAK_LOG(...) \
    do { \
        LOGD(__VA_ARGS__); \
    } while (0)
*/
#define MAX_NAMESIZE  128
#define DATA_FLAG  (0xfecf) 

//#define MaxFileNum   936

typedef struct
{
    int NameSize;					//the size of file name
    int FielStartAddr;				//the file offset address in content block
    int Filesize;					//the size of nvram files
    char cFileName[MAX_NAMESIZE];	//the name of nvram file
} File_Title;

//the header in title block
typedef struct
{
    short int iApBootNum;			//the numbers of nvram file which will resotre ervery boot in ap side.
    short int iApCleanNum;			//the numbers of nvram file which will resotre when clean boot in ap side .
    short int iMdBootNum;			//the numbers of nvram file which will resotre ervery boot in modem side.    
    short int iMdCleanNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
    short int iMdImpntNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
    short int iMdCoreNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
    short int iMdDataNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
    //Add for second modem for MT658*
    short int iMd2BootNum;			//the numbers of nvram file which will resotre ervery boot in modem side.    
    short int iMd2CleanNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
    short int iMd2ImpntNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
    //End of Comment
#ifdef MTK_LTE_SUPPORT
	short int iMd5BootNum;			//the numbers of nvram file which will resotre ervery boot in modem side.    
	short int iMd5CleanNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
	short int iMd5ImpntNum;			//the numbers of nvram file which will resotre when clean boot in modem side.
#endif
#ifdef EVDO_DT_SUPPORT
    short int iViaNum;                //test for VIA
#endif
    int       iFileBufLen;			//the size of file content.
    int       BackupFlag;			//the flag of valid block
} File_Title_Header;

//the type of nvram file
typedef enum
{
    APBOOT=0,						//resotre ervery boot in ap side.
    APCLN,							//resotre when clean boot in ap side.
    MDBOOT,							//resotre ervery boot in modem side.
    MDCLN,							//resotre when clean boot in modem side.
    MDIMP,
    MDCOR,
    MDDATA,
    //Add for second modem for MT658*
    MD2BOOT,							//resotre ervery boot in modem side.
    MD2CLN,							//resotre when clean boot in modem side.
    MD2IMP,
    //End of Comment
#ifdef MTK_LTE_SUPPORT	
    //LTE support
    MD5BOOT,							//resotre ervery boot in modem side.
    MD5CLN,							//resotre when clean boot in modem side.
    MD5IMP,
#endif 
#ifdef EVDO_DT_SUPPORT
    VIA,
#endif
    ALL								//all files
} MetaData;

#ifndef bool
#define bool int
#define false 0
#define true 1
#endif

extern char *strDMFileFolderPath;

#ifdef __cplusplus
extern "C"
{
#endif

    /********************************************************************************
    //FUNCTION:
    //		FileOp_CreateNVMFolder
    //DESCRIPTION:
    //		this function is called to create the directory for ap side
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		TRUE is success, otherwise is fail
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
	bool FileOp_CreateNVMFolder(void );


    /********************************************************************************
    //FUNCTION:
    //		RestoreData
    //DESCRIPTION:
    //		this function is called to read the the information and content of nvram files in binregion and generate
    //		the file of title and content
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		TRUE is success, otherwise is fail
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    bool FileOp_RestoreData(MetaData eRestoreType);

    /********************************************************************************
    //FUNCTION:
    //		BackupData
    //DESCRIPTION:
    //		this function is called to read the the information and content of nvram files in binregion and generate
    //		the file of title and content
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		TRUE is success, otherwise is fail
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
	bool FileOp_BackupData(MetaData eBackupType);

    /********************************************************************************
    //FUNCTION:
    //		BackupAll
    //DESCRIPTION:
    //		this function is called to backup data from fat2 parittion to bin region
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		None
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    void FileOp_BackupAll(void );

    /********************************************************************************
    //FUNCTION:
    //		RestoreAll
    //DESCRIPTION:
    //		this function is called to restore all files from bin region to fat2 partition
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		None
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    void FileOp_RestoreAll(void );
    
    /********************************************************************************
    //FUNCTION:
    //		DeleteAll
    //DESCRIPTION:
    //		this function is called to delete all files in fat2 partition
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		None
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    void FileOp_DeleteAll(void );

    /********************************************************************************
    //FUNCTION:
    //		DeleteAll
    //DESCRIPTION:
    //		this function is called to delete all files in fat2 partition
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		None
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    void FileOp_DeleteRdebData(void );
	

    /********************************************************************************
    //FUNCTION:
    //		RestoreBootData
    //DESCRIPTION:
    //		this function is called to restore the APBOOT and MDBOOT file to FAT2 partition from binregion
    //
    //PARAMETERS:
    //		None
    //
    //RETURN VALUE:
    //		None
    //
    //DEPENDENCY:
    //		None
    //
    //GLOBALS AFFECTED
    //		None
    ********************************************************************************/
    void FileOp_RestoreBootData(void );

    bool FileOp_BackupAll_NvRam(void);
    bool FileOp_RestoreAll_NvRam(void);
    bool FileOp_RestoreFromBinRegion(bool bCleanBoot);
    bool FileOp_BackupToBinRegion_All();
	bool FileOp_RestoreFromBinRegion_ToFile();
	bool FileOp_CheckBackUpResult();
    
    bool FileOp_RecoveryData();
    bool FileOp_SetCleanBootFlag(bool bSetFlag);
    bool FileOp_GetCleanBootFlag(unsigned int * iCleanBootFlag);
    bool FileOp_RestoreFromFiles(int eBackupType);
    bool FileOp_BackupDataToFiles(int * iFileMask,bool bWorkForBinRegion);
    bool FileOp_RestoreData_All(void);
    bool FileOp_BackupData_Special(char * buffer, int count ,int mode);
    bool FileOp_CreateBinRegionBadBlockBitMap();
	bool FileOp_CmpBackupFileNum();
    
    bool FileOp_RestoreFromBinRegionForDM();
    bool FileOp_BackupToBinRegionForDM();
    bool FileOp_BackupToBinRegion_All_Ex(int value);
#ifdef __cplusplus
}
#endif

#endif /* __FILE_OP_H__ */

