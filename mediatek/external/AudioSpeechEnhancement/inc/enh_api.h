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

#ifndef _ENH_API_H
#define _ENH_API_H

typedef unsigned short uWord16;
typedef signed short Word16;
typedef signed long Word32;
typedef unsigned long uWord32;

typedef struct
{

    uWord32 enhance_pars[28];
    //  uWord32 error_flag;
    Word32 App_table;
    Word32 Fea_Cfg_table;
    Word32 MIC_DG;
    Word32 sample_rate;
    Word32 frame_rate;
    Word32 MMI_ctrl;
    Word32 RCV_DG;      // for VoIP, 0xE3D, downlink PGA
    Word16 DMNR_cal_data[76];
    Word16 Compen_filter[270];
    //  Word16 ne_out[960];
    //    Word16 fe_out[960];
    Word16 PCM_buffer[1920];
    Word16 EPL_buffer[4160];
    Word32 Device_mode;
    Word32 MMI_MIC_GAIN;
    Word32 Near_end_vad;
    Word32 *SCH_mem; // caster to (SCH_mem_struct*) in every alloc function
} SPH_ENH_ctrl_struct;


Word16  ENH_API_Rst(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
Word32  ENH_API_Get_Memory(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
Word16  ENH_API_Alloc(SPH_ENH_ctrl_struct *Sph_Enh_ctrl, Word32 *mem_ptr);
void ENH_API_Process(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
Word16  ENH_API_Free(SPH_ENH_ctrl_struct *Sph_Enh_ctrl);
#endif