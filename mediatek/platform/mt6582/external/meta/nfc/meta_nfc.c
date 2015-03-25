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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_nfc.c
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   NFC META implementation.
 *
 * Author:
 * -------
 *  
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 10 25 2011 nina.hsu
 * [ALPS00080644] [New_Feature][NFC] Merge NFC feature.
 * .
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <sys/un.h>
#include <errno.h>   /* Error number definitions */


#include <ctype.h>
#include <dirent.h>
#include "meta_nfc_para.h"
#include "WM2Linux.h"
#include "./../../../external/mtknfc/inc/mtk_nfc_sys.h"

#define USING_LOCAL_SOCKET// For Local socket

static NFC_CNF nfc_cnf;
static int nfc_service_sockfd = -1;
static pthread_t  read_cnf_thread_handle;
static unsigned char bStop_ReadThread = 0;

#ifdef META_NFC_SELF_TEST_EN
static NFC_CNF_CB meta_nfc_cnf_cb = NULL;
#endif
extern BOOL WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len);

EM_OPT_ACTION gTagAction = 0xff;

unsigned short gBackUpToken;


//
/********************************/
#define NFC_STATUS_SUCCESS              (0x0)
#define NFC_STATUS_FAIL                 (0x1)
#define NFC_STATUS_INVALID_PARAMS       (0x2)
#define NFC_STATUS_INVALID_STATE            (0x10)
#define NFC_STATUS_NOT_INITIALISED          (0x11)

#define NFC_STATUS_INVALID_NDEF_FORMAT      (0x20) // FOR NDEF USE
#define NFC_STATUS_INVALID_FORMAT           (0x21) // FOR NDEF USE
#define NFC_STATUS_NDEF_EOF_REACHED         (0x22) 
#define NFC_STATUS_TARGET_LOST              (0x0092)


#define MTK_NFC_NDEFRECORD_TNF_EMPTY        (0x00)  /**< Empty Record, no type, ID or payload present. */
#define MTK_NFC_NDEFRECORD_TNF_NFCWELLKNOWN (0x01)  /**< NFC well-known type (RTD). */
#define MTK_NFC_NDEFRECORD_TNF_MEDIATYPE    (0x02)  /**< Media Type. */
#define MTK_NFC_NDEFRECORD_TNF_ABSURI       (0x03)  /**< Absolute URI. */
#define MTK_NFC_NDEFRECORD_TNF_NFCEXT       (0x04)  /**< Nfc Extenal Type (following the RTD format). */
#define MTK_NFC_NDEFRECORD_TNF_UNKNOWN      (0x05)  /**< Unknown type; Contains no Type information. */
#define MTK_NFC_NDEFRECORD_TNF_UNCHANGED    (0x06)  /**< Unchanged: Used for Chunked Records. */
#define MTK_NFC_NDEFRECORD_TNF_RESERVED     (0x07)  /**< RFU, must not be used. */


#define MTK_NFC_NDEFRECORD_TNFBYTE_MASK       (0x07) /** \internal For masking */
#define MTK_NFC_NDEFRECORD_BUF_INC1           1               /** \internal Increment Buffer Address by 1 */
#define MTK_NFC_NDEFRECORD_BUF_INC2           2               /** \internal Increment Buffer Address by 2 */
#define MTK_NFC_NDEFRECORD_BUF_INC3           3               /** \internal Increment Buffer Address by 3 */
#define MTK_NFC_NDEFRECORD_BUF_INC4           4               /** \internal Increment Buffer Address by 4 */
#define MTK_NFC_NDEFRECORD_BUF_INC5           5               /** \internal Increment Buffer Address by 5 */
#define MTK_NFC_NDEFRECORD_BUF_TNF_VALUE      (0x00) /** \internal If TNF = Empty, Unknown and Unchanged, the id, type and payload length is ZERO  */
#define MTK_NFC_NDEFRECORD_FLAG_MASK          (0xF8) /** \internal To Mask the Flag Byte */


#define MTK_NFC_NDEFRECORD_FLAGS_MB       (0x80)  /**< This marks the begin of a NDEF Message. */
#define MTK_NFC_NDEFRECORD_FLAGS_ME       (0x40)  /**< Set if the record is at the Message End. */
#define MTK_NFC_NDEFRECORD_FLAGS_CF       (0x20)  /**< Chunk Flag: The record is a record chunk only. */
#define MTK_NFC_NDEFRECORD_FLAGS_SR       (0x10)  /**< Short Record: Payload Length is encoded in ONE byte only. */
#define MTK_NFC_NDEFRECORD_FLAGS_IL       (0x08)  /**< The ID Length Field is present. */



typedef struct MTK_NFC_RTD_DATA
{
    UINT8                   *pbuffer;
    UINT32                  u4length;
}MTK_NFC_RTD_DATA;
typedef struct MTK_NFC_URI_DATA
{
    UINT8                   *pbuffer;
    UINT32                  u4length;
}MTK_NFC_URI_DATA;

typedef struct MTK_NFC_RTD_URI_INFO
{
    UINT8                       u1URIIdentifier;
    MTK_NFC_URI_DATA             URI;
}MTK_NFC_RTD_URI_INFO;


typedef enum MTK_NFC_TEXT_ENCODE_TYPE
{
    MTK_NFC_RTD_UTF_8 =0, 
    MTK_NFC_RTD_UTF_16    
}MTK_NFC_TEXT_ENCODE_TYPE;

typedef struct MTK_NFC_TEXT_DATA
{
    UINT8                   *pbuffer;
    UINT32                  u4length;
}MTK_NFC_TEXT_DATA;


typedef struct MTK_NFC_TEXT_RTD_INFO
{
    MTK_NFC_TEXT_ENCODE_TYPE   eEncodeType;
    UINT32                     u4IANA_lang_code_length;
    UINT8                      pIANA_lang_code[31];
    MTK_NFC_TEXT_DATA          sText;
}MTK_NFC_TEXT_RTD_INFO;


typedef struct mtkNfc_NdefRecord
{
    uint8_t                 Flags;
    uint8_t                 Tnf;
    uint8_t                 TypeLength;
    uint8_t                *Type;     
    uint8_t                 IdLength;
    uint8_t                *Id;
    uint32_t                PayloadLength;
    uint8_t                *PayloadData;
} mtkNfc_NdefRecord_t;




static uint8_t mtk_nfc_NdefRecord_NdefFlag(uint8_t Flags,uint8_t Mask)
{
    uint8_t check_flag = 0x00;
    check_flag = ((Flags) & (Mask));
    return check_flag;
}


uint32_t mtk_nfc_NdefRecord_GetLength(mtkNfc_NdefRecord_t *Record)
{
    uint32_t RecordLength=1;
    uint8_t  FlagCheck=0;

    FlagCheck=mtk_nfc_NdefRecord_NdefFlag(Record->Tnf,MTK_NFC_NDEFRECORD_TNFBYTE_MASK);

    /* ++ is for the Type Length Byte */
    RecordLength++;
    if( FlagCheck != MTK_NFC_NDEFRECORD_TNF_EMPTY &&
            FlagCheck != MTK_NFC_NDEFRECORD_TNF_UNKNOWN &&
            FlagCheck != MTK_NFC_NDEFRECORD_TNF_UNCHANGED )
    {
        RecordLength += Record->TypeLength;
    }

    /* to check if payloadlength is 8bit or 32bit*/
    FlagCheck=mtk_nfc_NdefRecord_NdefFlag(Record->Flags,MTK_NFC_NDEFRECORD_FLAGS_SR);
    if(FlagCheck!=0)
    {
        /* ++ is for the Payload Length Byte */
        RecordLength++;/* for short record*/
    }
    else
    {
        /* + PHFRINFCNDEFRECORD_NORMAL_RECORD_BYTE is for the Payload Length Byte */
        RecordLength += (4);/* for normal record*/
    }

    /* for non empty record */
    FlagCheck=mtk_nfc_NdefRecord_NdefFlag(Record->Tnf,MTK_NFC_NDEFRECORD_TNFBYTE_MASK);
    if(FlagCheck != MTK_NFC_NDEFRECORD_TNF_EMPTY)
    {
        RecordLength += Record->PayloadLength;
    }

    /* ID and IDlength are present only if IL flag is set*/
    FlagCheck=mtk_nfc_NdefRecord_NdefFlag(Record->Flags,MTK_NFC_NDEFRECORD_FLAGS_IL);
    if(FlagCheck!=0)
    {
        RecordLength +=Record->IdLength;
        /* ++ is for the ID Length Byte */
        RecordLength ++;
    }
    return RecordLength;
}


NFCSTATUS MTK_NFC_RTD_TEXT_GEN(
    MTK_NFC_TEXT_RTD_INFO                 TextRTDInfor,       //i
    UINT32                                u4RtdBufferMaxLen,  //i
    MTK_NFC_RTD_DATA                     *pRTD_buffer         //o
)
{
    uint8_t u1StatusByte;
    uint32_t startPos;
    
    if ((pRTD_buffer->pbuffer == NULL) || (TextRTDInfor.sText.pbuffer == NULL))
    {
        return NFC_STATUS_FAIL;
    }

    if (u4RtdBufferMaxLen < (1 + TextRTDInfor.u4IANA_lang_code_length + TextRTDInfor.sText.u4length))
    {
        return NFC_STATUS_FAIL;
    }

    pRTD_buffer->u4length = (1 + TextRTDInfor.u4IANA_lang_code_length + TextRTDInfor.sText.u4length);

    u1StatusByte = (TextRTDInfor.eEncodeType << 7) + (TextRTDInfor.u4IANA_lang_code_length & 0x1F) ;
    pRTD_buffer->pbuffer[0] = u1StatusByte; 
    
    memcpy(&pRTD_buffer->pbuffer[1],&TextRTDInfor.pIANA_lang_code[0],TextRTDInfor.u4IANA_lang_code_length); 

    startPos = 1 + TextRTDInfor.u4IANA_lang_code_length;
    memcpy(&pRTD_buffer->pbuffer[startPos],TextRTDInfor.sText.pbuffer,TextRTDInfor.sText.u4length);

    return NFC_STATUS_SUCCESS;
    
}


NFCSTATUS MTK_NFC_RTD_URI_GEN(
    MTK_NFC_RTD_URI_INFO                  RTDURIInfor,       //i
    UINT32                                u4RtdBufferMaxLen, //i 
    MTK_NFC_RTD_DATA                     *pRTD_buffer         //o
)
{
    if ((pRTD_buffer->pbuffer == NULL) || (RTDURIInfor.URI.pbuffer == NULL))
    {
        return NFC_STATUS_FAIL;
    }
    
    if (u4RtdBufferMaxLen < (1 + RTDURIInfor.URI.u4length))
    {
        return NFC_STATUS_FAIL;
    }
    
    pRTD_buffer->u4length = (1 + RTDURIInfor.URI.u4length);
    pRTD_buffer->pbuffer[0] = RTDURIInfor.u1URIIdentifier;
    memcpy(&pRTD_buffer->pbuffer[1],RTDURIInfor.URI.pbuffer, RTDURIInfor.URI.u4length);

    return NFC_STATUS_SUCCESS;
    
}


