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

/******************************************************************************
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
******************************************************************************/

//*****************************************************************************
// [File] MTK_BEE.h : Defines the entry point for BEE library
// [Version] v1.0
// [Revision Date] 2008-03-31
// [Author] YC Chien, yc.chien@mediatek.com, 21558
//          WG Yau, wg.yau@mediatek.com, 26967
// [Description]
//*****************************************************************************


#ifndef MTK_BEE_H
#define MTK_BEE_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "MTK_Type.h"
#include "mtk_gps_type.h"


//*****************************************************************************
// MTK_BEE_Init : Initialize BEE kernel data
//
// PARAMETER : void
//
// RETURN : 1 for success, 0 for error
//          If return value is 0, you should NOT proceed with any BEE function

MTK_BEE_INT MTK_BEE_Init (void);


//*****************************************************************************
// MTK_BEE_Init : Uninitialize BEE kernel data
//
// PARAMETER : void
//
// RETURN : void

MTK_BEE_VOID MTK_BEE_Uninit (void);


//*****************************************************************************
// MTK_BEE_Set_File_Path : Set File Path for BEE kernel file
//
// PARAMETER : szFilePath [IN] - (1) Input file path for ARC.BIN, BEE.BIN and 
//                                   BEET000A. 
//                               (2) Maximal path length is 256 bytes.
//                               (3) MUST BE null-terminated string.
//                               (4) MUST BE called before MTK_BEE_Init().
//                               (5) MUST BE ended with a directory separator
//                                   character. (ex: '\\')
//                               (6) If this function has not been called,
//                                   then no file path is set. 
// EXAMPLE :
//    MTK_BEE_Set_File_path("C:\\BEE\\"); 
//    MTK_BEE_Init();
//    ===> Input File path before MTK_BEE_Init() is called.
//
// RETURN : 1 for success, 0 for error
//          If return value is 0, you should NOT proceed with any BEE function

MTK_BEE_INT MTK_BEE_Set_File_Path (char szFilePath[256]);


//*****************************************************************************
// MTK_BEE_Get_Table_Health : Get the health status of BEE data file (BEET000A)
//
// PARAMETER : void
//
// RETURN : 0 -> BEE data file is healthy
//          1 -> BEE data file is valid for BEE functionality, but an update for this file is suggested
//          2 -> BEE data file is expired so that BEE functionality has been stopped
//          3 -> User has updated BEE data file to an older version,
//               BEE functionality has been stopped, need to update data file

MTK_BEE_INT MTK_BEE_Get_Table_Health (void);


//*****************************************************************************
// MTK_BEE_Proc_Eph : Save broadcast ephemeris into BEE kernel database
//
// PARAMETER : u1SvId  [IN] - satellite ID
//             au4Word [IN] - 24-word ephemeris raw data (word 3-8 of subframe 1,2,3)
//
// RETURN : void

MTK_BEE_VOID MTK_BEE_Proc_Eph (unsigned char u1SvId, unsigned int au4Word[24]);


//*****************************************************************************
// MTK_BEE_Have_Enough_Eph : Check how many satellites have enough ephemeris for BEE generation
//
// PARAMETER : i2WeekNo    [IN] - week number (count from Jan. 6, 1980)
//             i4Tow       [IN] - time of week
//
// RETURN : Number of satellites with enough ephemeris for BEE generation
//          Ex : 5 -> 5 satellites have enough ephemeris
//
// NOTE : i2WeekNo and i4Tow SHOULD BE set to the correct current time ( > 0), 
//        If i2WeekNo = 0 and i4Tow = 0 this function will not do optimization !!! 
//
// EXAMPLE :
//    short i2WeekNo;
//    int i4Tow;
//    int i4NumSvToGen = MTK_BEE_Have_Enough_Eph(i2WeekNo, i4Tow);
//    if ( i4NumSvToGen >= 3 )  ===> Do BEE generation when at least 3 satellites have enough ephemeris
//    {
//        MTK_BEE_Gen_Auto(i2WeekNo, i4Tow);
//    }

MTK_BEE_INT MTK_BEE_Have_Enough_Eph (short i2WeekNo, int i4Tow);


//*****************************************************************************
// MTK_BEE_Gen_All : Generate BEE ephemeris of all satellites
//
// PARAMETER : void
//
// RETURN : void

MTK_BEE_VOID MTK_BEE_Gen_All (void);


//*****************************************************************************
// MTK_BEE_Gen_Auto : Auto Generate BEE ephemeris of all satellites
//
// PARAMETER : i2WeekNo    [IN] - week number (count from Jan. 6, 1980)
//             i4Tow       [IN] - time of week
//
// RETURN : 1 -> BEE data is already FULL generated
//          0 -> BEE data is not FULL generated
//
// NOTE : i2WeekNo and i4Tow MUST BE set to the correct current time ( > 0), 
//        otherwise this function may have wrong generated data !!!
//        If i2WeekNo = 0 and i4Tow = 0 this function will not do optimization !!! 

