/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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
 * BesLoudness_exp.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * BesLoudness interface
 *
 * Author:
 * -------
 * HP Cheng
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 * 
 * 12 11 2012 scholar.chang
 * [WCPSP00000681] [Audio][SWIP][BesSound]BesLoudness Modification
 * Fix the sudden gain change and DC offset problem.
 * Apply only memory copy when byass (no DRC and all zero filter coefficients).
 * Move SWIP protection mechanism from process to initialization to reduce the computational complexity.
 * 
 * 10 19 2012 scholar.chang
 * [WCPSP00000681] [Audio][SWIP][BesSound]BesLoudness Modification
 * BesLoudness v3.9.4, fix first frame gain smaller problem.
 * 
 * 09 24 2012 scholar.chang
 * [WCPSP00000681] [Audio][SWIP][BesSound]BesLoudness Modification
 * BesLoudness v4.1.2, fix first frame gain problem and wrap with new libdrvb.a for ALPS.JB.
 * 
 * 08 15 2012 scholar.chang
 * [WCPSP00000681] [Audio][SWIP][BesSound]BesLoudness Modification
 * Fix the BesLoudnessV3 first sound louder problem.
 *
 * 06 12 2012 scholar.chang
 * [WCPSP00000681] [Audio][SWIP][BesSound]BesLoudness Modification
 * BesLoudness V4 modification.
 *
 * 03 12 2012 scholar.chang
 * [WCPSP00000667] [Audio][SWIP]WCP2 Release
 * .
 *
 * 02 19 2011 wn.chen
 * [WCPSP00000574] BesLoudness V3
 * .
 *
 * 10 07 2010 richie.hsieh
 * [WCPSP00000522] [BesSound SWIP] Assertion removal
 * .
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

/* interface functions */
#ifndef BLOUDEXP_H
#define BLOUDEXP_H

#include "BesSound_exp.h"

/* NULL definition */
#ifndef NULL
#define NULL    0
#endif

/* Structure definition */
typedef struct {
   unsigned int HSF_COEFF[9][4];     
   unsigned int BPF_Coeff[4][6][3]; 
   unsigned int DRC_Forget_Table[9][2];
   unsigned int WS_Gain_Max;           // Q2.14   
   unsigned int WS_Gain_Min;           // Q2.14       
   unsigned int Filter_First;          // 0: DRC First, 1: Filter First    
   char Gain_Map_In[5];                // in DB
   char Gain_Map_Out[5];               // in DB
}BLOUD_CustomParam_V3;

/*---------------------------------------------------------------------------
   Enumerator Definition                                                      
  ---------------------------------------------------------------------------*/
typedef enum {
    BLOUD_MONO    = 1,  // Mono
    BLOUD_STEREO  = 2,  // Stereo
} BLOUD_CHANNEL_NUM;

typedef enum {
    BLOUD_FALSE = 0,    // False
    BLOUD_TRUE  = 1,    // True
} BLOUD_BOOL;

typedef enum {
    FILT_MODE_NONE     = 0, // Disable Filter
    FILT_MODE_COMP_FLT = 1, // Compenastion Filter
    FILT_MODE_LOUD_FLT = 2, // Loudness Filter
    FILT_MODE_COMP_HDP = 3, // Compensation Filter for Headphone
    FILT_MODE_AUD_ENH  = 4, // Enable BesAudEng Algorithnm
} FILT_MODE_ENUM;