NFCSTATUS mtk_nfc_NdefRecord_Generate(mtkNfc_NdefRecord_t *Record,
                                      uint8_t               *Buffer,
                                      uint32_t               MaxBufferSize,
                                      uint32_t              *BytesWritten)
{
    uint8_t     FlagCheck,
                TypeCheck=0,
                *temp,
                i;
    uint32_t    i_data=0;

    if(Record==NULL ||Buffer==NULL||BytesWritten==NULL||MaxBufferSize == 0)
    {
        return (NFC_STATUS_INVALID_STATE);
    }

    if (Record->Tnf == MTK_NFC_NDEFRECORD_TNF_RESERVED)
    {
        return (NFC_STATUS_INVALID_STATE);
    }

    /* calculate the length of the record and check with the buffersize if it exceeds return */
    i_data = mtk_nfc_NdefRecord_GetLength(Record);
    if(i_data > MaxBufferSize)
    {
        return (NFC_STATUS_FAIL);
    }
    *BytesWritten = i_data;

    /*fill the first byte of the message(all the flags) */
    /*increment the buffer*/
    *Buffer = ( (Record->Flags & MTK_NFC_NDEFRECORD_FLAG_MASK) | (Record->Tnf & MTK_NFC_NDEFRECORD_TNFBYTE_MASK));
    Buffer++;

    /* check the TypeNameFlag for PH_FRINFC_NDEFRECORD_TNF_EMPTY */
    FlagCheck = mtk_nfc_NdefRecord_NdefFlag(Record->Tnf,MTK_NFC_NDEFRECORD_TNFBYTE_MASK);
    if(FlagCheck == MTK_NFC_NDEFRECORD_TNF_EMPTY)
    {
        /* fill the typelength idlength and payloadlength with zero(empty message)*/
        for(i=0; i<3; i++)
        {
            *Buffer=MTK_NFC_NDEFRECORD_BUF_TNF_VALUE;
            Buffer++;
        }
        return (NFC_STATUS_SUCCESS);
    }

    /* check the TypeNameFlag for PH_FRINFC_NDEFRECORD_TNF_RESERVED */
    /* TNF should not be reserved one*/
    FlagCheck = mtk_nfc_NdefRecord_NdefFlag(Record->Tnf,MTK_NFC_NDEFRECORD_TNFBYTE_MASK);
    if(FlagCheck == MTK_NFC_NDEFRECORD_TNF_RESERVED)
    {
        return (NFC_STATUS_INVALID_PARAMS);
    }

    /* check for TNF Unknown or Unchanged */
    FlagCheck = mtk_nfc_NdefRecord_NdefFlag(Record->Tnf,MTK_NFC_NDEFRECORD_TNFBYTE_MASK);
    if(FlagCheck == MTK_NFC_NDEFRECORD_TNF_UNKNOWN || \
            FlagCheck == MTK_NFC_NDEFRECORD_TNF_UNCHANGED)
    {
        *Buffer = MTK_NFC_NDEFRECORD_BUF_TNF_VALUE;
        Buffer++;
    }
    else
    {
        *Buffer = Record->TypeLength;
        Buffer++;
        TypeCheck=1;
    }

    /* check for the short record bit if it is then payloadlength is only one byte */
    FlagCheck = mtk_nfc_NdefRecord_NdefFlag(Record->Flags,MTK_NFC_NDEFRECORD_FLAGS_SR);
    if(FlagCheck!=0)
    {
        *Buffer = (uint8_t)(Record->PayloadLength & 0x000000ff);
        Buffer++;
    }
    else
    {
        /* if it is normal record payloadlength is 4 byte(32 bit)*/
        *Buffer = (uint8_t)((Record->PayloadLength & 0xff000000) >> (24));
        Buffer++;
        *Buffer = (uint8_t)((Record->PayloadLength & 0x00ff0000) >> (16));
        Buffer++;
        *Buffer = (uint8_t)((Record->PayloadLength & 0x0000ff00) >> (8));
        Buffer++;
        *Buffer = (uint8_t)((Record->PayloadLength & 0x000000ff));
        Buffer++;
    }

    /*check for IL bit set(Flag), if so then IDlength is present*/
    FlagCheck = mtk_nfc_NdefRecord_NdefFlag(Record->Flags,MTK_NFC_NDEFRECORD_FLAGS_IL);
    if(FlagCheck!=0)
    {
        *Buffer=Record->IdLength;
        Buffer++;
    }

    /*check for TNF and fill the Type*/
    temp=Record->Type;
    if(TypeCheck!=0)
    {
        for(i=0; i<(Record->TypeLength); i++)
        {
            *Buffer = *temp;
            Buffer++;
            temp++;
        }
    }

    /*check for IL bit set(Flag), if so then IDlength is present and fill the ID*/
    FlagCheck = mtk_nfc_NdefRecord_NdefFlag(Record->Flags,MTK_NFC_NDEFRECORD_FLAGS_IL);
    temp=Record->Id;
    if(FlagCheck!=0)
    {
        for(i=0; i<(Record->IdLength); i++)
        {
            *Buffer = *temp;
            Buffer++;
            temp++;
        }
    }

    temp=Record->PayloadData;
    /*check for SR bit and then correspondingly use the payload length*/
    FlagCheck = mtk_nfc_NdefRecord_NdefFlag(Record->Flags,MTK_NFC_NDEFRECORD_FLAGS_SR);
    for(i_data=0; i_data < (Record->PayloadLength) ; i_data++)
    {
        *Buffer = *temp;
        Buffer++;
        temp++;
    }

    return (NFC_STATUS_SUCCESS);
}



int mtk_nfc_Generate_NDEF(int ndef_type,int ndef_lang, s_mtk_nfc_meta_tag_write_ndef_data *data, uint8_t *outBuf)
{
    int result=FALSE;
    int len = 0;;
    if(ndef_type == nfc_ndef_type_text)
    {
        MTK_NFC_TEXT_RTD_INFO    TextRTDInfor;
        MTK_NFC_RTD_DATA         rRTDData;
        TextRTDInfor.u4IANA_lang_code_length = 2;

        switch(ndef_lang)
        {
           case nfc_ndef_lang_DE:
            TextRTDInfor.pIANA_lang_code[0] = 'd';
            TextRTDInfor.pIANA_lang_code[1] = 'e';
            TextRTDInfor.eEncodeType = MTK_NFC_RTD_UTF_8;
            break;
           case nfc_ndef_lang_FR:
            TextRTDInfor.pIANA_lang_code[0] = 'f';
            TextRTDInfor.pIANA_lang_code[1] = 'r';
            TextRTDInfor.eEncodeType = MTK_NFC_RTD_UTF_8;
            break;
           case nfc_ndef_lang_EN:
           default:
            TextRTDInfor.pIANA_lang_code[0] = 'e';
            TextRTDInfor.pIANA_lang_code[1] = 'n';
            TextRTDInfor.eEncodeType = MTK_NFC_RTD_UTF_8;
            break;
        }
        
        TextRTDInfor.sText.pbuffer = &data->TX_Data.data[0];
        TextRTDInfor.sText.u4length = (UINT32)data->TX_Data.DataLength;
        rRTDData.pbuffer = malloc(sizeof(char)*256);
        if (NULL != rRTDData.pbuffer)
        {
            result = MTK_NFC_RTD_TEXT_GEN(TextRTDInfor,256,&rRTDData);
            if (MTK_NFC_SUCCESS == result)
            {
                mtkNfc_NdefRecord_t Record;
                uint8_t type = 'T', id = 0x00;
                uint32_t u4BytesWritten=0;
                uint8_t local_buf[256];                
                //local_buf = mtk_nfc_sys_mem_alloc(sizeof(char)*256);
                memset(&Record,0,sizeof(mtkNfc_NdefRecord_t));

                Record.Tnf = MTK_NFC_NDEFRECORD_TNF_NFCWELLKNOWN;
                Record.Type = &type;
                Record.TypeLength = 0x01;
                Record.Id =&id;
                Record.IdLength =0x00;
                Record.Flags = 0xD1;
                Record.PayloadLength = rRTDData.u4length;
                Record.PayloadData = rRTDData.pbuffer;
                
                result = mtk_nfc_NdefRecord_Generate(&Record, &local_buf[0],256, &u4BytesWritten);
                
                if (MTK_NFC_SUCCESS == result)
                {
                    //memcpy(g_ndefData.databuffer, &local_buf[0], u4BytesWritten);
                    //g_ndefData.datalen = u4BytesWritten;
                    memcpy(outBuf, &local_buf[0], u4BytesWritten);
                    len = u4BytesWritten;
                }
                else
                {
                    META_LOG("mtk_nfc_DT_tag_write, mtk_nfc_NdefRecord_Generate fail \n");
                    return (FALSE);                      
                }
                
                //PrintDBG("[READER_MODE], TAG,WRITE,%x,%d\r\n",g_ndefData.databuffer,g_ndefData.datalen);    

                
                //mtk_nfc_ndef_write(1,&g_ndefData,&vNFCMainNdefWriteCb, (void*)pcontext);
            }
            else
            {
                META_LOG("mtk_nfc_DT_tag_write, MTK_NFC_RTD_TEXT_GEN fail \n");
                return (FALSE);                      
            }
        }
        else
        {
           result = FALSE;
        }

    }
    else if (ndef_type == nfc_ndef_type_uri)
    {
        MTK_NFC_RTD_URI_INFO     RTDInfor;
        MTK_NFC_RTD_DATA         rRTDData;
        RTDInfor.u1URIIdentifier = 0x01;//
        RTDInfor.URI.pbuffer = &data->URL_Data.URLData[0];
        RTDInfor.URI.u4length = (UINT32)data->URL_Data.URLLength;
        rRTDData.pbuffer = malloc(sizeof(char)*256);

        META_LOG("LC_TEST,%d",RTDInfor.URI.u4length);
        if(NULL != rRTDData.pbuffer)
        {
            //result = MTK_NFC_RTD_TEXT_GEN(TextRTDInfor,256,&rRTDData);
            result = MTK_NFC_RTD_URI_GEN(RTDInfor,256,&rRTDData);
            META_LOG("LC_TEST,result,%d",result);
            if(MTK_NFC_SUCCESS == result)
            {
            #if 1
                mtkNfc_NdefRecord_t Record;
                uint8_t type = 'U', id = 0x00;
                uint32_t u4BytesWritten=0;
                uint8_t local_buf[256];                
                //local_buf = mtk_nfc_sys_mem_alloc(sizeof(char)*256);
                memset(&Record,0,sizeof(mtkNfc_NdefRecord_t));
     
                Record.Tnf = MTK_NFC_NDEFRECORD_TNF_NFCWELLKNOWN;
                Record.Type = &type;
                Record.TypeLength = 0x01;
                Record.Id =&id;
                Record.IdLength =0x00;
                Record.Flags = 0xD1;
                Record.PayloadLength = rRTDData.u4length;
                Record.PayloadData = rRTDData.pbuffer;
                
                result = mtk_nfc_NdefRecord_Generate(&Record, &local_buf[0],256, &u4BytesWritten);
                
                if (MTK_NFC_SUCCESS == result)
                {
                    //memcpy(g_ndefData.databuffer, &local_buf[0], u4BytesWritten);
                    //g_ndefData.datalen = u4BytesWritten;
                    memcpy(outBuf, &local_buf[0], u4BytesWritten);
                    len = u4BytesWritten;
                }
                else
                {
                    META_LOG("mtk_nfc_DT_tag_write, mtk_nfc_NdefRecord_Generate fail \n");
                    return (FALSE);                      
                }

                #endif

            }
            else
            {
                result = FALSE;
            }
        }
        else
        {
            result = FALSE;
        }

    }
    return len;
}


