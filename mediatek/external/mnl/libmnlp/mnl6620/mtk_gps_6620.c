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

#define _MTK_GPS_6620_C_
#include "mnl_linux_6620.h"
#include <sys/ioctl.h>
#include <sys/time.h>
//#include <linux/mtgpio.h>
//for EPO file
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
//for read NVRAM
#include "libnvram.h"
#include "CFG_GPS_File.h"
#include "CFG_GPS_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"

/******************************************************************************
* Macro & Definition
******************************************************************************/
//#define LIB_MQUEUE
#define MNL_MSG_RING_SIZE 128
/******************************************************************************
* Static variables
******************************************************************************/
#if defined(LIB_MQUEU)
// message queue file descriptor
static mqd_t mnl_agps_mq_fd = -1;
struct mq_attr mnl_agps_mq_attr;
#else
#endif
/******************************************************************************
* Extern Variables
******************************************************************************/
extern FILE *dbglog_fp;
extern MTK_GPS_BOOL enable_dbg_log;

extern int deltat_read_clear(long *diff_sec);   // newT - oldT
//for read NVRAM
extern MNL_CONFIG_T mnl_config;
extern ap_nvram_gps_config_struct stGPSReadback;
extern int gps_nvram_valid;


/*=============================================================================
*
*   Utility functions for NMEA
*
=============================================================================*/

#if defined(LIB_MQUEU)
#else
#endif
#define PMTK_FS_REQ_MEAS                736
#define PMTK_FRAME_TIME_ACK             737
#define PMTK_FS_SLEEPMODE               738


