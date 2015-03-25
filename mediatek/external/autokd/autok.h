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
 *   meta_cpu_para.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Juju Sung (mtk04314) 10/21/2013
 *
 *******************************************************************************/

#ifndef __AUTOK_NODES_H__
#define __AUTOK_NODES_H__

//#include <iostream>
//#include <list>


#ifdef __cplusplus
extern "C"
{
#endif

#define LOGD printf
#define LOGE printf
#define LOGI printf
#define LOGW printf
#define PARAM_COUNT_DEVNODE "/sys/autok/param_count"
#define STAGE2_DEVNODE "/sys/autok/stage2"
#define STAGE1_DEVNODE "/sys/autok/stage1"
#define DEBUG_DEVNODE "/sys/autok/debug"
#define READY_DEVNODE "/sys/autok/ready"
#define RESULT_FILE_PREFIX "autok"
#define AUTOK_NVRAM_PATH   "/data/nvram/APCFG/APRDCL/SDIO"
#define AUTOK_RES_PATH   "/data"  // with format "autok_[id]_[vol]"

#define BUF_LEN     1024
extern unsigned int g_autok_vcore[];
extern int VCORE_NO;

typedef struct
{
    unsigned int sel;
}S_AUTOK_DATA;

typedef union
{
    unsigned int version;
    unsigned int freq;
    S_AUTOK_DATA data;
}U_AUTOK_INTERFACE_DATA;

struct autok_predata{
    char vol_count;
    char param_count;
    unsigned int *vol_list;
    U_AUTOK_INTERFACE_DATA **ai_data;
};

struct host_progress{
    int host_id;
    int is_done;  
};
int get_node_data(char *filename, char **data, int *len);
int set_node_data(char *filename, char *data, int len);

// PARAM_COUNT
int get_param_count();
// Debug
int get_debug();
int set_debug(int debug);
// Stage2-id
struct autok_predata get_stage2(int id);
int set_stage2(int id, struct autok_predata *predata);
// Stage1 - DONE - id 
int get_stage1_done(int id);
int set_stage1_done(int id, int data);
// Stage1 - PARAMS - id
struct autok_predata get_stage1_params(int id);
int set_stage1_params(int id, struct autok_predata *predata);
// Stage1 - VOLTAGE - id
int get_stage1_voltage(int id);
int set_stage1_voltage(int id, int data);

int set_ready(int id);
//std::list<struct host_progress*> get_ready();

// Format utility
struct autok_predata get_param(char *filename);
int get_param_data_from_buf(struct autok_predata *test_predata, char *buf);
int pack_param(struct autok_predata *test_predata, unsigned int *vol_list, int vol_count, unsigned int *param_list, int param_count);
int serilize_predata(struct autok_predata *predata, char **buf);

// Data transfer 
int from_dev_to_data(char *from, char *to);
int data_copy(char *from, char *to);
int write_to_file(char *filename, char *data_buf, int length);

// Release function
void release_predata(struct autok_predata *predata);

// NVRAM Relate
//int write_file_to_nvram(char *filename, int id);
int write_dev_to_nvram(char *filename, int id);
int write_nvram(unsigned char* write_buf, int length, int id, int file_idx, int file_count);
int read_from_nvram(int id, int voltage, unsigned char **file_data, int *length);
int is_nvram_data_exist(int id, int voltage);
//int sdio_read_nvram(unsigned char *ucNvRamData);
int init_autok_nvram();
int get_nvram_param_count(int id);
int close_nvram();

int autok_flow();

#ifdef __cplusplus
}
#endif

#endif
