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

#ifdef __cplusplus
extern "C" {
#endif

#include "kal_release.h"
#include "unistd.h"
#include <stdio.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "matvctrl.h"

#define TAG         "[MATV] "

enum {
    PLT_STR = 0,
    CFO,
    DRO_CVBS_SNR,
    RF_Gain_Idx,
    BB_Gain_Idx,
    AGC_Status,
    TVD_LOCK,
    TVD_NrLvl,
    TVD_BurstLock,  
    MAX_INFO_NUM,
};

enum
{
	//export to standard enum (mandatory)
	EXPORT_SIG_RSSI=0,
	EXPORT_SIG_SNR,
	EXPORT_SIG_STRENGTH,
	EXPORT_TVD_MaxFrameRate,
	EXPORT_AUD_OutputMode,
	EXPORT_AUD_SWMute,
	//local enum (optional)
	MTK_CMetric=100,
	MTK_CFO_DEV,
	MTK_OVM_IDX,
	MTK_PLT_STR,
	MTK_CR_Status,
	MTK_CFO,
	MTK_CPO,
	MTK_Avg_CFO,
	MTK_Aud_CFO,
	//MTK_DRO,
	MTK_DRO_Gain,
	MTK_DRO_CVBS_SNR,
	//MTK_POA,
	MTK_POA_Gain,
	//MTK_VOP,
	MTK_VOP_CPO,
	MTK_BaseBand_Chara,
	MTK_DC_Offset_I,
	MTK_DC_Offset_Q,
	MTK_IQGain,
	MTK_Avg_COR,
	MTK_Avg_POW,
	MTK_IQ_Phase,
	MTK_IQ_Cos,
	MTK_IQ_Sin,
	//MTK_MAGC,
	MTK_RF_Gain_Idx,
	MTK_BB_Gain_Idx,
	MTK_BB_Gain_Idx_Dbg,
	MTK_Tuner_BB_Gain_Idx,
	MTK_AGC_Status,
	MTK_IB_Peak_AVG,
	MTK_IB_Peak,
	MTK_IB_AVG,
	MTK_IB_Converged,
	MTK_WB_Peak_AVG,
	MTK_WB_Peak,
	MTK_WB_AVG,
	MTK_WB_Converged,
	MTK_Mobile_Detector,
	//MTK_FM_Indicator,
	MTK_FM_WB_Power,
	MTK_FM_IB_Power,
	MTK_FM_WB_IB_PowRatio,
	MTK_FM_ACI_Level,
	MTK_TVD_Indicator,
	MTK_TVD_PhaseError,
	MTK_TVD_VSync_loss,
	MTK_TVD_HSync_loss,
	MTK_TVD_VPress,
	MTK_TVD_VLock,
	MTK_TVD_HLock,
	MTK_TVD_BLock,
	MTK_TVD_WBar,
	MTK_TVD_V_Position,
	MTK_EX_INFO_SIZE,
	//tvd part
	MTK_TVD_ColorSystem,
	MTK_TVD_Lock,
	MTK_TVD_NrLvl,
	MTK_TVD_BurstLock,
};

int tvscan_finish=0;
int tvscan_progress=0;

void atv_autoscan_progress_cb(void* cb_param, kal_uint8 precent,kal_uint8 ch,kal_uint8 chnum)
{	
    matv_chscan_state scan_state;
    matv_chscan_query(&scan_state); 
    LOGD(TAG "CB.autoscan_progress: %d%% ,update CH-%02d(%c)\n",      
            precent,scan_state.ch_latest_updated,       
            scan_state.updated_entry.flag?'O':'X'       
            );
    tvscan_progress = precent;
}
void atv_fullscan_progress_cb(void* cb_param, kal_uint8 precent,kal_uint32 freq,kal_uint32 freq_start,kal_uint32 freq_end){  
    LOGD(TAG "CB.fullscan_progress: %d%%\n",precent);
    tvscan_progress = precent;
}


void atv_scanfinish_cb(void* cb_param, kal_uint8 chnum){    
    LOGD(TAG "CB.scanfinish: chnum:%d\n",chnum);  
    tvscan_finish=1;    
}

void atv_audioformat_cb(void* cb_param, kal_uint32 format){    
    LOGD(TAG "CB.audioformat: %08x\n",format);
}

int matv_ts_init()
{    
    int ret;  
    tvscan_finish = 0;
    ret = matv_init();    
    matv_register_callback(0,       
                           atv_autoscan_progress_cb,       
                           atv_fullscan_progress_cb,       
                           atv_scanfinish_cb,      
                           atv_audioformat_cb);        

    return (ret);
}

int matv_ts_scan(unsigned char country)
{
    matv_ch_entry  ch_ent;
    int ch_candidate=0;
    int i;
    tvscan_finish = 0;
    tvscan_progress = 0;
    matv_set_country(country/*TV_TAIWAN*/);
    LOGD(TAG "Auto Scan!\n");
    matv_chscan(MATV_QUICKSCAN/*MATV_AUTOSCAN*/); 
    return (0);
}

int matv_ts_get_chttable(int index, matv_ch_entry* p_ch_ent)
{
    int ret;
    ret = matv_get_chtable(index,p_ch_ent);
    return ret;
}

int matv_ts_set_chttable(int index, matv_ch_entry *p_ch_ent)
{
    int ret;
    ret = matv_set_chtable(index,p_ch_ent);
    return ret;
}


int matv_ts_change_channel(int index)
{
    matv_change_channel(index);
    return (0);
}

int matv_ts_get_info(int info_index)
{
    int value = 0;
    switch (info_index)
    { 
        case PLT_STR:
            value = matv_get_chipdep(MTK_PLT_STR);
            break;
        case CFO:
            value = matv_get_chipdep(MTK_CFO);
            break;
        case DRO_CVBS_SNR:
            value = matv_get_chipdep(MTK_DRO_CVBS_SNR);
            break;
        case RF_Gain_Idx:
            value = matv_get_chipdep(MTK_RF_Gain_Idx);
            break;
        case BB_Gain_Idx:
            value = matv_get_chipdep(MTK_BB_Gain_Idx);
            break;
        case AGC_Status:
            value = matv_get_chipdep(MTK_AGC_Status);
            break;
        case TVD_LOCK:
            value = matv_get_chipdep(MTK_TVD_Lock);
            break;
        case TVD_NrLvl:
            value = matv_get_chipdep(MTK_TVD_NrLvl);
            break;
        case TVD_BurstLock:
            value = matv_get_chipdep(MTK_TVD_BurstLock);
            break;
        default:
            break;
    };
    
    return (value);
}

int matv_ts_shutdown()
{
    LOGD(TAG "shut down\n");
    tvscan_finish = 0;
    tvscan_progress = 0;
    matv_shutdown();
    return (0);
}

unsigned char matv_ts_get_country_id(const char *ptr)
{
    unsigned char country = 0;

    if(!strcmp(ptr,"TV_AFGHANISTAN"))
        country = TV_AFGHANISTAN;
    else if (!strcmp(ptr,"TV_ARGENTINA"))
        country = TV_ARGENTINA;
    else if (!strcmp(ptr,"TV_AUSTRALIA"))
        country = TV_AUSTRALIA;
    else if (!strcmp(ptr,"TV_BRAZIL"))
        country = TV_BRAZIL;
    else if (!strcmp(ptr,"TV_BURMA"))
        country = TV_BURMA;
    else if (!strcmp(ptr,"TV_CAMBODIA"))
        country = TV_CAMBODIA;
    else if (!strcmp(ptr,"TV_CANADA"))
        country = TV_CANADA;
    else if (!strcmp(ptr,"TV_CHILE"))
        country = TV_CHILE;
    else if (!strcmp(ptr,"TV_CHINA"))
        country = TV_CHINA;
    else if (!strcmp(ptr,"TV_CHINA_HONGKONG"))
        country = TV_CHINA_HONGKONG;
    else if (!strcmp(ptr,"TV_CHINA_SHENZHEN"))
        country = TV_CHINA_SHENZHEN;
    else if (!strcmp(ptr,"TV_EUROPE_EASTERN"))
        country = TV_EUROPE_EASTERN;
    else if (!strcmp(ptr,"TV_EUROPE_WESTERN"))
        country = TV_EUROPE_WESTERN;
    else if (!strcmp(ptr,"TV_FRANCE"))
        country = TV_FRANCE;
    else if (!strcmp(ptr,"TV_FRENCH_COLONIE"))
        country = TV_FRENCH_COLONIE;
    else if (!strcmp(ptr,"TV_INDIA"))
        country = TV_INDIA;
    else if (!strcmp(ptr,"TV_INDONESIA"))
        country = TV_INDONESIA;
    else if (!strcmp(ptr,"TV_IRAN"))
        country = TV_IRAN;
    else if (!strcmp(ptr,"TV_ITALY"))
        country = TV_ITALY;
    else if (!strcmp(ptr,"TV_JAPAN"))
        country = TV_JAPAN;
    else if (!strcmp(ptr,"TV_KOREA"))
        country = TV_KOREA;
    else if (!strcmp(ptr,"TV_LAOS"))
        country = TV_LAOS;
    else if (!strcmp(ptr,"TV_MALAYSIA"))
        country = TV_MALAYSIA;
    else if (!strcmp(ptr,"TV_MEXICO"))
        country = TV_MEXICO;
    else if (!strcmp(ptr,"TV_NEWZEALAND"))
        country = TV_NEWZEALAND;
    else if (!strcmp(ptr,"TV_PAKISTAN"))
        country = TV_PAKISTAN;
    else if (!strcmp(ptr,"TV_PARAGUAY"))
        country = TV_PARAGUAY;
    else if (!strcmp(ptr,"TV_PHILIPPINES"))
        country = TV_PHILIPPINES;
    else if (!strcmp(ptr,"TV_PORTUGAL"))
        country = TV_PORTUGAL;
    else if (!strcmp(ptr,"TV_RUSSIA"))
        country = TV_RUSSIA;
    else if (!strcmp(ptr,"TV_SINGAPORE"))
        country = TV_SINGAPORE;
    else if (!strcmp(ptr,"TV_SOUTHAFRICA"))
        country = TV_SOUTHAFRICA;
    else if (!strcmp(ptr,"TV_SPAIN"))
        country = TV_SPAIN;
    else if (!strcmp(ptr,"TV_TAIWAN"))
        country = TV_TAIWAN;
    else if (!strcmp(ptr,"TV_THAILAND"))
        country = TV_THAILAND;
    else if (!strcmp(ptr,"TV_TURKEY"))
        country = TV_TURKEY;
    else if (!strcmp(ptr,"TV_UNITED_ARAB_EMIRATES"))
        country = TV_UNITED_ARAB_EMIRATES;
    else if (!strcmp(ptr,"TV_UNITED_KINGDOM"))
        country = TV_UNITED_KINGDOM;
    else if (!strcmp(ptr,"TV_USA"))
        country = TV_USA;
    else if (!strcmp(ptr,"TV_URUGUAY"))
        country = TV_URUGUAY;
    else if (!strcmp(ptr,"TV_VENEZUELA"))
        country = TV_VENEZUELA;
    else if (!strcmp(ptr,"TV_VIETNAM"))
        country = TV_VIETNAM;
    else if (!strcmp(ptr,"TV_IRELAND"))
        country = TV_IRELAND;
    else if (!strcmp(ptr,"TV_MOROCCO"))
        country = TV_MOROCCO;
    else if (!strcmp(ptr,"TV_BANGLADESH"))
        country = TV_BANGLADESH;
    
    return country;
}

#ifdef __cplusplus
};
#endif

