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
 *   FT_fnc.c
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *    driver main function
 *
 * Author:
 * -------
 *   Lu.Zhang (MTK80251) 09/11/2009
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 09 05 2010 siyang.miao
 * [ALPS00003981] Add reboot feature in meta
 * .
 *
 * 09 02 2010 sean.cheng
 * [ALPS00003477] [Need Patch] [Volunteer Patch] ALPS.W10.34 migrate camera function from 2.1 to 2.2
 * .Turn the CCAP / CCT Meta function
 *
 * 08 28 2010 qiuhuan.zhao
 * [ALPS00123522] [GPS] Android 2.2 porting
 * GPS META and FM porting.
 *
 * 08 28 2010 chunhui.li
 * [ALPS00123709] [Bluetooth] meta mode check in
 * for META mode check in.
 *
 * 08 14 2010 chipeng.chang
 * [ALPS00003297] [Need Patch] [Volunteer Patch] android 2.2 migration
 * add audio meta ft main.
 *
 * 07 16 2010 siyang.miao
 * [ALPS00122025]TST/FT for G-Sensor calibration
 * .
 *
 * 04 24 2010 lu.zhang
 * [ALPS00005327]CCAP
 * [ALPS00005327] CCAP
 * .
 *
 * 04 24 2010 lu.zhang
 * [ALPS00005327]CCAP
 * .
 *
 * 03 19 2010 lu.zhang
 * [ALPS00004362]CCAP APIs
 * .
 *
 * 03 18 2010 lu.zhang
 * [ALPS00004362]CCAP APIs
 * for CCAP APIs
 *
 * 03 09 2010 ch.yeh
 * [ALPS00001276][BT]Migration to Android 2.1
 * [BT][meta]enable Bluetooth META function.
 *
 * 02 26 2010 lu.zhang
 * [ALPS00004332]Create META
 * .
 *
 * 01 20 2010 lu.zhang
 * [ALPS00004332]Create META
 * .
 *
 *
 *
 *******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "ft_main.h"
#include "FT_Cmd_Para.h"
#include "libfile_op.h"
#include "meta.h"
#include <cutils/properties.h>

#include <string.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include "tst_main.h"
#include "FT_Public.h"
#include <DfoDefines.h>

#include "hardware/ccci_intf.h"

// required META_DLL version
#define FT_REQUIRED_META_VER	0x03050002
#define	MAX_PATH				256

#define VERSION_FILE_PATH           "/system/build.prop"

#define RELEASE_SW_TOKEN            "ro.mediatek.version.release"
#define RELEASE_PLATFORM_TOKEN      "ro.mediatek.platform"
#define RELEASE_CHIP_TOKEN          "ro.mediatek.chip_ver"
#define RELEASE_PRODUCT_TOKEN       "ro.product.name"
#define RELEASE_BUILD_TIME_TOKEN    "ro.build.date"
#define RELEASE_BUILD_DISP_ID_TOKEN "ro.build.display.id"

#define FT_PREFIX   "FT: "
#define FT_LOG(fmt, arg ...) META_LOG(FT_PREFIX fmt, ##arg)
//#define FT_LOG(fmt, arg...) printf(FT_PREFIX fmt, ##arg)
#define BOOT_MODE_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"
#define BOOT_MODE_STR_LEN 1

#define COM_PORT_TYPE_FILE "/sys/bus/platform/drivers/meta_com_type_info/meta_com_type_info"
#define COM_PORT_TYPE_STR_LEN 1

static int dumpLogState = 0;
static int md5_device_note = -1;

/********************************************************************************
//FUNCTION:
//		FT_Module_Init
//DESCRIPTION:
//		this function is called to initial the FT module and other neccessary module.
//
//PARAMETERS:
//		None
//RETURN VALUE:
//		TRUE is success, otherwise is fail
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

int FT_Module_Init(int device_note)
{
    //cpu and pmic is comon module, we init these here.
    md5_device_note = device_note;
    dumpLogState = 0;
    return 1;
}


/********************************************************************************
//FUNCTION:
//		FT_Module_Deinit
//DESCRIPTION:
//		this function is called to releaes all meta module
//
//PARAMETERS:
//		None
//RETURN VALUE:
//		TRUE is success, otherwise is fail
//
//DEPENDENCY:
//		FT_Module_Init must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
int FT_Module_Deinit(void)
{
    //deinit all modules to release the system resource.

    //META_CCAP_deinit();
    return 1;
}
/********************************************************************************
//FUNCTION:
//		FT_MODEM_INFO_OP
//DESCRIPTION:
//		this function is called to get modem information
//
//PARAMETERS:
//		req:		[IN]	refers to the define of "FT_MODEM_REQ"
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

void FT_MODEM_INFO_OP(FT_MODEM_REQ *pLocalBuf, char *pft_PeerBuf, kal_int16 ft_peer_len)
{
	FT_MODEM_CNF ft_cnf;
	int modem_req=0;
	int modem_number=0;
	int active_modem_id=0;
	int modem_type=0;
	int fd = -1;
	int type_define=0;
	char dev_node[32] = {0};

	FT_LOG("[FTT_Drv:] FT_MODEM_INFO_OP ");
	memset(&ft_cnf, 0, sizeof(FT_MODEM_CNF));
	ft_cnf.status = META_FAILED;

	if(pLocalBuf->type == FT_MODEM_OP_QUERY_INFO)
	{
		type_define = 1;
		modem_number = FT_GetModemType(&active_modem_id,&modem_type);			

		ft_cnf.result.query_modem_info_cnf.modem_number = modem_number;
		ft_cnf.result.query_modem_info_cnf.modem_id = active_modem_id;

		ft_cnf.status = META_SUCCESS;
	}
	else if(pLocalBuf->type == FT_MODEM_OP_CAPABILITY_LIST)
	{
		type_define = 1;
		MODEM_CAPABILITY_LIST modem_capa;
		memset(&modem_capa, 0, sizeof(MODEM_CAPABILITY_LIST));
		FT_GetModemCapability(&modem_capa);
		memcpy(&ft_cnf.result.query_modem_cap_cnf,&modem_capa,sizeof(MODEM_CAPABILITY_LIST));
		ft_cnf.status = META_SUCCESS;
	}

#ifdef MTK_TLR_SUPPORT

	if(MTK_ENABLE_MD5)
	{			
		fd = md5_device_note;
	}
	else
	{		
		snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_META_IOCTL,MD_SYS1));
		fd = open(dev_node, O_RDWR|O_NOCTTY|O_NDELAY );		
	}

	if(fd < 0 && (pLocalBuf->type == FT_MODEM_OP_SET_MODEMTYPE || pLocalBuf->type == FT_MODEM_OP_GET_CURENTMODEMTYPE|| pLocalBuf->type == FT_MODEM_OP_QUERY_MDIMGTYPE))
	{
		ft_cnf.status = META_FAILED;
		FT_LOG("[FTT_Drv:]Open device note: %s fail %d",dev_node,fd );
		ft_cnf.header.id = pLocalBuf->header.id +1;
    	ft_cnf.header.token = pLocalBuf->header.token;
		ft_cnf.type = pLocalBuf->type;	
	
		WriteDataToPC(&ft_cnf, sizeof(FT_MODEM_CNF),NULL, 0);
		return;
	}
	

	if(pLocalBuf->type == FT_MODEM_OP_SET_MODEMTYPE)
	{
		type_define = 1;
		unsigned int modem_type = pLocalBuf->cmd.set_modem_type_req.modem_type;
		 
	
		if (0 == ioctl(fd, CCCI_IOC_RELOAD_MD_TYPE, &modem_type))
		{
			if (0 == ioctl(fd, CCCI_IOC_MD_RESET))
			{
				ft_cnf.status = META_SUCCESS;		
			}
			else
			{
				ft_cnf.status = META_FAILED;
				FT_LOG("[FTT_Drv:]ioctl CCCI_IOC_MD_RESET fail " );	
			}
		}
		else
		{
			ft_cnf.status = META_FAILED;
			FT_LOG("[FTT_Drv:]ioctl CCCI_IOC_RELOAD_MD_TYPE fail modem_type = %d", modem_type);	
		}
	}
	else if(pLocalBuf->type == FT_MODEM_OP_GET_CURENTMODEMTYPE)
	{
		unsigned int modem_type=0;
		type_define = 1;
		
		if (0 == ioctl(fd, CCCI_IOC_GET_MD_TYPE, &modem_type))
		{
			ft_cnf.status = META_SUCCESS;
			ft_cnf.result.get_currentmodem_type_cnf.current_modem_type = modem_type;	
			FT_LOG("[FTT_Drv:]ioctl CCCI_IOC_GET_MD_TYPE success modem_type = %d", modem_type);
		}
		else
		{
			ft_cnf.status = META_FAILED;
			FT_LOG("[FTT_Drv:]ioctl CCCI_IOC_GET_MD_TYPE fail");	
		}	
		
	}
	else if(pLocalBuf->type == FT_MODEM_OP_QUERY_MDIMGTYPE)
	{
		unsigned int mdimg_type[16]={0};
		type_define = 1;

		if (0 == ioctl(fd, CCCI_IOC_GET_MD_IMG_EXIST, &mdimg_type))
		{
			ft_cnf.status = META_SUCCESS;
			memcpy(ft_cnf.result.query_modem_imgtype_cnf.mdimg_type,mdimg_type,16*sizeof(unsigned int));

			for(int i = 0;i<16;i++)
			{
				FT_LOG("mdimg_type[%d] %d",i, mdimg_type[i]);	
			}
				
		}
		else
		{
			ft_cnf.status = META_FAILED;
			FT_LOG("[FTT_Drv:]ioctl CCCI_IOC_GET_MD_IMG_EXIST fail");	
		}	
	}

	if(!MTK_ENABLE_MD5)
	{
		close(fd);
	}
#endif

	if(type_define == 0)
	{
		FT_LOG("[FTT_Drv:]FT_MODEM_REQ have no this type %d",pLocalBuf->type );	
	}
	
	ft_cnf.header.id = pLocalBuf->header.id +1;
    ft_cnf.header.token = pLocalBuf->header.token;
	ft_cnf.type = pLocalBuf->type;	
	
	WriteDataToPC(&ft_cnf, sizeof(FT_MODEM_CNF),NULL, 0);
}


/********************************************************************************
//FUNCTION:
//		FT_TestAlive
//DESCRIPTION:
//		this function is called to tell pc the ft module is alive
//
//PARAMETERS:
//		req:		[IN]	refers to the define of "FT_IS_ALIVE_REQ"
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_TestAlive(FT_IS_ALIVE_REQ *req)
{
    FT_IS_ALIVE_CNF ft_cnf;

    FT_LOG("[FTT_Drv:] FT_TestAlive ");

    memset(&ft_cnf, 0, sizeof(FT_IS_ALIVE_CNF));
    //just give the respone.
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    WriteDataToPC(&ft_cnf, sizeof(FT_IS_ALIVE_CNF),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_GetVersionInfo
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_VER_INFO_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_GetVersionInfo(FT_VER_INFO_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len)
{
    FT_VER_INFO_CNF ft_cnf;
	unsigned int dwRc ;
	unsigned int dwValSize;
	unsigned int dwValType;
	char szBuffer[MAX_PATH];
    FILE *fd = 0;
    char str[256] = {0};
    char *loc = NULL;

    memset(&ft_cnf, 0, sizeof(ft_cnf));
    memset(szBuffer,0, sizeof(szBuffer));
	char* tmp = NULL;
	char platform[256] = {0};
	char chipVersion[256] = {0};

    //initail the value of ft header
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.status = META_FAILED;

	FT_LOG("[FTT_Drv:] FT_GetVersionInfo ");

    if((fd = fopen(VERSION_FILE_PATH,"r"))==NULL)
    {
        FT_LOG("FT_GetVersionInfo Can't open file : %s\n", VERSION_FILE_PATH);
    }

    while(!feof(fd))
    {
        if(fgets(str, 256, fd)!=NULL)
        {
            tmp = str;
            loc = strsep(&tmp, "=");
            if(!strcmp(loc, RELEASE_SW_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] SW Version = %s\n", tmp);
                strncpy((char*)ft_cnf.sw_ver, tmp, 63);
            }
            if(!strcmp(loc, RELEASE_PLATFORM_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Platform = %s\n", tmp);
				strncpy(platform, tmp, 255);
            }
            if(!strcmp(loc, RELEASE_PRODUCT_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Product Name = %s\n", tmp);
            }
            if(!strcmp(loc, RELEASE_CHIP_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Chip Version = %s\n", tmp);
                strncpy(chipVersion, tmp, 255);
                strncpy((char*)ft_cnf.hw_ver, tmp, 63);
            }
            if(!strcmp(loc, RELEASE_BUILD_TIME_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Build Time = %s\n", tmp);
                strncpy((char*)ft_cnf.sw_time, tmp, 63);
            }
        }
    }

	int i = 0;
        int k = 0;
	while (i < 256)
	{
		if (platform[i] != '\r' && platform[i] != '\n')
			szBuffer[k++] = platform[i++];				
		else
			break;
	}
	szBuffer[k++] = ',';
	i = 0;
	while (i < 256)
	{
		if (chipVersion[i] != '\r' && chipVersion[i] != '\n')
			szBuffer[k++] = chipVersion[i++];
		else
			break;
	}
	szBuffer[k++] = '\0';
	if (strlen(szBuffer) <= 64)
	{
		strcpy((char*)ft_cnf.bb_chip, szBuffer);
	}
	else
	{
		FT_LOG("[FTT_Drv:] String is too long, length=%d ", strlen(szBuffer));
	}
    fclose(fd);

	/* Get Software version : ft_cnf.sw_ver */
	FT_LOG("[FTT_Drv:] ft_cnf.sw_ver = %s ", ft_cnf.sw_ver);

	/* Get the build time : ft_cnf.sw_ver */
	FT_LOG("[FTT_Drv:] ft_cnf.sw_time = %s ", ft_cnf.sw_time);

	/* Get the chip version : ft_cnf.sw_ver */
	FT_LOG("[FTT_Drv:] ft_cnf.bb_chip = %s ", ft_cnf.bb_chip);
	FT_LOG("[FTT_Drv:] ft_cnf.hw_ver = %s ", ft_cnf.hw_ver);

    ft_cnf.status = META_SUCCESS;

