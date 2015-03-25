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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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
 * Filename:
 * ---------
 *    usim_fcp_parser.h
 *
 * Project:
 * --------
 *    MONZA
 *
 * Description:
 * ------------
 *   
 *
 * Author:
 * -------
 *    PH Shih
 *
 *******************************************************************************/
#ifndef USIM_FCP_PARSER_H
#define USIM_FCP_PARSER_H

#define TRUE  1
#define FALSE 0

#define USIM_MAX_RSP_FCP_LEN 128
#define FCP_TEMP_T 0x62

/* Define application template Data Object  */
#define USIM_APP_TEMP_TAG 0x61
#define USIM_APP_ID_TAG 0x4F
#define USIM_APP_LABEL_TAG 0x50

/* USIM AUTHENTICATE TAG */
typedef enum
{
    AUTH_3G_SUCCESS_T = 0xDB,
    AUTH_3G_SYNC_FAILURE_T = 0xDC
} usim_auth_tag_enum;

/* FCP template tag */
typedef enum
{
    FCP_FILE_DES_T = 0x82,
    FCP_FILE_ID_T = 0x83,
    FCP_DF_NAME_T = 0x84,
    FCP_PROPRIETARY_T = 0xA5,
    FCP_LIFE_CYCLE_T = 0x8A,
    /* FCP_SEC_ATTRIBUTE_QUERY is for query */
    FCP_SEC_ATTRIBUTE_QUERY = 0xFF,
    FCP_SEC_COMPACT_T = 0x8C,
    FCP_SEC_EXPANDED_T = 0xAB,
    FCP_SEC_REF_T = 0x8B,
    /* end of security attribute query */
    FCP_PIN_DO_T = 0xC6,
    FCP_TOTAL_FILE_SIZE_T = 0x81,
    FCP_FILE_SIZE_T = 0x80,
    FCP_SFI_T = 0x88
} usim_fcp_tag_enum;

/* FCP proprietary information tag */
typedef enum
{
    PROP_UICC_CHAR_T = 0x80,
    PROP_APP_PWR_T = 0x81,
    PROP_MIN_APP_CLK_T = 0x82,
    PROP_AVAIL_MEM_T = 0x83
} usim_fcp_proprietary_info_tag_enum;

/* FCP security attributes tag */
typedef enum
{
    SECURITY_COMPACT_T = 0x8C,
    SECURITY_EXPANDED_T = 0xAB,
    SECURITY_REF_T = 0x8B
} usim_fcp_security_attributes_tag_enum;

/* FCP PIN status template DO tag */
typedef enum
{
    PIN_PS_DO_T = 0x90,
    PIN_USAGE_QUALIFIER_T = 0x95,
    PIN_KEY_REF_T = 0x83
} usim_fcp_pin_status_tag_enum;

typedef struct
{
    unsigned char fd;
    unsigned char data_coding;
    unsigned short rec_len;
    unsigned char num_rec;
} usim_file_descriptor_struct;

typedef struct
{
    unsigned short file_id;
} usim_file_identifier_struct;

typedef struct
{
    unsigned char length;
    unsigned char df_name[16];
} usim_df_name_struct;

/* USIM proprietary information */
#define SET_PROP_UICC_CHAR_EXIST(X)        ((X) |= 0x01)
#define SET_PROP_APP_PWR_EXIST(X)           ((X) |= 0x02)
#define SET_PROP_MIN_APP_CLK_EXIST(X)    ((X) |= 0x04)
#define SET_PROP_AVAIL_MEM_EXIST(X)       ((X) |= 0x08)

#define IF_PROP_UICC_CHAR_EXIST(X)      ((X)&0x01)
#define IF_PROP_APP_PWR_EXIST(X)         ((X)&0x02)
#define IF_PROP_MIN_APP_CLK_EXIST(X)  ((X)&0x04)
#define IF_PROP_AVAIL_MEM_EXIST(X)      ((X)&0x08)

typedef struct
{
    unsigned char do_flag;              /* data object exist flags */
    unsigned char char_byte;            /* usim uicc characteristics */
    unsigned char supp_volt_class;      /* application power consumption */
    unsigned char app_pwr_consump;      /* application power consumption */
    unsigned char pwr_ref_freq;         /* application power consumption */
    unsigned char app_min_freq;         /* minimum application clock frequency */
    unsigned int available_mem_bytes; /* limit to 4 bytes */
} usim_proprietary_information_struct;

/* ---------End of USIM proprietary nformation--------- */

typedef struct
{
    unsigned char life_cycle_status;
} usim_life_cycle_status_integer_struct;

typedef struct
{
    unsigned char do_flag;
} usim_security_attributes_struct;

typedef enum
{
    USIM_PIN1_APP1 = 0x01,
    USIM_PIN1_APP2 = 0x02,
    USIM_PIN1_APP3 = 0x03,
    USIM_PIN1_APP4 = 0x04,
    USIM_PIN1_APP5 = 0x05,
    USIM_PIN1_APP6 = 0x06,
    USIM_PIN1_APP7 = 0x07,
    USIM_PIN1_APP8 = 0x08,    
    USIM_PIN_ADM1 = 0x0A,
    USIM_PIN_ADM2 = 0x0B,
    USIM_PIN_ADM3 = 0x0C,
    USIM_PIN_ADM4 = 0x0D,
    USIM_PIN_ADM5 = 0x0E,
    USIM_PIN_UNIV = 0x11,
    USIM_PIN2_APP1 = 0x81,
    USIM_PIN2_APP2 = 0x82,
    USIM_PIN2_APP3 = 0x83,
    USIM_PIN2_APP4 = 0x84,
    USIM_PIN2_APP5 = 0x85,
    USIM_PIN2_APP6 = 0x86,
    USIM_PIN2_APP7 = 0x87,
    USIM_PIN2_APP8 = 0x88,
    USIM_PIN_ADM6 = 0x8A,
    USIM_PIN_ADM7 = 0x8B,
    USIM_PIN_ADM8 = 0x8C,
    USIM_PIN_ADM9 = 0x8D,
    USIM_PIN_ADM10 = 0x8E
}usim_pin_type_enum;

