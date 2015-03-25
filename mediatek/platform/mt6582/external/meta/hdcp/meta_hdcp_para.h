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

#ifndef __META_HDCP_H_
#define __META_HDCP_H_

#include "FT_Public.h"


#define HDCP_CNF_OK     0
#define HDCP_CNF_FAIL   1

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * OP 
 */
typedef enum {
    HDCP_OP_INSTALL = 0
   ,HDCP_END
} HDCP_OP;


/*
 * Install REQ & CNF structure 
 */
typedef struct {
    unsigned char data[572];
    unsigned char cek_data[16];
    unsigned int data_len;
    unsigned int cek_len;
} HDCP_INSTALL_REQ_T;

typedef struct {
    unsigned int install_result;
} HDCP_INSTALL_CNF_T;


/* 
 * HDCP REQ & CNF union 
 */
typedef union {
    HDCP_INSTALL_REQ_T hdcp_install_req;
} META_HDCP_CMD_U;

typedef union {
    HDCP_INSTALL_CNF_T hdcp_install_cnf;
} META_HDCP_CNF_U;


/* 
 * HDCP REQ & CNF 
 */
typedef struct {
    FT_H            header;  //module do not need care it
    HDCP_OP         op;
    META_HDCP_CMD_U cmd;
} HDCP_REQ;

typedef struct {
    FT_H            header;  //module do not need care it
    HDCP_OP         op;
    META_HDCP_CNF_U result;   
    unsigned int    status;
} HDCP_CNF;



bool META_HDCP_init();
void META_HDCP_deinit();
void META_HDCP_OP(HDCP_REQ *req, char *peer_buff, unsigned short peer_len) ;

#ifdef __cplusplus
};
#endif

#endif
