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

/* //hardware/ril/reference-ril/ril_nw.h
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/
#ifndef RIL_NW_H 
#define RIL_NW_H 1
            
extern void requestSignalStrength(void * data, size_t datalen, RIL_Token t);
extern void requestRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void requestGprsRegistrationState(void * data, size_t datalen, RIL_Token t);
extern void requestOperator(void * data, size_t datalen, RIL_Token t);
extern void requestRadioPower(void * data, size_t datalen, RIL_Token t);
extern void requestGetImei(void * data, size_t datalen, RIL_Token t);
extern void requestGetImeisv(void * data, size_t datalen, RIL_Token t);
extern void requestQueryNetworkSelectionMode(void * data, size_t datalen, RIL_Token t);
extern void requestSetNetworkSelectionAutomatic(void * data, size_t datalen, RIL_Token t);
extern void requestSetNetworkSelectionManual(void * data, size_t datalen, RIL_Token t);
extern void requestQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t);
extern void requestAbortQueryAvailableNetworks(void * data, size_t datalen, RIL_Token t);
extern void requestBasebandVersion(void * data, size_t datalen, RIL_Token t);
extern void requestSetBandMode(void * data, size_t datalen, RIL_Token t);
extern void requestQueryAvailableBandMode(void * data, size_t datalen, RIL_Token t);
extern void requestSetPreferredNetworkType(void * data, size_t datalen, RIL_Token t);
extern void requestGetPreferredNetworkType(void * data, size_t datalen, RIL_Token t);
extern void requestGetNeighboringCellIds(void * data, size_t datalen, RIL_Token t);
extern void requestSetLocationUpdates(void * data, size_t datalen, RIL_Token t);
extern void requestGetPacketSwitchBearer(RILId rid);
extern void requestSN(RILId rid);
extern void onRadioState(char* urc, RILId rid);
extern void onNetworkStateChanged(char *urc, RILId rid);
extern void onNitzTimeReceived(char *urc, RILId rid);
extern void onRestrictedStateChanged(RILId rid);
extern void onMMRRStatusChanged(char *urc, RILId rid);

/* Add-BY-JUNGO-20101008-CTZV SUPPORT */
extern void onNitzTzReceived(char *urc, RILId rild);
extern void updateNitzOperInfo(RILId rid);


extern int isRadioOn(RILId rid);
extern int queryRadioState(RILId rid);

extern int rilNwMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilNwUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

//extern int getSingnalStrength(char *line, int *rssi, int *ber, int *rssi_qdbm, int *rscp_qdbm, int *ecn0_qdbm);
extern int getSingnalStrength(char *line, int *response);

/* MTK proprietary start */

// GCG switcher feature
extern void requestSetGCFSwitch(RILId rid);
// GCG switcher feature


extern void requestSetGprsConnectType(void * data, size_t datalen, RIL_Token t);
extern void requestSetGprsTransferType(void * data, size_t datalen, RIL_Token t);
extern void requestMobileRevisionAndIMEI(void * data, size_t datalen, RIL_Token t);
extern void requestSetNetworkSelectionManualWithAct(void * data, size_t datalen, RIL_Token t);
extern void requestSetSimRecoveryOn(void * data, size_t datalen, RIL_Token t);
extern void requestGetSimRecoveryOn(void * data, size_t datalen, RIL_Token t);
extern void requestSetTRM (void * data, size_t datalen, RIL_Token t);
extern void requestGetCalibrationData(void * data, size_t datalen, RIL_Token t);
extern void requestVoiceRadioTech(void * data, size_t datalen, RIL_Token t);
extern void requestGetGcfMode(RILId rid);
extern void requestRadioPowerOff(void * data, size_t datalen, RIL_Token t);
extern void requestRadioPowerOn(void * data, size_t datalen, RIL_Token t);

extern void requestGetCellInfoList(void * data, size_t datalen, RIL_Token t);
extern void requestSetCellInfoListRate(void * data, size_t datalen, RIL_Token t);
extern void requestGetFemtoCellList(void * data, size_t datalen, RIL_Token t);
extern void requestAbortFemtoCellList(void * data, size_t datalen, RIL_Token t);
extern void requestSelectFemtoCell(void * data, size_t datalen, RIL_Token t);
extern void onFemtoCellInfo(char *urc, const RILId rid);
extern void onCellInfoList(char *urc, const RILId rid);