static NFCSTATUS mtk_nfc_NdefRecord_RecordIDCheck ( uint8_t       *Record,
        uint8_t       *TypeLength,
        uint8_t       *TypeLengthByte,
        uint8_t       *PayloadLengthByte,
        uint32_t      *PayloadLength,
        uint8_t       *IDLengthByte,
        uint8_t       *IDLength)
{
    int   Status = MTK_NFC_SUCCESS;

    /* Check for Tnf bits 0x07 is reserved for future use */
    if ((*Record & MTK_NFC_NDEFRECORD_TNFBYTE_MASK) ==
            MTK_NFC_NDEFRECORD_TNF_RESERVED)
    {
        /* TNF 07  Error */
        Status = NFC_STATUS_INVALID_FORMAT;
        return Status;
    }

    /* Check for Type Name Format  depending on the TNF,  Type Length value is set*/
    if ((*Record & MTK_NFC_NDEFRECORD_TNFBYTE_MASK)==
            MTK_NFC_NDEFRECORD_TNF_EMPTY)
    {
        *TypeLength = *(Record + MTK_NFC_NDEFRECORD_BUF_INC1);

        if (*(Record + MTK_NFC_NDEFRECORD_BUF_INC1) != 0)
        {
            /* Type Length  Error */
            Status = NFC_STATUS_INVALID_FORMAT;
            return Status;
        }

        *TypeLengthByte = 1;

        /* Check for Short Record */
        if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_SR) == MTK_NFC_NDEFRECORD_FLAGS_SR)
        {
            /* For Short Record, Payload Length Byte is 1 */
            *PayloadLengthByte = 1;
            /*  1 for Header byte */
            *PayloadLength = *(Record + *TypeLengthByte + 1);
            if (*PayloadLength != 0)
            {
                /* PayloadLength  Error */
                Status = NFC_STATUS_INVALID_FORMAT;
                return Status;
            }
        }
        else
        {
            /* For Normal Record, Payload Length Byte is 4 */
            *PayloadLengthByte = (4);
            *PayloadLength =    ((((uint32_t)(*(Record + MTK_NFC_NDEFRECORD_BUF_INC2))) << (24)) +
                                 (((uint32_t)(*(Record + MTK_NFC_NDEFRECORD_BUF_INC3))) << (16)) +
                                 (((uint32_t)(*(Record + MTK_NFC_NDEFRECORD_BUF_INC4))) << (8))  +
                                 *(Record + MTK_NFC_NDEFRECORD_BUF_INC5));
            if (*PayloadLength != 0)
            {
                /* PayloadLength  Error */
                Status = NFC_STATUS_INVALID_FORMAT;
                return Status;
            }
        }

        /* Check for ID Length existence */
        if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_IL) == MTK_NFC_NDEFRECORD_FLAGS_IL)
        {
            /* Length Byte exists and it is 1 byte */
            *IDLengthByte = 1;
            /*  1 for Header byte */
            *IDLength = (uint8_t)*(Record + *PayloadLengthByte + *TypeLengthByte + MTK_NFC_NDEFRECORD_BUF_INC1);
            if (*IDLength != 0)
            {
                /* IDLength  Error */
                Status = NFC_STATUS_INVALID_FORMAT;
                return Status;
            }
        }
        else
        {
            *IDLengthByte = 0;
            *IDLength = 0;
        }
    }
    else
    {
        if ((*Record & MTK_NFC_NDEFRECORD_TNFBYTE_MASK)== MTK_NFC_NDEFRECORD_TNF_UNKNOWN
                || (*Record & MTK_NFC_NDEFRECORD_TNFBYTE_MASK) ==
                MTK_NFC_NDEFRECORD_TNF_UNCHANGED)
        {
            if (*(Record + MTK_NFC_NDEFRECORD_BUF_INC1) != 0)
            {
                /* Type Length  Error */
                Status = NFC_STATUS_INVALID_FORMAT;
                return Status;
            }
            *TypeLength = 0;
            *TypeLengthByte = 1;
        }
        else
        {
            /*  1 for Header byte */
            *TypeLength = *(Record + MTK_NFC_NDEFRECORD_BUF_INC1);
            *TypeLengthByte = 1;
        }

        /* Check for Short Record */
        if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_SR) ==
                MTK_NFC_NDEFRECORD_FLAGS_SR)
        {
            /* For Short Record, Payload Length Byte is 1 */
            *PayloadLengthByte = 1;
            /*  1 for Header byte */
            *PayloadLength = *(Record + *TypeLengthByte + MTK_NFC_NDEFRECORD_BUF_INC1);
        }
        else
        {
            /* For Normal Record, Payload Length Byte is 4 */
            *PayloadLengthByte = (4);
            *PayloadLength =    ((((uint32_t)(*(Record + MTK_NFC_NDEFRECORD_BUF_INC2))) << (24)) +
                                 (((uint32_t)(*(Record + MTK_NFC_NDEFRECORD_BUF_INC3))) << (16)) +
                                 (((uint32_t)(*(Record + MTK_NFC_NDEFRECORD_BUF_INC4))) << (8))  +
                                 *(Record + MTK_NFC_NDEFRECORD_BUF_INC5));
        }

        /* Check for ID Length existence */
        if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_IL) ==
                MTK_NFC_NDEFRECORD_FLAGS_IL)
        {
            *IDLengthByte = 1;
            /*  1 for Header byte */
            *IDLength = (uint8_t)*(Record + *PayloadLengthByte + *TypeLengthByte + MTK_NFC_NDEFRECORD_BUF_INC1);
        }
        else
        {
            *IDLengthByte = 0;
            *IDLength = 0;
        }
    }
    return Status;
}


/* Calculate the Flags of the record */
static uint8_t mtk_nfc_NdefRecord_RecordFlag ( uint8_t    *Record)
{
    uint8_t flag = 0;

    if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_MB) == MTK_NFC_NDEFRECORD_FLAGS_MB )
    {
        flag = flag | MTK_NFC_NDEFRECORD_FLAGS_MB;
    }
    if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_ME) == MTK_NFC_NDEFRECORD_FLAGS_ME )
    {
        flag = flag | MTK_NFC_NDEFRECORD_FLAGS_ME;
    }
    if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_CF) == MTK_NFC_NDEFRECORD_FLAGS_CF )
    {
        flag = flag | MTK_NFC_NDEFRECORD_FLAGS_CF;
    }
    if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_SR) == MTK_NFC_NDEFRECORD_FLAGS_SR )
    {
        flag = flag | MTK_NFC_NDEFRECORD_FLAGS_SR;
    }
    if ((*Record & MTK_NFC_NDEFRECORD_FLAGS_IL) == MTK_NFC_NDEFRECORD_FLAGS_IL )
    {
        flag = flag | MTK_NFC_NDEFRECORD_FLAGS_IL;
    }
    return flag;
}

/* Calculate the Type Name Format for the record */
static uint8_t mtk_nfc_NdefRecord_TypeNameFormat ( uint8_t    *Record)
{
    uint8_t     tnf = 0;

    switch (*Record & MTK_NFC_NDEFRECORD_TNFBYTE_MASK)
    {
        case MTK_NFC_NDEFRECORD_TNF_EMPTY:
            tnf = MTK_NFC_NDEFRECORD_TNF_EMPTY;
            break;

        case MTK_NFC_NDEFRECORD_TNF_NFCWELLKNOWN:
            tnf = MTK_NFC_NDEFRECORD_TNF_NFCWELLKNOWN;
            break;

        case MTK_NFC_NDEFRECORD_TNF_MEDIATYPE:
            tnf = MTK_NFC_NDEFRECORD_TNF_MEDIATYPE;
            break;

        case MTK_NFC_NDEFRECORD_TNF_ABSURI:
            tnf = MTK_NFC_NDEFRECORD_TNF_ABSURI;
            break;

        case MTK_NFC_NDEFRECORD_TNF_NFCEXT:
            tnf = MTK_NFC_NDEFRECORD_TNF_NFCEXT;
            break;

        case MTK_NFC_NDEFRECORD_TNF_UNKNOWN:
            tnf = MTK_NFC_NDEFRECORD_TNF_UNKNOWN;
            break;

        case MTK_NFC_NDEFRECORD_TNF_UNCHANGED:
            tnf = MTK_NFC_NDEFRECORD_TNF_UNCHANGED;
            break;

        case MTK_NFC_NDEFRECORD_TNF_RESERVED:
            tnf = MTK_NFC_NDEFRECORD_TNF_RESERVED;
            break;
        default :
            tnf = 0xFF;
            break;
    }

    return tnf;
}


int mtk_nfc_NdefRecord_Parse(mtkNfc_NdefRecord_t *Record,
                                   uint8_t               *RawRecord)
{
    int       Status = 0x00;
    uint8_t         PayloadLengthByte = 0,
                    TypeLengthByte = 0,
                    TypeLength = 0,
                    IDLengthByte = 0,
                    IDLength = 0,
                    Tnf     =   0;
    uint32_t        PayloadLength = 0;


    if (Record == NULL || RawRecord == NULL)
    {
        Status = 0x01;
    }

    else
    {

        /* Calculate the Flag Value */
        Record->Flags = mtk_nfc_NdefRecord_RecordFlag ( RawRecord);

        /* Calculate the Type Namr format of the record */
        Tnf = mtk_nfc_NdefRecord_TypeNameFormat( RawRecord);
        if(Tnf != 0xFF)
        {
            Record->Tnf = Tnf;
            /* To Calculate the IDLength and PayloadLength for short or normal record */
            Status = mtk_nfc_NdefRecord_RecordIDCheck (    RawRecord,
                     &TypeLength,
                     &TypeLengthByte,
                     &PayloadLengthByte,
                     &PayloadLength,
                     &IDLengthByte,
                     &IDLength);
            Record->TypeLength = TypeLength;
            Record->PayloadLength = PayloadLength;
            Record->IdLength = IDLength;
            RawRecord = (RawRecord +  PayloadLengthByte + IDLengthByte + TypeLengthByte + MTK_NFC_NDEFRECORD_BUF_INC1);
            Record->Type = RawRecord;

            RawRecord = (RawRecord + Record->TypeLength);

            if (Record->IdLength != 0)
            {
                Record->Id = RawRecord;
            }

            RawRecord = RawRecord + Record->IdLength;
            Record->PayloadData = RawRecord;
        }
        else
        {
            Status = 0x01;
        }
    }
    return Status;
}


//