typedef enum {
    LOUD_MODE_NONE       = 0, // None DRC
    LOUD_MODE_BASIC      = 1, // Basic DRC
    LOUD_MODE_ENHANCED   = 2, // Enhanced DRC
    LOUD_MODE_AGGRESSIVE = 3, // Aggressive DRC
} LOUD_MODE_ENUM;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_FilterCoef
|
|   MEMBERS
|
|   HPF_COEF [2][9][5]      High-pass filter coefficients
|             |  |  |                          -1        -2 
|             |  |  |               b0 + b1 x z  + b2 x z   
|             |  |  |       H(z) = -------------------------
|             |  |  |                          -1        -2 
|             |  |  |                1 + a1 x z  + a2 x z   
|             |  |  |                                       
|             +-----------> HPF index
|                |  |       0: HPF1
|                |  |       1: HPF2
|                |  |        
|                +--------> Sampling rate
|                   |       0: 48000Hz
|                   |       1: 44100Hz
|                   |       2: 32000Hz
|                   |       3: 24000Hz
|                   |       4: 22050Hz
|                   |       5: 16000Hz
|                   |       6: 12000Hz
|                   |       7: 11025Hz
|                   |       8:  8000Hz
|                   |
|                   +-----> HPF coefficients
|                           (FILT_MODE_COMP_FLT or FILT_MODE_LOUD_FLT)
|                           0:  b0, format: signed Q5.27
|                           1:  b1, format: signed Q5.27
|                           2:  b2, format: signed Q5.27
|                           3: [-a1 | -a2], format: [signed Q2.14 | signed Q2.14]
|                           4:  all zeros
|                           (FILT_MODE_COMP_HDP)
|                           0:  b0, format: signed Q5.27
|                           1:  b1, format: signed Q5.27
|                           2:  b2, format: signed Q5.27
|                           3: [-a1 | -a2], format: [signed Q2.30 upper 16-bit | signed Q2.30 upper 16-bit]
|                           4: [-a1 | -a2], format: [signed Q2.30 lower 16-bit | signed Q2.30 lower 16-bit]
|
|   BPF_COEF [8][6][3]      Band-pass filter coefficients
|             |  |  |                          -1        -2
|             |  |  |               b0 + b1 x z  + b2 x z
|             |  |  |       H(z) = -------------------------
|             |  |  |                          -1        -2
|             |  |  |                1 + a1 x z  + a2 x z
|             |  |  |     
|             +-----------> BPF index
|                |  |       0: BPF1
|                |  |       1: BPF2
|                |  |       2: BPF3
|                |  |       3: BPF4
|                |  |       4: BPF5
|                |  |       5: BPF6
|                |  |       6: BPF7
|                |  |       7: BPF8
|                |  |     
|                +--------> Sampling rate
|                   |       0: 48000Hz
|                   |       1: 44100Hz
|                   |       2: 32000Hz
|                   |       3: 24000Hz
|                   |       4: 22050Hz
|                   |       5: 16000Hz
|                   |     
|                   +-----> BPF coefficients
|                           0: [ b0 |  b1], format: [signed Q2.14 | signed Q2.14]
|                           1: [ b2 | -a1], format: [signed Q2.14 | signed Q2.14]
|                           2: [-a2 |   0], format: [signed Q2.14 |    all zeros]
|
|   LPF_COEF [6][3]         Low-pass filter coefficients
|             |  |                             -1        -2 
|             |  |                  b0 + b1 x z  + b2 x z   
|             |  |          H(z) = -------------------------
|             |  |                             -1        -2 
|             |  |                   1 + a1 x z  + a2 x z   
|             |  |                                          
|             +-----------> Sampling rate
|                |          0: 48000Hz
|                |          1: 44100Hz
|                |          2: 32000Hz
|                |          3: 24000Hz
|                |          4: 22050Hz
|                |          5: 16000Hz
|                |       
|                +--------> LPF coefficients
|                           0: [ b0 |  b1], format: [signed Q2.14 | signed Q2.14]
|                           1: [ b2 | -a1], format: [signed Q2.14 | signed Q2.14]
|                           2: [-a2 |   0], format: [signed Q2.14 |    all zeros]
|                           
|**************************************************************************************/