#define APP1_PIN1_M 0x00000001
#define APP1_PIN2_M 0x00000002
#define APP2_PIN1_M 0x00000004
#define APP2_PIN2_M 0x00000008
#define APP3_PIN1_M 0x00000010
#define APP3_PIN2_M 0x00000020
#define APP4_PIN1_M 0x00000040
#define APP4_PIN2_M 0x00000080
#define APP5_PIN1_M 0x00000100
#define APP5_PIN2_M 0x00000200
#define APP6_PIN1_M 0x00000400
#define APP6_PIN2_M 0x00000800
#define APP7_PIN1_M 0x00001000
#define APP7_PIN2_M 0x00002000
#define APP8_PIN1_M 0x00004000
#define APP8_PIN2_M 0x00008000
#define ADM01_M        0x00010000
#define ADM02_M        0x00020000
#define ADM03_M        0x00040000
#define ADM04_M        0x00080000
#define ADM05_M        0x00100000
#define ADM06_M        0x00200000
#define ADM07_M        0x00400000
#define ADM08_M        0x00800000
#define ADM09_M        0x01000000
#define ADM10_M        0x02000000
#define UNIV_PIN_M        0x80000000

typedef struct
{
    unsigned int pin_flag;
    unsigned int enabled_pin_flag;
} usim_pin_status_temp_do_struct;

typedef struct
{
    unsigned int tot_file_size;
} usim_total_file_size_struct;

typedef struct
{
    unsigned short file_size;
} usim_file_size_struct;

typedef enum
{
    SFI_NOT_SUPPORT,
    SFI_FILE_ID,
    SFI_PROP_ID
} sfi_usage_enum;

typedef struct
{
    sfi_usage_enum sfi_usage;
    unsigned char sfi_prop;
} usim_short_file_identifier_struct;

/* File descriptor byte */
#define IS_SHAREABLE_FILE(X) (0x40 & (X))
#define IS_DF_ADF(X)    (((0x38 & (X)) == 0x38) ? TRUE : FALSE)
#define IS_TRANSPARENT_EF(X) (((0x07 & (X)) == 0x01) ? TRUE : FALSE)
#define IS_LINEAR_FIXED_EF(X) (((0x07 & (X)) == 0x02) ? TRUE : FALSE)
#define IS_CYCLIC_EF(X) (((0x07 & (X)) == 0x06) ? TRUE : FALSE)

#define COMMAND_READ_BINARY     0xb0
#define COMMAND_UPDATE_BINARY   0xd6
#define COMMAND_READ_RECORD     0xb2
#define COMMAND_UPDATE_RECORD   0xdc
#define COMMAND_SEEK            0xa2
#define COMMAND_GET_RESPONSE    0xc0

//***** types of files  TS 11.11 9.3
#define EF_TYPE_TRANSPARENT  0
#define EF_TYPE_LINEAR_FIXED  1
#define EF_TYPE_CYCLIC  3

//***** types of files  TS 11.11 9.3
#define TYPE_RFU 0
#define TYPE_MF  1
#define TYPE_DF  2
#define TYPE_EF  4

// size of GET_RESPONSE for EF's
#define GET_RESPONSE_EF_SIZE_BYTES 15
#define GET_RESPONSE_EF_IMG_SIZE_BYTES 10

// Byte order received in response to COMMAND_GET_RESPONSE
// Refer TS 51.011 Section 9.2.1
#define RESPONSE_DATA_RFU_1 0
#define RESPONSE_DATA_RFU_2 1

#define RESPONSE_DATA_FILE_SIZE_1 2
#define RESPONSE_DATA_FILE_SIZE_2 3

#define RESPONSE_DATA_FILE_ID_1 4
#define RESPONSE_DATA_FILE_ID_2 5
#define RESPONSE_DATA_FILE_TYPE 6
#define RESPONSE_DATA_RFU_3 7
#define RESPONSE_DATA_ACCESS_CONDITION_1 8
#define RESPONSE_DATA_ACCESS_CONDITION_2 9
#define RESPONSE_DATA_ACCESS_CONDITION_3 10
#define RESPONSE_DATA_FILE_STATUS 11
#define RESPONSE_DATA_LENGTH 12
#define RESPONSE_DATA_STRUCTURE 13
#define RESPONSE_DATA_RECORD_LENGTH 14

extern unsigned char usim_fcp_query_tag(unsigned char *, unsigned short, usim_fcp_tag_enum, void *);
extern unsigned char fcp_tlv_search_tag(unsigned char *in_ptr, unsigned short len, usim_fcp_tag_enum tag, unsigned char **out_ptr);
extern int hexStringToByteArray(unsigned char * hexString, unsigned char ** byte);
extern unsigned char *byteArrayToHexString(unsigned char* array,int length);

#endif /* USIM_FCP_PARSER_H */ 

