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

#ifndef __BGW_CCCI_H__
#define __BGW_CCCI_H__

#include "BGWC.h"
#include <fcntl.h>
#include "wmt_ioctl.h"
//#include <linux/ccci_ipc_task_ID.h>
#include "kal_general_types.h"

#ifdef BGW_CCCI_DEVICE
#undef BGW_CCCI_DEVICE
#endif
/*this macro is useless after KK*/
#define BGW_CCCI_DEVICE "/dev/ccci_ipc_2"

#ifdef MTK_LOCAL_PARA_HDR
#undef MTK_LOCAL_PARA_HDR
#endif

#ifdef MTK_PEER_BUFF_HDR
#undef MTK_PEER_BUFF_HDR
#endif

#ifdef LOCAL_PARA_HDR
#undef LOCAL_PARA_HDR
#endif

#ifdef PEER_BUFF_HDR
#undef PEER_BUFF_HDR
#endif


#define MTK_LOCAL_PARA_HDR \
	kal_uint8  ref_count; \
	kal_uint8  lp_reserved; \
	kal_uint16 msg_len;

#define MTK_PEER_BUFF_HDR \
   kal_uint16 pdu_len; \
   kal_uint8  ref_count; \
   kal_uint8  pb_resvered; \
   kal_uint16 free_header_space; \
   kal_uint16 free_tail_space;

#define LOCAL_PARA_HDR \
   kal_uint8  ref_count; \
   kal_uint8  lp_reserved; \
   kal_uint16 msg_len;

#define PEER_BUFF_HDR \
   kal_uint16 pdu_len; \
   kal_uint8  ref_count; \
   kal_uint8  pb_resvered; \
   kal_uint16 free_header_space; \
   kal_uint16 free_tail_space;



//#define MTK_LOCAL_PARA_HDR mtk_local_para_struct
typedef struct mtk_local_para_struct {
    MTK_LOCAL_PARA_HDR
} mtk_local_para_struct;

typedef struct
{
    MTK_PEER_BUFF_HDR
} mtk_peer_buff_struct;

typedef struct {
    LOCAL_PARA_HDR
} local_para_struct;

typedef struct {
    PEER_BUFF_HDR
} peer_buff_struct;


typedef struct
{
    module_type       src_mod_id;
    module_type       dest_mod_id;
    sap_type          sap_id;
    msg_type          msg_id;
    mtk_local_para_struct *local_para_ptr;
    mtk_peer_buff_struct  *peer_buff_ptr;
} mtk_ilm_struct;


/*
mode = 0 disable MD report data;
mode = 1 enable MD report data to AP
*/
typedef struct
{
	LOCAL_PARA_HDR
	kal_uint8 mode;
}l4c_rf_info_req_struct;

/*
	MD report data to AP
	tx_freq is data
*/

typedef struct
{
	LOCAL_PARA_HDR
	kal_uint16 tx_freq[64];
}l4cps_rf_info_ind_struct;


typedef enum
{
    IPC_MSG_ID_INVALID_TYPE = 0x80000000,
    //------- Include  msg_id file------------------
    #include"msg_id.h"
    //---------------------------------------------
    IPC_MSG_ID_END
}mtk_agps_mmi_ss_msg_type;


/*here we reuse AGPS dest mod definition*/
#ifdef AGPS_MD_MOD_L4C_1
#undef AGPS_MD_MOD_L4C_1
#endif

#ifdef AGPS_MD_MOD_L4C_2
#undef AGPS_MD_MOD_L4C_2
#endif

#ifdef AGPS_MD_MOD_L4C_3
#undef AGPS_MD_MOD_L4C_3
#endif

#ifdef AGPS_MD_MOD_L4C_4
#undef AGPS_MD_MOD_L4C_4
#endif

#ifdef AGPS_MD_MOD_L4C_undef
#undef AGPS_MD_MOD_L4C_undef
#endif


#define AGPS_MD_MOD_L4C_1 0
#define AGPS_MD_MOD_L4C_2 1
#define AGPS_MD_MOD_L4C_3 2
#define AGPS_MD_MOD_L4C_4 3
#define AGPS_MD_MOD_L4C_undef 4



extern int init_ccci();
extern int get_data_from_ccci();
extern int enable_coexist();
extern int disable_coexist();
extern int md_data_process();

#endif