typedef struct {
    unsigned int HPF_COEF[2][9][5];
    unsigned int BPF_COEF[8][6][3]; 
    unsigned int LPF_COEF[6][3];
}BLOUD_FilterCoef;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_CustomParam
|
|   MEMBERS
|
|   WS_Gain_Max             Wave shaping maximum gain, format Q2.14, set to 0 would use the default value
|   WS_Gain_Min             Wave shaping minimum gain, format Q2.14, set to 0 would use the default value
|   
|   Filter_First            Choose the position of the DRC / filter stage order
|                           0: DRC First
|                           1: Filter First
|                           
|   Att_Time                Attack time , unit: 0.1 ms / 6dB
|   Rel_Time                Release time, unit: 0.1 ms / 6dB
|   
|   Gain_Map_In[5]          Gain mapping for input signal , unit: dB
|               |       
|               +-------->  Input level index
|                           0: Th, noise gate threshold
|                           1: Ex, expansion end point
|                           2: Cp, compression start point
|                           3: Ed, target level
|                           4: Lm, limitation end point
|
|   Gain_Map_Out[5]         Gain mapping for output signal, unit: dB
|                |       
|                +------->  Output level index
|                           0: Th, noise gate threshold
|                           1: Ex, expansion end point
|                           2: Cp, compression start point
|                           3: Ed, target level
|                           4: Lm, limitation end point
|**************************************************************************************/

typedef struct {
    unsigned int WS_Gain_Max;               // Q2.14
    unsigned int WS_Gain_Min;               // Q2.14
    unsigned int Filter_First;              // 0: DRC First, 1: Filter First    
    unsigned int Att_Time;                  // unit: 0.1 ms / 6dB
    unsigned int Rel_Time;                  // unit: 0.1 ms / 6dB
    char Gain_Map_In[5];                    // in DB
    char Gain_Map_Out[5];                   // in DB
}BLOUD_CustomParam;

/***************************************************************************************
|   STRUCTURE
|
|       BLOUD_InitParam
|
|   MEMBERS
|
|   Channel_Num             Channel number, only support: 1 or 2
|   Sample_Rate             Input signal sampling rate, unit: Hz
|                           Support 9 kinds of sampling rates:
|                           48000, 44100, 32000, 
|                           24000, 22050, 16000,
|                           12000, 11025,  8000
|   
|   Filter_Mode             Audio compensation filter mode selection
|                           0 (FILT_MODE_NONE): 
|                               Disable audio compensation filter
|                           1 (FILT_MODE_COMP_FLT): 
|                               Use the customized filter coefficients for loudspeaker
|                           2 (FILT_MODE_LOUD_FLT):
|                               If no customized filter coefficients, 
|                               SWIP would apply the default loudness filters
|                           3 (FILT_MODE_COMP_HDP):
|                               Use the customized filter coefficients for headphone,
|                               which support more precise filter resolution for HPF
|
|   Loudness_Mode           Loudness mode selection
|                           0 (LOUD_MODE_NONE): 
|                               Disable dynamic range control (DRC)
|                           1 (LOUD_MODE_BASIC): 
|                               Enable DRC
|                           2 (LOUD_MODE_ENHANCED):
|                               Enable DRC and basic wave shaping, 
|                               only valid under filter mode: FILT_MODE_LOUD_FLT
|                           3 (LOUD_MODE_AGGRESSIVE):
|                               Enable DRC and aggressive wave shaping
|                               only valid under filter mode: FILT_MODE_LOUD_FLT
|                           
|   pCustom_Param           Pointer to the custom parameter fields
|
|   pFilter_Coef            Pointer to the filter coefficient fields
|
|**************************************************************************************/
typedef struct {
   unsigned int Channel_Num;
   unsigned int Sample_Rate;
   unsigned int Filter_Mode;
   unsigned int Loudness_Mode;
   BLOUD_CustomParam *pCustom_Param;
   BLOUD_FilterCoef  *pFilter_Coef;
}BLOUD_InitParam;

int BLOUD_SetHandle(BS_Handle *pHandle);

#endif
