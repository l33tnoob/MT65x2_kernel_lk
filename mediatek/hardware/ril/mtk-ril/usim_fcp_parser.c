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
 *    usim_fcp_parser.c
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
#include <telephony/ril.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include <getopt.h>
#include <cutils/properties.h>

#include "usim_fcp_parser.h"

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

/*****************************************************************************
 * FUNCTION
 *  fcp_tlv_search_tag
 * DESCRIPTION
 *  Search for the tag in the input ptr and return the length and value ptr of the tag.
 * PARAMETERS
 *  in_ptr      [IN]        Kal_uint8 * input buffer pointer (from the value part of fcp template)
 *  len         [IN]        Kal_int16  input buffer length
 *  tag         [IN]        Usim_fcp_tag_enum tag to be found
 *  out_ptr     [OUT]       Kal_uint8 ** the address of found data value.
 * RETURNS
 *  the length of the tag searched
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_tlv_search_tag(unsigned char *in_ptr, unsigned short len, usim_fcp_tag_enum tag, unsigned char **out_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char tag_len = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    for (*out_ptr = in_ptr; len > 0; *out_ptr += tag_len)
    {
        tag_len = (*(*out_ptr + 1) + 2);
        if (**out_ptr == (unsigned char) tag)
        {
            *out_ptr += 2;
            return *(*out_ptr - 1);
        }
        len -= tag_len;
    }
    if(len != 0)
    {
        assert(0);
    }
    *out_ptr = NULL;
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *  prop_info_tlv_search_tag
 * DESCRIPTION
 *  Search for the tag in the input ptr and return the length and value ptr of the tag.
 * PARAMETERS
 *  in_ptr      [IN]        Kal_uint8 * input buffer pointer (from the value part of fcp template)
 *  len         [IN]        Kal_int16  input buffer length
 *  tag         [IN]        Usim_fcp_tag_enum tag to be found
 *  out_ptr     [OUT]       Kal_uint8 ** the address of found data value.
 * RETURNS
 *  the length of the tag searched
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char prop_info_tlv_search_tag(
            unsigned char *in_ptr,
            unsigned short len,
            usim_fcp_proprietary_info_tag_enum tag,
            unsigned char **out_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    for (*out_ptr = in_ptr; len > 0; len -= (*(*out_ptr + 1) + 2), *out_ptr += (*(*out_ptr + 1) + 2))
    {
        if (**out_ptr == (unsigned char) tag)
        {
            *out_ptr += 2;
            return *(*out_ptr - 1);
        }
    }
    if(len != 0)
    {
        assert(0);
    }		
    *out_ptr = NULL;
    return 0;
}


/*****************************************************************************
 * FUNCTION
 *  fcp_file_descriptor_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_file_descriptor_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_file_descriptor_query(unsigned char *in_ptr, unsigned short len, usim_file_descriptor_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_file_descriptor_struct *query_ptr = filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_FILE_DES_T, &out_ptr);
    /* File descriptor is mandatory for all FCP template */
    if(!((NULL != out_ptr) && ((value_len == 2) || (value_len == 5))))
    {
        assert(0);
    }	
    query_ptr->fd = out_ptr[0];
    query_ptr->data_coding = out_ptr[1];
    if (value_len == 5)
    {
        query_ptr->rec_len = (short) ((out_ptr[2] << 8) | out_ptr[3]);
        query_ptr->num_rec = out_ptr[4];
    }
    else
    {
        query_ptr->rec_len = 0;
        query_ptr->num_rec = 0;
    }
    return TRUE;
}