/*=============================================================================
*
*   Porting Layer functions
*
=============================================================================*/
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_init
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 *****************************************************************************/
int mtk_gps_sys_init()
{
    //int gps_nvram_fd = 0;
    F_ID gps_nvram_fd;
    int file_lid = AP_CFG_CUSTOM_FILE_GPS_LID;
    int rec_size;
    int rec_num;
    int i;
    /*create message queue*/
#if defined(LIB_MQUEUE)
    mnl_agps_mq_attr.mq_maxmsg = 72;
    mnl_agps_mq_attr.mq_msgsize = sizeof(MTK_GPS_AGPS_AGENT_MSG);
    mnl_agps_mq_attr.mq_flags   = 0;
    mnl_agps_mq_fd = mq_open (MNL_AGPS_MQ_NAME, O_CREAT|O_RDWR|O_EXCL, PMODE, &mnl_agps_mq_attr);

    if (mnl_agps_mq_fd == -1)
    {
        MNL_MSG("Fail to create mnl_agps_msg_queue, errno=%s\n",strerror(errno));
        if (errno == EEXIST) 
        {
            MNL_MSG("mnl_agps_msg_queue already exists, unlink it now ...\n");
            mq_unlink(MNL_AGPS_MQ_NAME);
        }
        return MTK_GPS_ERROR;
    } 
#else
#endif
            memset(&stGPSReadback, 0, sizeof(stGPSReadback));
            //gps_nvram_fd.iFileDesc = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
            #ifdef MTK_GPS_NVRAM
            gps_nvram_fd = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
        if(gps_nvram_fd.iFileDesc != 0)
            {
            read(gps_nvram_fd.iFileDesc, &stGPSReadback , rec_size*rec_num);
                NVM_CloseFileDesc(gps_nvram_fd);
    
                if(strlen(stGPSReadback.dsp_dev) != 0)
                {
                    gps_nvram_valid = 1;
                    //strncpy(mnl_config.dev_dsp, stGPSReadback.dsp_dev, sizeof(mnl_config.dev_dsp));
    
                    MNL_MSG("GPS NVRam (%d * %d) : \n", rec_size, rec_num);
                    //MNL_MSG("dsp_dev(mt6620:/dev/stpgps) : %s\n", stGPSReadback.dsp_dev);
                    //MNL_MSG("gps_if_type : %d\n", stGPSReadback.gps_if_type);
                    MNL_MSG("gps_tcxo_hz : %d\n", stGPSReadback.gps_tcxo_hz);
                    MNL_MSG("gps_tcxo_ppb : %d\n", stGPSReadback.gps_tcxo_ppb);
                    MNL_MSG("gps_tcxo_type : %d\n", stGPSReadback.gps_tcxo_type);
                    MNL_MSG("gps_lna_mode : %d\n", stGPSReadback.gps_lna_mode);
                    //MNL_MSG("gps_sbas_mode : %d\n", stGPSReadback.gps_sbas_mode);
                }
                else
                {
                    MNL_MSG("GPS NVRam mnl_config.dev_dsp == NULL \n");
                }
            }
            else
            {
                MNL_MSG("GPS NVRam gps_nvram_fd == NULL \n");
            }
            #endif
    return MTK_GPS_SUCCESS;
}
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uninit
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 *****************************************************************************/
int mtk_gps_sys_uninit()
{
#if defined(LIB_MQUEUE)
    mq_close(mnl_mq_fd);         /* Close message queue in parent */
    mq_unlink(MNL_MQ_NAME);      /* Unlink message queue */
    mq_close(mnl_agps_mq_fd);    /* Close message queue in parent */
    mq_unlink(MNL_AGPS_MQ_NAME); /* Unlink message queue */
#else    
#endif

    return MTK_GPS_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_nmea_output_to_app
 * DESCRIPTION
 *  Transmit driver debug message to APP
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  msg         [IN]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 
mtk_gps_sys_nmea_output_to_app(const char* buffer, UINT32 length)
{
    if (enable_dbg_log == MTK_GPS_TRUE) //Need to use prop to control debug on/of
    {
        MNL_MSG("%s", buffer );
    }
    return MTK_GPS_SUCCESS;
}

#if  0 // No-support
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_start_result_handler
 * DESCRIPTION
 *  Handler routine for the result of restart command
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  result         [IN]  the result of restart
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_start_result_handler(MTK_GPS_START_RESULT result)
{
    if ((INT32) MTK_GPS_ERROR == (INT32)result)
    {
    // To do, handle restart result if needed by the host.
    }
    return MTK_GPS_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_spi_poll
 * DESCRIPTION
 *  Polling data input routine for SPI during dsp boot up stage.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  void
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_spi_poll(void)
{
    // spi interface will need this function to read the SPI input data
    return MTK_GPS_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_set_spi_mode
 * DESCRIPTION
 *  Set SPI interrupt/polling and support burst or not.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  enable_int         [IN]  1 for enter interrupt mode , 0 for entering polling mode
 *  enable_burst       [IN]  1 for enable burst transfer, 0 for disable burst transfer
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_set_spi_mode(UINT8 enable_int, UINT8 enable_burst)
{
    // spi interface will need this function to handle mode and transfer in driver if needed.
    return MTK_GPS_SUCCESS;
}

#endif


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_epo_open
 * DESCRIPTION
 *  Open EPO file
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_epo_open (void)
{
    return MTK_GPS_ERROR; //0
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_epo_close
 * DESCRIPTION
 *  Close EPO file
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_epo_close (void)
{
    return;
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_epo_read
 * DESCRIPTION
 *  Read EPO file
 *  (blocking read until reaching 'length' or EOF)
 * PARAMETERS
 *  buffer      [OUT]
 *  offset      [IN]
 *  length      [IN]
 *  p_nRead     [OUT]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_epo_read (void* buffer, UINT32 offset, UINT32 length,
                      UINT32* p_nRead)
{
    return MTK_GPS_ERROR; //0
}

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_pmtk_cmd_cb
 * DESCRIPTION
 *  Notify porting layer that MNL has received one PMTK command.
 * PARAMETERS
 *  UINT16CmdNum        [IN]  The received PMTK command number.
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_pmtk_cmd_cb(UINT16 UINT16CmdNum)
{
    ;
}
unsigned char
calc_nmea_checksum1 (const char* sentence)
{
	unsigned char checksum = 0;

	while (*sentence)
	{
		checksum ^= (unsigned char)*sentence++;
	}

	return  checksum;
}
    
INT32 mtk_gps_sys_frame_sync_meas_req(MTK_GPS_FS_WORK_MODE mode)
{
    char szBuf_cipher[64];
    char sztmp[64];
    char outbuf[64];
    
    memset(outbuf,0,sizeof(outbuf));
    memset(sztmp,0,sizeof(sztmp));
    memset(szBuf_cipher,0,sizeof(szBuf_cipher));
    sprintf(sztmp,"PMTK%d,1,%d",PMTK_FS_REQ_MEAS,mode);
    sprintf(outbuf,"$%s*%02X\r\n",sztmp,calc_nmea_checksum1(sztmp));
    
   // #ifdef ENABLE_SUPL_PMTK_ENCRYPTION
    SUPL_encrypt((unsigned char *)outbuf, (unsigned char *)szBuf_cipher, strlen(outbuf));
   // #else
   // memcpy(szBuf_cipher, outbuf, strlen(outbuf));
   // #endif
    mtk_gps_sys_agps_disaptcher_callback(MTK_AGPS_CB_SUPL_PMTK, strlen(szBuf_cipher), szBuf_cipher);


    
    return MTK_GPS_SUCCESS;
}


INT32 mtk_gps_sys_frame_sync_enable_sleep_mode(unsigned char mode)
{
    char szBuf_cipher[64];
    char sztmp[64]; 
    char outbuf[64];
    
    memset(outbuf,0,sizeof(outbuf));
    memset(sztmp,0,sizeof(sztmp));
    memset(szBuf_cipher,0,sizeof(szBuf_cipher));
    sprintf(sztmp,"PMTK%d,%d",PMTK_FS_SLEEPMODE,mode);
    sprintf(outbuf,"$%s*%02X\r\n",sztmp,calc_nmea_checksum1(sztmp));
 
  
   //#ifdef ENABLE_SUPL_PMTK_ENCRYPTION
    SUPL_encrypt((unsigned char *)outbuf, (unsigned char *)szBuf_cipher, strlen(outbuf));
   // #else
   // memcpy(szBuf_cipher, outbuf, strlen(outbuf));
   // #endif
    mtk_gps_sys_agps_disaptcher_callback(MTK_AGPS_CB_SUPL_PMTK, strlen(szBuf_cipher), szBuf_cipher);

   
    return MTK_GPS_SUCCESS;
}
INT32 mtk_gps_sys_frame_sync_meas_req_by_network(void)
{
    char szBuf_cipher[64];
    char sztmp[64]; 
    char outbuf[64];
    
    memset(outbuf,0,sizeof(outbuf));
    memset(sztmp,0,sizeof(sztmp));
    memset(szBuf_cipher,0,sizeof(szBuf_cipher));
    sprintf(sztmp,"PMTK%d,0,0",PMTK_FS_REQ_MEAS);
    sprintf(outbuf,"$%s*%02X\r\n",sztmp,calc_nmea_checksum1(sztmp));
    
   // #ifdef ENABLE_SUPL_PMTK_ENCRYPTION
    SUPL_encrypt((unsigned char *)outbuf, (unsigned char *)szBuf_cipher, strlen(outbuf));
   // #else
   // memcpy(szBuf_cipher, outbuf, strlen(outbuf));
   // #endif
    mtk_gps_sys_agps_disaptcher_callback(MTK_AGPS_CB_SUPL_PMTK, strlen(szBuf_cipher), szBuf_cipher);


    
    return MTK_GPS_SUCCESS;
}
