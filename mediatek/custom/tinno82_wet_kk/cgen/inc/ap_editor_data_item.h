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


#ifndef AP_EDTIOR_DATA_ITEM_SYSTEM_H
#define AP_EDTIOR_DATA_ITEM_SYSTEM_H

#include "NVRAM_Data_Type.h"
#include "cfg_module_file.h"
#include "CFG_file_lid.h"
//#include "CFG_File_LID.h"

// add by chipeng , temporary modify
//#include "../../custom/oppo/cgen/inc/Custom_NvRam_LID.h"
#include "Custom_NvRam_LID.h"

/***************************************************************************** 
* Include
*****************************************************************************/

/***************************************************************************** 
* Define
*****************************************************************************/
typedef struct
{
	char	cFileVer[4];
}File_Ver_Struct;



/***************************************************************************** 
* META Description
*****************************************************************************/
BEGIN_NVRAM_DATA


/***********************************************************************
  ***  This is a nvram data item bit level description for meta tools nvram editor
  ***
  ***  Logical Data Item ID : AP_CFG_FILE_AUXADC_LID
  ***
  ***  Module: 
  ***
  ***  Description:  
  ***
  ***  Maintainer: 
  ***
  ***********************************************************************/
LID_BIT VER_LID(AP_CFG_RDCL_FILE_AUXADC_LID)
     AUXADC_CFG_Struct *CFG_FILE_AUXADC_REC_TOTAL
     {
 
     };
     
LID_BIT VER_LID(AP_CFG_RDEB_FILE_BT_ADDR_LID)
     ap_nvram_btradio_mt6610_struct *CFG_FILE_BT_ADDR_REC_TOTAL
     {
 
     };      

LID_BIT VER_LID(AP_CFG_RDCL_FACTORY_LID)     
    FACTORY_CFG_Struct *CFG_FILE_FACTORY_REC_TOTAL     
    {

    };  

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_PARA_LID)     
    NVRAM_CAMERA_PARA_STRUCT *CFG_FILE_CAMERA_PARA_REC_TOTAL     
    {

    }; 

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_3A_LID)     
    NVRAM_CAMERA_3A_STRUCT *CFG_FILE_CAMERA_3A_REC_TOTAL     
    {

    }; 

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_SHADING_LID)     
    NVRAM_CAMERA_SHADING_STRUCT *CFG_FILE_CAMERA_SHADING_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_DEFECT_LID)     
    NVRAM_CAMERA_DEFECT_STRUCT *CFG_FILE_CAMERA_DEFECT_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_SENSOR_LID)     
    NVRAM_SENSOR_DATA_STRUCT *CFG_FILE_CAMERA_SENSOR_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_LENS_LID)     
    NVRAM_LENS_PARA_STRUCT *CFG_FILE_CAMERA_LENS_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_BWCS_LID)
    ap_nvram_bwcs_config_struct *CFG_FILE_BWCS_CONFIG_TOTAL
    {
        
    }; 
     
LID_BIT VER_LID(AP_CFG_RDCL_HWMON_ACC_LID)
    NVRAM_HWMON_ACC_STRUCT *CFG_FILE_HWMON_ACC_REC_TOTAL
    {
        
    }; 
      
END_NVRAM_DATA

#endif /* AP_EDTIOR_DATA_ITEM_SYSTEM_H */ 

