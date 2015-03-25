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

/******************************************************************************
*[File]           MTK_SDK_Bee.h
*[Version]        v0.1
*[Revision Date]  2008-01-25
*[Author]         WG yau, wg.yau@mediatek.com, 26967, 2008-01-25
*[Description]
*  This program is BEE Related Function for the GPS SDK
*[Copyright]
*    Copyright (C) 2005 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/

#ifndef MTK_SDK_BEE_H
#define MTK_SDK_BEE_H

#ifdef __cplusplus
   extern "C" {
#endif

//*****************************************************************************************************
// MTK_Bee_Feed_Eph  :  Feed the ephemeris to Bee Host
//
// Description  :  MTK_Bee_Feed_Eph ( unsigned char Svid , unsigned int au4Word[24] )
//                 Svid : GPS satellite PRN (1~32)
//                 au4Word --  24 words ephemeris data 
//                
// Returns: zero(fail), nonzero(pass)
//
// Example :
//    unsigned char SvId = 0;
//    unsigned int Ephdata[24];
//    iRes = MTK_Bee_Feed_Eph ( SvId, Ephdata);
//
//    =====> Ephdata is ephemeris data and need to transfer to BEE HOST 
//           If iRes > 0 --> Data is valid and can be transfered to BEE HOST

int MTK_Bee_Feed_Eph ( unsigned char Svid , unsigned int au4Word[24] );

//*****************************************************************************************************
// MTK_Bee_Receive_Bee_Data  :  Receive the Bee data from HOST
//
// Description  :  MTK_Bee_Receive_Bee_Data ( char *BeeData )
//                 BeeData --  BEE data from HOST for aiding. 
//                             BEE data buf mest be greater than 96 bytes.
//                
// Returns: zero(fail), nonzero(pass)
//
// Example :
//    char Beedatabuf[96];   // must be >= 96 bytes
//    iRes = MTK_Bee_Receive_Bee_Data ( Beedatabuf );
//
//    =====> Beedatabuf is BEE data and need to feed to GPS receiver 
//           If iRes > 0 --> Bee data aiding is valid.

int MTK_Bee_Receive_Bee_Data ( char *BeeData );
 
//*****************************************************************************************************
// MTK_Bee_Request_All  :  Send request BEE data command to HOST.
//
// Description  :  MTK_Bee_Request_All ( void )
//                 Send a Nmea command to HOST for request the BEE data. 
//                
// Example :
//    MTK_Bee_Request_All();
//
//    =====> $PMTKRQTBEE*17 

void MTK_Bee_Request_All ( void );

//********************************************************************************************************
// MTK_Bee_Get_Eph_Info  :  Get the information whether satellites has Ephemeris (BRDC / BEE) or not
//
//    Note  :  Eph is arrays with 32 unsigned char.
//             0       ---> This SV has no Ephemeris.
//             1       ---> This SV has BRDC Ephemeris.
//             2       ---> This SV has 1st day BEE Ephemeris.
//             3       ---> This SV has 2nd day BEE Ephemeris.
//             4       ---> This SV has 3rd day BEE Ephemeris.
//
// Example:
//    unsigned char Eph[32];
//    MTK_Bee_Get_Eph_Info(Eph);
//    MTK_UART_OutputData("SDK: Eph = %d", Eph[16]);
//
// =====> SDK: Eph = 1  ---> SV17 has BRDC Ephemeris. 
// =====> SDK: Eph = 2  ---> SV17 has 1st day BEE Ephemeris. 

void MTK_Bee_Get_Eph_Info (unsigned char Eph[32]);

//********************************************************************************************************
// MTK_Bee_Get_New_Eph_Info  :  Get the information whether satellites has new Ephemeris or not
//
//    Note  :  NewEph is 1 unsigned int with 32 bit.
//             bit x = 0       ---> No  New Ephemeris, x+1 = GPS satellite PRN.
//             bit x = 1       ---> Has New Ephemeris, x+1 = GPS satellite PRN.
// Example:
//    int i;
//    unsigned int NewEph,EPhMask;
//    MTK_Bee_Get_Eph_Info(&NewEph);
//    MTK_UART_OutputData("SDK: NewEph = %d", NewEph);
//    if ( NewEph != 0 )
//    {
//       EPhMask = 1;
//       for (i = 0; i < 32; i++)
//       {         
//          if ( (NewEph & EPhMask) != 0 )
//          {                
//                MTK_UART_OutputData("SV%d: NewEph = %d", i+1, NewEph);                
//          }
//          EPhMask = EPhMask << 1;
//       }                   
//    }
// =====> SDK: NewEph = 1   ---> Has New Ephemeris. 
// =====> SV3: NewEph = 1   ---> SV3 has New Ephemeris. 
// =====> SV17: NewEph = 1  ---> SV17 has New Ephemeris. 

void MTK_Bee_Get_New_Eph_Info (unsigned int *NewEph);

//********************************************************************************************************
// MTK_Bee_Req_Info  :  Get the information whether need to request BEE data or not
//
//    Note  :  BeeReq is arrays with 32 unsigned char.
//             0       ---> This SV no need to request BEE Ephemeris.
//             1       ---> This SV need to request BEE Ephemeris.
// Example:
//    unsigned char BeeReq[32];
//    MTK_Bee_Req_Info(BeeReq);
//    MTK_UART_OutputData("SDK: BeeReq = %d", BeeReq[16]);
//
// =====> SDK: BeeReq = 1  ---> SV17 need to request BEE Ephemeris. 
// =====> SDK: BeeReq = 0  ---> SV17 no need to request BEE Ephemeris. 

void MTK_Bee_Req_Info (unsigned char BeeReq[32]);

//********************************************************************************************************
// MTK_Bee_Age  :  Get the age information of BEE data
//
//    Note  :  BeeAge is arrays with 32 short.
//             -1      ---> BEE Ephemeris is not valid .
//             nonzero ---> BEE Ephemeris age.
// Example:
//    short BeeAge [32];
//    MTK_Bee_Age(BeeAge);
//    MTK_UART_OutputData("SDK: BeeAge = %d", BeeAge[16]);
//
// =====> SDK: BeeReq = 300  ---> SV17 BEE Ephemeris has been used 300 sec. 
// =====> SDK: BeeReq = -1   ---> SV17 BEE Ephemeris is not valid. 

void MTK_Bee_Age ( short BeeAge[32]);

//********************************************************************************************************
// MTK_Bee_Send_New_Eph  :  Send new ephemeris to Host
//
// Description : MTK_Bee_Send_New_Eph (unsigned int *NewEphSv);
//               NewEphSv -- 32-bit mask to indicate if satellites have new ephemeirs or not
//               bit x = 0  ---> No  New Ephemeris, x+1 = GPS satellite PRN
//               bit x = 1  ---> Has New Ephemeris, x+1 = GPS satellite PRN
//
// Return : 0  ---> No new BRDC
//          1  ---> Have new BRDC, Receiver should start handshaking with Host
//          2  ---> Have new BRDC and handshaking is OK, Receiver should send BRDC data to Host
//
// Example :
//    void SDK_Main (void)
//    {
//       int NewEphStep;
//       unsigned char SvId;
//       unsigned int EphData[24];
//       NewEphStep = MTK_Bee_Send_New_Eph();
//       if (NewEphStep == 1)
//       {
//          ... Send handshaking message to Host ...
//       }
//       else if (NewEphStep == 2)
//       {
//          if ( MTK_Bee_Get_New_Eph_Data(&SvId, EphData) )
//          {
//             ... Send ephemeris data to Host ...
//          }
//       }
//    }

int MTK_Bee_Send_New_Eph (unsigned int *NewEphSv);

//********************************************************************************************************
// MTK_Bee_Get_New_Eph_Data  :  Get ephemeris data to send
//
// Description : MTK_Bee_Get_New_Eph_Data ( unsigned char *SvId, unsigned int EphData[24] )
//               SvId    -- pointer to Satellite ID
//               EphData -- 24-word ephemeris data
//
// Note : This function automatically determines which ephemeris should be sent at current time
//
// Return : 0  ---> No ephemeris data needed to be sent
//          1  ---> Have ephemeris data to be sent
//
// Example :
//    unsigned char SvId;
//    unsigned int EphData[24];
//    iRes = MTK_Bee_Get_New_Eph_Data(&SvId, EphData);
//    if ( iRes )
//    {
//        for ( i = 0; i < 24; i++ )
//        {
//            sprintf(( buf + i*7 ), ",%06X", EphData[i]);
//        } 
//        MTK_NMEA_OutputData("PMTKDTEPH,%02X%s", SvId, buf);
//    }

int MTK_Bee_Get_New_Eph_Data (unsigned char *SvId, unsigned int EphData[24]);

//********************************************************************************************************
// MTK_Bee_Receive_New_Eph_Host_Ready  :  Receive message of Host ready to receive new ephemeris
//
// Description : MTK_Bee_Receive_New_Eph_Ack ( unsigned char SvId )
//               SvId -- Satellite ID for ACK
//
// Example :
//    MTK_Bee_Receive_New_Eph_Host_Ready();
//    MTK_UART_OutputData("READY TO SEND BRDC DATA");

void MTK_Bee_Receive_New_Eph_Host_Ready (void);

//********************************************************************************************************
// MTK_Bee_Receive_New_Eph_Ack  :  Receive ack for new ephemeris data from Host
//
// Description : MTK_Bee_Receive_New_Eph_Ack ( unsigned char SvId )
//               SvId -- Satellite ID for ACK
//
// Return : 0  ---> Invalid ACK, satellite ID does not match the ephemeris sent
//          1  ---> ACK is valid
//
// Example :
//    unsigned char SvId;
//    iRes = MTK_Bee_Receive_New_Eph_Ack(SvId);
//    if ( iRes )
//    {
//       MTK_UART_OutputData("BRDC ACK SV %d OK", SvId);
//    }
//    else
//    {
//       MTK_UART_OutputData("BRDC ACK SV %d FAIL", SvId);
//    }

int MTK_Bee_Receive_New_Eph_Ack (unsigned char SvId);

//********************************************************************************************************
// MTK_Bee_Request_Data  :  Get a list of satellites which need BEE data
//
// Description : MTK_Bee_Request_Data ( unsigned char BeeReq[32] )
//               BeeReq -- Array with 32 unsigned char
//               0  ---> Seed BEE request for this satellite
//               1  ---> Do not send BEE request for this satellite
//
//    Note  :  If MTK_Bee_Disable_BEE() is called to diable BEE,
//             this function will not request any BEE data.
//             Only after MTK_Bee_Enable_BEE() is called,
//             this function will restart to request BEE data.
//
// Example :
//    int NumSv, NumSvOnce, i;
//    unsigned char BeeReq[32];
//    char buf[256], tmp[16];
//    MTK_Bee_Request_Data(BeeReq);
//    for ( i = 0; i < 32; i++ )
//    {
//        if ( BeeReq[i] )
//        {
//            NumSv++;
//            sprintf(tmp, ",%d", i + 1);
//            strcat(buf, tmp);
//        }
//    }
//    NumSvOnce = 6;
//    if ( NumSv > 0 )
//    {
//        MTK_NMEA_OutputData("PMTKRQTBEE,%d,%d%s", NumSvOnce, NumSv, buf);
//    }

void MTK_Bee_Request_Data (unsigned char BeeReq[32]);

//********************************************************************************************************
// MTK_Bee_Receive_Host_Info  :  Receive a list of satellites which Host has their BEE data
//
// Description : MTK_Bee_Receive_Host_Info ( int NumSv, unsigned char HostBeeInfo[32] )
//               NumSv       -- Number of satellites in 
//               HostBeeInfo -- Array with 32 unsigned char
//               0  ---> Host does not have BEE data for this satellite
//               1  ---> Host has BEE data for this satellite
//
// Return : 0  ---> Fail, input information is not valid
//          1  ---> OK, input information is valid
//
// Example :
//    int NumSv, Res;
//    unsigned char HostBeeInfo[32];
//    Res = MTK_Bee_Receive_Host_Info(NumSv, HostBeeInfo);
//    if ( Res )
//    {
//        MTK_UART_OutputData("HOST BEE INFO OK");
//    }
//    else
//    {
//        MTK_UART_OutputData("HOST BEE INFO FAIL");
//    }

int MTK_Bee_Receive_Host_Info (int NumSv, unsigned char HostBeeInfo[32]);

//********************************************************************************************************
// MTK_Bee_Receive_End_Data  :  Receive end of BEE data transmission message from Host
//
// Description : The receiver stops checking which satellite needs BEE data after
//               MTK_Bee_Receive_Host_Info() is called.
//               The receiver will restart to check which satellite needs BEE data after
//               MTK_Bee_Receive_End_Data() is called.
//
// Example :
//    MTK_Bee_Receive_End_Data();
//    MTK_UART_OutputData("BEE TRANSMISSION FINISH");

void MTK_Bee_Receive_End_Data (void);

//*****************************************************************************************************
// MTK_Bee_Disable_BEE  :  Disable ALL BEE data
//
// Description  :  If no need using BEE data for generate fix, then 
//                 using MTK_Bee_Disable_BEE() to disable BEE data.
//                
// Returns : NONE
//
// Example :
//      Receive some special command( DISABLE_BEE ), then Disable BEE data    
//      UART_RECEIVE()
//      {
//        ......
//        case DISABLE_BEE:
//              MTK_Bee_Disable_BEE();
//              break;
//        ......
//      }

void MTK_Bee_Disable_BEE ( void );

//*****************************************************************************************************
// MTK_Bee_Enable_BEE  :  Enable ALL BEE data
//
// Description  :  If needed using BEE data for generate fix, then 
//                 using MTK_Bee_Enable_BEE() to enable BEE data.
//                
// Returns : NONE
//
// Example :
//      Receive some special command( ENABLE_BEE ), then Enable BEE data    
//      UART_RECEIVE()
//      {
//        ......
//        case ENABLE_BEE:
//              MTK_Bee_Enable_BEE();
//              break;
//        ......
//      }

void MTK_Bee_Enable_BEE ( void );

//*****************************************************************************************************
// MTK_Bee_Set_BEE  :  Set enable / disable BEE data for individual satellite
//
// Description : MTK_Bee_Set_BEE ( unsigned char EnableBEE[32] )
//               EnableBEE -- Array with 32 unsigned char
//               0  ---> Disable use of BEE data in the fix for this satellite
//               1  ---> Enable use of BEE data in the fix for this satellite
//
// Returns : NONE
//
// Example :
//      Receive some special command( SET_BEE ), then Set enable / disable BEE data    
//      UART_RECEIVE()
//      {
//        ......
//        case SET_BEE:
//              unsigned char EnableBEE[32];
//              EnableBEE[5] = 1;
//              EnableBEE[12] = 0;
//              EnableBEE[16] = 1;
//              MTK_Bee_Set_BEE(EnableBEE);
//              break;
//        ......
//      }
//
// =====> Enable  use BEE data of PRN 5, 16 in the fix
//        Disable use BEE data of PRN 12    in the fix

void MTK_Bee_Set_BEE ( unsigned char EnableBEE[32] );

//*****************************************************************************************************
// MTK_Bee_Disable_BRDC  :  Disable ALL BRDC data
//
// Description  :  If no need using BRDC data for generate fix, then 
//                 using MTK_Bee_Disable_BRDC() to disable BRDC data.
//                
// Returns : NONE
//
// Example :
//      Receive some special command( DISABLE_BRDC ), then Disable BRDC data    
//      UART_RECEIVE()
//      {
//        ......
//        case DISABLE_BRDC:
//              MTK_Bee_Disable_BRDC();
//              break;
//        ......
//      }

void MTK_Bee_Disable_BRDC ( void );

//*****************************************************************************************************
// MTK_Bee_Enable_BRDC  :  Enable ALL BRDC data
//
// Description  :  If needed using BRDC data for generate fix, then 
//                 using MTK_Bee_Enable_BRDC() to enable BRDC data.
//                
// Returns : NONE
//
// Example :
//      Receive some special command( ENABLE_BRDC ), then Enable BRDC data    
//      UART_RECEIVE()
//      {
//        ......
//        case ENABLE_BRDC:
//              MTK_Bee_Enable_BRDC();
//              break;
//        ......
//      }

void MTK_Bee_Enable_BRDC ( void );

//*****************************************************************************************************
// MTK_Bee_Set_BRDC  :  Set enable / disable BRDC data for individual satellite
//                
// Description : MTK_Bee_Set_BRDC ( unsigned char EnableBRDC[32] )
//               EnableBRDC -- Array with 32 unsigned char
//               0  ---> Disable use of BRDC data in the fix for this satellite
//               1  ---> Enable use of BRDC data in the fix for this satellite
//
// Returns : NONE
//
// Example :
//      Receive some special command( SET_BRDC ), then Set enable / disable BRDC data    
//      UART_RECEIVE()
//      {
//        ......
//        case SET_BRDC:
//              unsigned char EnableBRDC[32];
//              EnableBRDC[5] = 1;
//              EnableBRDC[12] = 0;
//              EnableBRDC[16] = 1;
//              MTK_Bee_Set_BRDC(EnableBRDC);
//              break;
//        ......
//      }
//
// =====> Enable  use BRDC ephemeris of PRN 5, 16 in the fix
//        Disable use BRDC ephemeris of PRN 12    in the fix

void MTK_Bee_Set_BRDC ( unsigned char EnableBRDC[32] );


//*****************************************************************************************************
// MTK_Bee_Query_BRDC_Status  :  Query BRDC ephemeris status
//
// Description : MTK_Bee_Query_BRDC_Status ( unsigned char QueryBRDC[32] )
//               QueryBRDC -- Array with 32 unsigned char
//               0  ---> BRDC ephemeris is disable use in the fix for this satellite
//               1  ---> BRDC ephemeris is enable  use in the fix for this satellite
//                
// Returns : NONE
//
// Example :
//      Receive some special command( QUERY_BRDC ), then Query BRDC ephemeris    
//      UART_RECEIVE()
//      {
//        ......
//        case QUERY_BRDC:
//              unsigned char QueryBRDC[32];
//              MTK_Bee_Query_BRDC_Status(QueryBRDC);
//              break;
//        ......
//      }
//
// =====> if QueryBRDC[5] = 1; QueryBRDC[19] = 0; QueryBRDC[25] = 1;
//        BRDC ephemeris of PRN 5, 25 is enable use in the fix
//        BRDC ephemeris of PRN 19    is disable use in the fix

void MTK_Bee_Query_BRDC_Status ( unsigned char QueryBRDC[32] );


//*****************************************************************************************************
// MTK_Bee_Query_BEE_Status  :  Query BEE data status
//
// Description : MTK_Bee_Query_BRDC_Status ( unsigned char QueryBEE[32] )
//               QueryBRDC -- Array with 32 unsigned char
//               0  ---> BEE data is disable use in the fix for this satellite
//               1  ---> BEE data is enable  use in the fix for this satellite
//                
// Returns : NONE
//
// Example :
//      Receive some special command( QUERY_BEE ), then Query BEE data    
//      UART_RECEIVE()
//      {
//        ......
//        case QUERY_BEE:
//              unsigned char QueryBEE[32];
//              MTK_Bee_Query_BEE_Status(QueryBEE);
//              break;
//        ......
//      }
//
// =====> if QueryBEE[5] = 1; QueryBEE[19] = 0; QueryBEE[25] = 1;
//        BEE data of PRN 5, 25 is enable use in the fix
//        BEE data of PRN 19    is disable use in the fix

void MTK_Bee_Query_BEE_Status ( unsigned char QueryBEE[32] );

#ifdef __cplusplus
   }
#endif

#endif /* MTK_SDK_BEE_H */

