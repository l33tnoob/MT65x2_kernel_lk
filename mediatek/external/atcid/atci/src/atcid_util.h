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

#ifndef ATCID_UTIL_H
#define ATCID_UTIL_H

#define STX 0x02
#define ETX 0x03
#define CR  0x0D 
#define LF  0x0A

#define AT_A 'A'
#define AT_T 'T'
#define AT_END '\n'

#define AT_SET "="
#define AT_READ "?"
#define AT_TEST "=?"

#define AT_STRING "AT"
#define AT_OK_STRING "OK"
#define AT_ERROR_STRING "ERROR"
#define AT_NOT_IMPL_STRING "Not Implement"
#define AT_UNKNOWN_STRING "UNKNOWN"
#define AT_CRLF_STRING "\r\n"

#define AT_EXTEND_CHAR '+'
#define AT_EXTEND_STAR_CHAR '*'
#define AT_EXTEND_PER_CHAR '%'

#define AT_STX 0x02
#define AT_ETX 0x03
#define AT_CR 0x0D
#define AT_LF 0x0A


typedef enum
{
    AT_NONE_OP = 1,
    AT_BASIC_OP = 2,
    AT_ACTION_OP = 4,       //Action Commands    
    AT_READ_OP = 8,         //Query Commands 
    AT_TEST_OP= 16,          //Range Commands 
    AT_SET_OP= 32          //Set Commands 
} ATOP_t;

typedef enum 
{  
  AT_OK             = 0,    /*  AT: OK            */
  AT_CONNECT        = 1,    /*  AT: CONNECT       */
  AT_RING           = 2,    /*  AT: RING          */
  AT_NO_CARRIER     = 3,    /*  AT: NO CARRIER    */
  AT_ERROR          = 4,    /*  AT: ERROR         */
  AT_NOT_IMPL       = 101,   /*  Not Implement     */
} ATRESPONSE_t;

#define UCASE(A) (((A >= 'a') && (A <= 'z')) ? (A-0x20) : (A))

void  convertToUpperCase(char * str, char endCh);
void  ascii_to_hex(char *in_ascii_string, char *out_hex_string, int in_ascii_string_size);
int   trim_string(char **start);
ATOP_t getAtOp(char** cmdLine);
#endif

