/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "RILC"

#include <hardware_legacy/power.h>

#include <telephony/ril.h>
#include <telephony/ril_cdma_sms.h>
#include <cutils/sockets.h>
#include <cutils/jstring.h>
#include <cutils/record_stream.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>
#include <pthread.h>
#include <binder/Parcel.h>
#include <cutils/jstring.h>

#include <sys/types.h>
#include <pwd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <alloca.h>
#include <sys/un.h>
#include <assert.h>
#include <netinet/in.h>
#include <cutils/properties.h>

namespace android {


static void
printRawData(const void *data, size_t len)
{
    unsigned int i;
    char *pBuf = (char *) data;
    for (i=0;i <= (len - 1)/16; i++)
    {
        LOGI("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                pBuf[16*i],pBuf[16*i+1],pBuf[16*i+2],pBuf[16*i+3],pBuf[16*i+4],pBuf[16*i+5],pBuf[16*i+6],pBuf[16*i+7],
                pBuf[16*i+8],pBuf[16*i+9],pBuf[16*i+10],pBuf[16*i+11],pBuf[16*i+12],pBuf[16*i+13],pBuf[16*i+14],pBuf[16*i+15]);
    }
}


static void writeStringToParcel(Parcel& parcel, const char* str)
{
    if (str) {
        parcel.writeString16(String16(str));
    } else {
        parcel.writeString16(NULL, 0);
    }
}

/** response is a char **, pointing to an array of char *'s */
static int packStrings(Parcel &p, void *cmd, size_t cmdlen) {
    int numStrings;

    if (cmd == NULL && cmdlen != 0) {
        LOGE("RilUtil: invalid cmd NULL");
        return -1;
    }
    if (cmdlen % sizeof(char *) != 0) {
        LOGE("RilUtil: invalid cmd length %d expected multiple of %d\n",
            (int)cmdlen, (int)sizeof(char *));
        return -1;
    }

    if (cmd == NULL) {
        p.writeInt32 (0);
    } else {
        char **p_cur = (char **) cmd;

        numStrings = cmdlen / sizeof(char *);
        p.writeInt32 (numStrings);

        /* each string*/
        for (int i = 0 ; i < numStrings ; i++) {
            writeStringToParcel (p, p_cur[i]);
        }
    }
    return 0;
}

static char *
strdupReadString(Parcel &p)
{
    size_t stringlen;
    const char16_t *s16;
            
    s16 = p.readString16Inplace(&stringlen);
    
    return strndup16to8(s16, stringlen);
}

/** Callee expects const char ** */
static char *
unpackStrings (Parcel &p) {
    int32_t countStrings;
    status_t status;
    size_t datalen;
    char **pStrings;

    status = p.readInt32 (&countStrings);

    if (status != NO_ERROR) {
        goto invalid;
    }

    if (countStrings == 0) {
        // just some non-null pointer
        pStrings = (char **)alloca(sizeof(char *));
        pStrings[0] = NULL;
        datalen = 0;
    } else if (((int)countStrings) == -1) {
        goto invalid;
    } else {
        datalen = sizeof(char *) * countStrings;
        
        pStrings = (char **)alloca(datalen);

        assert(countStrings == 1); /* We only support 1 string now */
        
        for (int i = 0 ; i < countStrings ; i++) {
            pStrings[i] = strdupReadString(p);
        }
    }

    return pStrings[0]; /* only return 1st strings */
invalid:
	LOGE("RilUtil: unpackStrings() fail\n");
    return NULL;
}


/* 
 * packRILcommandString
 * Used for packing standard AT command string to RIL command.
 * 
 * IN   *inStr      : AT command string with NULL terminate
 * IN   *prefix     : prefix of AT response
 * OUT  **outCmd    : RAW RIL command out. Caller is responsible to free this resource.
 * RETUURN          : Length of outCmd data.   
 */
size_t packRILCommand(char *inStr, char *prefix, char **outCmd)
{
	/* |Request Enum |Request Token|Number of Strings|Srings.....|
	 * |<--4 bytes-->|<--4 bytes-->|<--- 4 bytes --->|<------  ->|	
	 */
    size_t length = 0;
    char *cmdStr[2] = {NULL,NULL};
    char *pData = NULL;
    Parcel p;
    static int s_token = 0;

    if ((NULL == inStr)||(NULL == outCmd)) {
        return 0;
    }

    cmdStr[0] = inStr;
    cmdStr[1] = prefix;

   // p.writeInt32(length); /* fake write to reserve space */
    p.writeInt32(RIL_REQUEST_OEM_HOOK_STRINGS);
    p.writeInt32(s_token++);
    
    packStrings(p,cmdStr,2*sizeof(char *)); /* ONLY support 1 string now */

    length = p.dataSize();
    
#if 0
    offset = p.dataPosition(); /* Store data position */

    p.setDataPosition(0); /* Move to the buffer pointer to head */

    p.writeInt32(length - 4); /* Update data length */

    p.setDataPosition(offset); /* Restore data position */
#endif /* 0 */

    pData = (char *) malloc(length);

    if (NULL != pData) {
        memcpy(pData,p.data(),length);
        *outCmd = pData;
        LOGI("packRILCommand: %d bytes\n",length);
        printRawData((const void *) pData, length);
        
    } else {
        return 0;
    }

	return length;
}


/*
 * unpackRILResponse
 * Used for unpacking RIL response to AT command response.
 *
 * IN   *inRsp      : RAW RIL response from socket. 
 * IN   length      : Length of inRsp data.
 * RETURN           : AT response data string. Caller is responsible to free this resource
 */

#define RESPONSE_SOLICITED      0
#define RESPONSE_UNSOLICITED    1

char * unpackRILResponseFull(char *inRsp, size_t length, int32_t *out_ril_errno, int32_t *out_token)
{
    Parcel p;
    int32_t type;
    int32_t token;
    status_t status;
    RIL_Errno e;
    
    p.setData((uint8_t *) inRsp, length);

    // status checked at end
    status = p.readInt32(&type);

    if (type == RESPONSE_UNSOLICITED) {
        status = p.readInt32(&token);
        LOGD("unpackRILResponse: Discard UNSOLICITED response:%d\n",token);
        return NULL;
    }
    
    status = p.readInt32(&token);
    if (out_token != NULL) {
        
        *out_token = token;
    }
    
    status = p.readInt32((int32_t *)&e);
    if ((NO_ERROR != status)) {
        
        LOGE("unpackRILResponse: Error!");
        return NULL;
    }
    
    if (out_ril_errno != NULL) {
        
        *out_ril_errno = e;
        LOGD("unpackRILResponse: Return error cause: %d", e);
    }
    if (RIL_E_SUCCESS != e) {
    
        return NULL;
    }
    
    return  unpackStrings(p);
}

char * unpackRILResponse(char *inRsp, size_t length)
{
    return unpackRILResponseFull(inRsp, length, NULL, NULL);
}

char * unpackRILResponseWithResult(char *inRsp, size_t length, int *ril_errno)
{
    if (ril_errno != NULL) {
        *ril_errno = -1;
    }
    return unpackRILResponseFull(inRsp, length, ril_errno, NULL);
}


} /* namespace android */
