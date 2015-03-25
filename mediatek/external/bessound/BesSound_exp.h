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
 * BesSound_exp.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * BesSound Common header file.
 *
 * Author:
 * -------
 * Guyger Fan
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __BESSOUND_EXP_H__
#define __BESSOUND_EXP_H__
#ifndef NULL
#define NULL    0
#endif

#ifndef ASSERT
#define ASSERT(x)
#endif

typedef struct BS_EngineInfoStruct BS_EngineInfo;
struct BS_EngineInfoStruct{
   // The number of samples that applications have to feed to 
   // engines for flushing out the audio samples buffered in engines,
   // or to flush out the tail.
   // L/R pair counts as 2 samples.
   int FlushOutSampleCount;
};

typedef struct BS_HandleStruct BS_Handle;
struct BS_HandleStruct{
   //============================================================//
   // Get required buffer size for BesSound engine
   // pHandle:               input,
   //                        Handle of current BesSound engine.
   // uiInternalBufferSize:  output, the required internal buffer size in byte
   // uiTempBufferSize:      output, the required temp buffer size in byte
   //============================================================//
   void (*GetBufferSize)(BS_Handle *pHandle,
                         unsigned int *uiInternalBufferSize,
                         unsigned int *uiTempBufferSize);

   //============================================================//
   // Initialize and enable BesSound engine.
   // pHandle:             input,
   //                      Handle of current BesSound engine.
   // pInternalBuffer:     input,
   //                      Buffer allocated by application for BesSound
   //                      engine internal use.
   // cBytes               input,
   //                      The size of parameters pointed by pInitParam
   // pInitParam           input,
   //                      Pointer to the data that contains
   //                      parameters for initialization of BesSound enigne.
   // bEnableOLA					 input,
   //                      0      ==> Disable OLA when switching effect on
   //                      none 0 ==> Enable OLA when switching effect on
   //============================================================//
   void (*Open)(  BS_Handle *pHandle,
                  char *pInternalBuffer, 
                  int cBytes,
                  const void *pInitParam,
                  int bEnableOLA);
   //============================================================//
   // Disable current BesSound engine.
   // pHandle:             input,
   //                      Handle of current BesSound engine.
   // bEnableOLA					 input,
   //                      0      ==> Disable OLA when switching effect off
   //                      none 0 ==> Enable OLA when switching effect off
   //============================================================//
   void (*Close)(BS_Handle *pHandle, int bEnableOLA);

   //============================================================//
   // Set runtime parameters for BesSound engine
   // pHandle:             input, 
   //                      Handle of current BesSound engine
   // cBytes:              input,
   //                      size of parameters in byte pointed by pRuntimeParam
   // pRuntimeParam        input,
   //                      Pointer to the parameter structure that contains
   //                      parameters for run time configuration.
   //============================================================//
   void (*SetParameters)(BS_Handle *pHandle, 
                         int cBytes,
                         const void *pRuntimeParam);

   //============================================================//
   // Get the information about the audio post processing engine.
   // pHandle:             input, 
   //                      Handle of current BesSound engine
   // pEngineInfo          output,
   //                      Pointer to the structure that contains
   //                      the information about the engine that
   //                      is exported to users.
   //============================================================//
   void (*GetEngineInfo)(BS_Handle *pHandle, 
                         BS_EngineInfo *pEngineInfo);


   //============================================================//
   // Process data from input buffer to output buffer
   // pHandle:             the handle of current BesSound engine
   // pTempBuffer:         input,
   // [I]                  pointer to the temp buffer used in processing the data.  
   // pInputBuffer:        input, 
   // [I]                  pointer to input buffer containing the input stereo 
   //                      PCM samples.
   //                      the layout of LR is L/R/L/R...
   // InputSampleCount:    input,
   // [I/O]                the number of valid stereo PCM samples in input buffer
   //                      one L/R pair counts as two samples.
   //	                     output,
   //                      the number of samples consumed by BesSound engine
   //                      one L/R pair counts as two samples.
   // pOutputBuffer:       input, 
   // [I]                  pointer to output buffer that BesSound engine fills 
   //                      the processed PCM samples.
   //                      the layout of LR is L/R/L/R...
   // OutputSampleCount:   output, the number of valid stereo PCM samples in 
   // [O]                  output buffer.
   //                      one L/R pair counts as two samples.
   //                      output only.
   //============================================================//
   void (*Process)(BS_Handle *pHandle,
                   char *pTempBuffer,
                   const short *pInputBuffer,
                   int   *InputSampleCount,
                   short *pOutputBuffer,
                   int   *OutputSampleCount);

   int bEnabled;
   //============================================================//
   // Internal handle or any opaque data
   //============================================================//
   void *pInternalHandle;

   int FadeInCount;
   int FadeOutCount;
   int FadeGain;
   int bEnableOLA;
};

/* implemented on target to provide Assert Check*/
extern void BES_ASSERT( int expression );

#endif /* __BESSOUND_H__ */