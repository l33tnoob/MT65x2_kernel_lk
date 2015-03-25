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
 * MixLim_exp.h
 *
 * Project:
 * --------
 * SWIP
 *
 * Description:
 * ------------
 * Mixer & limiter export header file
 *
 * Author:
 * -------
 * Scholar Chang
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

#ifndef __AUDIO_COMPONENT_ENGINE_EXP_H__
#define __AUDIO_COMPONENT_ENGINE_EXP_H__

namespace android {

typedef enum {
    ACE_SUCCESS = 0,
    ACE_INVALIDE_PARAMETER,
    ACE_INVALIDE_OPERATION,
    ACE_NOT_INIT,
    ACE_NOT_OPEN,
} ACE_ERRID;

typedef enum {
    ACE_STATE_NONE,
    ACE_STATE_INIT,
    ACE_STATE_OPEN,
}ACE_STATE;

class AudioComponentEngineBase
{
public:
    //AudioComponentEngineBase() {};
    virtual ACE_ERRID SetParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID GetParameter(uint32_t paramID, void *param) = 0;
    virtual ACE_ERRID Open(void) = 0;
    virtual ACE_ERRID Close(void) = 0;
    virtual ACE_ERRID ResetBuffer(void) = 0;
    /* Return: consumed input buffer size(byte)                             */
    virtual ACE_ERRID Process(void *pInputBuffer,   /* Input, pointer to input buffer */
                 uint32_t *InputSampleCount,        /* Input, length(byte) of input buffer */ 
                                                    /* Output, length(byte) left in the input buffer after conversion */ 
                 void *pOutputBuffer,               /* Input, pointer to output buffer */
                 uint32_t *OutputSampleCount) = 0;  /* Input, length(byte) of output buffer */ 
                                                    /* Output, output data length(byte) */ 
    virtual ~AudioComponentEngineBase() {};
protected:
    ACE_STATE mState;
};

}; // namespace android

#endif // __AUDIO_COMPONENT_ENGINE_EXP_H__