/*****************************************************************************
 * FUNCTION
 *  fcp_file_identifier_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_file_identifier_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_file_identifier_query(unsigned char *in_ptr, unsigned short len, usim_file_identifier_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_file_identifier_struct *query_ptr = (usim_file_identifier_struct*) filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_FILE_ID_T, &out_ptr);
    if (NULL != out_ptr)
    {
        if(value_len != 2)
        {
            assert(0);
        }			
        query_ptr->file_id = (short) ((out_ptr[0] << 8) | out_ptr[1]);
        return TRUE;
    }
    else
    {
        query_ptr->file_id = 0;
        return FALSE;
    }
}


/*****************************************************************************
 * FUNCTION
 *  fcp_df_name_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_df_name_sruct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_df_name_query(unsigned char *in_ptr, unsigned short len, usim_df_name_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_df_name_struct *query_ptr = (usim_df_name_struct*) filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_DF_NAME_T, &out_ptr);
    if(value_len > 16)
    {
        assert(0);
    }		
    memset(query_ptr->df_name, 0x00, 16);
    if (NULL != out_ptr)
    {
        query_ptr->length = value_len;
        memcpy(query_ptr->df_name, out_ptr, value_len);
        return TRUE;
    }
    else
    {
        query_ptr->length = 0;
        return FALSE;
    }
}


/*****************************************************************************
 * FUNCTION
 *  fcp_proprietary_info_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_proprietary_information_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_proprietary_info_query(
            unsigned char *in_ptr,
            unsigned short len,
            usim_proprietary_information_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_proprietary_information_struct *query_ptr = filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    memset(query_ptr, 0x00, sizeof(usim_proprietary_information_struct));

    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_PROPRIETARY_T, &out_ptr);
	if(value_len > 17) {
	    LOGW("WARNING_FCP_PROP_INFO_SIZE:%d",value_len);
    }

    if (value_len)
    {
        unsigned char *prop_ptr = NULL;
        unsigned char prop_len = 0;

        /* Query UICC characteristics */
        prop_len = prop_info_tlv_search_tag(out_ptr, (unsigned short) value_len, PROP_UICC_CHAR_T, &prop_ptr);
        if (prop_len)
        {
            if(prop_len != 1)
            {
                assert(0);
            }				
            SET_PROP_UICC_CHAR_EXIST(query_ptr->do_flag);
            query_ptr->char_byte = prop_ptr[0];
        }
        /* Query Application power consumption */
        prop_len = prop_info_tlv_search_tag(out_ptr, (unsigned short) value_len, PROP_APP_PWR_T, &prop_ptr);
        if (prop_len)
        {
            if(prop_len != 3)
            {
                assert(0);
            }				
            SET_PROP_APP_PWR_EXIST(query_ptr->do_flag);
            query_ptr->supp_volt_class = prop_ptr[0];
            query_ptr->app_pwr_consump = prop_ptr[1];
            query_ptr->pwr_ref_freq = prop_ptr[2];
        }

        /* Query Minimum application clock frequency */
        prop_len = prop_info_tlv_search_tag(out_ptr, (unsigned short) value_len, PROP_MIN_APP_CLK_T, &prop_ptr);

        if (prop_len)
        {
            if(prop_len != 1)
            {
                assert(0);
            }				
            SET_PROP_MIN_APP_CLK_EXIST(query_ptr->do_flag);
            query_ptr->app_min_freq = out_ptr[0];
        }

        /* Query Amount of available memory */
        prop_len = prop_info_tlv_search_tag(out_ptr, (unsigned short) value_len, PROP_AVAIL_MEM_T, &prop_ptr);
        if (prop_len)
        {
            unsigned char i = 0;

            if(prop_len > 4)
            {
                assert(0);
            }				
            SET_PROP_AVAIL_MEM_EXIST(query_ptr->do_flag);
            for (i = 0; i < prop_len; i++)
            {
                query_ptr->available_mem_bytes |= (unsigned int) (prop_ptr[prop_len - i - 1] << i * 8);
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*****************************************************************************
 * FUNCTION
 *  fcp_life_cycle_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_life_cycle_status_integer_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_life_cycle_query(
            unsigned char *in_ptr,
            unsigned short len,
            usim_life_cycle_status_integer_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_life_cycle_status_integer_struct *query_ptr = filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_LIFE_CYCLE_T, &out_ptr);
    if(!((NULL != out_ptr) && (value_len == 1)))  /* Life Cycle Status Integer is mandatory */
    {
          assert(0);
    }
    query_ptr->life_cycle_status = *out_ptr;
    return TRUE;
}


/*****************************************************************************
 * FUNCTION
 *  fcp_security_attribute_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_security_attributes_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_security_attribute_query(
            unsigned char *in_ptr,
            unsigned short len,
            usim_security_attributes_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    //unsigned char value_len = 0;
    //unsigned char * out_ptr = NULL; 
    //usim_security_attributes_struct * query_ptr = filled_struct_ptr;            
    return FALSE;

}


/*****************************************************************************
 * FUNCTION
 *  fcp_pin_status_do_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_pin_status_temp_do_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_pin_status_do_query(unsigned char *in_ptr, unsigned short len, usim_pin_status_temp_do_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned short value_len = 0;
    unsigned char *out_ptr = NULL;
    unsigned char *tag = NULL;
    unsigned char *pin_ps_do_ptr = NULL;
    unsigned char pin_ps_do_len = 0;
    unsigned char length = 0, count = 0;
    usim_pin_status_temp_do_struct *query_ptr = filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_PIN_DO_T, &out_ptr);

    if (NULL != out_ptr)
    {
        tag = out_ptr;
        while (value_len > 0)
        {
            length = *(tag + 1);
            switch (*tag)
            {
                case PIN_PS_DO_T:
                    /* keep for next time used! */
                    pin_ps_do_ptr = &(tag[2]);
                    pin_ps_do_len = length;
                    break;
                case PIN_USAGE_QUALIFIER_T:
                    LOGD("PIN_DO: usage qualifier found!");
                    break;
                case PIN_KEY_REF_T:
                    if((count / 8) >= length)
                    {
                        assert(0);
                    }						
                    switch (tag[2])
                    {
                        case USIM_PIN1_APP1:
                            query_ptr->pin_flag |= APP1_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP1_PIN1_M;
                            }
                            break;
                        case USIM_PIN1_APP2:
                            query_ptr->pin_flag |= APP2_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP2_PIN1_M;
                            }

                            break;
                        case USIM_PIN1_APP3:
                            query_ptr->pin_flag |= APP3_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP3_PIN1_M;
                            }
                            break;
                        case USIM_PIN1_APP4:
                            query_ptr->pin_flag |= APP4_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP4_PIN1_M;
                            }
                            break;
                        case USIM_PIN1_APP5:
                            query_ptr->pin_flag |= APP5_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP5_PIN1_M;
                            }
                            break;
                        case USIM_PIN1_APP6:
                            query_ptr->pin_flag |= APP6_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP6_PIN1_M;
                            }
                            break;
                        case USIM_PIN1_APP7:
                            query_ptr->pin_flag |= APP7_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP7_PIN1_M;
                            }
                            break;
                        case USIM_PIN1_APP8:
                            query_ptr->pin_flag |= APP8_PIN1_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP8_PIN1_M;
                            }
                            break;
                        case USIM_PIN2_APP1:
                            query_ptr->pin_flag |= APP1_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP1_PIN2_M;
                            }
                            break;
                        case USIM_PIN2_APP2:
                            query_ptr->pin_flag |= APP2_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP2_PIN2_M;
                            }
                            break;
                        case USIM_PIN2_APP3:
                            query_ptr->pin_flag |= APP3_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP3_PIN2_M;
                            }
                            break;
                        case USIM_PIN2_APP4:
                            query_ptr->pin_flag |= APP4_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP4_PIN2_M;
                            }
                            break;
                        case USIM_PIN2_APP5:
                            query_ptr->pin_flag |= APP5_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP5_PIN2_M;
                            }
                            break;
                        case USIM_PIN2_APP6:
                            query_ptr->pin_flag |= APP6_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP6_PIN2_M;
                            }
                            break;
                        case USIM_PIN2_APP7:
                            query_ptr->pin_flag |= APP7_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP7_PIN2_M;
                            }
                            break;
                        case USIM_PIN2_APP8:
                            query_ptr->pin_flag |= APP8_PIN2_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= APP8_PIN2_M;
                            }
                            break;
                        case USIM_PIN_ADM1:
                            query_ptr->pin_flag |= ADM01_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM01_M;
                            }
                            break;
                        case USIM_PIN_ADM2:
                            query_ptr->pin_flag |= ADM02_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM02_M;
                            }
                            break;
                        case USIM_PIN_ADM3:
                            query_ptr->pin_flag |= ADM03_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM03_M;
                            }
                            break;
                        case USIM_PIN_ADM4:
                            query_ptr->pin_flag |= ADM04_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM04_M;
                            }
                            break;
                        case USIM_PIN_ADM5:
                            query_ptr->pin_flag |= ADM05_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM05_M;
                            }
                            break;
                        case USIM_PIN_ADM6:
                            query_ptr->pin_flag |= ADM06_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM06_M;
                            }
                            break;
                        case USIM_PIN_ADM7:
                            query_ptr->pin_flag |= ADM07_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM07_M;
                            }
                            break;
                        case USIM_PIN_ADM8:
                            query_ptr->pin_flag |= ADM08_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM08_M;
                            }
                            break;
                        case USIM_PIN_ADM9:
                            query_ptr->pin_flag |= ADM09_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM09_M;
                            }
                            break;
                        case USIM_PIN_ADM10:
                            query_ptr->pin_flag |= ADM10_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= ADM10_M;
                            }
                            break;
                        case USIM_PIN_UNIV:
                            query_ptr->pin_flag |= UNIV_PIN_M;
                            if (pin_ps_do_ptr[count / 8] & (0x80 >> (count % 8)))
                            {
                                query_ptr->enabled_pin_flag |= UNIV_PIN_M;
                            }
                            break;
                        default:
                            break;
                    }
                    count++;
                    break;
                default:
                    assert(0);
                    break;
            }
            tag += (length + 2);
            value_len -= (length + 2);
        }
        return TRUE;
    }
    else
    {
        query_ptr->pin_flag = 0;
        return FALSE;
    }
}


