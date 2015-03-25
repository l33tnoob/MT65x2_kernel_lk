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
 * MtkAudioSrc.h
 *
 * Project:
 * --------
 * Android
 *
 * Description:
 * ------------
 * *   This file implements Mtk Audio Ssampl Rate Converter Header
 *
 * Author:
 * -------
 * JY Huang
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime$
 * $Log$
 *
 * 08 07 2013 kh.hung
 * Add 32 bits version.
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#ifndef __MTK_AUDIO_SRC_EXP_H__
#define __MTK_AUDIO_SRC_EXP_H__

#include <utils/threads.h>
#include "MtkAudioComponent.h"
extern "C" {
#include "Blisrc_exp.h"
}

typedef enum {
    SRC_IN_Q1P15_OUT_Q1P15 = 0,   // 16-bit Q1.15 input, 16-bit Q1.15 output
    SRC_IN_Q1P15_OUT_Q1P31 = 1,   // 16-bit Q1.15 input, 32-bit Q1.31 output
    SRC_IN_Q9P23_OUT_Q1P31 = 2,   // 32-bit Q9.23 input, 32-bit Q1.31 output
    SRC_IN_Q1P31_OUT_Q1P31 = 3,   // 32-bit Q1.31 input, 32-bit Q1.31 output
    SRC_IN_END,
} SRC_PCM_FORMAT;

typedef enum {
    SRC_PAR_SET_PCM_FORMAT,
    SRC_PAR_GET_PCM_FORMAT,
    SRC_PAR_SET_INPUT_SAMPLE_RATE,
    SRC_PAR_GET_INPUT_SAMPLE_RATE,
    SRC_PAR_SET_OUTPUT_SAMPLE_RATE,
    SRC_PAR_GET_OUTPUT_SAMPLE_RATE,
    SRC_PAR_SET_INPUT_CHANNEL_NUMBER,
    SRC_PAR_GET_INPUT_CHANNEL_NUMBER,
    SRC_PAR_SET_OUTPUT_CHANNEL_NUMBER,
    SRC_PAR_GET_OUTPUT_CHANNEL_NUMBER,
} SRC_PARAMETER;

namespace android {

class MtkAudioSrc : public AudioComponentEngineBase
{
    public:
        MtkAudioSrc();
        MtkAudioSrc(uint32_t input_SR, uint32_t input_channel_num, uint32_t output_SR, uint32_t output_channel_num, SRC_PCM_FORMAT format);
        virtual ACE_ERRID SetParameter(uint32_t paramID, void *param);
        virtual ACE_ERRID GetParameter(uint32_t paramID, void *param);
        virtual ACE_ERRID Open(void);
        virtual ACE_ERRID Close(void);
        virtual ACE_ERRID ResetBuffer(void);
        virtual ACE_ERRID Process(void *pInputBuffer,   /* Input, pointer to input buffer */
                     uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */ 
                                                        /* Output, length(byte) left in the input buffer after conversion */ 
                     void *pOutputBuffer,               /* Input, pointer to output buffer */
                     uint32_t *OutputSampleCount);      /* Input, length(byte) of output buffer */ 
                                                        /* Output, output data length(byte) */ 
        ~MtkAudioSrc();
    private:
        uint32_t mTempBufSize; // in byte
        uint32_t mInternalBufSize; // in byte
        void *mpTempBuf;
        void *mpInternalBuf;
        Blisrc_Param mBlisrcParam;
        Blisrc_Handle *mBlisrcHandler;
        Mutex mLock;
};

}; // namespace android

#endif // __MTK_AUDIO_SRC_EXP_H__