extern void onSignalStrenth(char* urc, const RILId rid);
extern void onNeighboringCellInfo(char* urc, const RILId rid);
extern void onNetworkInfo(char* urc, const RILId rid);
extern void onSimInsertChanged(const char *s,RILId rid);
extern void onInvalidSimInfo(char *urc,RILId rid); //ALPS00248788
extern void onACMT(char *urc,RILId rid);
extern void requestSetRegSuspendEnabled(void * data, size_t datalen, RIL_Token t);
extern void requestResumeRegistration(void * data, size_t datalen, RIL_Token t);
extern void onPLMNListChanged(char *urc, const RILId rid);
extern void onRegistrationSuspended(char *urc, const RILId rid);

extern int sim_inserted_status;
extern void requestSimReset(RILId rid);
extern void requestSimInsertStatus(RILId rid);
extern void requestSendOplmn(void *data, size_t datalen, RIL_Token t);
extern void requestGetOplmnVersion(void *data, size_t datalen, RIL_Token t);
#ifdef MTK_GEMINI
void requestRadioMode(void * data, size_t datalen, RIL_Token t);
#endif
/* MTK proprietary end */

/* RIL Network Structure */

typedef struct
{
    unsigned short arfcn;
    unsigned char bsic;
    unsigned char rxlev;
} gas_nbr_cell_meas_struct;

typedef struct
{
    unsigned short mcc;
    unsigned short mnc;
    unsigned short lac;
    unsigned short ci;
} global_cell_id_struct;

typedef struct
{
    unsigned char nbr_meas_num;
    gas_nbr_cell_meas_struct nbr_cells[15];
} gas_nbr_meas_struct;

typedef struct
{
    global_cell_id_struct gci;
    unsigned char nbr_meas_rslt_index;
} gas_cell_info_struct;

typedef struct
{
    gas_cell_info_struct serv_info;
    unsigned char ta;
    unsigned char ordered_tx_pwr_lev;
    unsigned char nbr_cell_num;
    gas_cell_info_struct nbr_cell_info[6];
    gas_nbr_meas_struct nbr_meas_rslt;
} gas_nbr_cell_info_struct;

typedef struct
{
    unsigned char need_revise; // Lexel: this fake, can remove if have other element
} uas_nbr_cell_info_struct; //Lexel: Not define uas_nbr_cell_info_struct yet

typedef union
{
    gas_nbr_cell_info_struct gas_nbr_cell_info;
    uas_nbr_cell_info_struct uas_nbr_cell_info;
} ps_nbr_cell_info_union_type;

/* RIL Network Enumeration */

typedef enum
{
    GSM_BAND_900    = 0x02,
    GSM_BAND_1800   = 0x08,
    GSM_BAND_1900   = 0x10,
    GSM_BAND_850    = 0x80
} GSM_BAND_ENUM;

typedef enum
{
    UMTS_BAND_I     = 0x0001,
    UMTS_BAND_II    = 0x0002,
    UMTS_BAND_III   = 0x0004,
    UMTS_BAND_IV    = 0x0008,
    UMTS_BAND_V     = 0x0010,
    UMTS_BAND_VI    = 0x0020,
    UMTS_BAND_VII   = 0x0040,
    UMTS_BAND_VIII  = 0x0080,
    UMTS_BAND_IX    = 0x0100,
    UMTS_BAND_X     = 0x0200
} UMTS_BAND_ENUM;

typedef enum
{
    BM_AUTO_MODE,
    BM_EURO_MODE,
    BM_US_MODE,
    BM_JPN_MODE,
    BM_AUS_MODE,
    BM_AUS2_MODE,
    BM_CELLULAR_MODE,
    BM_PCS_MODE,
    BM_CLASS_3,
    BM_CLASS_4,
    BM_CLASS_5,
    BM_CLASS_6,
    BM_CLASS_7,
    BM_CLASS_8,
    BM_CLASS_9,
    BM_CLASS_10,
    BM_CLASS_11,
    BM_CLASS_15,
    BM_CLASS_16
} BAND_MODE;

