/******************************************************************************
  Copyright Statement:*  --------------------*  This software is protected by 
Copyright and the information contained*  herein is confidential. The 
software may not be copied and the information*  contained herein may not be 
used or disclosed except with the written*  permission of MediaTek Inc. (C) 
2011**  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND 
AGREES*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE
")*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER 
ON*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL 
WARRANTIES,*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR 
NONINFRINGEMENT.*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
RESPECT TO THE*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
INCORPORATED IN, OR*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES 
TO LOOK ONLY TO SUCH*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. 
MEDIATEK SHALL ALSO*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES 
MADE TO BUYER'S*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR 
OPEN FORUM.**  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND 
CUMULATIVE*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED 
HEREUNDER WILL BE,*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK 
SOFTWARE AT ISSUE,*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE 
PAID BY BUYER TO*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. **  THE 
TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE*  WITH 
THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF*  LAWS 
PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND*  
RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER*  
THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC)
.*****************************************************************************/
#ifndef ES_REG_H
#define ES_REG_H



/******************************************************************************
 * CONSTANT DEFINITIONS                                                       
 ******************************************************************************/
#define INCORRECT_REG_INDEX          0xFFFFFFFF    /* incorrect register index */


/******************************************************************************
 * TYPE DEFINITIONS                                                            
******************************************************************************/
typedef enum
{   
    E_SPARE0 = 1,
    E_SPARE1,
    E_SPARE2,
    E_HW_RES0,
    E_HW_RES1,
    E_HW_RES2,
    E_MAX
} REG_INDEX;

typedef enum
{   
    MASK_R_SPARE0_ADC_CALI_EN = 0,
    MASK_R_SPARE0_DEGC_CALI,        
    MASK_R_SPARE0_O_SLOPE_SIGN,      
    MASK_R_SPARE0_O_VGS,
    MASK_R_SPARE0_O_VTS,
    MASK_R_SPARE0_O_SLOPE,
    MASK_R_SPARE1_ADC_OE,
    MASK_R_SPARE1_NON_ANALOG_PART,
    MASK_R_SPARE2_ADC_GE,
    MASK_R_SPARE2_NON_ANALOG_PART,
    MASK_R_HW_RES0_AUXADC_DAT,
    MASK_R_HW_RES0_NON_ANALOG_PART,
    MASK_R_HW_RES1_MIPI_DAT0,
    MASK_R_HW_RES1_MIPI_DAT1,
    MASK_R_HW_RES1_MIPI_DAT2,
    MASK_R_HW_RES1_MIPI_DAT3,
    MASK_R_HW_RES1_VDAC_EN,
    MASK_R_HW_RES1_VDAC_TRIM_VAL,
    MASK_R_HW_RES1_VDAC_DAT_REV,
    MASK_R_HW_RES2_BGR_CTRL,
    MASK_R_HW_RES2_BGR_RSEL,
    MASK_R_HW_RES2_BGR_CTRL_EN,
    MASK_R_HW_RES2_BGR_RSEL_EN,
    MASK_R_HW_RES2_NON_ANALOG_PART,
} REG_MASK_INDEX;


/******************************************************************************
 * EXPORT FUNCTION
 ******************************************************************************/
extern unsigned int seclib_get_param(REG_INDEX reg_index, REG_MASK_INDEX mask);


#endif /* ES_REG_H*/

