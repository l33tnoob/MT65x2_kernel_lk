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
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_battery_para.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   This file is battery Driver meta file.
 *   
 *
 * Author:
 * -------
 *  Ting Sun 
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * Jun 13 2009 mtk80256
 * [DUMA00116871] PC-Link modify code and check in
 * 
 *
 * Apr 21 2009 mtk80256
 * [DUMA00112902] [Battery] NLED flashed perpetual after shutdown device by emergency shutdown
 * 
 *
 * Apr 18 2009 mtk80256
 * [DUMA00114958] battery meta update FW
 * 
 *
 * Feb 23 2009 mtk80306
 * [DUMA00109277] add meta _battery mode.
 * 
 *
 * 
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __METABATTPARA_H__
#define __METABATTPARA_H__
 #include "FT_Public.h"  
  
 typedef enum{
 	WM_BAT_DLImage =0,
 	WM_BAT_UPimage
 }WM_Bat_CMD_TYPE;
 
 typedef enum{
 	WM_BAT_READ_FW_INFO =0,
 	WM_BAT_READ_SOC,
 	WM_BAT_WRITE_SOC
 }WM_Bat_INFO_CMD_TYPE;

 typedef enum{
  WM_BAT_WRITE_DATA=0,
  WM_BAT_COMPARA_DATA,
  WM_BAT_DELAY
}WM_BAT_Upload_OP_TYPE;
 
 
 typedef enum{
 	BAT_FILE_Success =0,
 	BAT_FILE_Fail
 }WM_Bat_File_Err_TYPE;
 
   
 typedef enum{
    BAT_READ_INFO_FAILED=0, 
    BAT_READ_INFO_SUCCESS,
}  WM_Bat_READ_FW_TYPE;
 
    
 typedef enum{
 	BAT_FILE_START=0,
 	BAT_FILE_ONGOING,
 	BAT_FILE_CLOSE,
 	BAT_FILE_ONCE
 }WM_Bat_Image_OP_TYPE;
    
 typedef struct{
 	WM_Bat_Image_OP_TYPE	nReqWriteFileStatus;
 	unsigned short			image_size;  //the size of whole image
 	unsigned short			block_size;  //the size of current transfer.
 }WM_Bat_DLImage_REQ_T;
 
 typedef struct{
 	WM_Bat_Image_OP_TYPE	nReqStartStatus;
 	WM_BAT_Upload_OP_TYPE   nReqUpdateOpStatus;
    BYTE  StartAddress;
    unsigned short delaytime;
}WM_Bat_UPImage_REQ_T;
 
  typedef struct{
  int ibatteryinfo; //N/A
 }WM_Bat_READ_FW_INFO_REQ_T;
 
 typedef enum{
 	    WM_BAT_SOC1_SETTINGS=0,
        WM_BAT_SOC1_CLEAR
 }WM_BAT_SOC_TYPE;
 	
 
 typedef struct{
  WM_BAT_SOC_TYPE  nReqReadSocType;
 }WM_Bat_READ_SOC_REQ_T; 
 
 typedef struct{
  BYTE              SocValue; 
  BYTE              SocClearValue;
  WM_BAT_SOC_TYPE   nReqWriteSocType; 
 }WM_Bat_WRITE_SOC_REQ_T; 
 
 
 typedef struct{
 	WM_Bat_Image_OP_TYPE	    nCnfWriteFileStatus;  /* the same to the WM_Bat_DLImage_REQ_T's nReqWriteFileStatus*/
 	WM_Bat_File_Err_TYPE    	DL_Status;
 }WM_Bat_DLImage_CNF_T;
 
 typedef struct{
 	WM_Bat_Image_OP_TYPE	nCnfStartStatus;  /* the same to the WM_Bat_DLImage_REQ_T's nReqWriteFileStatus*/
 	WM_Bat_File_Err_TYPE    nCnfUpdateStatus;
 }WM_Bat_UPImage_CNF_T;
 
   
 typedef struct{
    unsigned short          BQ27500_FW_Version;       //0xFFFF
    unsigned short          BQ27500_Battery_Voltage; 	  //0xFFFF  
  	unsigned short   		BQ27500_DFI_Version;          //0xFFFF
 	WM_Bat_READ_FW_TYPE    	Drv_Status; 
 	
 }WM_Bat_READ_FW_INFO_CNF_T;
 
  typedef struct{	
    BYTE                    BQ27500_SOC_Threshold;
    BYTE                    BQ27500_Soc_Clear_Threshold;
 	WM_Bat_READ_FW_TYPE    	Drv_Status;  	
 }WM_Bat_READ_SOC_CNF_T;
 
  typedef struct{	    
 	WM_Bat_READ_FW_TYPE    	        Drv_Status;  	
 }WM_Bat_WRITE_SOC_CNF_T;
 typedef union{
 	WM_Bat_DLImage_REQ_T			m_rWmCmdWriteReq;
 	WM_Bat_UPImage_REQ_T            m_rWmCmdUpdateReq;               
 }WM_Bat_REQ_U;
 
 typedef union{
 	WM_Bat_DLImage_CNF_T			m_rWmCmdWriteResult;
 	WM_Bat_UPImage_CNF_T            m_rWmCmdUpdateResult;
 }WM_Bat_CNF_U;
 
  typedef union{
 	WM_Bat_READ_FW_INFO_REQ_T			m_rWmFwReadReq;
 	WM_Bat_READ_SOC_REQ_T               m_rWmSocReadReq;
 	WM_Bat_WRITE_SOC_REQ_T              m_rWmSocWriteReq; 
 }WM_Bat_READ_INFO_REQ_U;
 
 typedef union{
 	WM_Bat_READ_FW_INFO_CNF_T			m_rWmFwReadResult;
 	WM_Bat_READ_SOC_CNF_T               m_rWmSocReadResult;
 	WM_Bat_WRITE_SOC_CNF_T              m_rWmSocWriteResult;
 }WM_Bat_READ_INFO_CNF_U;
 typedef struct{
     FT_H                   	    header;	
     WM_Bat_CMD_TYPE	            type;
     WM_Bat_REQ_U      	            cmd;
 }FT_BATT_REQ;
 
 typedef struct{
     FT_H                   	header;	
     WM_Bat_CMD_TYPE	        type;   /* same to the ft_batt_req's type*/
     WM_Bat_CNF_U      	        result;
     unsigned char	            status;
 }FT_BATT_CNF;
 typedef struct{
     FT_H                   	header;	
     WM_Bat_INFO_CMD_TYPE	    type;
     WM_Bat_READ_INFO_REQ_U  	cmd;
 }FT_BATT_READ_INFO_REQ;
 
 typedef struct{
     FT_H                   	    header;	
     WM_Bat_INFO_CMD_TYPE	        type;   /* same to the ft_batt_req's type*/
     WM_Bat_READ_INFO_CNF_U         result;
     unsigned char	                status;
 }FT_BATT_READ_INFO_CNF; 

#ifdef __cplusplus
extern "C" {
#endif
     //Battery interface define for FT
 BOOL Meta_Battery_Init();
 
 void Meta_Battery_OP(FT_BATT_REQ* req, BYTE* peer_buf,unsigned short peer_len);
 
 void Meta_Battery_Read_FW(FT_BATT_READ_INFO_REQ *req);
 void Meta_Battery_UPdate_FW(FT_BATT_REQ* req,  BYTE* bDataAddress,unsigned short data_number);
 BOOL Meta_Battery_Deinit();
#ifdef __cplusplus
}
#endif     
     
#endif    
     