/*****************************************************************************
 * FUNCTION
 *  fcp_total_file_size_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_total_file_size_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_total_file_size_query(unsigned char *in_ptr, unsigned short len, usim_total_file_size_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_total_file_size_struct *query_ptr = filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_TOTAL_FILE_SIZE_T, &out_ptr);
    if (NULL != out_ptr)
    {
        unsigned char i = 0;

        if(value_len > 4)
        {
            assert(0);
        }			
        for (i = 0; i < value_len; i++)
        {
            query_ptr->tot_file_size |= (unsigned int) (out_ptr[value_len - i - 1] << i * 8);
        }
        return TRUE;
    }
    else
    {
        query_ptr->tot_file_size = 0;
        return FALSE;
    }
}


/*****************************************************************************
 * FUNCTION
 *  fcp_file_size_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_file_size_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_file_size_query(unsigned char *in_ptr, unsigned short len, usim_file_size_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_file_size_struct *query_ptr = filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_FILE_SIZE_T, &out_ptr);
    if (NULL != out_ptr)
    {
        if(value_len != 2)
        {
            assert(0);
        }			
        query_ptr->file_size = (short) (out_ptr[0] << 8 | out_ptr[1]);
        return TRUE;
    }
    else
    {
        query_ptr->file_size = 0;
        return FALSE;
    }
}


/*****************************************************************************
 * FUNCTION
 *  fcp_sfi_query
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  filled_struct_ptr       [OUT]       Usim_short_file_identifier_struct *
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char fcp_sfi_query(unsigned char *in_ptr, unsigned short len, usim_short_file_identifier_struct *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = 0;
    unsigned char *out_ptr = NULL;
    usim_short_file_identifier_struct *query_ptr = filled_struct_ptr;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    value_len = fcp_tlv_search_tag(in_ptr, len, FCP_SFI_T, &out_ptr);
    if (NULL != out_ptr)
    {
        if (value_len == 0)
        {
            query_ptr->sfi_usage = SFI_NOT_SUPPORT;
        }
        else
        {
            if(value_len != 1)
            {
                assert(0);
            }				
            query_ptr->sfi_usage = SFI_PROP_ID;
            query_ptr->sfi_prop = out_ptr[0];
        }
    }
    else
    {
        query_ptr->sfi_usage = SFI_FILE_ID;
    }
    /* This is because this TLV always its meaning even when it is not present */
    return TRUE;
}