Ver_error:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		Meta_Mobile_Log
//DESCRIPTION:
//		this function is called to send stop command to mobilelog if eng build
//
//PARAMETERS:
//		None
//RETURN VALUE:
//		BOOL: if success the return value is TRUE, else it is FALSE.
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
BOOL Meta_Mobile_Log()
{        
    int fd = 0;
	int len = 0;
	BOOL ret = FALSE;
	char tempstr[5]={0};
	FT_LOG("[FTT_Drv:] Meta_Mobile_Log ");

    //support end load and user load,send stop command to mobilelog 

	fd = socket_local_client("mobilelogd", ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
	if (fd < 0) 
	{
		FT_LOG("socket fd <0 ");
		return FALSE;
	}
	FT_LOG("socket ok\n");
	if((len = write(fd, "stop", sizeof("stop"))) < 0)
	{
		FT_LOG("socket write error!");
		ret = FALSE;
	}
	else
	{
		FT_LOG("write %d Bytes.", len);
		ret = TRUE;
	}
	close(fd);
	sleep(4);	      
	return ret;
	
}



/********************************************************************************
//FUNCTION:
//		FT_PowerOff
//DESCRIPTION:
//		this function is called to power off target
//
//PARAMETERS:
//		None
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

void FT_PowerOff(FT_POWER_OFF_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len )
{
    //MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_PowerOff ")));
    FT_LOG("[FTT_Drv:] FT_PowerOff ");

    //power off target side after finishing the meta
    //KernelIoControl(IOCTL_HAL_POWERDOWN, NULL, NULL, NULL, 0, NULL);
	sync();
    usleep(200*1000);

    int comPortType;
    comPortType = getComportType();
    if(comPortType == META_USB_COM)
    {

	    FILE *PUsbFile = NULL;
        PUsbFile = fopen("sys/devices/platform/mt_usb/cmode","w");
	    if(PUsbFile == NULL)
	    {
            // printf("Could not open sys/devices/platform/mt_usb/cmode: %s",strerror(errno));
            FT_LOG("[FTT_Drv:] Could not open sys/devices/platform/mt_usb/cmode ");
	    }
	    else
	    {
            fputc('0',PUsbFile);
			fclose(PUsbFile);
	    }
	
    }
    else
    {
//        FT_LOG("[FTT_Drv:] com port type is uart! ");
    }
    if(pFTReq->dummy == 0)
    {
        usleep(1000 * 1000);
	    reboot(RB_POWER_OFF);
    }
    if(pFTReq->dummy == 2)
    {
	    usleep(1000 * 1000);
	    reboot(RB_AUTOBOOT);
    }
}


/********************************************************************************
//FUNCTION:
//		FT_CheckMetaDllVersion
//DESCRIPTION:
//		this function is called to check dll version with meta dll. this function is reserved now.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_CHECK_META_VER_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
/*
void FT_CheckMetaDllVersion(FT_CHECK_META_VER_REQ  *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{

    FT_CHECK_META_VER_CNF	ft_cnf;
    memset(&ft_cnf, 0, sizeof(ft_cnf));

    //initail the value of ft header
    ft_cnf.header.id = pFTReq->header.id +1;;
    ft_cnf.header.token = pFTReq->header.token;
    ft_cnf.status= META_SUCCESS;

    //if the version is not right ,just give the error information
    if ( ((pFTReq->meta_ver_from_pc&0xFFFF0000) != (FT_REQUIRED_META_VER&0xFFFF0000)) ||
            ((pFTReq->meta_ver_from_pc&0x0000FFFF) <  (FT_REQUIRED_META_VER&0x0000FFFF)))
    {
        MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_CheckMetaDllVersion is not right ")));
    }

    ///ft_cnf.header.ft_msg_id = FT_CHECK_META_VER_CNF_ID;
    ft_cnf.meta_ver_required_by_target = FT_REQUIRED_META_VER;

    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}
*/


/********************************************************************************
//FUNCTION:
//		FT_CPURegW_OP
//DESCRIPTION:
//		this function is called to write the value of Chip registor to chip
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_REG_WRITE_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

void FT_CPURegW_OP(FT_REG_WRITE_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_REG_WRITE_CNF CpuWriteCnf;
    FT_LOG("[FTT_Drv:] FT_CPURegW_OP META Test ");

    memset(&CpuWriteCnf, 0, sizeof(FT_REG_WRITE_CNF));

    //write to the chip
    CpuWriteCnf = META_CPURegW_OP(pFTReq);

    //initail the value of ft header
    CpuWriteCnf.header.id = pFTReq->header.id +1;
    CpuWriteCnf.header.token = pFTReq->header.token;
    CpuWriteCnf.status = META_SUCCESS;

    WriteDataToPC(&CpuWriteCnf, sizeof(FT_REG_WRITE_CNF),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_CPURegR_OP
//DESCRIPTION:
//		this function is called to read the value of Chip registor
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_REG_READ_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_CPURegR_OP(FT_REG_READ_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{

    FT_REG_READ_CNF CpuReadCnf;

    FT_LOG("[FTT_Drv:] FT_CPURegR_OP META Test: %d ", sizeof(FT_REG_READ_CNF) );
    memset(&CpuReadCnf, 0, sizeof(FT_REG_READ_CNF));

    //read from chip
    CpuReadCnf = META_CPURegR_OP(pFTReq);
    //initail the value of ft header
    CpuReadCnf.header.id = pFTReq->header.id +1;
    CpuReadCnf.header.token = pFTReq->header.token;

    CpuReadCnf.status= META_SUCCESS;

    WriteDataToPC(&CpuReadCnf, sizeof(FT_REG_READ_CNF),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_PMICRegR_OP
//DESCRIPTION:
//		this function is called to read the value of PMIC chip registor
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_PMIC_REG_READ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_PMICRegR_OP(FT_PMIC_REG_READ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_PMIC_REG_READ_CNF ft_cnf;

    //MetaLogMsg(g_bFTLogEnable, (TEXT("[FTT_Drv:] FT_PMICRegR_OP META Test ")));
    FT_LOG("[FTT_Drv:] FT_PMICRegR_OP META Test ");
    memset(&ft_cnf, 0, sizeof(FT_PMIC_REG_READ_CNF));

    //read from meta_pmic moudle
    //ft_cnf = META_PMICR_OP(pFTReq);
    //initial the ft module header
    ft_cnf.header.id = pFTReq->header.id +1;;
    ft_cnf.header.token = pFTReq->header.token;
    ft_cnf.status= META_SUCCESS;

    WriteDataToPC(&ft_cnf, sizeof(FT_PMIC_REG_READ_CNF),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_PMICRegW_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_PMIC_REG_WRITE"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_PMICRegW_OP(FT_PMIC_REG_WRITE *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_PMIC_REG_WRITE_CNF ft_cnf;

    //MetaLogMsg(g_bFTLogEnable, (TEXT("[FTT_Drv:] FT_PMICRegW_OP META Test ")));
    FT_LOG("[FTT_Drv:] FT_PMICRegW_OP META Test ");
    memset(&ft_cnf, 0, sizeof(FT_PMIC_REG_WRITE_CNF));

    ///write meta_pmic moudle
    //ft_cnf = META_PMICW_OP(pFTReq);  ///some error in meta_pmic lib
    ////initial the ft module header
    ft_cnf.header.id = pFTReq->header.id +1;;
    ft_cnf.header.token = pFTReq->header.token;
    ft_cnf.status= META_SUCCESS;

    WriteDataToPC(&ft_cnf, sizeof(FT_PMIC_REG_WRITE_CNF),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_UtilCheckIfFuncExist
//DESCRIPTION:
//		this function is called to which module are suppoted in target side.
//
//PARAMETERS:
//		req:			[IN]		refers to the define of "FT_UTILITY_COMMAND_REQ"
//		cnf: 		[OUT]	refers to the define of "FT_UTILITY_COMMAND_REQ"
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_UtilCheckIfFuncExist(FT_UTILITY_COMMAND_REQ  *req, FT_UTILITY_COMMAND_CNF  *cnf)
{

    kal_uint32	query_ft_msg_id = req->cmd.CheckIfFuncExist.query_ft_msg_id;
    kal_uint32	query_op_code = req->cmd.CheckIfFuncExist.query_op_code;
    FT_LOG("[FTT_Drv:] FT_UtilCheckIfFuncExist META Test ");
    cnf->status = FT_CNF_FAIL;

	FT_LOG("request id = %d op = %d",query_ft_msg_id,query_op_code);
    switch (query_ft_msg_id)
    {

#ifdef FT_FM_FEATURE 
    	case FT_FM_REQ_ID:
			if(query_op_code == 0)//FT_FM_OP_READ_CHIP_ID
			{
				cnf->status = FT_CNF_OK; 
			}        	
        	break;
#endif

#ifdef FT_EMMC_FEATURE
		case FT_CRYPTFS_REQ_ID:
			if(query_op_code == 0)//FT_CRYPTFS_OP_QUERYSUPPORT
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == 1)//FT_CRYPTFS_OP_VERITIFY
			{
				cnf->status = FT_CNF_OK;
			}
        	break;
#endif
			
		case FT_MODEM_REQ_ID:
			if(query_op_code == FT_MODEM_OP_QUERY_INFO )
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_CAPABILITY_LIST)
			{
				cnf->status = FT_CNF_OK;
			}
#ifdef MTK_TLR_SUPPORT
			else if(query_op_code == FT_MODEM_OP_SET_MODEMTYPE)
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_GET_CURENTMODEMTYPE)
			{
				cnf->status = FT_CNF_OK;
			}
			else if(query_op_code == FT_MODEM_OP_QUERY_MDIMGTYPE )
			{
				cnf->status = FT_CNF_OK;
			}
#endif
			break;
    	default:
			FT_LOG("[FTT_Drv:] NOT FOUND THE PRIMITIVE_ID");
        	cnf->status = FT_CNF_FAIL;		
        	break;
    }
    

    // assign return structure
    cnf->result.CheckIfFuncExist.query_ft_msg_id = query_ft_msg_id;
    cnf->result.CheckIfFuncExist.query_op_code = query_op_code;
}


/********************************************************************************
//FUNCTION:
//		FT_Peripheral_OP
//DESCRIPTION:
//		this function is called to do the peripheral related module test: lcd backlight, keypad backlight,
//		nled, vibrate, query single flash featuure and RTC, and set clean boot flag.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_UTILITY_COMMAND_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_Peripheral_OP(FT_UTILITY_COMMAND_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_UTILITY_COMMAND_CNF UtilityCnf;
    //PROCESS_INFORMATION cleanBootProcInfo;
    static META_BOOL bLCDBKInitFlag_Peri = FALSE;
    static META_BOOL bLCDFtInitFlag_Peri = FALSE;
    static META_BOOL bVibratorInitFlag_Peri = FALSE;
	int nNVRAMFlag = 0;

    //cleanBootProcInfo.hProcess = NULL;
    //cleanBootProcInfo.hThread = NULL;

    FT_LOG("[FTT_Drv:] FT_Peripheral_OP META Test ");

    memset(&UtilityCnf, 0, sizeof(FT_UTILITY_COMMAND_CNF));

    UtilityCnf.header.id = pFTReq->header.id +1;
    UtilityCnf.header.token = pFTReq->header.token;
    UtilityCnf.type = pFTReq->type;
    UtilityCnf.status= META_FAILED;

    //do the related test.
    switch (pFTReq->type)
    {
    case FT_UTILCMD_CHECK_IF_FUNC_EXIST:		//query the supported modules
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_CHECK_IF_FUNC_EXIST ");
        FT_UtilCheckIfFuncExist(pFTReq, &UtilityCnf);
        break;

    case FT_UTILCMD_QUERY_LOCAL_TIME:			//query RTC from meta cpu lib
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_QUERY_LOCAL_TIME ");
        UtilityCnf.result.m_WatchDogCnf= META_RTCRead_OP(pFTReq->cmd.m_WatchDogReq);
        UtilityCnf.status= META_SUCCESS;

        break;

    case FT_UTILCMD_MAIN_SUB_LCD_LIGHT_LEVEL:	//test lcd backlight from meta lcd backlight lig
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_MAIN_SUB_LCD_LIGHT_LEVEL ");
	    if(bLCDBKInitFlag_Peri==FALSE)
     	{   		
			if (!Meta_LCDBK_Init())
			{
				FT_LOG("[FTT_Drv:] FT_Peripheral_OP Meta_LCDBK_Init Fail ");
				goto Per_Exit;
			}   		
		    bLCDBKInitFlag_Peri = TRUE;
    	}        
        UtilityCnf.result.m_LCDCnf = Meta_LCDBK_OP(pFTReq->cmd.m_LCDReq);
        UtilityCnf.status= META_SUCCESS;
        break;

	case FT_UTILCMD_LCD_COLOR_TEST:
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_LCD_COLOR_TEST ");
	    if(bLCDFtInitFlag_Peri==FALSE)
     	{   					
			if (!Meta_LCDFt_Init())
			{
			    FT_LOG("[FTT_Drv:] FT_Peripheral_OP Meta_LCDFt_Init Fail ");
			    goto Per_Exit;
			}			
		    bLCDFtInitFlag_Peri = TRUE;
    	}        
        UtilityCnf.result.m_LCDColorTestCNF = Meta_LCDFt_OP(pFTReq->cmd.m_LCDColorTestReq);
        UtilityCnf.status= META_SUCCESS;
        break;

    case FT_UTILCMD_SIGNAL_INDICATOR_ONOFF:
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SIGNAL_INDICATOR_ONOFF ");
        UtilityCnf.result.m_NLEDCnf = Meta_Vibrator_OP(pFTReq->cmd.m_NLEDReq);
        UtilityCnf.status= META_SUCCESS;
        break;

    case FT_UTILCMD_VIBRATOR_ONOFF:				//test vibrate and indicator from meta nled lib
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_VIBRATOR_ONOFF ");
	    if(bVibratorInitFlag_Peri==FALSE)
     	{
    		if (!Meta_Vibrator_Init())
        	{
            	FT_LOG("[FTT_Drv:] FT_Peripheral_OP Meta_Vibrator_Init Fail ");
            	goto Per_Exit;
        	}	
		    bVibratorInitFlag_Peri = TRUE;
    	}         
        UtilityCnf.result.m_NLEDCnf = Meta_Vibrator_OP(pFTReq->cmd.m_NLEDReq);
        UtilityCnf.status= META_SUCCESS;
        break;

    case FT_UTILCMD_KEYPAD_LED_ONOFF:	
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_KEYPAD_LED_ONOFF ");
        UtilityCnf.result.m_NLEDCnf = Meta_Vibrator_OP(pFTReq->cmd.m_NLEDReq);
        UtilityCnf.status= META_SUCCESS;
        break;

    case FT_UTILCMD_SET_CLEAN_BOOT_FLAG:		
        nNVRAMFlag = pFTReq->cmd.m_SetCleanBootFlagReq.Notused;		
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SET_CLEAN_BOOT_FLAG, nNVRAMFlag =%d",nNVRAMFlag);
        FT_LOG("[FTT_Drv:] FT_UTILCMD_SET_CLEAN_BOOT_FLAG META Test %s,%d,%s",__FILE__,__LINE__,__FUNCTION__);	
		if ( nNVRAMFlag == 1 || nNVRAMFlag == 2 || nNVRAMFlag == 3 ) //For NVRAM to record write barcode(1) and IMEI(2) and both barcode and IMEI(3) history
        {
		    UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = FileOp_BackupToBinRegion_All_Ex(nNVRAMFlag);  
		}
		else //default
        {
            UtilityCnf.result.m_SetCleanBootFlagCnf.drv_statsu = FileOp_BackupToBinRegion_All(); 
        }
		UtilityCnf.status=META_SUCCESS;
        break;

    case FT_UTILCMD_CHECK_IF_LOW_COST_SINGLE_BANK_FLASH:	//query the single flash feature, we now just return.
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_CHECK_IF_LOW_COST_SINGLE_BANK_FLASH ");
        UtilityCnf.status=META_SUCCESS;
        break;

    case FT_UTILCMD_SAVE_MOBILE_LOG:                           //save mobile log
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_SAVE_MOBILE_LOG ");
        FT_LOG("[FTT_Drv:] FT_UTILCMD_SAVE_MOBILE_LOG META Test %s,%d,%s",__FILE__,__LINE__,__FUNCTION__);
        UtilityCnf.result.m_SaveMobileLogCnf.drv_status = Meta_Mobile_Log();
        UtilityCnf.status = META_SUCCESS;
		break;
   case FT_UTILCMD_OPEN_DUMP_LOG:                          
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type is FT_UTILCMD_OPEN_DUMP_LOG ");
        FT_LOG("[FTT_Drv:] FT_UTILCMD_OPEN_DUMP_LOG META Test %s,%d,%s",__FILE__,__LINE__,__FUNCTION__);
		dumpLogState = 1;
        UtilityCnf.result.m_OpenDumpLogCnf.drv_status = true;
        UtilityCnf.status = META_SUCCESS;
		break;

    default:
        FT_LOG("[FTT_Drv:] FT_Peripheral_OP pFTReq->type error ");
        UtilityCnf.status= META_FAILED;
        break;

    }

Per_Exit:
    WriteDataToPC(&UtilityCnf, sizeof(FT_UTILITY_COMMAND_CNF),NULL, 0);

}

/********************************************************************************
//FUNCTION:
//		FT_APEditorRead_OP
//DESCRIPTION:
//		this function is called to read a record of nvram file.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_AP_Editor_read_req"
//
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_APEditorRead_OP(FT_AP_Editor_read_req *pFTReq)
{
    FT_LOG("[FTT_Drv:] FT_APEditorRead_OP META Test ");

    // just call the inferface of ap_editor lib which will reture the data after reading sucessfully
    if (!META_Editor_ReadFile_OP(pFTReq))
        FT_LOG("[FTT_Drv:] FT_APEditorR_OP META Test Fail");
}

/********************************************************************************
//FUNCTION:
//		FT_APEditorWrite_OP
//DESCRIPTION:
//		this function is called to write a record of nvram file to flash
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_AP_Editor_write_req"
//		pft_PeerBuf: 	[IN]	peer buff (the data of record)
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_APEditorWrite_OP(FT_AP_Editor_write_req *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_AP_Editor_write_cnf ft_cnf;

    FT_LOG("[FTT_Drv:] FT_APEditorW_OP META Test ");
    memset(&ft_cnf, 0, sizeof(FT_AP_Editor_write_cnf));

    //// just call the inferface of ap_editor lib
    ft_cnf = META_Editor_WriteFile_OP(pFTReq, pPeerBuf, peer_len);

    //fill the ft module header
    ft_cnf.header.id = pFTReq->header.id +1;
    ft_cnf.header.token = pFTReq->header.token;
    ft_cnf.status = META_SUCCESS;

    WriteDataToPC(&ft_cnf, sizeof(FT_AP_Editor_write_cnf),NULL, 0);

}

/********************************************************************************
//FUNCTION:
//		FT_APEditorReset_OP
//DESCRIPTION:
//		this function is called to reset a nvram file to default value.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_VER_INFO_REQ"
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
void FT_APEditorReset_OP(FT_AP_Editor_reset_req *pFTReq)
{
    FT_AP_Editor_reset_cnf ft_cnf;

    FT_LOG("[FTT_Drv:] FT_APEditorReset_OP META Test ");
    memset(&ft_cnf, 0, sizeof(FT_AP_Editor_reset_cnf));

    //if the reset_category and file_idx is 0xfc and 0xfccf, we reset all nvram files.
    if ((pFTReq->reset_category == 0xfc )&& (pFTReq->file_idx ==0xfccf))
        ft_cnf = META_Editor_ResetAllFile_OP(pFTReq);	//reset all files
    else
        ft_cnf = META_Editor_ResetFile_OP(pFTReq);		//reset one nvram file

    //fill the ft module header
    ft_cnf.header.id = pFTReq->header.id +1;
    ft_cnf.header.token = pFTReq->header.token;
    ft_cnf.status = META_SUCCESS;

    WriteDataToPC(&ft_cnf, sizeof(FT_AP_Editor_reset_cnf),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_BT_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "BT_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

#ifdef FT_BT_FEATURE  

void FT_BT_OP(BT_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
#if 0  //Modify the BT init/deinit flow
    BT_CNF ft_cnf;
    static META_BOOL bInitFlag_BT = FALSE;

    memset(&ft_cnf, 0, sizeof(BT_CNF));

    if (FALSE == bInitFlag_BT)
    {
        // initial the bt module when it is called first time
        if (!META_BT_init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.op= pFTReq->op;
            ft_cnf.status = META_FAILED;

            //MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_BT_OP META_BT_init Fail ")));
            goto BT_Exit;
        }
        bInitFlag_BT = TRUE;
    }
#endif

    //MetaLogMsg(g_bFTLogEnable, (TEXT("[FTT_Drv:] FT_BT_OP META Test req: %d , %d "), sizeof(BT_REQ), sizeof(BT_CNF)));
    //do the bt test by called the interface in meta bt lib
    FT_LOG("[FTT_Drv:] FT_BT_OP META Test ");

    META_BT_OP(pFTReq, pPeerBuf, peer_len);
    return;

#if 0  //Modify the BT init/deinit flow
BT_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
#endif

}

#endif


/********************************************************************************
//FUNCTION:
//		FT_FM_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "BT_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

#ifdef FT_FM_FEATURE 

void FT_FM_OP(FM_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FM_CNF ft_cnf;
    static META_BOOL bInitFlag_FM = FALSE;

    FT_LOG("[FTT_Drv:] FT_FM_OP META Test ");
    memset(&ft_cnf, 0, sizeof(FM_CNF));

    if (FALSE == bInitFlag_FM)
    {
        // initial the bt module when it is called first time
        if (!META_FM_init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.op= pFTReq->op;
            ft_cnf.status = META_FAILED;

            //RETAILMSG(FTT_DBG, (TEXT("[FTT_Drv:] FT_BT_OP META_BT_init Fail ")));
            goto FM_Exit;
        }
        bInitFlag_FM = TRUE;
    }

    //RETAILMSG(g_bFTLogEnable, (TEXT("[FTT_Drv:] FT_FM_OP META Test req: %d , %d "), sizeof(FM_REQ), sizeof(FM_CNF)));
    //do the bt test by called the interface in meta bt lib
    META_FM_OP(pFTReq, pPeerBuf, peer_len);
    return;

FM_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
}

#endif
/********************************************************************************
//FUNCTION:
//		FT_DVBT_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_DVB_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
/*
void FT_DVBT_OP(FT_DVB_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_DVB_CNF ft_cnf;
    static META_BOOL bInitFlag_DBT = FALSE;

    memset(&ft_cnf, 0, sizeof(FT_DVB_CNF));

    if (FALSE == bInitFlag_DBT)
    {
        // initial the DVB module when it is called first time
        if (!META_DVB_Init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.type = pFTReq->type;
            ft_cnf.status = META_FAILED;

            MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_DVBT_OP META_DVB_Init Fail ")));
            goto DVBT_Exit ;
        }
        bInitFlag_DBT = TRUE;
    }
    MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_DVBT_OPMETA Test req: %d , %d "), sizeof(FT_DVB_REQ), sizeof(FT_DVB_CNF)));

    META_DVB_T_OP(pFTReq, (PVOID *)pPeerBuf, peer_len);
    return;

DVBT_Exit:

    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}
*/

/********************************************************************************
//FUNCTION:
//		FT_BAT_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_BATT_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
/*
void FT_BAT_OP(FT_BATT_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_BATT_CNF ft_cnf;
    static META_BOOL bInitFlag_BAT = FALSE;

    memset(&ft_cnf, 0, sizeof(FT_BATT_CNF));

    if (FALSE == bInitFlag_BAT)
    {
        // initial the bat module when it is called first time
        if (!Meta_Battery_Init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.type = pFTReq->type;
            ft_cnf.status = META_FAILED;

            //MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_BAT_OP Meta_Battery_Init Fail ")));
            FT_LOG("[FTT_Drv:] FT_BAT_OP Meta_Battery_Init Fail ");
            goto  BAT_Exit;
        }
        bInitFlag_BAT = TRUE;
    }

    // //do the bat test by called the interface in meta bat lib
    Meta_Battery_OP(pFTReq, (BYTE *)pPeerBuf, peer_len);

    return;

BAT_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}
*/

/********************************************************************************
//FUNCTION:
//		FT_AUXADC_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "AUXADC_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_AUXADC_OP(AUXADC_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    AUXADC_CNF ft_cnf;
    static META_BOOL bInitFlag_ADC = FALSE;

    FT_LOG("[FTT_Drv:] FT_AUXADC_OP META Test ");
    memset(&ft_cnf, 0, sizeof(AUXADC_CNF));

    if (FALSE == bInitFlag_ADC)
    {
        // initial the adc module when it is called first time
        if (!Meta_AUXADC_Init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.status = META_FAILED;

            goto  ADC_Exit;
        }
        bInitFlag_ADC = TRUE;
    }

    //do the adc test by called the interface in meta adc lib
    Meta_AUXADC_OP(pFTReq, pPeerBuf, peer_len);

    return;

ADC_Exit:
    //MetaLogMsg(g_bFTLogEnable, (TEXT("[FTT_Drv:] FT_AUXADC_OP Test req: %d , %d "), sizeof(AUXADC_REQ), sizeof(AUXADC_CNF)));
    FT_LOG("[FTT_Drv:] FT_AUXADC_OP Test req: %d , %d ", sizeof(AUXADC_REQ), sizeof(AUXADC_CNF));
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_WIFI_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_WM_WIFI_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
#ifdef FT_WIFI_FEATURE 

void FT_WIFI_OP(FT_WM_WIFI_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
#if 0  //Modify the wifi init/deinit flow
    FT_WM_WIFI_CNF ft_cnf;
    static META_BOOL bInitFlag_wifi = FALSE;

	FT_LOG("[FTT_Drv:] FT_WIFI_OP ");

    if (FALSE == bInitFlag_wifi)
    {
        // initial the wifi module when it is called first time
        if (!META_WIFI_init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.status = META_FAILED;

            goto  WIFI_Exit;
        }
        bInitFlag_wifi = TRUE;
    }
#endif

    //do the Wifi test by called the interface in meta wifi lib
    FT_LOG("[FTT_Drv:] FT_WIFI_OP ");
    META_WIFI_OP(pFTReq, pPeerBuf, peer_len);
    return;

#if 0  //Modify the wifi init/deinit flow
WIFI_Exit:
    FT_LOG("[FTT_Drv:] FT_WIFI_OP Test Fail: %d , %d ", sizeof(FT_WM_WIFI_REQ), sizeof(FT_WM_WIFI_CNF));
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
#endif

}

#endif


/********************************************************************************
//FUNCTION:
//		FT_GPS_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "GPS_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
#ifdef FT_GPS_FEATURE 

void FT_GPS_OP(GPS_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_GPS_OP Test %d , %d ", sizeof(GPS_REQ), sizeof(GPS_CNF));
	META_GPS_OP(pFTReq, pPeerBuf, peer_len);

}

#endif



/********************************************************************************
//FUNCTION:
//		FT_NFC_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "GPS_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

#ifdef FT_NFC_FEATURE

void FT_NFC_OP(NFC_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
      // Call META NFC Init
     NFC_CNF ft_cnf;
     static META_BOOL bInitFlag_nfc = FALSE;

     FT_LOG("[FTT_Drv:] FT_NFC_OP ");

     if (FALSE == bInitFlag_nfc)
     {
        // Run nfc service process
         if (META_NFC_init() != 0)
         {
              ft_cnf.header.id = pFTReq->header.id +1;
              ft_cnf.header.token = pFTReq->header.token;
              ft_cnf.status = META_FAILED;
              FT_LOG("[FTT_Drv:] FT_NFC_OP Test Fail: %d , %d ", sizeof(NFC_REQ), sizeof(NFC_REQ));
              WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

	       META_NFC_deinit();
              return;
         }
         bInitFlag_nfc = TRUE;
     }
     FT_LOG("[FTT_Drv:] FT_NFC_OP Test %d , %d ", sizeof(NFC_REQ), sizeof(NFC_CNF));
     META_NFC_OP(pFTReq, pPeerBuf, peer_len);
     return;
}
#endif

/********************************************************************************
//FUNCTION:
//		FT_BAT_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_BATT_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_BAT_ChipUpdate_OP(FT_BATT_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_BATT_CNF ft_cnf;
    static META_BOOL bInitFlag_BAT = FALSE;

    FT_LOG("[FTT_Drv:] FT_BAT_ChipUpdate_OP META Test ");
    memset(&ft_cnf, 0, sizeof(FT_BATT_CNF));

    if (FALSE == bInitFlag_BAT)
    {
        // initial the bat module when it is called first time
        if (!Meta_Battery_Init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.type = pFTReq->type;
            ft_cnf.status = META_FAILED;

            //MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_BAT_OP Meta_Battery_Init Fail ")));
			FT_LOG("[FTT_Drv:] FT_BAT_OP Meta_Battery_Init Fail ");
            goto  BAT_Exit;
        }
        bInitFlag_BAT = TRUE;
    }

    // //do the bat test by called the interface in meta bat lib
    Meta_Battery_UPdate_FW(pFTReq, (BYTE *)pPeerBuf, peer_len);

    return;

BAT_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

void FT_BAT_FW_OP(FT_BATT_READ_INFO_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_BATT_READ_INFO_CNF ft_cnf;
	static META_BOOL bInitFlag_BAT = FALSE;

    FT_LOG("[FTT_Drv:] FT_BAT_FW_OP META Test ");
	memset(&ft_cnf, 0, sizeof(FT_BATT_CNF));

	if(FALSE == bInitFlag_BAT)
	{
		if(!Meta_Battery_Init())
		{
			ft_cnf.header.id = FTReq->header.id +1;
			ft_cnf.header.token = FTReq->header.token;
			ft_cnf.type = FTReq->type;
			ft_cnf.status = META_FAILED;

			//MetaLogMsg(FTT_DBG, (TEXT("[FTT_Drv:] FT_BAT_OP Meta_Battery_Init Fail ")));
			FT_LOG("[FTT_Drv:] FT_BAT_OP Meta_Battery_Init Fail ");
			goto  BAT_FW_Exit;
		}
		bInitFlag_BAT = TRUE;
	}

	Meta_Battery_Read_FW(FTReq);

	return;

BAT_FW_Exit:
	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

void FT_L4AUDIO_OP(FT_L4AUD_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_L4AUD_CNF ft_cnf;
	static META_BOOL bInitFlag_L4AUD = FALSE;

    FT_LOG("[FTT_Drv:] FT_L4AUDIO_OP META Test ");
	memset(&ft_cnf, 0, sizeof(FT_L4AUD_CNF));
	FT_LOG("+FT_L4AUDIO_OP");
	if(FALSE == bInitFlag_L4AUD)
	{
		if(!META_Audio_init())
		{
			ft_cnf.header.id = FTReq->header.id +1;
			ft_cnf.header.token = FTReq->header.token;
			ft_cnf.op = FTReq->op;
			ft_cnf.status = META_FAILED;

			FT_LOG("[FTT_Drv:] FT_L4AUD_OP META_Audio_init Fail ");
			goto  L4AUDIO_Exit;
		}
		bInitFlag_L4AUD = TRUE;
	}

	META_Audio_OP(FTReq,pPeerBuf,peer_len);

	FT_LOG("-FT_L4AUDIO_OP");
	return;

L4AUDIO_Exit:
	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
}


/********************************************************************************
//FUNCTION:
//		FT_SDcard_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "SDCARD_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_SDcard_OP(SDCARD_REQ *req, char *pPeerBuf, kal_int16 peer_len)
{

    SDCARD_CNF ft_cnf;
    static META_BOOL bInitFlag_SDcard = FALSE;

    FT_LOG("[FTT_Drv:] FT_SDcard_OP META Test ");
    memset(&ft_cnf, 0, sizeof(SDCARD_CNF));

    if (FALSE == bInitFlag_SDcard)
    {
        // initial the DVB module when it is called first time
        if (!Meta_SDcard_Init(req))
        {
            ft_cnf.header.id = req->header.id +1;
            ft_cnf.header.token = req->header.token;

            ft_cnf.status = META_FAILED;

            goto SDcard_Exit ;
        }
        bInitFlag_SDcard = TRUE;
    }
    FT_LOG("[FTT_Drv:] FT_SDcard_OP META Test req: %d , %d ",
        sizeof(SDCARD_REQ), sizeof(SDCARD_CNF));

    Meta_SDcard_OP(req, (char *)pPeerBuf, peer_len);
    return;

SDcard_Exit:
    FT_LOG("[FTT_Drv:] FT_SDcard_OP Meta_SDcard_Init Fail ");

    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

/********************************************************************************
//FUNCTION:
//		FT_LOW_POWER_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "LOW_POWER_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool lcd_brightness_set (int level)
{
	bool ret = false;
	int fd = -1;
#define BUF_LEN 16
	char wbuf[BUF_LEN] = {'\0'};
	char rbuf[BUF_LEN] = {'\0'};

	LOGD("lcd_brightness_set(%d)", level);

	if(level > 31)
		level = 31;
	else if (level < 0)
		level = 0;

#define BRIGHTNESS_FILE "/sys/class/leds/lcd-backlight/brightness"
	fd = open(BRIGHTNESS_FILE, O_RDWR, 0);
	if (fd == -1) {
		LOGE("Can't open %s\n", BRIGHTNESS_FILE);
		goto EXIT;
	}
	sprintf(wbuf, "%d\n", level);
	if (write(fd, wbuf, strlen(wbuf)) == -1) {
		LOGE("Can't write %s\n", BRIGHTNESS_FILE);
		goto EXIT;
	}
	close(fd);
	fd = open(BRIGHTNESS_FILE, O_RDWR, 0);
	if (fd == -1) {
		goto EXIT;
	}
	if (read(fd, rbuf, BUF_LEN) == -1) {
		goto EXIT;
	}
	if (!strncmp(wbuf, rbuf, BUF_LEN))
		ret = true;

EXIT:
	if (fd != -1)
		close(fd);
	return ret;
}


/********************************************************************************
//FUNCTION:
//		FT_LOW_POWER_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "LOW_POWER_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_LOW_POWER_OP(FT_LOW_POWER_REQ *req, char *pPeerBuf, kal_int16 peer_len)
{

   FT_LOW_POWER_CNF ft_cnf;

    //MetaLogMsg(g_bFTLogEnable, (TEXT("[FTT_Drv:] FT_LOW_POWER_OP META Test ")));
    FT_LOG("[FTT_Drv:] FT_LOW_POWER_OP META Test ");
    memset(&ft_cnf, 0, sizeof(FT_LOW_POWER_CNF));

    //read from meta_pmic moudle
    ft_cnf = META_LOW_POWER_OP(req);
    //initial the ft module header
    ft_cnf.header.id = req->header.id +1;;
    ft_cnf.header.token = req->header.token;
    ft_cnf.status= META_SUCCESS;
	ft_cnf.type = req->type;
	
    if (!lcd_brightness_set(0)) // Set LCD backlight OFF
    	ft_cnf.status= META_FAILED;

    WriteDataToPC(&ft_cnf, sizeof(FT_LOW_POWER_CNF),NULL, 0);

}


void FT_CCAP_OP(FT_CCT_REQ *FTReq, char *pPeerBuf, kal_int16 peer_len)
{

    FT_CCT_CNF ft_cnf;
    static bool bInitFlag_CCT = false;
    FT_LOG("[FTT_Drv:] FT_CCAP_OP META Test ");

    //if((FTReq->op != FT_CCT_OP_SUBPREVIEW_LCD_START) && (FTReq->op !=FT_CCT_OP_SUBPREVIEW_LCD_STOP))
   //	{
   		if(!META_CCAP_init())
   		{
   				ft_cnf.header.id = FTReq->header.id + 1;
   				ft_cnf.header.token = FTReq->header.token;
   				ft_cnf.op = FTReq->op;
   				ft_cnf.status = META_FAILED;
   				FT_LOG("[FTT_Drv:] FT_CCAP_OP META_CCT_init Fail ");
   				goto CCT_Exit;
   		}
   //	}
    //else
    //{
   //		META_LOG("[FTT_Drv:] Now is sub Camera, init will be do later");	
   //	}
    META_CCAP_OP(FTReq,pPeerBuf);
    return;

CCT_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}



/********************************************************************************
//FUNCTION:
//		FT_GPIO_OP
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "GPIO_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_GPIO_OP(GPIO_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    GPIO_CNF ft_cnf;
    static META_BOOL bInitFlag_GPIO = FALSE;

    FT_LOG("[FTT_Drv:] FT_GPIO_OP META Test ");
    memset(&ft_cnf, 0, sizeof(GPIO_CNF));

    if (FALSE == bInitFlag_GPIO)
    {
        // initial the bat module when it is called first time
        if (!Meta_GPIO_Init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            //ft_cnf.op = pFTReq->op;
            ft_cnf.status = META_FAILED;

            FT_LOG("[FTT_Drv:] FT_GPIO_OP Meta_GPIO_Init Fail ");
            goto  GPIO_Exit;
        }
        bInitFlag_GPIO = TRUE;
    }

    // //do the bat test by called the interface in meta bat lib
    ft_cnf = Meta_GPIO_OP(*pFTReq, (BYTE *)pPeerBuf, peer_len);

GPIO_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

const char* makepath(unsigned char file_ID)
{
	if(file_ID == 0)
		return "/data/nvram/AllMap";
	else
	{
		if(file_ID == 1)
			return "/data/nvram/AllFile";
		else
		{
			FT_LOG("[FTT_Drv:] makepath error: invalid file_ID %d! ", file_ID);
			return "";
		}
	}
}

unsigned int getFileSize(int fd)
{
	struct stat file_stat;
	if(fstat(fd, &file_stat) < 0)
	{
		return 0;
	}
	else
	{
		return (unsigned int)file_stat.st_size;
	}
}

/********************************************************************************
//FUNCTION:
//		SendNVRAMFile
//DESCRIPTION:
//		this function is called to read NVRAM file and send it to PC.
//
//PARAMETERS:
//		file_ID [IN] the id of NVRAM  file

//RETURN VALUE:
//		TRUE is success, otherwise is fail
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
bool SendNVRAMFile(unsigned char file_ID, FT_NVRAM_BACKUP_CNF* pft_cnf)
{
	int backupFd;
	int peer_buff_size = 0;
    char* peer_buf = 0;
	bool return_value = FALSE;

	backupFd = open(makepath(file_ID), O_RDWR);
	unsigned int fileLen = getFileSize(backupFd);

	if(backupFd >= 0)
	{
		FT_LOG("[FTT_Drv:] File%d opens succeed ! ",file_ID);

		peer_buf = (char*)malloc(NVRAM_PEER_MAX_LEN);
		memset(peer_buf, 0, NVRAM_PEER_MAX_LEN);

		pft_cnf->block.stage = BLK_CREATE;
		pft_cnf->block.file_ID = file_ID;

		while(!(pft_cnf->block.stage & BLK_EOF))
		{
			peer_buff_size = read(backupFd, peer_buf, NVRAM_PEER_MAX_LEN);

			if(peer_buff_size != -1)
			{
				pft_cnf->status = META_SUCCESS;
				if(peer_buff_size == 0)
				{
					pft_cnf->block.stage |= BLK_EOF;
					FT_LOG("[FTT_Drv:] File%d backups succeed! ",file_ID);
					pft_cnf->block.file_size = fileLen;

					close(backupFd);

					free(peer_buf);

					if(remove(makepath(file_ID)) == 0)
					{
						FT_LOG("[FTT_Drv:] File%d DeleteFile succeed! ",file_ID);
						return_value = TRUE;
						WriteDataToPC(pft_cnf, sizeof(FT_NVRAM_BACKUP_CNF),NULL, 0);
					}
					return return_value;
				}
				else
				{
					pft_cnf->block.stage |= BLK_WRITE;
					FT_LOG("[FTT_Drv:] File%d backups %d data ! ",file_ID,peer_buff_size);
					WriteDataToPC(pft_cnf, sizeof(FT_NVRAM_BACKUP_CNF),peer_buf, peer_buff_size);
					memset(peer_buf,0,NVRAM_PEER_MAX_LEN);
					pft_cnf->block.stage &= ~BLK_CREATE;
				}

			}
			else
			{
				pft_cnf->block.stage |= BLK_EOF;
				FT_LOG("[FTT_Drv:] File%d backups read failed ! ", file_ID);
			}

		}

		free(peer_buf);

	}
	else
	{
		FT_LOG("[FTT_Drv:] File%d backups open failed ! ", file_ID);
	}

	close(backupFd);
	return return_value;

}

/********************************************************************************
//FUNCTION:
//		FT_NVRAM_Backup_OP
//DESCRIPTION:
//		this function is called to backup nvram of target to PC.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_NVRAM_BACKUP_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size

//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_NVRAM_Backup_OP(FT_NVRAM_BACKUP_REQ* req, char* pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_NVRAM_Backup_OP ");

	FT_NVRAM_BACKUP_CNF ft_cnf;
	int bFileOpResult = 0;
	memset(&ft_cnf, 0, sizeof(FT_NVRAM_BACKUP_CNF));
	//init the header
	ft_cnf.header.id = req->header.id + 1;
	ft_cnf.header.token = req->header.token;
	ft_cnf.status = META_FAILED;

	if (req->count > 0)
	{
		FT_LOG("[FTT_Drv:] Count is %d, backup parts of NvRam!", req->count);
		bFileOpResult = FileOp_BackupData_Special(req->buffer, req->count, req->mode);
	}
	else
	{
		FT_LOG("[FTT_Drv:] Count is %d, backup all NvRam!", req->count);
		bFileOpResult = FileOp_BackupAll_NvRam();
	}
	
	if(bFileOpResult)
	{
	    FT_LOG("[FTT_Drv:] NVM_PcBackup_Get_Data Start ! ");
		if(SendNVRAMFile(0,&ft_cnf))
		{
			FT_LOG("[FTT_Drv:] Send file 0 succeed! ! ");
			//init the header
			ft_cnf.header.id = req->header.id +1;
			ft_cnf.header.token = req->header.token;
			ft_cnf.status = META_FAILED;
			ft_cnf.block.file_size = 0;

			if(SendNVRAMFile(1,&ft_cnf))
			{
				FT_LOG("[FTT_Drv:] Send file 1 succeed! ! ");
				return;
			}
		}
	}
	else
	{
		FT_LOG("[FTT_Drv:] Failed to backup NvRam!");
	}

	WriteDataToPC(&ft_cnf, sizeof(FT_NVRAM_BACKUP_CNF),NULL, 0);

}

/********************************************************************************
//FUNCTION:
//		FT_NVRAM_Restore_OP
//DESCRIPTION:
//		this function is called to backup nvram of target to PC.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_NVRAM_RESTORE_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size

//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_NVRAM_Restore_OP(FT_NVRAM_RESTORE_REQ * req, char* pPeerBuf, kal_int16 peer_len)
{

	FT_NVRAM_RESTORE_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(FT_NVRAM_RESTORE_CNF));

    //init the header
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
	ft_cnf.status = META_FAILED;

	int backupFd;
	unsigned int fileLen;
	FT_LOG("[FTT_Drv:] FT_NVRAM_Restore_OP receive block stage %x  file id %d file size %d!",req->block.stage,req->block.file_ID,req->block.file_size);
	if(req->block.stage & BLK_CREATE)
	{
		backupFd = open(makepath(req->block.file_ID), O_RDWR | O_TRUNC | O_CREAT, 0777);
	}
	else
	{
		backupFd = open(makepath(req->block.file_ID), O_RDWR | O_APPEND);
	}

	if(backupFd >= 0)
	{
		FT_LOG("[FTT_Drv:] FT_NVRAM_Restore_OP create or open file OK!");
		kal_uint16 sWriten = 0;
		sWriten = write(backupFd,pPeerBuf,peer_len);
		if(sWriten)
		{
			ft_cnf.status = META_SUCCESS;
			FT_LOG("[FTT_Drv:] FT_NVRAM_Restore_OP File%d write %d data total data %d!",req->block.file_ID,sWriten,peer_len);
			if(req->block.stage & BLK_EOF)
			{
				fileLen = getFileSize(backupFd);
				if(req->block.file_size == fileLen)
				{
					FT_LOG("[FTT_Drv:] FT_NVRAM_Restore_OP write file transfer success! ");
					close(backupFd);
					backupFd = -1;

					if(req->block.file_ID == 1)
					{
						if(!FileOp_RestoreAll_NvRam())
						{
							ft_cnf.status = META_FAILED;
							FT_LOG("[FTT_Drv:] META_Editor_PcRestore_Set_Data failed! ");

						}
					}
				}
				else
				{
					ft_cnf.status = META_FAILED;
					FT_LOG("[FTT_Drv:] FT_NVRAM_Restore_OP file %d size error! / %d ",req->block.file_ID,req->block.file_size);
				}
			}
		}
		else
		{
			FT_LOG("[FTT_Drv:] FT_NVRAM_Restore_OP write file failed!");
		}

		if(backupFd != -1)
			close(backupFd);


	}
	else
	{
		FT_LOG("[FTT_Drv:] FT_NVRAM_Restore_OP create or open file failed!");
	}

	WriteDataToPC(&ft_cnf, sizeof(FT_NVRAM_RESTORE_CNF),NULL, 0);

}

/********************************************************************************
//FUNCTION:
//		FT_GSENSOR_OP
//DESCRIPTION:
//		this function is called to perform G-Sensor operations.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "GS_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

void FT_GSENSOR_OP(GS_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_GSENSOR_OP");
    GS_CNF ft_cnf;
    static META_BOOL bInitFlag_GS = FALSE;

    memset(&ft_cnf, 0, sizeof(GS_CNF));

    if (FALSE == bInitFlag_GS)
    {
        // initial the G-Sensor module when it is called first time
        if (!Meta_GSensor_Open())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.status = META_FAILED;
		   ft_cnf.op = pFTReq->op;
			 
            FT_LOG("[FTT_Drv:] FT_GSENSOR_OP Meta_GSensor_Open Fail ");
            goto  GS_Exit;
        }
        bInitFlag_GS = TRUE;
    }

    //do the G-Sensor test by called the interface in meta G-Sensor lib
    Meta_GSensor_OP(pFTReq, pPeerBuf, peer_len);
	return;

GS_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}


/********************************************************************************
//FUNCTION:
//		FT_GYROSENSOR_OP
//DESCRIPTION:
//		this function is called to perform gyroscope operations.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "GYRO_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/

void FT_GYROSENSOR_OP(GYRO_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_GYROSENSOR_OP");
    GYRO_CNF ft_cnf;
    static META_BOOL bInitFlag_GYRO = FALSE;

    memset(&ft_cnf, 0, sizeof(GYRO_CNF));

    if (FALSE == bInitFlag_GYRO)
    {
        // initial the Gyroscope-Sensor module when it is called first time
        if (!Meta_Gyroscope_Open())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.status = META_FAILED;
	    ft_cnf.op = pFTReq->op;

            FT_LOG("[FTT_Drv:] FT_GYROSENSOR_OP Meta_GYROSensor_Open Fail ");
            goto  GYRO_Exit;
        }
        bInitFlag_GYRO = TRUE;
    }

    //do the Gyroscope-Sensor test by called the interface in meta Gyroscope-Sensor lib  
    Meta_Gyroscope_OP(pFTReq, pPeerBuf, peer_len);
	return;

GYRO_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}


#ifdef FT_MATV_FEATURE
/********************************************************************************
//FUNCTION:
//		FT_MATV_OP
//DESCRIPTION:
//		this function is called to perform MATV operations.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_MATV_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_MATV_OP(FT_MATV_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_LOG("[FTT_Drv:] FT_MATV_OP META Test ");
	META_MATV_OP(pFTReq);
}
#endif
/********************************************************************************
//FUNCTION:
//		FT_Reboot
//DESCRIPTION:
//		this function is called to reboot target
//
//PARAMETERS:
//		None
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		None
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_Reboot(FT_META_REBOOT_REQ *pFTReq)
{
    FT_LOG("[FTT_Drv:] FT_Reboot: Device will reboot after %d seconds. ", pFTReq->delay);
	sleep(pFTReq->delay);

    //Reboot target side after finishing the meta
	sync();
	reboot(RB_AUTOBOOT);
}

/********************************************************************************
//FUNCTION:
//		FT_CUSTOMER_OP
//DESCRIPTION:
//		this function is called to perform customer operations.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_CUSTOMER_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_CUSTOMER_OP(FT_CUSTOMER_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_LOG("[FTT_Drv:] FT_CUSTOMER_OP!");
    FT_CUSTOMER_CNF ft_cnf;
    memset(&ft_cnf, 0, sizeof(FT_CUSTOMER_CNF));
	int peer_buff_size = 0;
    char* peer_buf = 0;
	
	// Implement custom API logic here. The following is a sample code for testing.
    ft_cnf.header.id = pFTReq->header.id +1;
    ft_cnf.header.token = pFTReq->header.token;
	ft_cnf.type = pFTReq->type;
    ft_cnf.status = META_SUCCESS;
	peer_buf = (char*)malloc(peer_len);
	memcpy(peer_buf, pPeerBuf, peer_len);
	peer_buff_size = peer_len;
	FT_LOG("[FTT_Drv:] FT_CUSTOMER_OP successful, OP type is %d!", pFTReq->type);
	
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf), peer_buf, peer_buff_size);
}

/********************************************************************************
//FUNCTION:
//		FT_GET_CHIPID_OP
//DESCRIPTION:
//		this function is called to get chip ID.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_GET_CHIPID_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_GET_CHIPID_OP(FT_GET_CHIPID_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_LOG("[FTT_Drv:] FT_GET_CHIPID_OP!");
    FT_GET_CHIPID_CNF ft_cnf;
	int bytes_read = 0;
	int res = 0;
	
    memset(&ft_cnf, 0, sizeof(FT_GET_CHIPID_CNF));
	
    ft_cnf.header.id = pFTReq->header.id +1;
    ft_cnf.header.token = pFTReq->header.token;
	ft_cnf.status = META_FAILED;
	
	int fd = open(CHIP_RID_PATH, O_RDONLY);
	if (fd != -1)
	{
		while (bytes_read < CHIP_RID_LEN)
		{
			res = read(fd, ft_cnf.chipId + bytes_read, CHIP_RID_LEN);
			if (res > 0)
				bytes_read += res;
			else
				break;
		}
		close(fd);
    	ft_cnf.status = META_SUCCESS;
		FT_LOG("Chip rid=%s", ft_cnf.chipId);
	}
	else
	{
		if (errno == ENOENT)
		{
			ft_cnf.status = META_NOT_SUPPORT;
		}
		FT_LOG("Failed to open chip rid file %s, errno=%d", CHIP_RID_PATH, errno);
	}
	
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf), NULL, 0);
}

/********************************************************************************
//FUNCTION:
//		FT_MSENSOR_OP
//DESCRIPTION:
//		this function is called to perform M-Sensor operations.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_MSENSOR_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_MSENSOR_OP(FT_MSENSOR_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_MSENSOR_OP");
    FT_MSENSOR_CNF ft_cnf;
    static META_BOOL bInitFlag_MS = FALSE;
	int res = -1;


    memset(&ft_cnf, 0, sizeof(FT_MSENSOR_CNF));
	ft_cnf.header.id = pFTReq->header.id + 1;
	ft_cnf.header.token = pFTReq->header.token;
	ft_cnf.status = META_SUCCESS;

    if (FALSE == bInitFlag_MS)
    {
        // initial the M-Sensor module when it is called first time
        if (!Meta_MSensor_Open())
        {
            FT_LOG("[FTT_Drv:] FT_MSENSOR_OP Meta_MSensor_Open failed!");
			ft_cnf.status = META_FAILED;
            goto  MS_Exit;
        }
        bInitFlag_MS = TRUE;
    }
	
	res = Meta_MSensor_OP();
	if (0 == res)
	{
	    FT_LOG("[FTT_Drv:] FT_MSENSOR_OP Meta_MSensor_OP success!");
	    ft_cnf.status = META_SUCCESS;
	}
	else
	{
	    FT_LOG("[FTT_Drv:] FT_MSENSOR_OP Meta_MSensor_OP failed!");
	    ft_cnf.status = META_FAILED;
	}

    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
	return;

	
MS_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

/********************************************************************************
//FUNCTION:
//		FT_ALSPS_OP
//DESCRIPTION:
//		this function is called to perform ALS_PS operations.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_ALSPS_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_ALSPS_OP(FT_ALSPS_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_ALSPS_OP");
    FT_ALSPS_CNF ft_cnf;
    static BOOL bInitFlag_ALSPS = FALSE;
	int res = -1;


    memset(&ft_cnf, 0, sizeof(FT_ALSPS_CNF));
	ft_cnf.header.id = pFTReq->header.id + 1;
	ft_cnf.header.token = pFTReq->header.token;
	ft_cnf.status = META_SUCCESS;

    if (FALSE == bInitFlag_ALSPS)
    {
        // initial the M-Sensor module when it is called first time
        if (!Meta_ALSPS_Open())
        {
            FT_LOG("[FTT_Drv:] FT_ALSPS_OP Meta_ALSPS_Open failed!");
			ft_cnf.status = META_FAILED;
            goto  MS_Exit;
        }
        bInitFlag_ALSPS = TRUE;
    }

    res = Meta_ALSPS_OP();
    if (0 == res)
    {
        FT_LOG("[FTT_Drv:] FT_ALSPS_OP Meta_ALSPS_OP success!");
        ft_cnf.status = META_SUCCESS;
    }
	else
	{
	    FT_LOG("[FTT_Drv:] FT_ALSPS_OP Meta_ALSPS_OP failed!");
	    ft_cnf.status = META_FAILED;
	}
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
	
	return;


MS_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}
/******************************************************************************
**
//FUNCTION:
//		FT_CTP_OP
//DESCRIPTION:
//		this function is called to perform touch panel operations.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_CTP_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
*******************************************************************************
*/
void FT_CTP_OP(Touch_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_CTP_OP");
    Touch_CNF ft_cnf;
    static BOOL bInitFlag_CTP = FALSE;
	int res;

    memset(&ft_cnf, 0, sizeof(Touch_CNF));
	ft_cnf.header.id = pFTReq->header.id + 1;
	ft_cnf.header.token = pFTReq->header.token;
	ft_cnf.status = META_SUCCESS;
	ft_cnf.tpd_type = pFTReq->tpd_type;

    if (FALSE == bInitFlag_CTP)
    {
        // initial the touch panel module when it is called first time
        if (!Meta_Touch_Init())
        {
            META_LOG("[FTT_Drv:] FT_CTP_OP Meta_Touch_Init failed!");
			ft_cnf.status = META_FAILED;
            goto  MS_Exit;
        }
        bInitFlag_CTP = TRUE;
    }

    Meta_Touch_OP(pFTReq, pPeerBuf, peer_len);
	
	return;

MS_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}

#ifdef FT_EMMC_FEATURE
void FT_CLR_EMMC_OP(FT_EMMC_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
	FT_LOG("[FTT_Drv:] FT_CLR_EMMC_OP");
    META_CLR_EMMC_OP(pFTReq);	
	return;
}
void FT_CRYPTFS_OP(FT_CRYPTFS_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    FT_LOG("[FTT_Drv:] FT_CRYPTFS_OP");
    META_CRYPTFS_OP(pFTReq);    
    return;
}
#endif



/******************************************************************************
**
//FUNCTION:
//		FT_DFO_OP
//DESCRIPTION:
//		this function is called to read and write dfo value.
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "FT_DFO_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
*******************************************************************************
*/
void FT_DFO_OP(FT_DFO_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    META_LOG("[FTT_Drv:] FT_DFO_OP");
	FT_DFO_CNF ft_cnf;
    static META_BOOL bInitFlag_DFO = FALSE;

    memset(&ft_cnf, 0, sizeof(FT_DFO_CNF));

    if (FALSE == bInitFlag_DFO)
    {
        // initial the DFO module when it is called first time
        if (!META_Dfo_Init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.op= pFTReq->op;
            ft_cnf.status = META_FAILED;
            goto DFO_Exit;
        }
        bInitFlag_DFO = TRUE;
    }

    if(pFTReq->op == DFO_OP_WRITE)
    {
       META_LOG("[FTT_Drv:] FT_DFO_OP write_req name:%s,value:%d",pFTReq->cmd.write_req.name,pFTReq->cmd.write_req.value); 
    }
    META_Dfo_OP(pFTReq);
    return;

DFO_Exit:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
}



/********************************************************************************
//FUNCTION:
//		FT_GetVersionInfoV2
//DESCRIPTION:
//		this function is called to get the version information of target side.
//
//PARAMETERS:
//		req:			[IN]	refers to the define of "FT_VER_INFO_V2_REQ"
//		pft_PeerBuf: 	[IN]	peer buff
//		ft_peer_len	[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
********************************************************************************/
void FT_GetVersionInfoV2(FT_VER_INFO_V2_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len)
{
    FT_VER_INFO_V2_CNF ft_cnf;
    unsigned int dwRc ;
    unsigned int dwValSize;
    unsigned int dwValType;
    char szBuffer[MAX_PATH];
    FILE *fd = 0;
    char str[256] = {0};
    char *loc = NULL;

    memset(&ft_cnf, 0, sizeof(ft_cnf));
    memset(szBuffer,0, sizeof(szBuffer));
    char* tmp = NULL;
    char platform[256] = {0};
    char chipVersion[256] = {0};

    //initail the value of ft header
    ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.status = META_FAILED;

    FT_LOG("[FTT_Drv:] FT_GetVersionInfoV2 ");

    if((fd = fopen(VERSION_FILE_PATH,"r"))==NULL)
    {
        FT_LOG("FT_GetVersionInfo Can't open file : %s\n", VERSION_FILE_PATH);
    }

    while(!feof(fd))
    {
        if(fgets(str, 256, fd)!=NULL)
        {
            tmp = str;
            loc = strsep(&tmp, "=");
            if(!strcmp(loc, RELEASE_SW_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] SW Version = %s\n", tmp);
				strncpy((char*)ft_cnf.sw_ver, tmp, 63);
            }
            if(!strcmp(loc, RELEASE_PLATFORM_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Platform = %s\n", tmp);
                strncpy(platform, tmp, 255);
            }
            if(!strcmp(loc, RELEASE_PRODUCT_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Product Name = %s\n", tmp);
            }
            if(!strcmp(loc, RELEASE_CHIP_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Chip Version = %s\n", tmp);
				strncpy(chipVersion, tmp, 255);
                strncpy((char*)ft_cnf.hw_ver, tmp, 63);
            }
            if(!strcmp(loc, RELEASE_BUILD_TIME_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Build Time = %s\n", tmp);
				strncpy((char*)ft_cnf.sw_time, tmp, 63);
            }
            if(!strcmp(loc, RELEASE_BUILD_DISP_ID_TOKEN))
            {
                FT_LOG("[FT_GetVersionInfo] Build Display ID = %s\n", tmp);
				strncpy((char*)ft_cnf.build_disp_id, tmp, 63);
            }
        }
    }

    int i = 0;
    int k = 0;
	while (i < 256)
	{
		if (platform[i] != '\r' && platform[i] != '\n')
			szBuffer[k++] = platform[i++];
		else
			break;
	}
	szBuffer[k++] = ',';
	i = 0;
	while (i < 256)
	{
		if (chipVersion[i] != '\r' && chipVersion[i] != '\n')
			szBuffer[k++] = chipVersion[i++];
		else
			break;
	}
	szBuffer[k++] = '\0';
	if (strlen(szBuffer) <= 64)
	{
		strcpy((char*)ft_cnf.bb_chip, szBuffer);
	}
	else
	{
		FT_LOG("[FTT_Drv:] String is too long, length=%d ", strlen(szBuffer));
	}
    fclose(fd);

	/* Get Software version : ft_cnf.sw_ver */
	FT_LOG("[FTT_Drv:] ft_cnf.sw_ver = %s ", ft_cnf.sw_ver);

	/* Get the build time : ft_cnf.sw_ver */
	FT_LOG("[FTT_Drv:] ft_cnf.sw_time = %s ", ft_cnf.sw_time);

	/* Get the chip version : ft_cnf.sw_ver */
	FT_LOG("[FTT_Drv:] ft_cnf.bb_chip = %s ", ft_cnf.bb_chip);
	FT_LOG("[FTT_Drv:] ft_cnf.hw_ver = %s ", ft_cnf.hw_ver);

    ft_cnf.status = META_SUCCESS;

Ver_error:
    WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
}


int getBootMode(void)
{
	int mode = -1;
	char buf[BOOT_MODE_STR_LEN + 1];
	int bytes_read = 0;
	int res = 0;
	int fd = open(BOOT_MODE_INFO_FILE, O_RDONLY);
	if (fd != -1)
	{
		memset(buf, 0, BOOT_MODE_STR_LEN + 1);
		while (bytes_read < BOOT_MODE_STR_LEN)
		{
			res = read(fd, buf + bytes_read, BOOT_MODE_STR_LEN);
			if (res > 0)
				bytes_read += res;
			else
				break;
		}
		close(fd);
		mode = atoi(buf);
	}
	else
	{
		META_LOG("Failed to open boot mode file %s", BOOT_MODE_INFO_FILE);
	}
	return mode;
}

void FT_BUILD_PROP_OP(FT_BUILD_PROP_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len)
{	
	FT_BUILD_PROP_CNF ft_cnf;

	
	FT_LOG("[FTT_Drv:] FT_BUILD_PROP_OP ");

	ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
	
    property_get((const char*)req->tag, (char *)ft_cnf.content, "unknown");	

	META_LOG("[FTT_Drv:] %s = %s ",req->tag,ft_cnf.content);

	ft_cnf.status = META_SUCCESS;

	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
	
}

int getComportType(void)
{
	int type = 0;
	char buf[COM_PORT_TYPE_STR_LEN + 1];
	int bytes_read = 0;
	int res = 0;
	int fd = open(COM_PORT_TYPE_FILE, O_RDONLY);
	if (fd != -1)
	{
		memset(buf, 0, COM_PORT_TYPE_STR_LEN + 1);
		while (bytes_read < COM_PORT_TYPE_STR_LEN)
		{
			res = read(fd, buf + bytes_read, COM_PORT_TYPE_STR_LEN);
			if (res > 0)
				bytes_read += res;
			else
				break;
		}
		close(fd);
		type = atoi(buf);
	}
	else
	{
		META_LOG("Failed to open com port type file %s", COM_PORT_TYPE_FILE);
	}
	return type;	
}

void FT_SIM_NUM_OP(FT_GET_SIM_REQ *req, char *pft_PeerBuf, kal_int16 ft_peer_len)
{
	FT_GET_SIM_CNF ft_cnf;
	
	FT_LOG("[FTT_Drv:] FT_SIM_NUM_OP ");

	ft_cnf.header.id = req->header.id +1;
    ft_cnf.header.token = req->header.token;
    ft_cnf.type = req->type;
    ft_cnf.status = META_SUCCESS;

    ft_cnf.number = 0;
    FT_LOG("[FTT_Drv:] The sim card number is zero");
    
	 #ifdef GEMINI
	 
	  ft_cnf.number = 2;
      FT_LOG("[FTT_Drv:] The sim card number is two");
     
	  #ifdef MTK_GEMINI_3SIM_SUPPORT	
	   ft_cnf.number = 3;
	   FT_LOG("[FTT_Drv:] The sim card number is three");
      #endif

	  #ifdef MTK_GEMINI_4SIM_SUPPORT
	   ft_cnf.number = 4;
	   FT_LOG("[FTT_Drv:] The sim card number is four");
      #endif    	   
	   
     #else    
       ft_cnf.number = 1;
	   FT_LOG("[FTT_Drv:] The sim card number is one");
	    
   	 #endif
   	
   	WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);

}


void FT_ADC_OP(ADC_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
      META_LOG("[FTT_Drv:] FT_ADC_OP");
      Meta_ADC_OP(pFTReq, pPeerBuf, peer_len);	
      return;
}


/******************************************************************************
**
//FUNCTION:
//		FT_HDCP_OP
//DESCRIPTION:
//		this function is called to execute HDCP related function
//
//PARAMETERS:
//		pFTReq:		[IN]	refers to the define of "HDCP_REQ"
//		pPeerBuf: 	[IN]	peer buff
//		peer_len:		[IN]	peer buff size
//RETURN VALUE:
//		None
//
//DEPENDENCY:
//		the FT module must have been loaded.
//
//GLOBALS AFFECTED
//		None
*******************************************************************************
*/
#ifdef FT_HDCP_FEATURE
void FT_HDCP_OP(HDCP_REQ *pFTReq, char *pPeerBuf, kal_int16 peer_len)
{
    META_LOG("[FTT_Drv:] FT_HDCP_OP");
	HDCP_CNF ft_cnf;
	memset(&ft_cnf, 0, sizeof(ft_cnf));
    static META_BOOL bInitFlagHDCP = FALSE;

    if (FALSE == bInitFlagHDCP)
    {
        // initial the HDCP module when it is called first time
        if (!META_HDCP_init())
        {
            ft_cnf.header.id = pFTReq->header.id +1;
            ft_cnf.header.token = pFTReq->header.token;
            ft_cnf.op= pFTReq->op;
            ft_cnf.status = META_FAILED;
            WriteDataToPC(&ft_cnf, sizeof(ft_cnf),NULL, 0);
			return;
        }
        bInitFlagHDCP = TRUE;
    }

    META_HDCP_OP(pFTReq, pPeerBuf, peer_len);
    return;   
}
#endif

int FT_GetDumpLogState()
{
	
	FT_LOG("[FTT_Drv:] Dump Log State %d",dumpLogState);
	return dumpLogState;
}