MTK_BEE_INT MTK_BEE_Gen_Auto (short i2WeekNo, int i4Tow);


//*****************************************************************************
// MTK_BEE_Gen_Data : Generate BEE ephemeris of the specified satellite
//
// PARAMETER : u1SvId      [IN] - satellite ID
//             i2WeekNo    [IN] - week number (count from Jan. 6, 1980)
//             i4Tow       [IN] - time of week
//             u1GenLength [IN] - generate length ( MUST > 0 )
//                                =1 -> Generate 1 Day  Data 
//                                =2 -> Generate 2 Days Data 
//                                =3 -> Generate 3 Days Data 
//                                >3 -> Generate 3 Days Data 
//
// RETURN : 0 -> No data has been generated / No BEE data
//          1 -> 1 day  data is already generated
//          2 -> 2 days data is already generated
//          3 -> 3 days data is already generated
//
// NOTE : If i2WeekNo and i4Tow is set to the current time ( > 0), then this function will delete expired BEE data 
//        (ex. 3 days ago data) based on this current time. Therefore, return value will be 0.
//        Set i2WeekNo = 0 and i4Tow = 0 to bypass this time checking.
//
// EXAMPLE :
//    unsigned char u1GenState;
//    unsigned char u1SvId = 1;
//    (1)
//    u1GenState = MTK_BEE_Gen_Data(u1SvId, 0, 0, 3);
//    ===> Generate 3 days BEE data of SV 1, bypass time checking.
//         If u1GenState = 3, means already has 3 days BEE data, not necessary to generate data within this period,
//         unless receive new ephemeris.              
//    (2)
//    u1GenState = MTK_BEE_Gen_Data(u1SvId, 0, 0, 1);
//    ===> Generate 1 day BEE data of SV 1, bypass time checking.
//    (3)
//    u1GenState = MTK_BEE_Gen_Data(u1SvId, 1467, 313200, 1);
//    ===> Generate 1 day BEE data of SV 1, using time checking.
//
//    (4) Methods for 3 days BEE data generation
//    ------------------------------------------
//    (a) u1GenState = MTK_BEE_Gen_Data(u1SvId, 1467, 313200, 1);  ==> u1GenState = 1 (has 1 day BEE data)
//        u1GenState = MTK_BEE_Gen_Data(u1SvId, 1467, 313300, 1);  ==> u1GenState = 2 (has 2 days BEE data)
//        u1GenState = MTK_BEE_Gen_Data(u1SvId, 1467, 313400, 1);  ==> u1GenState = 3 (has 3 days BEE data)
//
//    (b) u1GenState = MTK_BEE_Gen_Data(u1SvId, 1467, 313200, 2);  ==> u1GenState = 2 (has 2 day BEE data)
//        u1GenState = MTK_BEE_Gen_Data(u1SvId, 1467, 313300, 1);  ==> u1GenState = 3 (has 3 days BEE data)
//
//    (c) u1GenState = MTK_BEE_Gen_Data(u1SvId, 1467, 313200, 3);  ==> u1GenState = 3 (has 3 day BEE data)

MTK_BEE_INT MTK_BEE_Gen_Data (unsigned char u1SvId, short i2WeekNo, int i4Tow, unsigned char u1GenLength);


//*****************************************************************************
// MTK_BEE_Get_Progress : Get the progress of BEE generation
//
// PARAMETER : void
//
// RETURN : Progress of current BEE generation, represented in percentage (%)
//          100    : No action (no generation under going / generation finished)
//          0 - 99 : Progress

MTK_BEE_INT MTK_BEE_Get_Progress (void);


//*****************************************************************************
// MTK_BEE_Get_Data : Get BEE ephemeris of the specified satellite at the specified time
//
// PARAMETER : u1SvId   [IN]  - satellite ID
//             i2WeekNo [IN]  - week number (count from Jan. 6, 1980)
//             i4Tow    [IN]  - time of week
//             BeeData  [OUT] - BEE ephemeris
//
// RETURN : 1 for success, 0 for no BEE data

MTK_BEE_INT MTK_BEE_Get_Data (unsigned char u1SvId, short i2WeekNo, int i4Tow, unsigned char BeeData[48]);


//*****************************************************************************
// MTK_BEE_Get_Available_Info : Get information about BEE ephemeris of which satellite
//
// PARAMETER : i2WeekNo [IN]  - week number (count from Jan. 6, 1980)
//             i4Tow    [IN]  - time of week
//             BeeAvail [OUT] - Array of 32 unsigned char for 32 satellites
//                              0 -- no BEE available for this satellite
//                           1-xx -- BEE is available for this satellite ( Valid Hours )
//
// RETURN : 1 for success

