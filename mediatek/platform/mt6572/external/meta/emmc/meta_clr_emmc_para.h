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
 *   meta_fm.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   FM meta data struct define.
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 07 03 2012 vend_am00076
 * [ALPS00269605] [MP Feature Patch Back]Shared sdcard feature support
 * shared sdcard --meta mode
 *
 * 03 12 2012 vend_am00076
 * [ALPS00251394] [Patch Request]
 * .
 *
 * 03 02 2012 vend_am00076
 * NULL
 * .
 *
 * 12 03 2010 hongcheng.xia
 * [ALPS00136616] [Need Patch] [Volunteer Patch]FM Meta bugs Fix
 * .
 *
 * 11 18 2010 hongcheng.xia
 * [ALPS00135614] [Need Patch] [Volunteer Patch]MT6620 FM Radio code check in
 * .
 *
 * 08 28 2010 chunhui.li
 * [ALPS00123709] [Bluetooth] meta mode check in
 * for FM meta enable

 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef __META_CLR_EMMC_H_
#define __META_CLR_EMMC_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
     FT_EMMC_OP_CLEAR = 0,
     FT_EMMC_OP_FORMAT_TCARD,
     FT_EMMC_OP_CLEAR_WITHOUT_TCARD,
     FT_EMMC_END
}FT_EMMC_OP;

typedef struct {       
    unsigned char   status;   // The operation whether success , 1 means success
} EMMC_CLEAR_CNF;

typedef struct {
    unsigned char status;
}EMMC_CLEAR_WITHOUT_TCARD_CNF;

typedef struct
{       
  unsigned char   status;   // The operation whether success , 1 means success
} EMMC_FORMAT_TCARD_CNF;

typedef union {
  EMMC_CLEAR_CNF     clear_cnf;
  EMMC_FORMAT_TCARD_CNF   form_tcard_cnf;
  EMMC_CLEAR_WITHOUT_TCARD_CNF clear_without_tcard_cnf;
} FT_EMMC_RESULT;

typedef struct {       
    unsigned char   sign;   // No means
} EMMC_CLEAR_REQ;

typedef struct
{
  unsigned char   sign;   // No means
} EMMC_FORMAT_TCARD_REQ;

typedef struct {
    unsigned char   sign;
} EMMC_CLEAR_WITHOUT_TCARD_REQ;

typedef union {
  EMMC_CLEAR_REQ     clear_req;
  EMMC_FORMAT_TCARD_REQ   format_tcard_req;
  EMMC_CLEAR_WITHOUT_TCARD_REQ clear_without_tcard_req;
} FT_EMMC_CMD;

typedef struct {
    FT_H	       header;  
    FT_EMMC_OP	       op;
    FT_EMMC_CMD  cmd;
} FT_EMMC_REQ;

typedef struct {
     FT_H	    header;
     FT_EMMC_OP	    op;
     unsigned char  m_status;   // The data frame state, 0 means normal
     FT_EMMC_RESULT  result;
} FT_EMMC_CNF;

bool META_CLR_EMMC_init();
void META_CLR_EMMC_deinit();
void META_CLR_EMMC_OP(FT_EMMC_REQ *req) ;

#ifdef __cplusplus
};
#endif

#endif