typedef enum
{
    NT_WCDMA_PREFERRED_TYPE,   //   WCDMA preferred (auto mode)
    NT_GSM_TYPE,    //   GSM only
    NT_WCDMA_TYPE,  //   WCDMA only
    NT_AUTO_TYPE  //    AUTO
    //MR2 merge
    ,NT_LTE_GSM_WCDMA_TYPE = 9
    ,NT_LTE_ONLY_TYPE = 11
    ,NT_LTE_WCDMA_TYPE = 12

    //MTK-START: add for LTE
    ,NT_MODE_MTK_BASE = 30
    // GSM/WCDMA, LTE (for MMDSDC "3G or 2G Preferred" item)
    ,NT_GSM_WCDMA_LTE = (NT_MODE_MTK_BASE+1)    
    ,NT_GSM_WCDMA_LTE_MMDC = (NT_MODE_MTK_BASE+2)
    // for MMDSDC "2/3/4G(auto)" item
    ,NT_LTE_GSM_WCDMA_MMDC = (NT_MODE_MTK_BASE+3)
    // for "2/4G" item (only for EM)
    ,NT_LTE_GSM = (NT_MODE_MTK_BASE+4)
    ,NT_LTE_GSM_MMDC = (NT_MODE_MTK_BASE+5)
    //MTK-END: add for LTE

    //ALPS00282643
    ,NT_RAT_ONLY_TYPE = 100
    ,NT_NO_PREFERRED_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE + NT_WCDMA_PREFERRED_TYPE)    
    ,NT_GSM_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE + NT_GSM_TYPE)
    ,NT_WCDMA_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE + NT_WCDMA_TYPE)
    ,NT_AUTO_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE + NT_AUTO_TYPE)
    //MTK-START: add for LTE
    ,NT_LTE_GSM_WCDMA_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE + NT_LTE_GSM_WCDMA_TYPE)    
    ,NT_LTE_ONLY_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE + NT_LTE_ONLY_TYPE)
    ,NT_LTE_WCDMA_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE+NT_LTE_WCDMA_TYPE)    
    ,NT_GSM_WCDMA_LTE_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE+NT_GSM_WCDMA_LTE)
    ,NT_GSM_WCDMA_LTE_MMDC_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE+NT_GSM_WCDMA_LTE_MMDC)
    ,NT_LTE_GSM_WCDMA_MMDC_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE+NT_LTE_GSM_WCDMA_MMDC)
    ,NT_LTE_GSM_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE+NT_LTE_GSM)
    ,NT_LTE_GSM_MMDC_RAT_ONLY_TYPE = (NT_RAT_ONLY_TYPE+NT_LTE_GSM_MMDC)
    //MTK-START: add for LTE
} NETWORK_TYPE;

typedef enum
{
    RAT_NONE = 0,
    RAT_GSM,
    RAT_UMTS,
    RAT_GSM_UMTS
} rat_enum;

/* RIL Network Constant */

#define NW_CHANNEL_CTX getRILChannelCtxFromToken(t)

#define RIL_NW_ALL_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_CS_NORMAL | \
    RIL_RESTRICTED_STATE_CS_EMERGENCY | \
    RIL_RESTRICTED_STATE_PS_ALL)

#define RIL_NW_ALL_CS_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_CS_NORMAL | \
    RIL_RESTRICTED_STATE_CS_EMERGENCY)

#define RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS (\
    RIL_RESTRICTED_STATE_CS_ALL | \
    RIL_RESTRICTED_STATE_PS_ALL)

#define OPER_FILE_PROPERTY  "rild.nw.operlist"
#define MAX_OPER_NAME_LENGTH    50
#define PROPERTY_GSM_BASEBAND_CAPABILITY    "gsm.baseband.capability"
#define PROPERTY_GSM_BASEBAND_CAPABILITY2   "gsm.baseband.capability2"
#define PROPERTY_GSM_BASEBAND_CAPABILITY3   "gsm.baseband.capability3"
#define PROPERTY_GSM_BASEBAND_CAPABILITY4   "gsm.baseband.capability4"

//ALPS00269882 
#define PROPERTY_GSM_CURRENT_ENBR_RAT    "gsm.enbr.rat"

// GCG switcher feature
#define PROPERTY_GSM_GCF_TEST_MODE  "gsm.gcf.testmode"
// GCG switcher feature
#define PROPERTY_SERIAL_NUMBER "gsm.serial"

#define PROPERTY_RIL_TESTSIM "gsm.sim.ril.testsim"
#define PROPERTY_RIL_TESTSIM_2 "gsm.sim.ril.testsim.2"


#ifdef  MTK_GEMINI
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
#define RIL_NW_NUM              4
#define RIL_NW_INIT_MUTEX       {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,\
	                             PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER}
#define RIL_NW_INIT_INT         {0, 0, 0, 0}
#define RIL_NW_INIT_STRING      {{0},{0},{0},{0}}
#define RIL_NW_INIT_STRUCT      {NULL, NULL, NULL, NULL}
#define RIL_NW_INIT_STATE   {RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS,\
                             RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS}
