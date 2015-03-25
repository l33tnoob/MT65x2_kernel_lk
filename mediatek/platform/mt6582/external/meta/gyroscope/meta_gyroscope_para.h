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
 *   Meta_gyroscope_Para.h
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
 *   MTK80198(Chunlei Wang)

 *******************************************************************************/


#ifndef __META_GYROSCOPE_PARA_H__
#define __META_GYROSCOPE_PARA_H__

#include "FT_Public.h"

typedef enum {
    GYRO_OP_CALI,
    GYRO_OP_READ_RAW,
    GYRO_OP_WRITE_NVRAM,
    GYRO_OP_READ_NVRAM,
    GYRO_OP_END
} GYRO_OP;

typedef struct {
    int num;
    int delay;
    int tolerance;
} GYRO_CMD_CALI;

typedef struct {  /*16bits -> 1g*/
    int x;
    int y;
    int z; 
} GYRO_ACK_CALI;

typedef struct {
    int dummy;
} GYRO_CMD_READ_RAW;

typedef struct {  /*16bits -> 1g*/
    int x;
    int y;
    int z;
} GYRO_ACK_READ_RAW;

typedef struct {
    int x;
    int y;
    int z;    
} GYRO_CMD_WRITE_NVRAM;

typedef struct {
    int   dummy;
} GYRO_ACK_WRITE_NVRAM;

typedef struct {
    int   dummy;
} GYRO_CMD_READ_NVRAM;

typedef struct {
    int x;
    int y;
    int z;    
} GYRO_ACK_READ_NVRAM;


typedef union {
    GYRO_ACK_CALI        cali;
    GYRO_ACK_READ_RAW    readraw;
    GYRO_ACK_WRITE_NVRAM writenv;
    GYRO_ACK_READ_NVRAM  readnv; 
} GYRO_ACK;

typedef union {
    GYRO_CMD_CALI        cali;
    GYRO_CMD_READ_RAW    readraw;    
    GYRO_CMD_WRITE_NVRAM writenv;
    GYRO_CMD_READ_NVRAM  readnv;
} GYRO_CMD;

typedef struct {
    FT_H header;
    GYRO_OP op;
    GYRO_CMD cmd; 		
} GYRO_REQ;

typedef struct {
    FT_H header;
    GYRO_OP op;
    GYRO_ACK ack;	
    int gyro_err;     /*gs->FT*/
    int status;     /*the status of the operation*/
} GYRO_CNF;

bool Meta_Gyroscope_Open(void);
void Meta_Gyroscope_OP(GYRO_REQ *req, char *peer_buff, unsigned short peer_len);
bool Meta_Gyroscope_Close(void);


#endif


