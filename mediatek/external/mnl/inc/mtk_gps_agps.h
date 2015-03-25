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

/******************************************************************************
*[File] Mtk_gps_agps.h
*[Version] v1.0
*[Revision Date] 2008-05-06
*[Author] Stanley Huang, Stanley_Huang@mtk.com.tw, 21372
*[Description]
*[Copyright]
*    Copyright (C) 2008 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/
#ifndef MTK_GPS_AGPS_H
#define MTK_GPS_AGPS_H


#ifdef __cplusplus
   extern "C" {
#endif

#include "mtk_gps_type.h"


#if ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000 ) ) 
// for ADS1.x
#elif ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 400000 ) ) 
// for RVCT2.x or RVCT3.x
#else
#pragma pack(4)
#endif


typedef struct
{
  UINT8 u1Arg1;
  UINT8 u1Arg2;
} MTK_GPS_AGPS_CMD_MODE_T;

typedef struct
{
  UINT8 u1Delay;  // Response time in unit of sec.
  UINT32 u4HAcc;  // Horizontal accuracy in unit of meter.
  UINT16 u2VAcc;  // Vertical accuracy in unit of meter.
  UINT16 u2PRAcc; // Pseudorange accuracy in unit of meter.
} MTK_GPS_AGPS_CMD_QOP_T;

typedef struct
{
  UINT8 u1SvId;
  UINT32 au4Word[24];
} MTK_GPS_ASSIST_EPH_T;

typedef struct
{
  UINT32 u1SvId;
  UINT32 au1Byte[55];
  UINT32 u1Lf;
} MTK_ASSIST_GLON_EPH_T;


typedef struct
{
  UINT8 u1SvId;
  UINT16 u2WeekNo;
  UINT32 au4Word[8];
} MTK_GPS_ASSIST_ALM_T;

typedef struct
{
  UINT16 u2WeekNo;
  double dfTow;        // sec
  double dfTowRms;     // ms
  double dfFS_Tow;     // sec, not used
  double dfFS_TowRms;  // ms, not used
} MTK_GPS_ASSIST_TIM_T;

typedef struct
{
  UINT16 u2WeekNo;
  double dfTow;        // sec
  double dfTowRms;     // ms
  double dfFTime;      // sec, Frame Time correspond to GPS TOW
  double dfFTimeRms;   // ms, Frame Time RMS accuracy
} MTK_GPS_ASSIST_FTA_T, MTK_GPS_AGPS_DT_FTIME_T;    // Fine Time Assistance

typedef struct
{
  double dfLat;        // Receiver Latitude in degrees
  double dfLon;        // Receiver Longitude in degrees
  double dfAlt;        // Receiver Altitude in meters
  float fAcc_Maj;     // semi-major RMS accuracy [m]
  float fAcc_Min;     // semi-minor RMS accuracy [m]
  UINT8 u1Maj_Bear;   // Bearing of semi-major axis in degrees
  float fAcc_Vert;    // Vertical RMS accuracy [m]
  UINT8 u1Confidence; // Position Confidence: range from 0 ~ 100 [%]
} MTK_GPS_ASSIST_LOC_T;

typedef struct
{
   double dUtc_hms;  // UTC: hhmmss.sss 
   double dUtc_ymd;  //  UTC: yyyymmdd  
   UINT8 u1FixType;  // the type of measurements performed by the MS [0: 2D or 1: 3D]
   double dfLat;     // latitude (degree)
   double dfLon;     // longitude (degree)
   INT16 i2Alt;      // altitude (m)
   float fUnc_SMaj;  // semi-major axis of error ellipse (m)
   float fUnc_SMin;  // semi-minor axis of error ellipse (m)
   UINT16 u2Maj_brg; // bearing of the semi-major axis (degrees) [0 - 179]
   float fUnc_Vert;  // Altitude uncertainty
   UINT8 u1Conf;     // The confidence by which the position is known to be within the shape description, expressed as a percentage. [0 ~ 100] (%)
   UINT16 u2HSpeed;  // Horizontal speed (km/hr)
   UINT16 u2Bearing; // Direction (degree) of the horizontal speed [0 ~ 359]
} MTK_GPS_AGPS_CMD_MA_LOC_T;
typedef struct
{
  double dfClkDrift;   // GPS Clock Frequency Error [nsec/sec]
  INT32 i4ClkRMSAcc;  // Frequency Measurement RSM Accuracy [nsec/sec]
  INT32 i4ClkAge;     // Age (sec) of the clock drift value since last estimated
} MTK_GPS_ASSIST_CLK_T;

typedef struct
{
  INT8 i1a0;         // Klobuchar - alpha 0  (seconds)           / (2^-30)
  INT8 i1a1;         // Klobuchar - alpha 1  (sec/semi-circle)   / (2^-27/PI)
  INT8 i1a2;         // Klobuchar - alpha 2  (sec/semi-circle^2) / (2^-24/PI^2)
  INT8 i1a3;         // Klobuchar - alpha 3  (sec/semi-circle^3) / (2^-24/PI^3)
  INT8 i1b0;         // Klobuchar - beta 0   (seconds)           / (2^11)
  INT8 i1b1;         // Klobuchar - beta 1   (sec/semi-circle)   / (2^14/PI)
  INT8 i1b2;         // Klobuchar - beta 2   (sec/semi-circle^2) / (2^16/PI^2)
  INT8 i1b3;         // Klobuchar - beta 3   (sec/semi-circle^3) / (2^16/PI^3)
} MTK_GPS_ASSIST_KLB_T;

typedef struct
{
  INT32 i4A1;         // UTC parameter A1 (seconds/second)/(2^-50)
  INT32 i4A0;         // UTC parameter A0 (seconds)/(2^-30)
  UINT8 u1Tot;        // UTC reference time of week (seconds)/(2^12)
  UINT8 u1WNt;        // UTC reference week number (weeks)
  INT8 i1dtLS;       // UTC time difference due to leap seconds before event (seconds)
  UINT8 u1WNLSF;      // UTC week number when next leap second event occurs (weeks)
  UINT8 u1DN;         // UTC day of week when next leap second event occurs (days)
  INT8 i1dtLSF;      // UTC time difference due to leap seconds after event (seconds)
} MTK_GPS_ASSIST_UCP_T;

typedef struct
{
  INT8 i1NumBad;        // Number of Bad Satellites listed
  UINT8 au1SvId[MTK_GPS_SV_MAX_PRN]; // A list of bad SV id
} MTK_GPS_ASSIST_BSV_T;


typedef struct
{
  UINT8 u1SV;          // SV PRN number (1 ~ 32) (0 means no data available)
  INT32 i4GPSTOW;      // TOW of last Acquisition Assistance data, Units 0.08 sec
  INT16 i2Dopp;        // Doppler value. Units 2.5 Hz
  INT8 i1DoppRate;    // Doppler rate of change. Units (1/42) Hz/s
  UINT8 u1DoppSR;      // Doppler search range. index. [0 ~ 4]
  UINT16 u2Code_Ph;     // C/A Code Phase chips [range 0..1022]
                    //    relative to the previous msec edge
  INT8 i1Code_Ph_Int; // Integer C/A Code msec into the GPS Data Bit
                    //    [range 0..19 msec]  (-1 if not known)
  INT8 i1GPS_Bit_Num; // GPS Data Bit Number, modulo 80 msec  [range 0..3]
                    //    (-1 if not known)
  UINT8 u1CodeSR;      // Code search range. index. [0 ~ 15]
  UINT8 u1Azim;        // Azimuth. Units 11.25 degrees
  UINT8 u1Elev;        // Elevation. Units 11.25 degrees
} MTK_GPS_ASSIST_ACQ_T;


#define RTCM_MAX_N_SAT 11
typedef struct
{
  UINT8 u1SatID;  // [1 - 32]
  UINT8 u1IODE;   // [0 - 255]
  UINT8 u1UDRE;   // [0 - 3]
  INT16 i2PRC;    // [-655.04 - 655.04], Units 0.32m
  INT8 i1RRC;    // [-4.064 - 4.064], Units 0.032m
} MTK_GPS_RTCM_SV_CORR_T;

typedef struct 
{
  UINT32 u4Tow;     // the baseline time for the corrections are valid [0 - 604799]
  UINT8 u1Status;  // the status of the differential corrections [0 - 7]
  UINT8 u1NumSv;   // the number of satellites for which differential corrections are available [1 - 11]
  MTK_GPS_RTCM_SV_CORR_T arSVC[RTCM_MAX_N_SAT];
} MTK_GPS_ASSIST_DGP_T;

#define TOW_MAX_N_SAT 11
typedef struct
{
    UINT8 u1SatID;   // [1 - 32]
    UINT16 u2TLM;    // [0 - 16383]
    UINT8 u1Anti_s;  // [0 - 1]
    UINT8 u1Alert;   // [0 - 1]
    UINT8 u1Reserved;  // [0 - 3]
} MTK_GPS_TOW_SV_T;

typedef struct 
{
  UINT16 u2WN;      // GPS week number (weeks)
  UINT32 u4Tow;     // GPS time of week  of the TLM message applied [0 - 604799]
  UINT8 u1NumSv;   // the number of satellites for which TOW assist are available [1 - 11]
  MTK_GPS_TOW_SV_T atSV[TOW_MAX_N_SAT];
} MTK_GPS_ASSIST_TOW_T;

typedef struct
{
  MTK_GPS_BOOL fgAcceptAlm;    // Satellite Almanac
  MTK_GPS_BOOL fgAcceptUcp;    // UTC Model
  MTK_GPS_BOOL fgAcceptKlb;    // Ionospheric Model
  MTK_GPS_BOOL fgAcceptEph;    // Navigation Model
  MTK_GPS_BOOL fgAcceptDgps;   // DGPS Corrections
  MTK_GPS_BOOL fgAcceptLoc;    // Reference Location
  MTK_GPS_BOOL fgAcceptTim;    // Reference Time
  MTK_GPS_BOOL fgAcceptAcq;    // Acquisition Assistance
  MTK_GPS_BOOL fgAcceptBsv;    // Real-Time Integrity
} MTK_GPS_AGPS_CMD_ACCEPT_MAP_T;

typedef struct
{
   UINT32 u4Frame;   // BTS Reference Frame number during which the location estimate was measured [0 - 65535]
   UINT16 u2WeekNo;  // the GPS week number for which the location estimate is valid
   UINT32 u4TowMS;   // the GPS TOW (ms) for which the location estimate is valid [0 - 604799999]
   UINT8 u1FixType;  // the type of measurements performed by the MS [0: 2D or 1: 3D]
   double dfLat;         // latitude (degree)
   double dfLon;         // longitude (degree)
   INT16 i2Alt;      // altitude (m)
   float fUnc_SMaj;      // semi-major axis of error ellipse (m)
   float fUnc_SMin;      // semi-minor axis of error ellipse (m)
   UINT16 u2Maj_brg; // bearing of the semi-major axis (degrees) [0 - 179]
   float fUnc_Vert;      // Altitude uncertainty
   UINT8 u1Conf;     // The confidence by which the position is known to be within the shape description, expressed as a percentage. [0 ~ 100] (%)
   UINT16 u2HSpeed;  // Horizontal speed (km/hr)
   UINT16 u2Bearing; // Direction (degree) of the horizontal speed [0 ~ 359]
} MTK_GPS_AGPS_DT_LOC_EST_T;


typedef struct               // Satellite Pseudorange Measurement Data
{
   UINT8 u1PRN_num;      // Satellite PRN number [1 - 32]
   UINT8 u1SNR;          // Satellite Signal to Noise Ratio [dBHz} (range 0-63)
   INT16 i2Dopp;         // Measured Doppler frequency [0.2 Hz] (range +/-6553.6)
   UINT16 u2Code_whole;   // Satellite Code phase measurement - whole chips
                    //   [C/A chips] (range 0..1022)
   UINT16 u2Code_fract;   // Satellite Code phase measurement - fractional chips
                    //   [2^-10 C/A chips] (range 0..1023)
   UINT8 u1Mul_Path_Ind; // Multipath indicator (range 0..3)
                    //   (see TIA/EIA/IS-801 Table 3.2.4.2-7)
   UINT8 u1Range_RMS_Exp;// Pseudorange RMS error: Exponent (range 0..7)
                    //   (see TIA/EIA/IS-801 Table 3.2.4.2-8)
   UINT8 u1Range_RMS_Man;// Pseudorange RMS error: Mantissa (range 0..7)
                    //   (see TIA/EIA/IS-801 Table 3.2.4.2-8)

} MTK_GPS_AGPS_PRM_SV_DATA_T;        // Satellite Pseudorange Measurement Data

#define AGPS_RRLP_MAX_PRM 14
typedef struct
{
   UINT32 u4Frame;         // [0 - 65535]
   UINT8 u1NumValidMeas;  // Number of valid measurements available (0..NUM_CH)
   UINT32 u4GpsTow;        // Time of validity [ms] modulus 14400000
   MTK_GPS_AGPS_PRM_SV_DATA_T SV_Data[AGPS_RRLP_MAX_PRM];  // Satellite Pseudorange Measurement Data
} MTK_GPS_AGPS_DT_GPS_MEAS_T;     // RRLP Pseudorange Data

typedef struct
{
    UINT8 u1Type;
} MTK_GPS_AGPS_DT_FTIME_ERR_T;

typedef struct
{
    UINT16 u2AckCmd;
    UINT8 u1Flag;
} MTK_GPS_AGPS_DT_ACK_T;

typedef struct
{
    UINT8 u1Type;
} MTK_GPS_AGPS_DT_LOC_ERR_T;

typedef struct
{
    UINT16 u2BitMap;
                        //  bit0 0x0001  // almanac
                        //  bit1 0x0002  // UTC model
                        //  bit2 0x0004  // ionospheric model
                        //  bit3 0x0008  // navigation data
                        //  bit4 0x0010  // DGPS corrections
                        //  bit5 0x0020  // reference location
                        //  bit6 0x0040  // reference time
                        //  bit7 0x0080  // acquisition assistance
                        //  bit8 0x0100  // Real-Time integrity
} MTK_GPS_AGPS_DT_REQ_ASSIST_T;

typedef struct
{
    UINT16 u2Cmd;  // PMTK command ID: 
                       // please get the data arguements in the following corresponding data structure.
    union
    {
        MTK_GPS_AGPS_DT_ACK_T rAck;              // PMTK001
        MTK_GPS_AGPS_CMD_MODE_T rAgpsMode;       // PMTK290
        MTK_GPS_AGPS_DT_REQ_ASSIST_T rReqAssist; // PMTK730
        MTK_GPS_AGPS_DT_LOC_EST_T rLoc;          // PMTK731
        MTK_GPS_AGPS_DT_GPS_MEAS_T rPRM;         // PMTK732
        MTK_GPS_AGPS_DT_LOC_ERR_T rLocErr;       // PMTK733
        MTK_GPS_AGPS_DT_FTIME_T rFTime;          // PMTK734
        MTK_GPS_AGPS_DT_FTIME_ERR_T rFTimeErr;   // PMTK735
    } uData;
} MTK_GPS_AGPS_RESPONSE_T;

typedef struct
{
  UINT16    u2Cmd;  // PMTK command ID: if the PMTK command has data arguments, 
                        // please assign the data in the following  corresponding data structure.
  union
  {
    MTK_GPS_AGPS_CMD_MODE_T rMode;              // PMTK290
    MTK_GPS_AGPS_CMD_ACCEPT_MAP_T rAcceptMap;   // PMTK292
    MTK_GPS_AGPS_CMD_QOP_T rQop;                // PMTK293
    MTK_GPS_ASSIST_EPH_T rAEph;             // PMTK710
    MTK_GPS_ASSIST_ALM_T rAAlm;             // PMTK711
    MTK_GPS_ASSIST_TIM_T rATim;             // PMTK712
    MTK_GPS_ASSIST_LOC_T rALoc;             // PMTK713
    MTK_GPS_ASSIST_CLK_T rAClk;             // PMTK714
    MTK_GPS_ASSIST_KLB_T rAKlb;             // PMTK715
    MTK_GPS_ASSIST_UCP_T rAUcp;             // PMTK716
    MTK_GPS_ASSIST_BSV_T rABsv;             // PMTK717
    MTK_GPS_ASSIST_ACQ_T rAAcq;             // PMTK718
    MTK_GPS_ASSIST_FTA_T rAFta;             // PMTK719
    MTK_GPS_ASSIST_DGP_T rARtcm;            // PMTK720
    MTK_GPS_ASSIST_TOW_T rATow;             // PMTK725
    MTK_GPS_AGPS_CMD_MA_LOC_T rAMA_Loc;     // PMTK739
  } uData;
} MTK_GPS_AGPS_CMD_DATA_T;


#if ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000 ) ) 
// for ADS1.x
#elif ( defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 400000 ) ) 
// for RVCT2.x or RVCT3.x
#else
#pragma pack()
#endif

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_get_param
 * DESCRIPTION
 *  Get the current setting of the AGPS Agent
 * PARAMETERS
 *  key         [IN]   the configuration you want to know
 *  value       [OUT]  the current setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_get_param (MTK_GPS_PARAM key, void* value, UINT16 srcMod, UINT16 dstMod);

/*****************************************************************************
 * FUNCTION
 *  mtk_agps_set_param
 * DESCRIPTION
 *  Change the behavior of the AGPS Agent
 * PARAMETERS
 *  key         [IN]   the configuration needs to be changed
 *  value       [IN]   the new setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_agps_set_param (MTK_GPS_PARAM key, const void* value, UINT16 srcMod, UINT16 dstMod);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_agps_disaptcher_callback
 * DESCRIPTION
 *  called by MNL when need send data
 * PARAMETERS
 *  type: msg type, length: payload length, data: payload pointer
 * RETURNS
 *  none
 *****************************************************************************/
INT32
mtk_gps_sys_agps_disaptcher_callback (UINT16 type, UINT16 length, char *data);

#ifdef __cplusplus
   }  /* extern "C" */
#endif

#endif


