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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   Meta_GPIO_Para.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   MTK80465(Changlei Gao)

 *******************************************************************************/


#ifndef __META_GSENSOR_PARA_H__
#define __META_GSENSOR_PARA_H__

#include "FT_Public.h"

typedef enum {
    GS_OP_CALI,
    GS_OP_READ_RAW,
    GS_OP_WRITE_NVRAM,
    GS_OP_READ_NVRAM,
    GS_OP_END
} GS_OP;

typedef struct {
    int num;
    int delay;
    int tolerance;
} GS_CMD_CALI;

typedef struct {  /*16bits -> 1g*/
    int x;
    int y;
    int z; 
} GS_ACK_CALI;

typedef struct {
    int dummy;
} GS_CMD_READ_RAW;

typedef struct {  /*16bits -> 1g*/
    int x;
    int y;
    int z;
} GS_ACK_READ_RAW;

typedef struct {
    int x;
    int y;
    int z;    
} GS_CMD_WRITE_NVRAM;

typedef struct {
    int   dummy;
} GS_ACK_WRITE_NVRAM;

typedef struct {
    int   dummy;
} GS_CMD_READ_NVRAM;

typedef struct {
    int x;
    int y;
    int z;    
} GS_ACK_READ_NVRAM;


typedef union {
    GS_ACK_CALI        cali;
    GS_ACK_READ_RAW    readraw;
    GS_ACK_WRITE_NVRAM writenv;
    GS_ACK_READ_NVRAM  readnv; 
} GS_ACK;

typedef union {
    GS_CMD_CALI        cali;
    GS_CMD_READ_RAW    readraw;    
    GS_CMD_WRITE_NVRAM writenv;
    GS_CMD_READ_NVRAM  readnv;
} GS_CMD;

typedef struct {
    FT_H header;
    GS_OP op;
    GS_CMD cmd; 		
} GS_REQ;

typedef struct {
    FT_H header;
    GS_OP op;
    GS_ACK ack;	
    int gs_err;     /*gs->FT*/
    int status;     /*the status of the operation*/
} GS_CNF;

bool Meta_GSensor_Open(void);
void Meta_GSensor_OP(GS_REQ *req, char *peer_buff, unsigned short peer_len);
bool Meta_GSensor_Close(void);


#endif


