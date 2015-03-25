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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   tst_assert_header_file.h
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
 *   Ning.F (MTK08139) 09/11/2008
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * May 19 2009 mtk80306
 * [DUMA00117870] add the version information of database
 * move the version file to custom folder
 *
 * Apr 9 2009 mtk02556
 * [DUMA00114139] [DUMA source code tree refactoring] meta, nvram
 * add to source control recursely
 *
 * Feb 23 2009 mtk80306
 * [DUMA00109277] add meta _battery mode.
 * add nvram dir
 *
 * Dec 17 2008 mbj08139
 * [DUMA00105099] create meta code
 * 
 *
 * Dec 8 2008 mbj08139
 * [DUMA00105099] create meta code
 * 
 *
 * Nov 24 2008 mbj08139
 * [DUMA00105099] create meta code
 * 
 *
 * Oct 29 2008 mbj08139
 * [DUMA00105099] create meta code
 * 
 *
 * 
 *
 *******************************************************************************/

typedef unsigned char           kal_uint8;
typedef char					kal_int8;
typedef char					kal_char;

typedef unsigned short			kal_uint16;
typedef short					kal_int16;
typedef unsigned short			WCHAR;

typedef unsigned int            kal_uint32;
typedef int						kal_int32;

typedef kal_uint8  				kal_bool;
typedef kal_int16  				ARFCN;
typedef kal_int8   				BSIC;
typedef kal_int16  				Power; 
typedef kal_int16  				Gain;
typedef kal_uint8				TimingAdvance;

typedef kal_int32				FS_HANDLE;

typedef enum 
{	
	MOD_NIL = 0,
	MOD_NVRAM,
	MOD_TST,
	MOD_TST_READER,
	MOD_ATCI,
	MOD_MM,
	MOD_CC,
	MOD_CISS,
	MOD_SMS,
	MOD_TIMER,
	MOD_SYSTEM,
	MOD_SYSDEBUG,
	MOD_CUSTOM_BEGIN,
	MOD_FT = 0,
	LAST_MOD_ID = (MOD_SYSDEBUG + 16)
}module_type;

typedef enum 
{
	INVALID_SAP = 0,
	GMMREG_SAP,
	MM_SMS_SAP,
	MM_CC_SAP,
	MM_SS_SAP,
	SME_READER_SAP,
	SME_TIMER_SAP,
	DATA_MPAL_SAP,
	L1_L1_SAP,
	L1_MPAL_SAP,
	MPAL_L1_SAP,
	DRIVER_PS_SAP,
	STACK_TIMER_SAP,
	TST_FT_SAP = 200,
	LAST_SAP_CODE = STACK_TIMER_SAP
}sap_type;

typedef enum 
{
	MSG_ID_INVALID_TYPE = 0,
	MSG_ID_MMCC_PROMPT_REJ,
	MSG_ID_MMCC_PROMPT_RSP,
	MSG_ID_MMCC_REL_REQ,
	MSG_ID_MMCC_EST_REQ,
	MSG_ID_MMCC_REEST_REQ,
	MSG_ID_MMCC_DATA_REQ,
	MSG_ID_MPHC_OPEN_TCH_LOOP_CNF,
	MSG_ID_MPHC_EXTENDED_MEAS_IND,
	MSG_ID_MPHC_BLOCK_QUALITY_IND,
	MSG_ID_MPHC_SERV_IDLE_MEAS_IND,
	MSG_ID_MPHC_SERV_DEDI_MEAS_IND,
	MSG_ID_MPHC_NEIGHBOR_MEAS_IND,
	MSG_ID_MPHC_NEIGHBOR_BSIC_IND,
	MSG_ID_L1TASK_WAKEUP,
	MSG_ID_TST_INJECT_STRING = 6000,
	MSG_ID_TIMER_EXPIRY =  9000,
	MSG_ID_FT = 9500,
	MSG_ID_END  =  (9000+1000)
}msg_type;

typedef enum 
{
	TRACE_FUNC,
	TRACE_STATE, 
	TRACE_INFO, 
	TRACE_WARNING, 
	TRACE_ERROR, 
	TRACE_GROUP_1, 
	TRACE_GROUP_2, 
	TRACE_GROUP_3, 
	TRACE_GROUP_4, 
	TRACE_GROUP_5, 
	TRACE_GROUP_6, 
	TRACE_GROUP_7, 
	TRACE_GROUP_8, 
	TRACE_GROUP_9, 
	TRACE_GROUP_10
}trace_class_enum;

typedef enum 
{
	TST_NULL_COMMAND,
	TST_LOG_ON_OFF,
	TST_SET_PRIM_MOD_FILTER,
	TST_SET_PRIM_SAP_FILTER,
	TST_SET_PS_TRC_FILTER,
	TST_SET_L1_TRC_FILTER,
	TST_INJECT_STRING_TO_MODULE,
	TST_INJECT_AT_CMD,
	TST_REBOOT_TARGET_CMD,
	TST_SET_TRAP_FILTER,
	TST_READ_GLOBAL_VARIABLE,
	TST_WRITE_GLOBAL_VARIABLE,
	TST_EM_MODE_CONFIG,
	TST_SIM_FILE_INFO_REQ,
	TST_SIM_READ_REQ,
	TST_SIM_WRITE_REQ,
	TST_FT_MODE_CONFIG,

	TST_CTI_COMMAND_CMD = 0x38
}tst_command_type;