MTK_BEE_INT MTK_BEE_Get_Available_Info (short i2WeekNo, int i4Tow, unsigned char BeeAvail[32]);

//*****************************************************************************
// MTK_BEE_Get_GNSS_Available_Info : Get information about BEE ephemeris of which satellite
//
// PARAMETER : i2WeekNo [IN]  - week number (count from Jan. 6, 1980)
//             i4Tow    [IN]  - time of week
//             BeeAvail [OUT] - Array of 256 unsigned char for 256 satellites
//                              0 -- no BEE available for this satellite
//                           1-xx -- BEE is available for this satellite ( Valid Hours )
//
// RETURN : 1 for success

MTK_BEE_INT MTK_BEE_Get_GNSS_Available_Info (short i2WeekNo, int i4Tow, unsigned char BeeAvail[256]);

//*****************************************************************************
// MTK_BEE_Enable_Eph_Update : Enable updating BEE with broadcast ephemeris
//                             for ALL satellites and save the configuration to file
//
// PARAMETER : void
//
// RETURN : 1 for success, 0 if fail to save configuration to file

MTK_BEE_INT MTK_BEE_Enable_Eph_Update (void);


//*****************************************************************************
// MTK_BEE_Disable_Eph_Update : Disable updating BEE with broadcast ephemeris
//                              for ALL satellites and save the configuration to file
//
// PARAMETER : void
//
// RETURN : 1 for success, 0 if fail to save configuration to file

MTK_BEE_INT MTK_BEE_Disable_Eph_Update (void);


//*****************************************************************************
// MTK_BEE_Set_Eph_Update : Set whether to update BEE with broadcast ephemeris
//                          for individual satellite and save the configuration to file
//
// PARAMETER : UpdateEph [IN] - Array of 32 unsigned char for 32 satellites
//                              0 -- disable updating broadcast ephemeris for this satellite
//                              1 -- enable updating broadcast ephemeris for this satellite
//
// RETURN : 1 for success, 0 if fail to save configuration to file

MTK_BEE_INT MTK_BEE_Set_Eph_Update (unsigned char UpdateEph[32]);


//*****************************************************************************
// MTK_BEE_Hot_Still_Test : Perform hot still test
//
// PARAMETER : u4Test_Mode [IN] - 
//                 bit0 : use old force model parameters
//
// RETURN : void

MTK_BEE_VOID MTK_BEE_Hot_Still_Test(unsigned long u4Test_Mode);


//*****************************************************************************
// MTK_BEE_Test : Perform BEE self-test, output data is stored in BEE_TEST.BIN
//
// PARAMETER : u1SvId [IN] - PRN of the satellite to be tested
//
// RETURN : void

MTK_BEE_VOID MTK_BEE_Test (unsigned char u1SvId);

//*****************************************************************************
// MTK_BEE_Shutdown : force HotStill run into shutdown mode. (shutdown all jobs after curretn prediction done)
//
// PARAMETER : fgEnableShutdown [IN] - 
//                 0 : disable shutdown to resume original state
//                 1 : enable shutdown
// RETURN : void
MTK_BEE_VOID MTK_BEE_Shutdown(unsigned char fgEnableShutdown);

//*****************************************************************************
// MTK_BEE_Get_Version_Info : Get BEE and HotStill related information
//
// PARAMETER : Version        [OUT] - Library version  
//                      Lib_Date      [OUT] - Library release dat information
//
// RETURN : void

MTK_BEE_VOID MTK_BEE_Get_Version_Info (double *Version, char *Lib_Date);

//*****************************************************************************
// MTK_BEE_Get_Calibration_Status : Get HotStill calibration status
//
// PARAMETER : void
//
// RETURN : 1 for calibration and gen in progess, 0 for calibration not in progress

MTK_BEE_INT MTK_BEE_Get_Calibration_Status(void);

//*****************************************************************************
// MTK_BEE_Set_Debug_Config : Set HotStill debug configuration
//
// PARAMETER : DbgType       [IN] - debug type
//             DbgLevel      [IN] - debug level
// RETURN : 1 success, 0 fail

MTK_BEE_INT MTK_BEE_Set_Debug_Config(MTK_DEBUG_TYPE DbgType, MTK_DEBUG_LEVEL DbgLevel);

//*****************************************************************************
// MTK_BEE_Get_Debug_Config : Get HotStill debug configuration
//
// PARAMETER : DbgType       [IN]  - debug type
//             *DbgLevel     [OUT] - debug level
//
// RETURN : 1 success, 0 fail
MTK_BEE_INT MTK_BEE_Get_Debug_Config(MTK_DEBUG_TYPE DbgType, MTK_DEBUG_LEVEL *DbgLevel);

#ifdef __cplusplus
}
#endif

#endif /* MTK_BEE_H */