/*****************************************************************************
 * FUNCTION
 *  usim_fcp_query_tag
 * DESCRIPTION
 *  Fill the queried structure if it exists.
 * PARAMETERS
 *  in_ptr                  [IN]        Kal_uint8 * input buffer pointer
 *  len                     [IN]        Kal_uint16  length of the input buffer
 *  tag                     [IN]        Usim_fcp_tag_enum tag to be found
 *  filled_struct_ptr       [OUT]       Void * (shall be casted by structure)
 * RETURNS
 *  unsigned char if the structure is successfully filled.
 * GLOBALS AFFECTED
 *  void
 *****************************************************************************/
unsigned char usim_fcp_query_tag(unsigned char *in_ptr, unsigned short len, usim_fcp_tag_enum tag, void *filled_struct_ptr)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char value_len = in_ptr[1];
    unsigned char *data_ptr = &in_ptr[2];

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if((len == 0) || (in_ptr[0] != FCP_TEMP_T))
    {
        assert(0);
    }		
    /* We shall handle the 2 bytes length now we only handle one */

    switch (tag)
    {
        case FCP_FILE_DES_T:
            return fcp_file_descriptor_query(data_ptr, value_len, (usim_file_descriptor_struct*) filled_struct_ptr);
            break;
        case FCP_FILE_ID_T:
            return fcp_file_identifier_query(data_ptr, value_len, (usim_file_identifier_struct*) filled_struct_ptr);
            break;
        case FCP_DF_NAME_T:
            return fcp_df_name_query(data_ptr, value_len, (usim_df_name_struct*) filled_struct_ptr);
            break;
        case FCP_PROPRIETARY_T:
            return fcp_proprietary_info_query(
                    data_ptr,
                    value_len,
                    (usim_proprietary_information_struct*) filled_struct_ptr);
            break;
        case FCP_LIFE_CYCLE_T:
            return fcp_life_cycle_query(
                    data_ptr,
                    value_len,
                    (usim_life_cycle_status_integer_struct*) filled_struct_ptr);
            break;
        case FCP_SEC_ATTRIBUTE_QUERY:
            return fcp_security_attribute_query(
                    data_ptr,
                    value_len,
                    (usim_security_attributes_struct*) filled_struct_ptr);
            break;
        case FCP_PIN_DO_T:
            return fcp_pin_status_do_query(data_ptr, value_len, (usim_pin_status_temp_do_struct*) filled_struct_ptr);
            break;
        case FCP_TOTAL_FILE_SIZE_T:
            return fcp_total_file_size_query(data_ptr, value_len, (usim_total_file_size_struct*) filled_struct_ptr);
            break;
        case FCP_FILE_SIZE_T:
            return fcp_file_size_query(data_ptr, value_len, (usim_file_size_struct*) filled_struct_ptr);
            break;
        case FCP_SFI_T:
            return fcp_sfi_query(data_ptr, value_len, (usim_short_file_identifier_struct*) filled_struct_ptr);
            break;
        default:
            assert(0);
            return FALSE;
            break;
    }
}


static int toByte(char c)
{
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    LOGE("toByte Error: %c",c);
    return 0;
}

int hexStringToByteArray(unsigned char* hexString, unsigned char ** byte)
{
    int length = strlen((char*)hexString);
    unsigned char* buffer = malloc(length / 2);
    int i = 0;

    for (i = 0 ; i < length ; i += 2)
    {
        buffer[i / 2] = (unsigned char)((toByte(hexString[i]) << 4) | toByte(hexString[i+1]));
    }

    *byte = buffer;
    
    return (length/2);
}    

const char HEX_DIGITS[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };


unsigned char* byteArrayToHexString(unsigned char* array,int length)
{
    unsigned char* buf = malloc(length*2+1);
    int bufIndex = 0;
    int i = 0;
    for (i = 0 ; i < length; i++) 
    {
        unsigned char b = array[i];
        buf[bufIndex++] = HEX_DIGITS[(b >> 4) & 0x0F];
        buf[bufIndex++] = HEX_DIGITS[b & 0x0F];
    }
    buf[bufIndex] = '\0';
    return buf;        
}