typedef struct 
{
	kal_uint8			enable;
}tst_em_mode_config_struct;

typedef struct 
{
	kal_uint8 file_idx;
}tst_sim_file_info_req_struct;

typedef struct 
{
	kal_uint8	file_idx;
	kal_uint16	para;
	kal_uint16	length;
}tst_sim_read_req_struct;

typedef struct 
{
	kal_uint8	file_idx;
	kal_uint16	para;
	kal_uint8	data[260];
	kal_uint16	length;
}tst_sim_write_req_struct;
       

typedef struct 
{
   kal_uint32					frame_number;
   kal_uint32					time_stamp;
   module_type				src_mod_id;
   module_type				dest_mod_id;
   sap_type				sap_id;
   msg_type				msg_id;
   kal_uint16					local_len;
   kal_uint16					peer_len;
}tst_log_prim_header_struct;

typedef struct 
{
   kal_uint32					frame_number;
   kal_uint32					time_stamp;
   module_type				src_mod_id;
   module_type				dest_mod_id;
   sap_type				sap_id;
   msg_type				msg_id;
   kal_uint16					local_len;
   kal_uint16					peer_len;
}tst_log_prim_header_struct_2g;

typedef struct 
{
   kal_uint32					frame_number;
   kal_uint32					time_stamp;
   kal_uint16					cfn;
   kal_uint16					sfn;
   module_type				src_mod_id;
   module_type				dest_mod_id;
   sap_type				sap_id;
   msg_type				msg_id;
   kal_uint16					local_len;
   kal_uint16					peer_len;
}tst_log_prim_header_struct_3g;

typedef struct 
{
   kal_uint32					frame_number;
   kal_uint32					time_stamp;
   kal_uint32					msg_index;
   trace_class_enum		trace_class;
   kal_uint16					buf_length;
}tst_index_trace_header_struct;

typedef struct 
{
   kal_uint32					frame_number;
   kal_uint32					time_stamp;
   module_type				module_id;
   kal_uint16					buf_length;
}tst_prompt_trace_header_struct;

typedef struct 
{
   tst_command_type		command_type;
   kal_uint16					cmd_len;
}tst_command_header_struct;


typedef struct 
{
   kal_bool						is_logging;
}tst_log_on_off_struct;

typedef struct 
{
   kal_uint8					mod_filter[(LAST_MOD_ID+7)/8];
}tst_set_prim_mod_filter_struct;

typedef struct 
{	
   kal_uint8					sap_filter[LAST_SAP_CODE+1];
}tst_set_prim_sap_filter_struct;

typedef struct 
{
   kal_uint16					sap_filter[LAST_MOD_ID+1];
}tst_set_ps_trc_filter_struct;
 
typedef struct 
{
   module_type				dest_mod;
   kal_uint8					index;
   kal_uint8					tring[(128)];
}tst_inject_string_to_module_struct;

typedef struct 
{
   kal_uint8					atcmd[(128)];
}tst_inject_at_cmd_struct;

typedef struct 
{
	kal_uint16					sap_id;
	kal_uint8					is_trap;
}tst_set_trap_filter_struct;

typedef struct 
{
    kal_uint32					token;
    kal_char					data[1];
}tst_read_global_variable_struct;

typedef struct 
{
    kal_uint16					size1;
    kal_uint16					size2;
}tst_write_global_variable_struct;

typedef struct  
{
	kal_uint8					ref_count;
	kal_uint16					msg_len;	  
}local_para_struct;

typedef struct 
{
	kal_uint16					token;
	kal_uint16					id;
}WM_FT_H;

typedef struct 
{
   kal_uint16					local_len;
   kal_uint16					peer_len;
}tst_inject_primitive_header_struct;

typedef struct 
{
	kal_uint8					enable;
}tst_ft_mode_config_struct;


typedef struct  {
   kal_uint32		frame_number;
   kal_uint32		time_stamp;
   kal_uint16		buf_length;
}tst_sys_trace_header_struct;

typedef struct  {
   kal_uint32		frame_number;
   kal_uint32		time_stamp;
   kal_uint16		buf_length;
}tst_sys_trace_header_struct_2g;

typedef struct  {
   kal_uint32		frame_number;
   kal_uint32		time_stamp;
   kal_uint16		cfn;
   kal_uint16		sfn;
   kal_uint16		buf_length;
}tst_sys_trace_header_struct_3g;

typedef struct {
	kal_uint8*		pData;
}tst_cti_command_cmd_struct;

typedef struct {
	kal_uint32		buf_length;
}tst_cti_response_header_struct;

typedef struct 
{
	kal_uint16					token;
	kal_uint16					id;
}FT_H;

typedef enum
{
	MOD_CUSTOM1 = MOD_CUSTOM_BEGIN,
	MOD_CUSTOM_END
}custom_module_type;

typedef kal_uint8 tst_null_command_struct;
typedef kal_uint8 tst_set_l1_trc_filter_struct;
typedef kal_uint8 tst_reboot_target_cmd_struct;