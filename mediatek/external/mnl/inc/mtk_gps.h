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
 *   mtk_gps.h
 *
 * Description:
 * ------------
 *   Prototype of MTK navigation library
 *
 ****************************************************************************/

#ifndef MTK_GPS_H
#define MTK_GPS_H

#include "mtk_gps_type.h"
#include "mtk_gps_agps.h"

#include "mtk_gps_driver_wrapper.h"
#ifdef USING_NAMING_MARC
#include "mtk_gps_macro.h"
#endif


#ifdef __cplusplus
  extern "C" {
#endif
/* ================= Application layer functions ================= */

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_run
 * DESCRIPTION
 *  The main routine for the MTK Nav Library task
 * PARAMETERS
 *  application_cb      [IN]
 *  default_cfg (mtk_init_cfg)       [IN]
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_run (MTK_GPS_CALLBACK application_cb, const MTK_GPS_INIT_CFG* default_cfg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_hotstill_run
 * DESCRIPTION
 *  The main routine for the HotStill task
 * PARAMETERS
  *  driver_cfg     [IN]  factory default configuration
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_hotstill_run (MTK_GPS_DRIVER_CFG* driver_cfg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_agent_run
 * DESCRIPTION
 *  The main routine for the Agent task
 * PARAMETERS
  *  driver_cfg     [IN]  factory default configuration
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_agent_run (MTK_GPS_DRIVER_CFG* driver_cfg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_update_gps_data
 * DESCRIPTION
 *  Force to write NV-RAM data to storage file
 * PARAMETERS
 *
 * RETURNS
 *  None
 *****************************************************************************/
INT32
mtk_gps_update_gps_data (void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_data_input
 * DESCRIPTION
 *
 * PARAMETERS
 *  buffer      [IN] the content of the gps measuremnt
 *  length      [IN] the length of the gps measurement
 *  p_accepted_length [OUT]  indicate how many data was actually accepted into library
 *                          if this value is not equal to length, then it means library internal
 *                          fifo is full, please let library task can get cpu usage to digest input data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_data_input (const char* buffer, UINT32 length, UINT32* p_accepted_length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_rtcm_input
 * DESCRIPTION
 *  accept RTCM differential correction data
 * PARAMETERS
 *  buffer      [IN]   the content of RTCM data
 *  length      [IN]   the length of RTCM data (no more than 1KB)
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_rtcm_input (const char* buffer, UINT32 length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_nmea_input
 * DESCRIPTION
 *  accept NMEA (PMTK) sentence raw data
 * PARAMETERS
 *  buffer      [IN]   the content of NMEA (PMTK) data
 *  length      [IN]   the length of NMEA (PMTK) data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 
mtk_gps_nmea_input (const char* buffer, UINT32 length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_agps_input
 * DESCRIPTION
 *  accept NMEA (PMTK) sentence raw data (only for agent using)
 * PARAMETERS
 *  buffer      [IN]   the content of NMEA (PMTK) data
 *  length      [IN]   the length of NMEA (PMTK) data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 
mtk_gps_agps_input (const char* buffer, UINT32 length);


/* ====================== Utility functions ====================== */
/*  These functions must be used in application_cb() callback
    function specified in mtk_gps_run()                            */

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_position
 * DESCRIPTION
 *  obtain detailed fix information
 * PARAMETERS
 *  pvt_data    [OUT]  pointer to detailed fix information
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_position (MTK_GPS_POSITION* pvt_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GPS/QZSS
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_sv_info (MTK_GPS_SV_INFO* sv_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_gleo_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GALIILEO
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_gleo_sv_info (MTK_GLEO_SV_INFO* sv_gleo_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_glon_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GALIILEO
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_glon_sv_info (MTK_GLON_SV_INFO* sv_glon_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_bedo_sv_info
 * DESCRIPTION
 *  obtain detailed information of all satellites for GALIILEO
 * PARAMETERS
 *  sv_data     [OUT]  pointer to satellites information
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_bedo_sv_info(MTK_BEDO_SV_INFO* sv_bedo_data);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_param
 * DESCRIPTION
 *  Get the current setting of the GPS receiver
 * PARAMETERS
 *  key         [IN]   the configuration you want to know
 *  value       [OUT]  the current setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 * EXAMPLE
 *  // get the current DGPS mode
 *  mtk_param_dgps_config param_dgps_config;
 *  mtk_gps_get_param(MTK_PARAM_DGPS_CONFIG, &param_dgps_config);
 *  printf("DGPS mode=%d", (int)param_dgps_config.dgps_mode);
 *****************************************************************************/

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sv_list
 * DESCRIPTION
 *  Return SV list (Elev >= 5)
 * PARAMETERS
 *   *UINT32 SV list bit map             
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_sv_list(UINT32 *svListBitMap);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_param
 * DESCRIPTION
 *  Get the current setting of the GPS receiver
 * PARAMETERS
 *  key         [IN]   the configuration you want to know
 *  value       [OUT]  the current setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 * EXAMPLE
 *  // get the current DGPS mode
 *  mtk_param_dgps_config param_dgps_config;
 *  mtk_gps_get_param(MTK_PARAM_DGPS_CONFIG, &param_dgps_config);
 *  printf("DGPS mode=%d", (int)param_dgps_config.dgps_mode);
 *****************************************************************************/
INT32
mtk_gps_get_param (MTK_GPS_PARAM key, void* value);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_param
 * DESCRIPTION
 *  Change the behavior of the GPS receiver
 * PARAMETERS
 *  key         [IN]   the configuration needs to be changed
 *  value       [IN]   the new setting
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_param (MTK_GPS_PARAM key, const void* value);

//*****************************************************************************
// mtk_gps_set_navigation_speed_threshold :
// The function will keep fix the postion output to the same point if the current
// estimated 3D speed is less than SpeedThd or the distance of true position and the fix points
// are large than 20 meter.
// Parameters : 
//              SpeedThd must be >= 0
//              The unit of SpeedThd is [meter/second]
//              if SpeedThd = 0,  The navigation speed threshold function will be disabled.
// For exapmle,
// The fix points will keep to the same points until estimated speed is large than 0.4m/s
//
// float SpeedThd = 0.4;
// MTK_Set_Navigation_Speed_Threshold(SpeedThd);
// 

INT32
mtk_gps_set_navigation_speed_threshold(float SpeedThd);

//*****************************************************************************
// mtk_gps_get_navigation_speed_threshold :
// Query the current Static Navigation Speed Threshold
// Parameters :  The unit of SpeedThd  is [meter/second]
// if SpeedThd = 0, The navigation speed threshold is not functional.

INT32
mtk_gps_get_navigation_speed_threshold(float *SpeedThd);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_pmtk_data
 * DESCRIPTION
 *  send PMTK command to GPS receiver
 * PARAMETERS
 *  prPdt       [IN]   pointer to the data structure of the PMTK command
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_pmtk_data (const MTK_GPS_PMTK_DATA_T *prPdt);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_pmtk_response
 * DESCRIPTION
 *  obtain detailed information of PMTK response
 * PARAMETERS
 *  rs_data     [OUT]  pointer to PMTK response data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_pmtk_response (MTK_GPS_PMTK_RESPONSE_T *prRsp);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_position
 * DESCRIPTION
 *  Set the receiver's initial position
 *  Notes: To make the initial position take effect, please invoke restart
 *         (hot start/warm start) after this function
 * PARAMETERS
 *  LLH         [IN]  LLH[0]: receiver latitude in degrees (positive for North)
 *                    LLH[1]: receiver longitude in degrees (positive for East)
 *                    LLH[2]: receiver WGS84 height in meters
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_position (const double LLH[3]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_time
 * DESCRIPTION
 *  Set the current GPS time
 *  Note:       The time will not be set if the receiver has better knowledge
 *              of the time than the new value.
 * PARAMETERS
 *  weekno      [IN]   GPS week number (>1024)
 *  TOW         [IN]   time of week (in second; 0.0~684800.0)
 *  timeRMS     [IN]   estimated RMS error of the TOW value (sec^2)
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_time (UINT16 weekno, double tow, float timeRMS);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_ephemeris
 * DESCRIPTION
 *  Upload ephemeris
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN (1~32)
 *  data        [IN]   binary ephemeris words from words 3-10 of subframe 1-3
 *                     all parity bits (bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3, subframe 1
 *                     data[1]: bit 21-14 of word 3, subframe 1
 *                     data[2]: bit 29-22 of word 3, subframe 1
 *                     data[3]: bit 13-6 of word 4, subframe 1
 *                     ......
 *                     data[71]: bit 29-22 of word 10, subframe 3
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_ephemeris (UINT8 svid, const char data[72]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_tcxo_mode
 * DESCRIPTION
 *  Set MNL TCXO mode
 * PARAMETERS
 * 

 *  MTK_TCXO_NORMAL,  //normal mode 
 *  MTK_TCXO_PHONE    //phone mode

 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_set_tcxo_mode (MTK_GPS_TCXO_MODE tcxo_mode);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_AIC_mode
 * DESCRIPTION
 *  Set AIC mode
 * PARAMETERS
 * 
 * disalbe = 0
 * enable = 1

 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_set_AIC_mode (MTK_GPS_AIC_MODE AIC_Enable);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_almanac
 * DESCRIPTION
 *  Upload almanac
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN (1~32)
 *  weekno      [IN]   the week number of the almanac data record
 *  data        [IN]   binary almanac words from words 3-10 of either subframe 4
 *                     pages 2-10 or subframe 5 pages 1-24
 *                     all parity bits (bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3
 *                     data[1]: bit 21-14 of word 3
 *                     data[2]: bit 29-22 of word 3
 *                     data[3]: bit 13-6 of word 4
 *                     ......
 *                     data[23]: bit 29-22 of word 10
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_almanac (UINT8 svid, UINT16 weekno, const char data[24]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_ephemeris
 * DESCRIPTION
 *  Download ephemeris
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN (1~32)
 *  data        [OUT]  binary ephemeris words from words 3-10 of subframe 1-3
 *                     all parity bits (bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3, subframe 1
 *                     data[1]: bit 21-14 of word 3, subframe 1
 *                     data[2]: bit 29-22 of word 3, subframe 1
 *                     data[3]: bit 13-6 of word 4, subframe 1
 *                     ......
 *                     data[71]: bit 29-22 of word 10, subframe 3
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_ephemeris (UINT8 svid, char data[72]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_almanac
 * DESCRIPTION
 *  Download almanac
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN (1~32)
 *  p_weekno    [OUT]  pointer to the week number of the almanac data record
 *  data        [OUT]  binary almanac words from words 3-10 of either subframe 4
 *                     pages 2-10 or subframe 5 pages 1-24
 *                     all parity bits (bit 5-0) have been removed
 *                     data[0]: bit 13-6 of word 3
 *                     data[1]: bit 21-14 of word 3
 *                     data[2]: bit 29-22 of word 3
 *                     data[3]: bit 13-6 of word 4
 *                     ......
 *                     data[23]: bit 29-22 of word 10
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_almanac (UINT8 svid, UINT16* p_weekno, char data[24]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_assist_bitmap
 * DESCRIPTION
 *  Set assist(EPH/ALM) bitmap
 * PARAMETERS
 *  UINT8 AssistBitMap
 *     If you want EPH assist data, please set AssistBitMap to 0x08
 *     If you want ALM assist data, please set AssistBitMap to 0x01
 *     If you want EPH and ALM assist data, please set AssistBitMap to 0x09
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_assist_bitmap(UINT16 AssistBitMap);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_assist_bitmap
 * DESCRIPTION
 *  Get Re-aiding assist(EPH/ALM) bitmap
 * PARAMETERS
 *  uint8* pAssistBitMap
 *     Get current Re-aiding assist bitmap
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_assist_bitmap(UINT16 *pAssistBitMap);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_clear_ephemeris
 * DESCRIPTION
 *  clear the ephemeris of the specified PRN
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN (1~32)
 *****************************************************************************/
void
mtk_gps_clear_ephemeris (UINT8 svid);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_clear_almanac
 * DESCRIPTION
 *  clear the almanac of the specified PRN
 * PARAMETERS
 *  svid        [IN]   GPS satellite PRN (1~32)
 *****************************************************************************/
void
mtk_gps_clear_almanac (UINT8 svid);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sbas_msg_amount
 * DESCRIPTION
 *  Get the number of SBAS message blocks received in this epoch.
 *  Later on, please use mtk_gps_get_sbas_msg() to get the message 
 *  content one by one.
 * PARAMETERS
 *  p_msg_amount  [OUT]   The number of SBAS message blocks received in this epoch
 * RETURNS
 *  
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_sbas_msg_amount (UINT32* p_msg_amount);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_sbas_msg
 * DESCRIPTION
 *  After calling mtk_gps_get_sbas_msg_amount(), we know the
 *  number of SBAS messages received in this epoch.
 *  mtk_gps_get_sbas_msg() gives a way to access each message
 *  data 
 * PARAMETERS
 *  index        [IN]   which message you want to read
 *  pSVID        [OUT]  the PRN of the SBAS satellite
 *  pMsgType     [OUT]  the SBAS message type
 *  pParityError [OUT]  nonzero(parity error); zero(parity check pass)
 *  data         [OUT]  The 212-bit message data excluding the preamble,
 *                      message type field, and the parity check.
 *                      Regarding to endian, the data[0] is the beginning the
 *                      message, such as IODP field. The data[26] is the end of
 *                      message, and the bit 3..0 are padding zero.
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 * EXAMPLE
 *   //dump the SBAS message to UART output
 *   int  i, count; 
 *   unsigned char PRN, MsgType, ParityError;
 *   char data[27];
 *
 *   mtk_gps_get_sbas_msg_amount(&count);
 *   for (i = 0; i < count; i++)
 *   {
 *      mtk_gps_get_sbas_msg(i, &PRN, &MsgType, &ParityError, data);
 *   }
 *****************************************************************************/
INT32
mtk_gps_get_sbas_msg (INT32 index, unsigned char* pSVID,
     unsigned char* pMsgType, unsigned char* pParityError, char data[27]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_chn_status
 * DESCRIPTION
 *  Get Channel SNR and Clock Drift Status in Channel Test Mode
 * PARAMETERS
 *  ChnSNR       [OUT]  Channel SNR
 *  ClkDrift     [OUT]  Clock Drift
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *  if you do not enter test mode first or Channel tracking not ready,
 *  return MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_gps_get_chn_test(float ChnSNR[16], float *ClkDrift);



/****************************************************************************
* FUNCTION
*mtk_gps_D2_Set_Enable  
*DESCRIPTION
* Bediou D2 data Enable/Disable functions
*PARAMETERS
* UsedD2Corr =1, Enable D2 correction data.
* UsedD2Corr =0, Disable D2 correction data.
*RETURNS
* None
 *****************************************************************************/

void mtk_gps_D2_Set_Enable(unsigned char bUsedD2Corr);



/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_glonass_chn_status
 * DESCRIPTION
 *  Get Channel SNR and Clock Drift Status in Channel Test Mode
 * PARAMETERS
 *  ChnSNR       [OUT]  Channel SNR
 *  ClkDrift     [OUT]  Clock Drift
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *  if you do not enter test mode first or Channel tracking not ready,
 *  return MTK_GPS_ERROR
 *****************************************************************************/
INT32
mtk_gps_get_glonass_chn_test(float ChnSNR[3]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_jammer_test
 * DESCRIPTION
 *  Obtain the CW jammer estimation result
 * PARAMETERS
 *  Freq         [OUT]  jammer frequency offset in KHz
 *  JNR          [OUT]  JNR of the associated jammer
 *  jammer_peaks [OUT]  The number (0~195) of jammer peaks if ready
 *                      0 means no jammer detected
 *                      Negative if not ready
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_jammer_test(INT32 *jammer_peaks, short Freq[195], UINT16 JNR[195]);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_jammer_test
 * DESCRIPTION
 *  Enter or leave Jammer Test Mode
 * PARAMETERS
 *  action [IN]   1 = enter Jammer test(old); 2 = enter Jammer test(new); 0 = leave Jammer test
 *  SVid [IN] please assign any value between 1~32
 *  range [IN] no use in new jammer scan, any value is ok.
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
*****************************************************************************/
INT32
mtk_gps_set_jammer_test(INT32 action, UINT16 mode, UINT16 arg);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_gnss_jammer_test
 * DESCRIPTION
 *   Enter or leave Jammer Test Mode
 * PARAMETERS
 *  action       [IN]   1 = enter Jammer test; 0 = leave Jammer test, not ready
 *  mode         [IN]   0 = GPS; 1 = GLONASS ;2 = Beidou
 *  arg          [IN]   unused
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_gnss_jammer_test(INT32 action, UINT16 mode, UINT16 arg);
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_phase_test
 * DESCRIPTION
 *  Obtain the last phase error calibration result
 * PARAMETERS
 *  result       [OUT]  0~64 (success)
 *                      Negative (failure or not ready)
 *                      The return value is 64*{|I|/sqrt(I*I + Q*Q)}
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_phase_test(INT32 *result);

 
//*************************************************************************************
// mtk_gps_get_sat_accurate_snr  :  Get the accurate SNR of all satellites
//
//    Note  :  SNR is an array with 32 float.
// Example:
//    float SNR[32];
//    mtk_gps_get_sat_accurate_snr(SNR);
//    //Get SNR of SV 17
//    MTK_UART_OutputData("SV17: SNR = %lf", SNR[16]);
//    
// =====> SV17: SNR = 38.1

void mtk_gps_get_sat_accurate_snr(float SNR[32]);


//*************************************************************************************
// mtk_gps_get_glon_sat_accurate_snr  :  Get the accurate SNR of all GLONASS satellites
//
//    Note  :  SNR is an array with [24] float.
// Example:
//    float SNR[24];
//    mtk_gps_get_glon_sat_accurate_snr(SNR);
//    //Get SNR of SV 1
//    MTK_UART_OutputData("GLON,SV1: SNR = %lf", SNR[0]);
//    
// =====>GLON,SV1: SNR = 38.1

void mtk_gps_get_glon_sat_accurate_snr ( float ASNR[24] );

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_per_test
 * DESCRIPTION
 *   Enable PER test
 * PARAMETERS
 *  Threshold       [IN]   power of 2 (1,2,4,8,...)
 *  SVid               [IN]   SVid for PER test that can be tracked in sky
 *  TargetCount   [IN]   Test time in 20ms unit
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_per_test(INT16 Threshold, UINT16 SVid, UINT16 TargetCount);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_set_agps_data
 * DESCRIPTION
 *  send A-GPS assistance data or command to GPS receiver
 * PARAMETERS
 *  pradt       [IN]   pointer to the data structure of the A-GPS command or data
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_set_agps_data (const MTK_GPS_AGPS_CMD_DATA_T *prAdt);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_agps_response
 * DESCRIPTION
 *  obtain detailed information of A-GPS reponse
 * PARAMETERS
 *  prRsp     [OUT]  pointer to A-GPS response data
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_agps_response (MTK_GPS_AGPS_RESPONSE_T *prRsp);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_agps_req_mod
 * DESCRIPTION
 *  obtain req module of A-GPS response
 * PARAMETERS
 *  ReqMod     [OUT]  pointer to request module
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_get_agps_req_mod (MTK_GPS_MODULE *pReqMod);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_time_change_notify
 * DESCRIPTION
 *  Notify MNL to handle RTC time change
 * PARAMETERS
 *  INT32RtcDiff      [IN]  System RTC time changed: old rtc time - new rtc time
 * RETURNS
 *  
 *****************************************************************************/
void
mtk_gps_time_change_notify(INT32 INT32RtcDiff);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_rtc_offset
 * DESCRIPTION
 *  Notify MNL to handle RTC time change
 * PARAMETERS
 *  r8RtcOffset      [OUT]  System RTC time changed
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_rtc_offset(double *r8RtcOffset);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_time_update_notify
 * DESCRIPTION
 *  Notify MNL to handle RTC time change
 * PARAMETERS
 *  fgSyncGpsTime      [IN]  System time sync to GPS time
 *  i4RtcDiff      [IN]  Difference of system RTC time changed
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_time_update_notify(UINT8 fgSyncGpsTime, INT32 i4RtcDiff);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_debug_config
 * DESCRIPTION
 *  config the debug log type and level
 * PARAMETERS
 *  DbgType         [IN]  Debug message category
 *  DbgLevel        [IN]  Debug message output level
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_sys_debug_config(MTK_GPS_DBG_TYPE DbgType, MTK_GPS_DBG_LEVEL DbgLevel);


/* =================== Porting layer functions =================== */
/*            The function body needs to be implemented            */
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_gps_mnl_callback
 * DESCRIPTION
 *  MNL CallBack function
 * PARAMETERS
 *  mtk_gps_notification_type
 * RETURNS
 *   success(MTK_GPS_SUCCESS)
 *****************************************************************************/
 
INT32 
mtk_gps_sys_gps_mnl_callback (MTK_GPS_NOTIFICATION_TYPE msg);

/*****************************************************************************
 * FUNCTION
*  mtk_gps_sys_time_tick_get
* DESCRIPTION
*  get the current system tick of target platform (msec)
* PARAMETERS
*  none
* RETURNS
*  system time tick
*****************************************************************************/
UINT32
mtk_gps_sys_time_tick_get (void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_time_tick_get_max
 * DESCRIPTION
 *  get the maximum system tick of target platform (msec)
 * PARAMETERS
 *  none
 * RETURNS
 *  system time tick
 *****************************************************************************/
UINT32
mtk_gps_sys_time_tick_get_max (void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_time_read
 * DESCRIPTION
 *  Read system time
 * PARAMETERS
 *  utctime     [IN/OUT] get the host system time
 * RETURNS
 *  success (MTK_GPS_SUCCESS)
 *  failed (MTK_GPS_ERROR)
 *  system time changed since last call (MTK_GPS_ERROR_TIME_CHANGED)
 *****************************************************************************/
INT32
mtk_gps_sys_time_read (MTK_GPS_TIME* utctime);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_task_sleep
 * DESCRIPTION
 *  Task sleep function
 * PARAMETERS
 *  milliseconds [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_task_sleep (UINT32 milliseconds);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_open
 * DESCRIPTION
 *  Open a non-volatile file
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_storage_open (void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_close
 * DESCRIPTION
 *  Close a non-volatile file
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_storage_close (void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_delete
 * DESCRIPTION
 *  Delete a non-volatile file  
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_storage_delete (void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_read
 * DESCRIPTION
 *  Read a non-volatite file 
 *  - blocking read until reaching 'length' or EOF
 * PARAMETERS
 *  buffer      [OUT]
 *  offset      [IN]
 *  length      [IN]
 *  p_nRead     [OUT]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_storage_read (void* buffer, UINT32 offset, UINT32 length,
                      UINT32* p_nRead);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_storage_write
 * DESCRIPTION
 *  Write a non-volatite file
 * PARAMETERS
 *  buffer      [IN]
 *  offset      [IN]
 *  length      [IN]
 *  p_nWritten  [OUT]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_storage_write (const void* buffer, UINT32 offset, UINT32 length,
                       UINT32* p_nWritten);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_mem_alloc
 * DESCRIPTION
 *  Allocate a block of memory
 * PARAMETERS
 *  size        [IN]   the length of the whole memory to be allocated
 * RETURNS
 *  On success, return the pointer to the allocated memory
 *  NULL (0) if failed
 *****************************************************************************/
void*
mtk_gps_sys_mem_alloc (UINT32 size);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_mem_free
 * DESCRIPTION
 *  Release unused memory
 * PARAMETERS
 *  pmem         [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_mem_free (void* pmem);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_delete
* DESCRIPTION
*  event delete for Android
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_delete(MTK_GPS_EVENT_ENUM event_idx);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_create
* DESCRIPTION
*  event create for Android
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_create(MTK_GPS_EVENT_ENUM event_idx);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_set
* DESCRIPTION
*  event set for Android
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_set(MTK_GPS_EVENT_ENUM event_idx);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_event_wait
* DESCRIPTION
*  event wait for android
*  
* PARAMETERS
*  event_idx         [IN] MTK_GPS_EVENT_ENUM
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_event_wait(MTK_GPS_EVENT_ENUM event_idx); 

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_init
 * DESCRIPTION
 *  Initiialize UART 
 * PARAMETERS
 *  portname      [IN]
 *  baudrate      [IN]
 *  txbufsize      [IN]
 *  rxbufsize      [IN]
 * RETURNS
 *  Result of Handler 
 *****************************************************************************/
INT32
mtk_gps_sys_uart_init( char* portname, UINT32 baudrate, UINT32 txbufsize,  
                      UINT32 rxbufsize);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_read
 * DESCRIPTION
 *  Initiialize UART 
 * PARAMETERS
 *  UARTHandle      [IN]
 *  buffer      [IN]
 *  bufsize      [IN]
 *  length      [IN]
 * RETURNS
 *  Result of Handler 
 *****************************************************************************/
INT32 
mtk_gps_sys_uart_read( INT32 UARTHandle, char* buffer, UINT32 bufsize,
                   INT32* length);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_write
 * DESCRIPTION
 *  Initiialize UART 
 * PARAMETERS
 *  UARTHandle      [IN]
 *  buffer      [IN]
 *  bufsize      [IN]
 *  length      [IN]
 * RETURNS
 *  Result of Handler 
 *****************************************************************************/
INT32  mtk_gps_sys_uart_write( INT32 UARTHandle, const char* buffer, UINT32 bufsize,
       INT32* wrotenlength);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_uninit
 * DESCRIPTION
 *  Initiialize UART 
 * PARAMETERS
 *  UARTHandle      [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void 
mtk_gps_sys_uart_uninit(INT32 UARTHandle);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_if_set_spd
 * DESCRIPTION
 *  Set baud rate at host side from GPS lib
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  baudrate         [IN] UART baudrate 
 *  hw_fc            [IN] UART hardware flow control 
 *                        0: without hardware flow contorl (defualt)
 *                        1: with hardware flow contorl
 * RETURNS
 *  success(0)
 *****************************************************************************/
INT32
mtk_gps_sys_if_set_spd (UINT32 baudrate, UINT8 hw_fc);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_data_output
 * DESCRIPTION
 *  Transmit data to the GPS chip
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  msg         [IN]
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_data_output (char* buffer, UINT32 length);

/*****************************************************************************
* FUNCTION
*  mtk_gps_sys_nmea_output
* DESCRIPTION
*  Transmit NMEA data out to task
*  (The function body needs to be implemented)
* PARAMETERS
*  buffer         [IN] data pointer
*  length         [IN] size of data
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_nmea_output (char* buffer, UINT32 length);


/*****************************************************************************
* FUNCTION
*  mtk_sys_nmea_output_to_app
* DESCRIPTION
*  Transmit NMEA data out to APP layer
*  (The function body needs to be implemented)
* PARAMETERS
*  buffer         [IN] data pointer
*  length         [IN] size of data
* RETURNS
*  success(MTK_GPS_SUCCESS)
*****************************************************************************/
INT32
mtk_gps_sys_nmea_output_to_app(const char * buffer, UINT32 length);

// Otehr platform

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_start_result_handler
 * DESCRIPTION
 *  Handler routine for the result of restart command
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  result         [IN]  the result of restart
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 
mtk_gps_sys_start_result_handler(MTK_GPS_START_RESULT result);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_spi_poll
 * DESCRIPTION
 *  Polling data input routine for SPI during dsp boot up stage.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  void
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_spi_poll(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_set_spi_mode
 * DESCRIPTION
 *  Set SPI interrupt/polling and support burst or not.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  enable_int         [IN]  1 for enter interrupt mode , 0 for entering polling mode
 *  enable_burst       [IN]  1 for enable burst transfer, 0 for disable burst transfer
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32
mtk_gps_sys_set_spi_mode(UINT8 enable_int, UINT8 enable_burst);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_dsp_boot_begin_handler
 * DESCRIPTION
 *  Handler routine for porting layer implementation right before GPS DSP boot up
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  none
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 
mtk_gps_sys_dsp_boot_begin_handler(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_dsp_boot_end_handler
 * DESCRIPTION
 *  Handler routine for porting layer implementation right after GPS DSP boot up
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  none
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 
mtk_gps_sys_dsp_boot_end_handler(void);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas
 * DESCRIPTION
 * PARAMETERS
 *  pFrameTime [OUT] frame time of the issued frame pulse (seconds)
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/

INT32 mtk_gps_sys_frame_sync_meas(double *pdfFrameTime);

INT32 mtk_gps_sys_frame_sync_enable_sleep_mode(unsigned char mode);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas_resp
 * DESCRIPTION
 *  accept a frame sync measurement response
 * PARAMETERS
 *  eResult     [IN] success to issue a frame sync meas request
 *  dfFrameTime [IN] frame time of the issued frame pulse (seconds) 
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_frame_sync_meas_resp(MTK_GPS_FS_RESULT eResult, double dfFrameTime);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas_req_by_network
 * DESCRIPTION
 *  issue a frame sync measurement request
 * PARAMETERS
 *  none
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *  fail(MTK_GPS_ERROR)
 *****************************************************************************/
INT32 
mtk_gps_sys_frame_sync_meas_req_by_network(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_frame_sync_meas_req
 * DESCRIPTION
 *  issue a frame sync measurement request
 * PARAMETERS
 *  mode       [out] frame sync request indication for aiding or maintain
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *  fail(MTK_GPS_ERROR)
 *****************************************************************************/
INT32 
mtk_gps_sys_frame_sync_meas_req(MTK_GPS_FS_WORK_MODE mode);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_initialize_mutex
 * DESCRIPTION
 *  Inialize mutex array
 * PARAMETERS
 *  void
  * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
#if defined(MTK_GPS_MT6620)
mtk_gps_sys_inialize_mutex(void);
#else
mtk_gps_sys_initialize_mutex(void);
#endif
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_create_mutex
 * DESCRIPTION
 *  Create a mutex object
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
  * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_sys_create_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_take_mutex
 * DESCRIPTION
 *  Request ownership of a mutex and if it's not available now, then block the thread execution
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_take_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_give_mutex
 * DESCRIPTION
 *  Release a mutex ownership
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_give_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_destroy_mutex
 * DESCRIPTION
 *  Destroy a mutex object
 * PARAMETERS
 *  mutex_num        [IN]  mutex index used by MTK Nav Library
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_sys_destroy_mutex(MTK_GPS_MUTEX_ENUM mutex_idx);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_pmtk_cmd_cb
 * DESCRIPTION
 *  Notify porting layer that MNL has received one PMTK command.
 * PARAMETERS
 *  UINT16CmdNum        [IN]  The received PMTK command number.
 * RETURNS
 *  void
 *****************************************************************************/
void
mtk_gps_sys_pmtk_cmd_cb(UINT16 UINT16CmdNum);

/*****************************************************************************
 * FUNCTION
 *  mtk_sys_spi_poll
 * DESCRIPTION
 *  Polling data input routine for SPI during dsp boot up stage.
 *  If use UART interface, this function can do nothing at all.
 *  (The function body needs to be implemented)
 * PARAMETERS
 *  void
 * RETURNS
 *  success(MTK_GPS_SUCCESS)
 *****************************************************************************/
INT32 mtk_gps_sys_spi_poll(void);

 /*****************************************************************************
 * FUNCTION
 *  mtk_gps_sys_uart_poll
 * DESCRIPTION
 *  GPS RX polling function 
 * PARAMETERS
 *  void
 * RETURNS
 *  success(0)
 *****************************************************************************/
INT32 mtk_gps_sys_uart_poll(void);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_get_rtc_info
 * DESCRIPTION
 *  
 * PARAMETERS
 *  dfrtcD        [OUT]  RTC clock drift (unit : ppm).
 *  dfage         [OUT]  RTC drift age : current gps time - gps time of latest rtc drift calculated. (unit : sec)
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32
mtk_gps_get_rtc_info(double *dfrtcD, double *dfage);

/*****************************************************************************
 * FUNCTION
 *  mtk_gps_delete_nv_data
 * DESCRIPTION
 *
 * PARAMETERS
 *  u4Bitmap    [INPUT] 
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/
INT32 mtk_gps_delete_nv_data(UINT32 assist_data_bit_map);


/*****************************************************************************
 * FUNCTION
 *  mtk_gps_config_gpevt
 * DESCRIPTION
 *  
 * PARAMETERS
 *  status    [INPUT] status = 1: enable gpevt sentence output, 
 *                           status = 0 : disable gpevt sentence output,
 *                           status = 2 : Query current status
 * RETURNS
 *  success(0xFFFFFFFF); failure (MTK_GPS_ERROR), status ==2 will return current status
 *****************************************************************************/
INT32
mtk_gps_config_gpevt(INT32 status);
/*****************************************************************************
 * FUNCTION
 *  mtk_gps_tsx_xvt
 * DESCRIPTION
 *
 * PARAMETERS
 *  u4Bitmap    [INPUT] 
 *
 * RETURNS
 *  success(MTK_GPS_SUCCESS); failure (MTK_GPS_ERROR)
 *****************************************************************************/

INT32
mtk_gps_tsx_xvt(UINT8 enable);

/*****************************************************************************
 * mtk_gps_set_wifi_location_aiding :
 *   set Wifi location information to MNL
 * Parameters : 
 *  latitude :Wifi latitude      unit: [degree]
 *  longitude:Wifi longitude     unit: [degree]
 *  posvar   :position accuracy  unit: [m]
 * Return Value: 0: fail; 1: pass
 *****************************************************************************/
INT32 mtk_gps_set_wifi_location_aiding(MTK_GPS_REF_LOCATION *RefLocation, double latitude,double longitude,double accuracy);


#ifdef __cplusplus
   }
#endif

#endif /* MTK_GPS_H */
