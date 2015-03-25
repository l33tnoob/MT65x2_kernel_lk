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

#ifndef __FM_RDS_H__
#define __FM_RDS_H__

//FM_RDS_DATA_CRC_FFOST
#define FM_RDS_GDBK_IND_A	 (0x08)	
#define FM_RDS_GDBK_IND_B	 (0x04)	
#define FM_RDS_GDBK_IND_C	 (0x02)	
#define FM_RDS_GDBK_IND_D	 (0x01)	
#define FM_RDS_DCO_FIFO_OFST (0x01E0)
#define	FM_RDS_READ_DELAY	 (0x80)

#define RDS_RX_BLOCK_PER_GROUP (4)
#define RDS_RX_GROUP_SIZE (2*RDS_RX_BLOCK_PER_GROUP)
#define MAX_RDS_RX_GROUP_CNT (12)
#define RDS_RT_MULTI_REV_TH 16

typedef struct rds_packet_t
{
    uint16_t blkA;
    uint16_t blkB;
    uint16_t blkC;
    uint16_t blkD;
    uint16_t crc; //crc checksum
    uint16_t cbc; //correct bit cnt
}rds_packet_t;

typedef struct rds_rx_t
{
    uint16_t sin;
    uint16_t cos;
    rds_packet_t data[MAX_RDS_RX_GROUP_CNT];
}rds_rx_t;

typedef enum rds_ps_state_machine_t
{
    RDS_PS_START = 0,
    RDS_PS_DECISION,
    RDS_PS_GETLEN,
    RDS_PS_DISPLAY,
    RDS_PS_FINISH,
    RDS_PS_MAX
}rds_ps_state_machine_t;

typedef enum rds_rt_state_machine_t
{
    RDS_RT_START = 0,
    RDS_RT_DECISION,
    RDS_RT_GETLEN,
    RDS_RT_DISPLAY,
    RDS_RT_FINISH,
    RDS_RT_MAX
}rds_rt_state_machine_t;


enum
{
    RDS_GRP_VER_A = 0,  //group version A
    RDS_GRP_VER_B
};

typedef enum rds_blk_t
{
    RDS_BLK_A = 0,
    RDS_BLK_B,
    RDS_BLK_C,
    RDS_BLK_D,
    RDS_BLK_MAX
}rds_blk_t;

//For RDS feature, these strcutures also be defined in "fm.h"
typedef struct rds_flag_t
{
   uint8_t TP;
   uint8_t TA;
   uint8_t Music;
   uint8_t Stereo;
   uint8_t Artificial_Head;
   uint8_t Compressed;
   uint8_t Dynamic_PTY;
   uint8_t Text_AB;
   uint32_t flag_status;
}rds_flag_t;

typedef struct rds_ct_t
{
   uint16_t Month;
   uint16_t Day;
   uint16_t Year;
   uint16_t Hour;
   uint16_t Minute;
   uint8_t Local_Time_offset_signbit;
   uint8_t Local_Time_offset_half_hour;
}rds_ct_t;

typedef struct rds_af_t
{
   int16_t AF_Num;
   int16_t AF[2][25];  //100KHz
   uint8_t Addr_Cnt;
   uint8_t isMethod_A;
   uint8_t isAFNum_Get;
}rds_af_t;

typedef struct rds_ps_t
{
   uint8_t PS[4][8];
   uint8_t Addr_Cnt;
}rds_ps_t;

typedef struct rds_rt_t
{
   uint8_t TextData[4][64];
   uint8_t GetLength;
   uint8_t isRTDisplay;
   uint8_t TextLength;
   uint8_t isTypeA;
   uint8_t BufCnt;
   uint16_t Addr_Cnt;
}rds_rt_t;

typedef struct rds_raw_t
{
    int dirty; //indicate if the data changed or not
    int len; //the data len form chip
    uint8_t data[146];
}rds_raw_t;