#elif (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
#define RIL_NW_NUM              3
#define RIL_NW_INIT_MUTEX       {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,\
	                             PTHREAD_MUTEX_INITIALIZER}
#define RIL_NW_INIT_INT         {0, 0 ,0}
#define RIL_NW_INIT_STRING      {{0},{0},{0}}
#define RIL_NW_INIT_STRUCT      {NULL, NULL, NULL}
#define RIL_NW_INIT_STATE   {RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS,\
                             RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS}
#else /* Gemini 2 SIM*/
#define RIL_NW_NUM              2
#define RIL_NW_INIT_MUTEX       {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER}
#define RIL_NW_INIT_INT         {0, 0}
#define RIL_NW_INIT_STRING      {{0},{0}}
#define RIL_NW_INIT_STRUCT      {NULL, NULL}
#define RIL_NW_INIT_STATE   {RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS, RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS}
#endif 
#else   /* MTK_GEMINI */
#define RIL_NW_NUM              1
#define RIL_NW_INIT_MUTEX       {PTHREAD_MUTEX_INITIALIZER}
#define RIL_NW_INIT_INT         {0}
#define RIL_NW_INIT_STRING      {{0}}
#define RIL_NW_INIT_STRUCT      {NULL}
#define RIL_NW_INIT_STATE       {RIL_NW_ALL_FLIGHTMODE_RESTRICTIONS}
#endif /* MTK_GEMINI */

#define PROPERTY_NITZ_OPER_CODE     "persist.radio.nitz_oper_code"
#define PROPERTY_NITZ_OPER_CODE2    "persist.radio.nitz_oper_code2"
#define PROPERTY_NITZ_OPER_CODE3    "persist.radio.nitz_oper_code3"
#define PROPERTY_NITZ_OPER_CODE4    "persist.radio.nitz_oper_code3"
#define PROPERTY_NITZ_OPER_LNAME    "persist.radio.nitz_oper_lname"
#define PROPERTY_NITZ_OPER_LNAME2   "persist.radio.nitz_oper_lname2"
#define PROPERTY_NITZ_OPER_LNAME3   "persist.radio.nitz_oper_lname3"
#define PROPERTY_NITZ_OPER_LNAME4   "persist.radio.nitz_oper_lname4"
#define PROPERTY_NITZ_OPER_SNAME    "persist.radio.nitz_oper_sname"
#define PROPERTY_NITZ_OPER_SNAME2   "persist.radio.nitz_oper_sname2"
#define PROPERTY_NITZ_OPER_SNAME3   "persist.radio.nitz_oper_sname3"
#define PROPERTY_NITZ_OPER_SNAME4   "persist.radio.nitz_oper_sname4"

/*RIL Network MACRO */
#define cleanCurrentRestrictionState(flags,x) (ril_nw_cur_state[x] &= ~flags)
#define setCurrentRestrictionState(flags,x) (ril_nw_cur_state[x] |= flags)



typedef enum
{
    NO_SIM_INSERTED     = 0x00,
    SIM1_INSERTED       = 0x01,
    SIM2_INSERTED       = 0x02,
    SIM3_INSERTED       = 0x04,
    SIM4_INSERTED       = 0x08,
    DUAL_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED),
    TRIPLE_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED | SIM3_INSERTED),
    QUAD_SIM_INSERTED   = (SIM1_INSERTED | SIM2_INSERTED | SIM3_INSERTED| SIM4_INSERTED)    
} sim_inserted_status_enum;

//#if defined(MTK_WORLD_PHONE) && defined(MTK_CMCC_WORLD_PHONE_TEST)
//For the Instance struct of CFUN in the queue.
typedef struct cfunParam{
    char cfun[12];
    RIL_Token token;
}CfunParam;

//Mark for EFUN = 0 has been send to Modem.
#define FLAG_AT_EFUN_DONE 0x00000001
//Mark for CFUN = 0 is in the Queue
#define FLAG_AT_CFUN_IN_Q 0x00000002

#define SET_RIL_FLAG(x, f) ((x)|=(f))
#define CLEAR_RIL_FLAG(x, f) ((x)&=(~f))
#define IS_SET_RIL_FLAG(x, f) ((x)&(f))
//#endif //MTK_WORLD_PHONE

#endif /* RIL_NW_H */