#ifdef META_NFC_SELF_TEST_EN
void META_NFC_Register(NFC_CNF_CB callback)
{
    meta_nfc_cnf_cb = callback;
}
#endif
static void* META_NFC_read_cnf(void *arg)
{
    int rec_bytes = 0;
    // Read resonse
    META_LOG("META_NFC_CMD:NFC read thread start");
    bStop_ReadThread = 0;
    while(bStop_ReadThread == 0)
    {    
        ilm_struct nfc_ilm_rec;
        nfc_msg_struct nfc_msg;
        unsigned char nfc_msg_length;
        unsigned char fgSupport = 1;
        UINT8 buffer[1024];
        int bufsize=1024;
        
         #if 0
        //clean struct buffer
        memset(&nfc_ilm_rec, 0, sizeof(ilm_struct));
        //read fd
        //if get response break
        rec_bytes = read(nfc_service_sockfd,(char*)&nfc_ilm_rec, sizeof(ilm_struct));             
        #else        
        rec_bytes = read(nfc_service_sockfd, &buffer[0], bufsize);           
        #endif
        
        META_LOG("META_NFC_CMD,gBackUpToken,%d",gBackUpToken);
        if (rec_bytes > 0)
        {
            // check msg id
            #if 0
            META_LOG("META_NFC_CMD:NFC read (msg_id,dest_mod_id) = (%d,%d)",nfc_ilm_rec.msg_id, nfc_ilm_rec.dest_mod_id);
            if ((nfc_ilm_rec.msg_id == MSG_ID_NFC_TEST_RSP) && (nfc_ilm_rec.dest_mod_id == MOD_NFC_APP))
            {
                nfc_msg_length = sizeof(nfc_msg_struct);
                memcpy( &nfc_msg, (nfc_msg_struct*)nfc_ilm_rec.local_para_ptr, nfc_msg_length);
                META_LOG("META_NFC_CMD:NFC read msg_type=%d,length=%d", nfc_msg.msg_type,nfc_msg_length);
                switch (nfc_msg.msg_type)
                {
                    case MSG_ID_NFC_SETTING_RSP:
                    { 
                        nfc_cnf.op = NFC_OP_SETTING;
                        memcpy(&nfc_cnf.result.m_setting_cnf, (nfc_setting_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_setting_response));
                        META_LOG("META_NFC_CMD:NFC NFC_OP_SETTING =%d/%d/%d/%d/%d/%d/%d/%d/%d/", 
                            nfc_cnf.result.m_setting_cnf.status,
                            nfc_cnf.result.m_setting_cnf.debug_enable,
                            nfc_cnf.result.m_setting_cnf.fw_ver,
                            nfc_cnf.result.m_setting_cnf.get_capabilities,
                            nfc_cnf.result.m_setting_cnf.sw_ver,
                            nfc_cnf.result.m_setting_cnf.hw_ver,
                            nfc_cnf.result.m_setting_cnf.fw_ver,
                            nfc_cnf.result.m_setting_cnf.reader_mode,
                            nfc_cnf.result.m_setting_cnf.card_mode);                        
                        break;
                    }
                    case MSG_ID_NFC_NOTIFICATION_RSP:
                    {
                        nfc_cnf.op = NFC_OP_REG_NOTIFY;
                        memcpy(&nfc_cnf.result.m_reg_notify_cnf, (nfc_reg_notif_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length),sizeof(nfc_reg_notif_response));
                        META_LOG("META_NFC_CMD:NFC NFC_OP_DISCOVERY =%d/", 
                            nfc_cnf.result.m_reg_notify_cnf.status);                              
                        break;
                    }
                    case MSG_ID_NFC_SE_SET_RSP:
                    {
                        nfc_cnf.op = NFC_OP_SECURE_ELEMENT;
                        memcpy(&nfc_cnf.result.m_se_set_cnf, (nfc_se_set_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_se_set_response));
                        META_LOG("META_NFC_CMD:NFC NFC_OP_SECURE_ELEMENT =%d/", 
                            nfc_cnf.result.m_se_set_cnf.status);                             
                        break;
                    }
                    case MSG_ID_NFC_DISCOVERY_RSP:
                    {
                        nfc_cnf.op = NFC_OP_DISCOVERY;
                        memcpy(&nfc_cnf.result.m_dis_notify_cnf, (nfc_dis_notif_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_dis_notif_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_DISCOVERY =%d/%d/", 
                            nfc_cnf.result.m_dis_notify_cnf.status,
                            nfc_cnf.result.m_dis_notify_cnf.type);                        
                        break;
                    }
                    case MSG_ID_NFC_TAG_READ_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_READ;
                        memcpy(&nfc_cnf.result.m_tag_read_cnf, (nfc_tag_read_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_read_response)); 
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_READ =%d/%d/", 
                            nfc_cnf.result.m_tag_read_cnf.status,
                            nfc_cnf.result.m_tag_read_cnf.type);                            
                        break;
                    }
                    case MSG_ID_NFC_TAG_WRITE_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_WRITE;
                        memcpy(&nfc_cnf.result.m_tag_write_cnf, (nfc_tag_write_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_write_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_WRITE =%d/%d/", 
                            nfc_cnf.result.m_tag_write_cnf.status,
                            nfc_cnf.result.m_tag_write_cnf.type);                         
                        break;
                    }                
                    case MSG_ID_NFC_TAG_DISCONN_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_DISCONN;
                        memcpy(&nfc_cnf.result.m_tag_discon_cnf, (nfc_tag_disconnect_request*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_disconnect_request));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_DISCONN =%d/", 
                            nfc_cnf.result.m_tag_discon_cnf.status);                           
                        break;
                    } 
                    case MSG_ID_NFC_TAG_F2NDEF_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_FORMAT_NDEF;
                        memcpy(&nfc_cnf.result.m_tag_fromat2Ndef_cnf, (nfc_tag_fromat2Ndef_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_fromat2Ndef_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_FORMAT_NDEF =%d/", 
                            nfc_cnf.result.m_tag_fromat2Ndef_cnf.status);                          
                        break;
                    } 
                    case MSG_ID_NFC_TAG_RAWCOM_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_RAW_COMM;
                        memcpy(&nfc_cnf.result.m_tag_raw_com_cnf, (nfc_tag_raw_com_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_raw_com_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_RAW_COMM =%d/%d/", 
                            nfc_cnf.result.m_tag_raw_com_cnf.status, 
                            nfc_cnf.result.m_tag_raw_com_cnf.type);                        
                        break;
                    }    
                    case MSG_ID_NFC_P2P_COMMUNICATION_RSP:
                    {
                        nfc_cnf.op = NFC_OP_P2P_COMM;
                        memcpy(&nfc_cnf.result.m_p2p_com_cnf, (nfc_p2p_com_response*)nfc_ilm_rec.local_para_ptr, sizeof(nfc_p2p_com_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_P2P_COMM =%d/%d/", 
                            nfc_cnf.result.m_p2p_com_cnf.status, 
                            nfc_cnf.result.m_p2p_com_cnf.length);    
                        
                        break;
                    }  
                    case MSG_ID_NFC_RD_COMMUNICATION_RSP:
                    {
                        nfc_cnf.op = NFC_OP_RD_COMM;
                        memcpy(&nfc_cnf.result.m_rd_com_cnf, (nfc_rd_com_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_rd_com_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_RD_COMM =%d/%d/", 
                            nfc_cnf.result.m_rd_com_cnf.status, 
                            nfc_cnf.result.m_rd_com_cnf.length);                                   
                        break;
                    }
                    case MSG_ID_NFC_TX_ALWAYSON_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TX_ALWAYSON_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TX_ALWAYSON_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                           
                        break;
                    }   
                    case MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TX_ALWAYSON_WO_ACK_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TX_ALWAYSON_WO_ACK_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }
                    case MSG_ID_NFC_CARD_EMULATION_MODE_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_CARD_MODE_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_CARD_MODE_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                        
                        break;
                    }      
                    case MSG_ID_NFC_READER_MODE_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_READER_MODE_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_READER_MODE_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }          
                    case MSG_ID_NFC_P2P_MODE_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_P2P_MODE_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_P2P_MODE_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }    
                    case MSG_ID_NFC_SWP_SELF_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_SWP_SELF_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_SWP_SELF_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }       
                    case MSG_ID_NFC_ANTENNA_SELF_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_ANTENNA_SELF_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_ANTENNA_SELF_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                           
                        break;
                    }                     
                    case MSG_ID_NFC_TAG_UID_RW_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_UID_RW;
                        memcpy(&nfc_cnf.result.m_script_uid_cnf, (nfc_script_uid_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_uid_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_UID_RW =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }
                    case MSG_ID_NFC_CARD_MODE_TEST_RSP:
                    case MSG_ID_NFC_STOP_TEST_RSP:
                    default:
                    {
                        fgSupport = 0;
                        META_LOG("META_NFC_CMD:Don't support CNF CMD %d",nfc_msg.msg_type);
                        break;
                    }
                }
                if (fgSupport == 1)
                {
                    META_LOG("META_NFC_CMD:NFC read nfc_cnf.op=%d,nfc_msg.msg_type=%d", nfc_cnf.op,nfc_msg.msg_type);
                    nfc_cnf.status = META_SUCCESS;
                    #ifdef META_NFC_SELF_TEST_EN
                    if (meta_nfc_cnf_cb)
                    {
                        meta_nfc_cnf_cb(&nfc_cnf, NULL, 0);
                    }
                    else
                    #endif
                    {
                        WriteDataToPC(&nfc_cnf, sizeof(NFC_CNF), NULL, 0);
                    }
                }
                else 
                {

                    META_LOG("META_NFC_CMD:Don't Write to PC MSGID,%d,",nfc_msg.msg_type);
                }
            }
            else 
            {
                META_LOG("META_NFC_CMD:Don't support MSGID,%d,DestID,%d",nfc_ilm_rec.msg_id, nfc_ilm_rec.dest_mod_id);
            }
            #endif
            MTK_NFC_MSG_T *nfc_msg_loc = (MTK_NFC_MSG_T *)buffer;
            
            META_LOG("META_NFC_CMD:NFC read (msg_id,length) = (%d,%d)",nfc_msg_loc->type, nfc_msg_loc->length);
            //New UI!!
            switch(nfc_msg_loc->type)
            {
               case MTK_NFC_EM_ALS_READER_MODE_RSP:
               {
                   s_mtk_nfc_em_als_readerm_rsp *locPtr = (s_mtk_nfc_em_als_readerm_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));
                   nfc_cnf.op = NFC_OP_ALS_READER_MODE;

                   nfc_cnf.result.m_nNfc_als_readerm_rsp.result = locPtr->result;
                   
                   //memcpy(&nfc_cnf.result.m_nNfc_als_readerm_rsp, (s_mtk_nfc_em_als_readerm_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_als_readerm_rsp));                            
                   META_LOG("META_NFC_CMD:NFC NFC_OP_ALS_READER_MODE_REQ =%02x/", 
                            nfc_cnf.result.m_nNfc_als_readerm_rsp.result);                                   
                   break;
               }
               case MTK_NFC_EM_ALS_READER_MODE_OPT_RSP:
               {                
                
                   mtkNfc_NdefRecord_t Record;
                   int       Status;
                   int i;
                   s_mtk_nfc_em_als_readerm_opt_rsp *locPtr = (s_mtk_nfc_em_als_readerm_opt_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));
                   
                   memset(&Record, 0, sizeof(mtkNfc_NdefRecord_t));
                   memset(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp, 0x00 , sizeof(s_mtk_nfc_meta_als_readerm_opt_rsp));


                   META_LOG("MTK_NFC_EM_ALS_READER_MODE_OPT_RSP,ACTION,%d",gTagAction);
                   if(gTagAction == NFC_EM_OPT_ACT_WRITE)
                   {
                       nfc_cnf.op = NFC_OP_ALS_READER_MODE_OPT;
                       nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.result = locPtr->result;
                       
                       META_LOG("nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.result",nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.result);
                   }
                   else if (gTagAction == NFC_EM_OPT_ACT_READ)
                   {
                   Status = mtk_nfc_NdefRecord_Parse(&Record, locPtr->ndef_read.data);

                       META_LOG("data ,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
                                              locPtr->ndef_read.data[0],
                                              locPtr->ndef_read.data[1],
                                              locPtr->ndef_read.data[2],
                                              locPtr->ndef_read.data[3],
                                              *Record.PayloadData,
                                              *(Record.PayloadData+1),
                                              *(Record.PayloadData+2),
                                              *(Record.PayloadData+3),
                                              Record.IdLength,
                                              Record.TypeLength);
    
                       
                       META_LOG("Status ,%d, %c", Status,*Record.Type );
    
                   if(Status == 0x00)
                   {
                      if (*Record.Type == 'T')
               {
                          MTK_NFC_RTD_DATA RTD_buffer;
                          RTD_buffer.pbuffer = Record.PayloadData;
                          RTD_buffer.u4length = Record.PayloadLength;
                              
                              META_LOG("(*Record.Type == 'T')");
                              
                          for (i =0;i < (RTD_buffer.pbuffer[0] & 0x1F);i++)
                   {
                              nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.lang[i] = RTD_buffer.pbuffer[1+i];
                              META_LOG("lang[%d]\r\n ",nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.lang[i]);
                          }
                          
                          memcpy(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data,
                          &RTD_buffer.pbuffer[1+i], (RTD_buffer.u4length-3));
                          
                              nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.length = (RTD_buffer.u4length-3);
    
                          nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.ndef_type  = nfc_ndef_type_text;

                      }
                      else if(*Record.Type == 'U')
                      {
                          MTK_NFC_RTD_DATA RTD_buffer;
                          RTD_buffer.pbuffer = Record.PayloadData;
                          RTD_buffer.u4length = Record.PayloadLength;
                              
                              META_LOG("(*Record.Type == 'U'),len,%d,PayloadLength,%d,%d,%d,%d,%d,%d",
                                (RTD_buffer.u4length - 1),
                                Record.PayloadLength,
                                RTD_buffer.pbuffer[0],
                                RTD_buffer.pbuffer[1],
                                RTD_buffer.pbuffer[2],
                                RTD_buffer.pbuffer[3],
                                RTD_buffer.pbuffer[4]);
                              
                              memcpy(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data,
                          &RTD_buffer.pbuffer[1], (RTD_buffer.u4length - 1));
                              nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.length = (RTD_buffer.u4length - 1);
                          nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.ndef_type = nfc_ndef_type_uri;
                      }
                      else
                      {
                              
                              META_LOG("(*Record.Type == 'default')");
                              //memcpy(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data, 0x00, 512);
                   }
               }
                   else
               {                
                           META_LOG("(statue != 0x00)");
                           //memcpy(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data, 0x00, 512);
                           //memcpy(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data, locPtr->ndef_read.data, 512);
                   }


                   nfc_cnf.op = NFC_OP_ALS_READER_MODE_OPT;
                   nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.result = locPtr->result;
                   nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.length = locPtr->ndef_read.length;
                   //nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.ndef_type = locPtr->ndef_read.ndef_type;
                   nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordFlags = Record.Flags;//locPtr->ndef_read.recordFlags;
                   nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordTnf = Record.Tnf;//locPtr->ndef_read.recordTnf;

                   //memcpy(nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.lang, locPtr->ndef_read.lang, 3);
                   //memcpy(nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data, locPtr->ndef_read.data, 512);
                       //memcpy(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordId, locPtr->ndef_read.recordId, 32);

                   //memcpy(&nfc_cnf.result.m_nNfc_als_readerm_opt_rsp, (s_mtk_nfc_em_als_readerm_opt_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_als_readerm_opt_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_ALS_READER_MODE_OPT_RSP =%02x,/",
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.result);     

                   META_LOG("META_NFC_CMD,MTK_NFC_EM_ALS_READER_MODE_OPT_RSP,[%x,%x,%x]",
                    sizeof(nfc_cnf),
                    sizeof(nfc_cnf.result.m_nNfc_als_readerm_opt_rsp),
                    sizeof(nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read));

                   
                   META_LOG("[%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x]",
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.lang[0],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.lang[1],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.lang[2],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordFlags,
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordId[0],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordId[1],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordId[2],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordId[3],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordId[4],
                        nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.ndef_type,
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.recordTnf,
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.length,
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data[0],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data[1],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data[2],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data[3],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data[4],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data[5],
                    nfc_cnf.result.m_nNfc_als_readerm_opt_rsp.ndef_read.data[6]);
                   }
                   else
                   {
                       META_LOG("Invalid gTagAction,%d",gTagAction);
                   }
                   
                   gTagAction = 0xFF;
                   break;
               }
               case MTK_NFC_EM_ALS_P2P_MODE_RSP:
               {
                   s_mtk_nfc_em_als_p2p_rsp *locPtr = (s_mtk_nfc_em_als_p2p_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));
                   nfc_cnf.op = NFC_OP_ALS_P2P_MODE;

                   nfc_cnf.result.m_nNfc_als_p2p_rsp.result = locPtr->result;
                   
                   //memcpy(&nfc_cnf.result.m_nNfc_als_p2p_rsp, (s_mtk_nfc_em_als_p2p_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_als_p2p_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_ALS_P2P_MODE_RSP =%02x,/", 
                    nfc_cnf.result.m_nNfc_als_p2p_rsp.result);                                  
                   break;
               }
               case MTK_NFC_EM_ALS_P2P_MODE_NTF:
               {
                   s_mtk_nfc_em_als_p2p_ntf *locPtr = (s_mtk_nfc_em_als_p2p_ntf*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   //
                   nfc_cnf.header.token = gBackUpToken;

                   nfc_cnf.op = NFC_OP_ALS_P2P_MODE;

                   nfc_cnf.result.m_nNfc_als_p2p_ntf.link_status= locPtr->link_status;

                   //memcpy(&nfc_cnf.result.m_nNfc_als_p2p_ntf, (s_mtk_nfc_em_als_p2p_ntf*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_als_p2p_ntf));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_ALS_P2P_MODE_NTF =%02x,/", 
                    nfc_cnf.result.m_nNfc_als_p2p_ntf.link_status);   
                   break;
               }
               case MTK_NFC_EM_ALS_CARD_MODE_RSP:
               {
                   s_mtk_nfc_em_als_cardm_rsp *locPtr = (s_mtk_nfc_em_als_cardm_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_ALS_CARD_MODE;

                   nfc_cnf.result.m_nNfc_als_cardm_rsp.result= locPtr->result;

                   //memcpy(&nfc_cnf.result.m_nNfc_als_cardm_rsp, (s_mtk_nfc_em_als_cardm_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_als_cardm_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_ALS_CARD_MODE_RSP =%02x,/", 
                    nfc_cnf.result.m_nNfc_als_cardm_rsp.result);                                   
                   break;
               }
               case MTK_NFC_EM_POLLING_MODE_RSP:
               {
                   s_mtk_nfc_em_polling_rsp *locPtr = (s_mtk_nfc_em_polling_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_POLLING_MODE;
                   
                   nfc_cnf.result.m_nNfc_als_cardm_rsp.result= locPtr->result;

                   
                   //memcpy(&nfc_cnf.result.m_nNfc_polling_rsp, (s_mtk_nfc_em_polling_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_polling_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_POLLING_MODE_RSP =%02x,/", 
                    nfc_cnf.result.m_nNfc_polling_rsp.result);                                    
                   break;
               }
               
               case MTK_NFC_EM_ALS_READER_MODE_NTF:
               {
                   s_mtk_nfc_em_als_readerm_ntf *locPtr = (s_mtk_nfc_em_als_readerm_ntf*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));
                   nfc_cnf.op = NFC_OP_ALS_READER_MODE;
                   nfc_cnf.result.m_nNfc_als_readerm_ntf.result = locPtr->result;
                   nfc_cnf.result.m_nNfc_als_readerm_ntf.isNDEF = locPtr->isNDEF;
                   nfc_cnf.result.m_nNfc_als_readerm_ntf.UidLen = locPtr->UidLen;
                   //
                   nfc_cnf.header.token = gBackUpToken;
                   
                   memcpy(&nfc_cnf.result.m_nNfc_als_readerm_ntf.Uid, locPtr->Uid , locPtr->UidLen);
                    
                   //memcpy(&nfc_cnf.result.m_nNfc_als_readerm_ntf, (s_mtk_nfc_em_als_readerm_ntf*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_als_readerm_ntf));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_ALS_READER_MODE_NTF =%02x,%02x,%02x,%02x,/", 
                    nfc_cnf.result.m_nNfc_als_readerm_ntf.isNDEF,
                    nfc_cnf.result.m_nNfc_als_readerm_ntf.result,
                    nfc_cnf.result.m_nNfc_als_readerm_ntf.Uid,
                    nfc_cnf.result.m_nNfc_als_readerm_ntf.UidLen); 
                   {
                      int i;
                      for(i=0;i<nfc_cnf.result.m_nNfc_als_readerm_ntf.UidLen;i++)
                      {
                          META_LOG("Uid[%d][%02x]",i,nfc_cnf.result.m_nNfc_als_readerm_ntf.Uid[i]);
                      }
                   }
                   break;
               }
               
               case MTK_NFC_EM_POLLING_MODE_NTF:
               {
                   s_mtk_nfc_em_polling_ntf *locPtr = (s_mtk_nfc_em_polling_ntf*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   //
                   nfc_cnf.header.token = gBackUpToken;
                   
                   nfc_cnf.op = NFC_OP_POLLING_MODE;
                   
                   nfc_cnf.result.m_nNfc_polling_func_ntf.detecttype= locPtr->detecttype;
                   nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.card.result= locPtr->ntf.card.result;
                   nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.p2p.link_status= locPtr->ntf.p2p.link_status;
                   nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.reader.isNDEF= locPtr->ntf.reader.isNDEF;
                   nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.reader.result= locPtr->ntf.reader.result;
                   nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.reader.UidLen= locPtr->ntf.reader.UidLen;

                   memcpy(&nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.reader.Uid, locPtr->ntf.reader.Uid, 10);

                   //memcpy(&nfc_cnf.result.m_nNfc_polling_func_ntf, (s_mtk_nfc_em_polling_func_ntf*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_polling_func_ntf));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_POLLING_MODE_NTF =%02x,%02x,%02x,%02x,/", 
                    nfc_cnf.result.m_nNfc_polling_func_ntf.detecttype,
                    nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.card.result,
                    nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.p2p.link_status,
                    nfc_cnf.result.m_nNfc_polling_func_ntf.ntf.reader.result);                                  
                   break;
               }
               case MTK_NFC_EM_TX_CARRIER_ALS_ON_RSP:
               {
                   s_mtk_nfc_em_tx_carr_als_on_rsp *locPtr = (s_mtk_nfc_em_tx_carr_als_on_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_TX_CARRIER_ALS_ON;
                   
                   nfc_cnf.result.m_nNfc_tx_carr_als_on_rsp.result= locPtr->result;


                   //memcpy(&nfc_cnf.result.m_nNfc_tx_carr_als_on_rsp, (s_mtk_nfc_em_tx_carr_als_on_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_tx_carr_als_on_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_TX_CARRIER_ALS_ON_RSP =%02x,/", 
                    nfc_cnf.result.m_nNfc_tx_carr_als_on_rsp.result);                                       
                   break;
               }
               case MTK_NFC_EM_VIRTUAL_CARD_RSP:
               {
                   s_mtk_nfc_em_virtual_card_rsp *locPtr = (s_mtk_nfc_em_virtual_card_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_VIRTUAL_CARD;

                   nfc_cnf.result.m_nNfc_virtual_card_rsp.result= locPtr->result;

                   //memcpy(&nfc_cnf.result.m_nNfc_virtual_card_rsp, (s_mtk_nfc_em_virtual_card_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_virtual_card_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_VIRTUAL_CARD_RSP =%02x,/", 
                    nfc_cnf.result.m_nNfc_virtual_card_rsp.result);                                
                   break;
               }
               case MTK_NFC_EM_PNFC_CMD_RSP:
               {
                   s_mtk_nfc_em_pnfc_new_rsp *locPtr = (s_mtk_nfc_em_pnfc_new_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_PNFC_CMD;
                   
                   nfc_cnf.result.m_nNfc_pnfc_new_rsp.result= locPtr->result;
                   nfc_cnf.result.m_nNfc_pnfc_new_rsp.datalen= locPtr->datalen;

                   memcpy(&nfc_cnf.result.m_nNfc_pnfc_new_rsp.data, locPtr->data, 256);

                   //memcpy(&nfc_cnf.result.m_nNfc_pnfc_new_rsp, (s_mtk_nfc_em_pnfc_new_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_em_pnfc_new_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_EM_PNFC_CMD_RSP =%02x,%02x,%s,/", 
                    nfc_cnf.result.m_nNfc_pnfc_new_rsp.result,
                    nfc_cnf.result.m_nNfc_pnfc_new_rsp.datalen,
                    nfc_cnf.result.m_nNfc_pnfc_new_rsp.data);                                    
                   break;
               }
               case MTK_NFC_TESTMODE_SETTING_RSP:
               {
                   s_mtk_nfc_test_mode_Setting_rsp_t *locPtr = (s_mtk_nfc_test_mode_Setting_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_TESTMODE_SETTING;
                   
                   nfc_cnf.result.m_nNfc_test_mode_Setting_rsp.result= locPtr->result;

                   //memcpy(&nfc_cnf.result.m_nNfc_test_mode_Setting_rsp, (s_mtk_nfc_test_mode_Setting_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_test_mode_Setting_rsp_t));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_TESTMODE_SETTING_RSP =%02x", 
                    nfc_cnf.result.m_nNfc_test_mode_Setting_rsp.result);                                   
                   break;
               }
               case MTK_EM_LOOPBACK_TEST_RSP:
               {
                   s_mtk_nfc_loopback_test_rsp_t *locPtr = (s_mtk_nfc_loopback_test_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_LOOPBACK_TEST;
                   nfc_cnf.result.m_nNfc_loopback_test_rsp.result= locPtr->result;
                   
                   //memcpy(&nfc_cnf.result.m_nNfc_loopback_test_rsp, (s_mtk_nfc_loopback_test_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_loopback_test_rsp_t));                            
                   META_LOG("META_NFC_CMD:NFC MTK_EM_LOOPBACK_TEST_RSP =%02x", 
                    nfc_cnf.result.m_nNfc_loopback_test_rsp.result);                                    
                   break;
               }
               case MTK_NFC_FM_SWP_TEST_RSP:
               {
                   s_mtk_nfc_fm_swp_test_rsp *locPtr = (s_mtk_nfc_fm_swp_test_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_SWP_TEST;
                   nfc_cnf.result.m_nNfc_swp_test_rsp.result= locPtr->result;

                   
                   //memcpy(&nfc_cnf.result.m_nNfc_swp_test_rsp, (s_mtk_nfc_fm_swp_test_rsp*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_fm_swp_test_rsp));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_FM_SWP_TEST_RSP =%02x", 
                    nfc_cnf.result.m_nNfc_swp_test_rsp.result);                                   
                   break;
               }
               case MTK_NFC_SW_VERSION_RESPONSE:
               {
                   s_mtk_nfc_sw_Version_rsp_t *locPtr = (s_mtk_nfc_sw_Version_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_SW_VERSION;
                   nfc_cnf.result.m_nNfc_sw_Version_rsp.fw_ver= locPtr->fw_ver;
                   nfc_cnf.result.m_nNfc_sw_Version_rsp.hw_ver= locPtr->hw_ver;

                   memcpy(&nfc_cnf.result.m_nNfc_sw_Version_rsp.mw_ver, locPtr->mw_ver, 19);

                   //memcpy(&nfc_cnf.result.m_nNfc_sw_Version_rsp, (s_mtk_nfc_sw_Version_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_sw_Version_rsp_t));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_SW_VERSION_RESPONSE =%02x,%02x,%s,", 
                    nfc_cnf.result.m_nNfc_sw_Version_rsp.fw_ver,
                    nfc_cnf.result.m_nNfc_sw_Version_rsp.hw_ver,
                    nfc_cnf.result.m_nNfc_sw_Version_rsp.mw_ver);                                    
                   break;
               }
               case MTK_NFC_META_GET_SELIST_RSP:                
               {
                   int i;
                   s_mtk_nfc_jni_se_get_list_rsp_t *locPtr = (s_mtk_nfc_jni_se_get_list_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T));

                   nfc_cnf.op = NFC_OP_GET_SELIST;
                   nfc_cnf.result.m_nNfc_se_get_list_req.status = locPtr->status;
                   nfc_cnf.result.m_nNfc_se_get_list_req.SeCount = locPtr->SeCount;
                   
                   META_LOG(",%d,%d,%d",
                    locPtr->SeInfor[0].seid,
                    locPtr->SeInfor[1].seid,
                    locPtr->SeInfor[2].seid);
                   
                   for(i=0;i<3;i++)
                   {
                       nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[i].connecttype = locPtr->SeInfor[i].connecttype;
                       nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[i].lowpowermode = locPtr->SeInfor[i].lowpowermode;
                       nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[i].pbf = locPtr->SeInfor[i].pbf;
                       nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[i].seid = locPtr->SeInfor[i].seid;
                       nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[i].status = locPtr->SeInfor[i].status;
                       nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[i].type = locPtr->SeInfor[i].type;
                   }
                   //memcpy(&nfc_cnf.result.m_nNfc_se_get_list_req, (s_mtk_nfc_jni_se_get_list_rsp_t*)((UINT8 *)nfc_msg_loc + sizeof(MTK_NFC_MSG_T)), sizeof(s_mtk_nfc_jni_se_get_list_rsp_t));                            
                   META_LOG("META_NFC_CMD:NFC MTK_NFC_GET_SELIST_RSP =%02x,%02x,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,", 
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeCount,
                    nfc_cnf.result.m_nNfc_se_get_list_req.status,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[0].seid,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[0].connecttype,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[0].lowpowermode,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[0].pbf,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[0].status,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[0].type,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[1].seid,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[1].connecttype,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[1].lowpowermode,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[1].pbf,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[1].status,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[1].type,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[2].seid,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[2].connecttype,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[2].lowpowermode,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[2].pbf,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[2].status,
                    nfc_cnf.result.m_nNfc_se_get_list_req.SeInfor[2].type);     
                   
                   break;
               }
               default:
                  fgSupport = 0;
                  META_LOG("META_NFC_CMD:NFC default case");                                    
                  break;
 
            }
            
            if (fgSupport == 1)
            {
                META_LOG("META_NFC_CMD:NFC read nfc_cnf.op=%d,nfc_msg.msg_type=%d", nfc_cnf.op,nfc_msg.msg_type);
                nfc_cnf.status = META_SUCCESS;
                #ifdef META_NFC_SELF_TEST_EN
                if (meta_nfc_cnf_cb)
                {
                    meta_nfc_cnf_cb(&nfc_cnf, NULL, 0);
                }
                else
                #endif
                {
                    WriteDataToPC(&nfc_cnf, sizeof(NFC_CNF), NULL, 0);
                }
            }
            else 
            {

                META_LOG("META_NFC_CMD:Don't Write to PC MSGID,%d,",nfc_msg.msg_type);
            }
        }        
      //  else
      //  {
        //    usleep(100000); // wake up every 0.1sec     
       // }
    }
    bStop_ReadThread = 1;
    META_LOG("META_NFC_CMD:NFC read thread stop");
    pthread_exit(NULL);
   return NULL;

}
/********************************************************************************
//FUNCTION:
//		META_NFC_init
//DESCRIPTION:
//		NFC Init for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		true : success
//      false: failed
//
********************************************************************************/
#ifdef USING_LOCAL_SOCKET
#define MTKNFC_COMM_SOCK    "/data/mtknfc_server"
#endif
int META_NFC_init(void)
{
    
    pid_t pid;
    //int portno;

    // Run nfc service process
    if ((pid = fork()) < 0) 
    {
        META_LOG("META_NFC_init: fork fails: %d (%s)\n", errno, strerror(errno));
        return (-2);
    } 
    else if (pid == 0)  /*child process*/
    {
        int err;
    
        META_LOG("nfc_open: execute: %s\n", "/system/xbin/nfcservice");
        err = execl("/system/xbin/nfcstackp", "nfcstackp", "NFC_TEST_MODE", NULL);
        if (err == -1)
        {
            META_LOG("META_NFC_init: execl error: %s\n", strerror(errno));
            return (-3);
        }
        return 0;
    } 
    else  /*parent process*/
    {
        META_LOG("META_NFC_init: pid = %d\n", pid);
    }

    // Create socket

    
    #ifdef USING_LOCAL_SOCKET
    struct sockaddr_un address;
    int len;
    
    //printf("nfc_open: SELF TEST COD"); 
    //nfc_sockfd = socket(AF_UNIX, SOCK_STREAM, 0); 
    nfc_service_sockfd = socket(AF_LOCAL, SOCK_STREAM, 0); 
    if (nfc_service_sockfd < 0)     
    {        
       META_LOG("nfc_open: ERROR opening socket");        
       return (-4);    
    } 

    address.sun_family = AF_LOCAL;//AF_UNIX;
    strcpy (address.sun_path, MTKNFC_COMM_SOCK);
    len = sizeof (address);
            
    sleep(3);  // sleep 5sec for libmnlp to finish initialization        
      
    META_LOG("connecting(%s)...\r\n",address.sun_path);   
            
    /* Now connect to the server */    
    if (connect(nfc_service_sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)     
    {         
       META_LOG("NFC_Open: ERROR connecting\r\n");         
       return (-6);    
    }    

    #else
    struct sockaddr_in serv_addr;
    struct hostent *server;	
    
    nfc_service_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfc_service_sockfd < 0) 
    {
        META_LOG("META_NFC_init: ERROR opening socket");
        return (-4);
    }
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        META_LOG("META_NFC_init: ERROR, no such host\n");
        return (-5);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(SOCKET_NFC_PORT);

    sleep(3);  // sleep 5sec for nfcservice to finish initialization
    
    /* Now connect to the server */
    if (connect(nfc_service_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
         META_LOG("META_NFC_init: ERROR connecting");
         return (-6);
    }

    #endif
    META_LOG("META_NFC_init: create read command thread\n"); 
    if(pthread_create(&read_cnf_thread_handle, NULL, META_NFC_read_cnf,
          NULL) != 0)
    {
       META_LOG("META_NFC_init:Fail to create read command thread");
       return (-7);
    }   

    META_LOG("META_NFC_init: done\n"); 
	return (0);
}

/********************************************************************************
//FUNCTION:
//		META_NFC_deinit
//DESCRIPTION:
//		NFC deinit for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//     
********************************************************************************/
void META_NFC_deinit()
{
    int err=0;
    /* stop RX thread */
    bStop_ReadThread = 1;
    
    /* wait until thread exist */
    pthread_join(read_cnf_thread_handle, NULL);

    /* Close socket port */
    if (nfc_service_sockfd > 0)
    {
        close (nfc_service_sockfd);
        nfc_service_sockfd = -1;
    }
    // kill service process
    META_LOG("META_NFC_deinit: kill: %s\n", "/system/xbin/nfcservice");
    err = execl("kill /system/xbin/nfcservice", "nfcservice", NULL);
    if (err == -1)
    {
        META_LOG("META_NFC_init: kill error: %s\n", strerror(errno));
    }
    return;   
}
/********************************************************************************
//FUNCTION:
//		META_NFC_CMD
//DESCRIPTION:
//		SEND MESSAGE to NFC driver
//      RECEIVE MESSAGE to NFC driver
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//     
********************************************************************************/
void META_NFC_CMD(ilm_struct* nfc_ilm_req_ptr)
{

    int ret = 0;
    int rec_bytes = 0;
    int rety_count = 0;
    META_LOG("META_NFC_CMD:write CMD");

    // Write request command
    ret = write(nfc_service_sockfd, (const char*)nfc_ilm_req_ptr, sizeof(ilm_struct));

    if ( ret <= 0)
    {
        META_LOG("META_NFC_CMD:write failure,%d",ret);
        return;
    }
    else
    {
        META_LOG("META_NFC_CMD:write CMD done,%d",ret);
    }
    return;
}

void META_NFC_SEND_SOCKET(MTK_NFC_MSG_T *msg)
{
    int32_t ret;
    if (msg == NULL)
    {
       // MNL_DEBUG_OUTPUT(MDBG_MSG, DBG_ERR,"MNLMsgS", "NULL", "");
        return MTK_NFC_ERROR;
    }
    ret = write(nfc_service_sockfd, msg, (sizeof(MTK_NFC_MSG_T) + msg->length)); 
    
    META_LOG("mtk_nfc_sys_msg_send: ret,%d\n",ret); 

    free( (VOID*)msg);

    return;
}


/********************************************************************************
//FUNCTION:
//		META_NFC_OP
//DESCRIPTION:
//		META NFC test main process function.
//
//PARAMETERS:
//		req: NFC Req struct
//      peer_buff: peer buffer pointer
//      peer_len: peer buffer length
//RETURN VALUE:
//		void
//      
********************************************************************************/
void META_NFC_OP(NFC_REQ *req, char *peer_buff, unsigned short peer_len) 
{     
    ilm_struct nfc_ilm_loc;
    nfc_msg_struct nfc_msg;
    memset(&nfc_cnf, 0, sizeof(NFC_CNF));
    memset(&nfc_msg, 0, sizeof(nfc_msg_struct));
    nfc_cnf.header.id = FT_NFC_CNF_ID;
    nfc_cnf.header.token = req->header.token;
    nfc_cnf.op = req->op;

    memset(&nfc_ilm_loc, 0, sizeof(ilm_struct));    
    nfc_ilm_loc.msg_id = MSG_ID_NFC_TEST_REQ;
    nfc_ilm_loc.src_mod_id = MOD_NFC_APP;
    nfc_ilm_loc.dest_mod_id = MOD_NFC;    
    
    switch(req->op)
    {
        META_LOG("META_NFC_OP:NFC request op=%d", req->op);

#if 0
        
        case NFC_OP_SETTING:
        {
            //Write handle function here
            nfc_msg.msg_length = sizeof(nfc_setting_request);
            nfc_msg.msg_type = MSG_ID_NFC_SETTING_REQ;
            META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);
            memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
            memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)), (char*)&req->cmd.m_setting_req, sizeof(nfc_setting_request));
            META_NFC_CMD(&nfc_ilm_loc);
            break;
       }
       case NFC_OP_REG_NOTIFY:
	   {
           //Write handle function here
	       nfc_msg.msg_length = sizeof(nfc_reg_notif_request);
           nfc_msg.msg_type = MSG_ID_NFC_NOTIFICATION_REQ;
           
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),(char*)&req->cmd.m_reg_notify_req, sizeof(nfc_reg_notif_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_SECURE_ELEMENT:
       {
            //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_se_set_request);
           nfc_msg.msg_type = MSG_ID_NFC_SE_SET_REQ;

           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_se_set_req, sizeof(nfc_se_set_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_DISCOVERY:
       {
        //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_dis_notif_request);
           nfc_msg.msg_type = MSG_ID_NFC_DISCOVERY_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_dis_notify_req, sizeof(nfc_dis_notif_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_READ:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_read_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_READ_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_read_req, sizeof(nfc_tag_read_request));
           break;
       }
       case NFC_OP_TAG_WRITE:
       {
            //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_write_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_WRITE_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_write_req, sizeof(nfc_tag_write_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_DISCONN:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_disconnect_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_DISCONN_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_discon_req, sizeof(nfc_tag_disconnect_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_FORMAT_NDEF:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_fromat2Ndef_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_F2NDEF_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_fromat2Ndef_req, sizeof(nfc_tag_fromat2Ndef_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_RAW_COMM:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_raw_com_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_RAWCOM_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_raw_com_req, sizeof(nfc_tag_raw_com_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_P2P_COMM:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_p2p_com_request);
           nfc_msg.msg_type = MSG_ID_NFC_P2P_COMMUNICATION_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_p2p_com_req, sizeof(nfc_p2p_com_request));
           if ((peer_buff != NULL) && (peer_len != 0))
           {
               memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct) + sizeof(nfc_p2p_com_request)), peer_buff, peer_len);          
           }
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_RD_COMM:
       {
            //Write handle function here
            nfc_msg.msg_length = sizeof(nfc_rd_com_request);
            nfc_msg.msg_type = MSG_ID_NFC_RD_COMMUNICATION_REQ;
            META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
            
            memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
            memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)), &req->cmd.m_rd_com_req, sizeof(nfc_rd_com_request));
            if ((peer_buff != NULL) && (peer_len != 0))
            {
                memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)+ sizeof(nfc_rd_com_request)), peer_buff, peer_len);          
            } 
            META_NFC_CMD(&nfc_ilm_loc);
            break;
       }
       case NFC_OP_TX_ALWAYSON_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_TX_ALWAYSON_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_tx_alwayson_req, sizeof(nfc_tx_alwayson_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_TX_ALWAYSON_WO_ACK_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_tx_alwayson_req, sizeof(nfc_tx_alwayson_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_CARD_MODE_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_CARD_EMULATION_MODE_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_card_emulation_req, sizeof(nfc_card_emulation_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_READER_MODE_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_READER_MODE_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_P2P_MODE_TEST: 
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_P2P_MODE_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_SWP_SELF_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_SWP_SELF_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }        
       case NFC_OP_ANTENNA_SELF_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_ANTENNA_SELF_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_TAG_UID_RW:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_uid_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_UID_RW_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
                
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_uid_req, sizeof(nfc_script_uid_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }    
#endif

       /*
       New UI
       */
       case NFC_OP_ALS_READER_MODE:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_als_readerm_req *locPrt= NULL;
           
           gBackUpToken = nfc_cnf.header.token;
           
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_als_readerm_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_als_readerm_req);
           msg->type = MTK_NFC_EM_ALS_READER_MODE_REQ;           

           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T));
           
           locPrt->action = req->cmd.m_nNfc_als_readerm_req.action;
           locPrt->supporttype = req->cmd.m_nNfc_als_readerm_req.supporttype;
           locPrt->typeA_datarate = req->cmd.m_nNfc_als_readerm_req.typeA_datarate;
           locPrt->typeB_datarate = req->cmd.m_nNfc_als_readerm_req.typeB_datarate;
           locPrt->typeF_datarate = req->cmd.m_nNfc_als_readerm_req.typeF_datarate;
           locPrt->typeV_datarate = req->cmd.m_nNfc_als_readerm_req.typeV_datarate;
           locPrt->typeV_subcarrier = req->cmd.m_nNfc_als_readerm_req.typeV_subcarrier;
           
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_als_readerm_req, sizeof(s_mtk_nfc_em_als_readerm_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);  

           META_LOG("%02x,%02x,%02x,%02x,%02x,%02x,%02x,",
                   req->cmd.m_nNfc_als_readerm_req.action,
                   req->cmd.m_nNfc_als_readerm_req.supporttype,
                   req->cmd.m_nNfc_als_readerm_req.typeA_datarate,
                   req->cmd.m_nNfc_als_readerm_req.typeB_datarate,
                   req->cmd.m_nNfc_als_readerm_req.typeF_datarate,
                   req->cmd.m_nNfc_als_readerm_req.typeV_datarate,
                   req->cmd.m_nNfc_als_readerm_req.typeV_subcarrier);
           
           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_ALS_P2P_MODE:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_als_p2p_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_als_p2p_req));

           gBackUpToken = nfc_cnf.header.token;

           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_als_p2p_req);
           msg->type = MTK_NFC_EM_ALS_P2P_MODE_REQ;           
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T));
           
           locPrt->action = req->cmd.m_nNfc_als_p2p_req.action;
           locPrt->isDisableCardM = req->cmd.m_nNfc_als_p2p_req.isDisableCardM;
           locPrt->mode = req->cmd.m_nNfc_als_p2p_req.mode;
           locPrt->role = req->cmd.m_nNfc_als_p2p_req.role;
           locPrt->supporttype = req->cmd.m_nNfc_als_p2p_req.supporttype;
           locPrt->typeA_datarate = req->cmd.m_nNfc_als_p2p_req.typeA_datarate;
           locPrt->typeF_datarate = req->cmd.m_nNfc_als_p2p_req.typeF_datarate;


           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_als_p2p_req, sizeof(s_mtk_nfc_em_als_p2p_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           META_LOG("%02x,%02x,%02x,%02x,%02x,%02x,%02x,",
                   req->cmd.m_nNfc_als_p2p_req.action,
                   req->cmd.m_nNfc_als_p2p_req.isDisableCardM,
                   req->cmd.m_nNfc_als_p2p_req.mode,
                   req->cmd.m_nNfc_als_p2p_req.role,
                   req->cmd.m_nNfc_als_p2p_req.supporttype,
                   req->cmd.m_nNfc_als_p2p_req.typeA_datarate,
                   req->cmd.m_nNfc_als_p2p_req.typeF_datarate);
                      
           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_ALS_CARD_MODE:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_als_cardm_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_als_cardm_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_als_cardm_req);
           msg->type = MTK_NFC_EM_ALS_CARD_MODE_REQ;           
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action = req->cmd.m_nNfc_als_cardm_req.action;
           locPrt->fgvirtualcard = req->cmd.m_nNfc_als_cardm_req.fgvirtualcard;
           locPrt->supporttype = req->cmd.m_nNfc_als_cardm_req.supporttype;
           locPrt->SWNum = req->cmd.m_nNfc_als_cardm_req.SWNum;

           
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_als_cardm_req, sizeof(s_mtk_nfc_em_als_cardm_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           META_LOG("%02x,%02x,%02x,%02x,",
                   req->cmd.m_nNfc_als_cardm_req.action,
                   req->cmd.m_nNfc_als_cardm_req.fgvirtualcard,
                   req->cmd.m_nNfc_als_cardm_req.supporttype,
                   req->cmd.m_nNfc_als_cardm_req.SWNum);

           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_POLLING_MODE:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_polling_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_polling_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_polling_req);
           msg->type = MTK_NFC_EM_POLLING_MODE_REQ;           
           
           gBackUpToken = nfc_cnf.header.token;
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action = req->cmd.m_nNfc_polling_req.action;
           locPrt->enablefunc = req->cmd.m_nNfc_polling_req.enablefunc;
           locPrt->Period = req->cmd.m_nNfc_polling_req.Period;
           locPrt->phase = req->cmd.m_nNfc_polling_req.phase;
           locPrt->cardM.action= req->cmd.m_nNfc_polling_req.cardM.action;
           locPrt->cardM.fgvirtualcard= req->cmd.m_nNfc_polling_req.cardM.fgvirtualcard;
           locPrt->cardM.supporttype= req->cmd.m_nNfc_polling_req.cardM.supporttype;
           locPrt->cardM.SWNum= req->cmd.m_nNfc_polling_req.cardM.SWNum;
           locPrt->p2pM.action= req->cmd.m_nNfc_polling_req.p2pM.action;
           locPrt->p2pM.isDisableCardM= req->cmd.m_nNfc_polling_req.p2pM.isDisableCardM;
           locPrt->p2pM.mode= req->cmd.m_nNfc_polling_req.p2pM.mode;
           locPrt->p2pM.role= req->cmd.m_nNfc_polling_req.p2pM.role;
           locPrt->p2pM.supporttype= req->cmd.m_nNfc_polling_req.p2pM.supporttype;
           locPrt->p2pM.typeA_datarate= req->cmd.m_nNfc_polling_req.p2pM.typeA_datarate;
           locPrt->p2pM.typeF_datarate= req->cmd.m_nNfc_polling_req.p2pM.typeF_datarate;
           locPrt->readerM.action= req->cmd.m_nNfc_polling_req.readerM.action;
           locPrt->readerM.supporttype= req->cmd.m_nNfc_polling_req.readerM.supporttype;
           locPrt->readerM.typeA_datarate= req->cmd.m_nNfc_polling_req.readerM.typeA_datarate;
           locPrt->readerM.typeB_datarate= req->cmd.m_nNfc_polling_req.readerM.typeB_datarate;
           locPrt->readerM.typeF_datarate= req->cmd.m_nNfc_polling_req.readerM.typeF_datarate;
           locPrt->readerM.typeV_datarate= req->cmd.m_nNfc_polling_req.readerM.typeV_datarate;
           locPrt->readerM.typeV_subcarrier= req->cmd.m_nNfc_polling_req.readerM.typeV_subcarrier;

           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_polling_req, sizeof(s_mtk_nfc_em_polling_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           META_LOG("%02x,%02x",
                   req->cmd.m_nNfc_polling_req.action,
                   req->cmd.m_nNfc_polling_req.enablefunc);
           
           META_LOG("%02x,%02x,%02x,%02x,%02x",
                   locPrt->p2pM.isDisableCardM,
                   locPrt->cardM.action,
                   locPrt->cardM.fgvirtualcard,
                   locPrt->cardM.supporttype,
                   locPrt->cardM.SWNum       
           );


           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_TX_CARRIER_ALS_ON:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_tx_carr_als_on_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_tx_carr_als_on_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_tx_carr_als_on_req);
           msg->type = MTK_NFC_EM_TX_CARRIER_ALS_ON_REQ;   
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action = req->cmd.m_nNfc_tx_carr_als_on_req.action;    
           
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_tx_carr_als_on_req, sizeof(s_mtk_nfc_em_tx_carr_als_on_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           META_LOG("%02x,",
                   req->cmd.m_nNfc_tx_carr_als_on_req.action);

           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_VIRTUAL_CARD:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_virtual_card_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_virtual_card_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_virtual_card_req);
           msg->type = MTK_NFC_EM_VIRTUAL_CARD_REQ;           
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action = req->cmd.m_nNfc_virtual_card_req.action; 
           locPrt->supporttype = req->cmd.m_nNfc_virtual_card_req.supporttype; 
           locPrt->typeF_datarate = req->cmd.m_nNfc_virtual_card_req.typeF_datarate;    
                 
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_virtual_card_req, sizeof(s_mtk_nfc_em_virtual_card_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           META_LOG("%02x,%02x,%02x,",
                   req->cmd.m_nNfc_virtual_card_req.action,
                   req->cmd.m_nNfc_virtual_card_req.supporttype,
                   req->cmd.m_nNfc_virtual_card_req.typeF_datarate);

           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_PNFC_CMD:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_pnfc_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_pnfc_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_pnfc_req);
           msg->type = MTK_NFC_EM_PNFC_CMD_REQ;           
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action= 0x00;//req->cmd.m_nNfc_pnfc_req.u4action;
           locPrt->datalen = req->cmd.m_nNfc_pnfc_req.rEmPnfcReq.datalen;
           memcpy(locPrt->data, req->cmd.m_nNfc_pnfc_req.rEmPnfcReq.data, 256);
           //locPrt->u4ReqMsg = 0x00;//req->cmd.m_nNfc_pnfc_req.u4ReqMsg;
           //locPrt->rEmPnfcReq.action = req->cmd.m_nNfc_pnfc_req.rEmPnfcReq.action;
           //locPrt->rEmPnfcReq.datalen = req->cmd.m_nNfc_pnfc_req.rEmPnfcReq.datalen;
           //memcpy(locPrt->rEmPnfcReq.data, req->cmd.m_nNfc_pnfc_req.rEmPnfcReq.data, 256);
  
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_pnfc_req, sizeof(s_mtk_nfc_em_pnfc_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           META_LOG("%02x,%02x",
                   locPrt->action,
                   locPrt->datalen);
           {
           int i ;
           for(i=0;i < locPrt->datalen;i++)
           {
              META_LOG("Idx,%d,data[%d,%c]",i,locPrt->data[i],locPrt->data[i]);
           }
   

           }
           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_SW_VERSION:
       {
           MTK_NFC_MSG_T *msg = NULL;
           //s_mtk_nfc_pnfc_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T));
           //Write handle function here
           msg->length = 0;//sizeof(s_mtk_nfc_test_mode_Setting_req_t);
           msg->type = MTK_NFC_SW_VERSION_QUERY;           
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_test_mode_Setting_req, sizeof(s_mtk_nfc_test_mode_Setting_req_t));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   
           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_TESTMODE_SETTING:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_test_mode_Setting_req_t *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_test_mode_Setting_req_t));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_test_mode_Setting_req_t);
           msg->type = MTK_NFC_TESTMODE_SETTING_REQ;           
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->forceDownLoad = req->cmd.m_nNfc_test_mode_Setting_req.forceDownLoad;
           locPrt->TagAutoPresenceChk = req->cmd.m_nNfc_test_mode_Setting_req.TagAutoPresenceChk;
           
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_test_mode_Setting_req, sizeof(s_mtk_nfc_test_mode_Setting_req_t));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           META_LOG("%02x,%02x,",
                   req->cmd.m_nNfc_test_mode_Setting_req.forceDownLoad,
                   req->cmd.m_nNfc_test_mode_Setting_req.TagAutoPresenceChk);

           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_LOOPBACK_TEST:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_loopback_test_req_t *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_loopback_test_req_t));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_loopback_test_req_t);
           msg->type = MTK_EM_LOOPBACK_TEST_REQ;           
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action = req->cmd.m_nNfc_loopback_test_req.action;          
           
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_loopback_test_req, sizeof(s_mtk_nfc_loopback_test_req_t));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   
           
           META_LOG("%02x,",
                   req->cmd.m_nNfc_loopback_test_req.action);
           
           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_SWP_TEST:
       {
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_fm_swp_test_req *locPrt = NULL;
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_fm_swp_test_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_fm_swp_test_req);
           msg->type = MTK_NFC_FM_SWP_TEST_REQ;
           
           locPrt = ((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action = req->cmd.m_nNfc_swp_test_req.action; 
           locPrt->SEmap = req->cmd.m_nNfc_swp_test_req.SEmap; 

           
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_swp_test_req, sizeof(s_mtk_nfc_fm_swp_test_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   

           
           META_LOG("%02x,",
                   req->cmd.m_nNfc_swp_test_req.action);
           
           META_NFC_SEND_SOCKET(msg);
       }
       break;
       case NFC_OP_ALS_READER_MODE_OPT:
       {
           int len;
           uint8_t data[256];
           int result=FALSE;
           MTK_NFC_MSG_T *msg = NULL;
           s_mtk_nfc_em_als_readerm_opt_req *locPrt = NULL;

           
           msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_als_readerm_opt_req));
           //Write handle function here
           msg->length = sizeof(s_mtk_nfc_em_als_readerm_opt_req);
           msg->type = MTK_NFC_EM_ALS_READER_MODE_OPT_REQ;
           
           memset(&data, 0x00 , 256);
           
           locPrt = (s_mtk_nfc_em_als_readerm_opt_req *)((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)); 

           locPrt->action = req->cmd.m_nNfc_als_readerm_opt_req.action;

           if(locPrt->action == NFC_EM_OPT_ACT_WRITE)
           {
               gTagAction = NFC_EM_OPT_ACT_WRITE;
               META_LOG("ACTION...WRITE");
               
           locPrt->ndef_write.length = req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.length;
           locPrt->ndef_write.ndef_type = req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_type;
           locPrt->ndef_write.language = req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.language;

               //locPrt->ndef_write.ndef_data.TX_Data.DataLength = req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.DataLength;
               //
               META_LOG("Tag_WRITE,%d",
                        locPrt->ndef_write.ndef_type);
    
               META_LOG(",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.language,
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.length,
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_type,
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.DataLength,
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLLength,
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.data[0],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.data[1],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.data[2],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.data[3],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.data[4],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLData[0],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLData[1],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLData[2],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLData[3],
                        req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLData[4]
               );
               
               if(locPrt->ndef_write.ndef_type == nfc_ndef_type_text)
               {
                  result = TRUE;
                  memcpy(locPrt->ndef_write.ndef_data.TX_Data.data , req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.data, 256);
           locPrt->ndef_write.ndef_data.TX_Data.DataLength = req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.TX_Data.DataLength;
               }
               else if (locPrt->ndef_write.ndef_type == nfc_ndef_type_uri)
               {
                  result = TRUE;
                   memcpy(locPrt->ndef_write.ndef_data.URL_Data.URLData , req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLData, 64);
                   locPrt->ndef_write.ndef_data.URL_Data.URLLength = req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data.URL_Data.URLLength;
               }
               #if 0
               if((locPrt->ndef_write.ndef_type == nfc_ndef_type_text) || 
                  (locPrt->ndef_write.ndef_type == nfc_ndef_type_uri))
               {
                   /**/
                   //mtk_nfc_Generate_NDEF(int ndef_type, s_mtk_nfc_tag_write_ndef_data *data, uint8_t *outBuf)
                   len = mtk_nfc_Generate_NDEF(locPrt->ndef_write.ndef_type,
                                               locPrt->ndef_write.language,
                                               &req->cmd.m_nNfc_als_readerm_opt_req.ndef_write.ndef_data,
                                               &data[0]);
                   
                   META_LOG("mtk_nfc_Generate_NDEF,len(%d),%d,%d,%d,%d,%d\r\n",
                            len,
                            data[0],
                            data[1],
                            data[2],
                            data[3],
                            data[4]); 

                    {
                    int i;
                    for(i=0;i<len;i++)
                    {
                        META_LOG("DATA,IDx,%d,[%x,%c]",i, data[i], data[i]);
                    }

                    }
                   
                   if(len > 0)
                   {
                      result = TRUE;
                      if(locPrt->ndef_write.ndef_type == nfc_ndef_type_text)
                      {
                         memcpy(locPrt->ndef_write.ndef_data.TX_Data.data , &data, 256);
                         locPrt->ndef_write.ndef_data.TX_Data.DataLength = len;
                      }
                      else if (locPrt->ndef_write.ndef_type == nfc_ndef_type_uri)
                      {
                          memcpy(locPrt->ndef_write.ndef_data.URL_Data.URLData , &data, 64);
                          locPrt->ndef_write.ndef_data.URL_Data.URLLength = len;
                      }
                      else
                      {
                          ;//
                      }
                   }
                   else
                   {
                      ;//
                   }
               }
               #endif
           }
           else
           {
               gTagAction = NFC_EM_OPT_ACT_READ;
               META_LOG("ACTION...READ");
               result = TRUE;
           }
           
           if(result == TRUE)
           {
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_als_readerm_opt_req, sizeof(s_mtk_nfc_em_als_readerm_opt_req));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   
           
           META_LOG("%02x,",
                   req->cmd.m_nNfc_als_readerm_opt_req.action);
           
           META_NFC_SEND_SOCKET(msg);
       }
           else
           {
               nfc_cnf.status = META_FAILED;
               WriteDataToPC(&nfc_cnf, sizeof(NFC_CNF), NULL, 0);
               META_LOG("NFC_OP_ALS_READER_MODE_OPT_ERR");
           }
       }
       break;
       case NFC_OP_GET_SELIST:
       {
           MTK_NFC_MSG_T *msg = NULL;
           //msg = malloc(sizeof(MTK_NFC_MSG_T) + sizeof(s_mtk_nfc_em_als_readerm_opt_req));
           msg = malloc(sizeof(MTK_NFC_MSG_T));
           //Write handle function here
           msg->length = 0;//sizeof(s_mtk_nfc_em_als_readerm_opt_req);
           msg->type = MTK_NFC_META_GET_SELIST_REQ;//MTK_NFC_GET_SELIST_REQ;
           //memcpy(((unsigned char*)msg + sizeof(MTK_NFC_MSG_T)), (unsigned char*)&req->cmd.m_nNfc_se_get_list_req, sizeof(s_mtk_nfc_jni_se_get_list_req_t));
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", msg->type, msg->length);   
           META_NFC_SEND_SOCKET(msg);
       }
       break;
       default:
       {
           nfc_cnf.status = META_FAILED;
           WriteDataToPC(&nfc_cnf, sizeof(NFC_CNF), NULL, 0);
           META_LOG("Unknow OP ID,%02x",req->op);
           
           break;
       }
    }
    META_LOG("META_NFC_OP,gBackUpToken,%d",gBackUpToken);
    return;
}