typedef struct rds_group_cnt_t
{
    unsigned long total;
    unsigned long groupA[16]; //RDS groupA counter
    unsigned long groupB[16]; //RDS groupB counter
}rds_group_cnt_t;

typedef enum rds_group_cnt_op_t
{
    RDS_GROUP_CNT_READ = 0,
    RDS_GROUP_CNT_WRITE,
    RDS_GROUP_CNT_RESET,
    RDS_GROUP_CNT_MAX
}rds_group_cnt_op_t;

typedef struct rds_group_cnt_req_t
{
    int err;
    enum rds_group_cnt_op_t op;
    struct rds_group_cnt_t gc;
}rds_group_cnt_req_t;

typedef struct rds_t
{
   struct rds_ct_t CT;
   struct rds_flag_t RDSFlag;
   uint16_t PI;
   uint8_t Switch_TP;
   uint8_t PTY;
   struct rds_af_t AF_Data;
   struct rds_af_t AFON_Data;
   uint8_t Radio_Page_Code;
   uint16_t Program_Item_Number_Code;
   uint8_t Extend_Country_Code;
   uint16_t Language_Code;
   struct rds_ps_t PS_Data;
   uint8_t PS_ON[8];   
   struct rds_rt_t RT_Data;
   uint16_t event_status; //will use RDSFlag_Struct RDSFlag->flag_status to check which event, is that ok? 
   struct rds_group_cnt_t gc;
}rds_t;


//Need care the following definition.
//valid Rds Flag for notify
typedef enum rds_flag_status_t
{
   RDS_FLAG_IS_TP              = 0x0001, // Program is a traffic program
   RDS_FLAG_IS_TA              = 0x0002, // Program currently broadcasts a traffic ann.
   RDS_FLAG_IS_MUSIC           = 0x0004, // Program currently broadcasts music
   RDS_FLAG_IS_STEREO          = 0x0008, // Program is transmitted in stereo
   RDS_FLAG_IS_ARTIFICIAL_HEAD = 0x0010, // Program is an artificial head recording
   RDS_FLAG_IS_COMPRESSED      = 0x0020, // Program content is compressed
   RDS_FLAG_IS_DYNAMIC_PTY     = 0x0040, // Program type can change 
   RDS_FLAG_TEXT_AB            = 0x0080  // If this flag changes state, a new radio text 					 string begins
} rds_flag_status_t;

typedef enum rds_event_status_t
{
   RDS_EVENT_FLAGS          = 0x0001, // One of the RDS flags has changed state
   RDS_EVENT_PI_CODE        = 0x0002, // The program identification code has changed
   RDS_EVENT_PTY_CODE       = 0x0004, // The program type code has changed
   RDS_EVENT_PROGRAMNAME    = 0x0008, // The program name has changed
   RDS_EVENT_UTCDATETIME    = 0x0010, // A new UTC date/time is available
   RDS_EVENT_LOCDATETIME    = 0x0020, // A new local date/time is available
   RDS_EVENT_LAST_RADIOTEXT = 0x0040, // A radio text string was completed
   RDS_EVENT_AF             = 0x0080, // Current Channel RF signal strength too weak, need do AF switch  
   RDS_EVENT_AF_LIST        = 0x0100, // An alternative frequency list is ready
   RDS_EVENT_AFON_LIST      = 0x0200, // An alternative frequency list is ready
   RDS_EVENT_TAON           = 0x0400,  // Other Network traffic announcement start
   RDS_EVENT_TAON_OFF       = 0x0800, // Other Network traffic announcement finished.
   RDS_EVENT_RDS            = 0x2000, // RDS Interrupt had arrived durint timer period  
   RDS_EVENT_NO_RDS         = 0x4000, // RDS Interrupt not arrived durint timer period  
   RDS_EVENT_RDS_TIMER      = 0x8000 // Timer for RDS Bler Check. ---- BLER  block error rate
} rds_event_status_t;

#endif //__FM_RDS_H__

