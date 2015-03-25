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

#include "atcid_util.h"
#include "atcid.h"
#include "at_tok.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

/********************************
 * Convert string to UpperCase *
 *********************************/
void  convertToUpperCase(char * str, char endCh)
{
    int ch = 0, i = 0;

    for(i=0; i < (int) strlen(str); i++)
    {
        if(str[i] == endCh)
            break;
        else {
            ch = toupper(str[i]);
            str[i] = ch;
        }
    }
}

/********************************
 * Tranaslate the ascii character to hex display *
 *********************************/
void ascii_to_hex(char *in_ascii_string, char *out_hex_string, int in_ascii_string_size)
{
    char   hex[3] = "";
    int    i = 0;
    int    j = 0;

    memset(hex, '\0', 3);

    j = 0;
    for (i=0; i < in_ascii_string_size; i++)
    {
        sprintf(hex, "%.2X", in_ascii_string[i]);

        out_hex_string[j++] = hex[0];
        out_hex_string[j++] = hex[1];
    }

    out_hex_string[j] = '\0';
}

/********************************
 * Trim the string *
 *********************************/

int trim_string(char **start) {
    char *end = *start;

    /* Advance to non-space character */
    while (*(*start) == ' ')
    {
        (*start)++;
    }

    /* Move to end of string */
    while (*end != '\0')
    {
        (end)++;
    }

    /* Backup to non-space character */
    while ((end >= *start) && *(--end) == ' ') {}

    /* Terminate string after last non-space character */
    *(++end) = '\0';
    return (end - *start);
} /* trim_string */

ATOP_t getAtOp(char** cmdLine) {
    char c = '\0', *tmpBuf = NULL;
    ATOP_t retCode = AT_NONE_OP;

    tmpBuf = malloc(strlen(*cmdLine) + 1);
    if(tmpBuf == NULL) {
        return AT_NONE_OP;
    }
    strcpy(tmpBuf, *cmdLine);

    if(strlen(tmpBuf) < strlen(AT_STRING)) {
        free(tmpBuf);
        return AT_NONE_OP;
    } else if(strlen(tmpBuf) == strlen(AT_STRING)) {    // AT command is "AT"
        free(tmpBuf);
        return AT_BASIC_OP;
    }

    c = tmpBuf[2];

    if(isalpha(c)) {
        free(tmpBuf);
        return AT_BASIC_OP;
    }

    if(c == AT_EXTEND_CHAR || c == AT_EXTEND_STAR_CHAR || AT_EXTEND_PER_CHAR) {
        if(strstr(tmpBuf, AT_TEST) != NULL) {
            retCode = AT_TEST_OP;
        } else if(strstr(tmpBuf, AT_READ) != NULL) {
            retCode = AT_READ_OP;
        } else if(strstr(tmpBuf, AT_SET) != NULL) {
            retCode = AT_SET_OP;
        } else {
            retCode = AT_ACTION_OP;
        }
    }

    if(retCode == AT_SET_OP) {
        at_tok_start_flag(cmdLine, '='); //Skip the command name
    }

    free(tmpBuf);
    return retCode;
}